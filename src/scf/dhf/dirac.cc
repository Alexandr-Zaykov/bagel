//
// BAGEL - Parallel electron correlation program.
// Filename: dirac.cc
// Copyright (C) 2012 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the BAGEL package.
//
// The BAGEL package is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// The BAGEL package is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with the BAGEL package; see COPYING.  If not, write to
// the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//


#include <src/scf/dhf/dirac.h>
#include <src/scf/dhf/dfock.h>
#include <src/mat1e/rel/relhcore.h>
#include <src/mat1e/rel/reloverlap.h>
#include <src/mat1e/giao/relhcore_london.h>
#include <src/mat1e/giao/reloverlap_london.h>
#include <src/wfn/relreference.h>
#include <src/util/constants.h>
#include <src/util/math/zmatrix.h>
#include <src/util/math/matrix.h>
#include <src/util/math/diis.h>
#include <src/util/muffle.h>
#include <src/scf/dhf/population_analysis.h>

using namespace std;
using namespace bagel;

Dirac::Dirac(const shared_ptr<const PTree> idata, const shared_ptr<const Geometry> geom,
             const shared_ptr<const Reference> re) : Method(idata, geom, re) {
  gaunt_ = idata->get<bool>("gaunt", false);
  breit_ = idata->get<bool>("breit", gaunt_);
  robust_ = idata->get<bool>("robust", false);

  // when computing gradient, we store half-transform integrals
  do_grad_ = idata->get<bool>("gradient", false);
  if (do_grad_ && geom_->magnetism()) throw runtime_error("Gradient integrals have not been implemented for a GIAO basis.");

  geom_ = geom->relativistic(gaunt_);
  common_init(idata);
}


void Dirac::common_init(const shared_ptr<const PTree> idata) {
  Timer init_time;
  cout << "  *** Dirac HF ***" << endl << endl;

  // reading input keywords
  max_iter_ = idata->get<int>("maxiter", 100);
  max_iter_ = idata->get<int>("maxiter_scf", max_iter_);
  diis_start_ = idata->get<int>("diis_start", 1);
  thresh_scf_ = idata->get<double>("thresh", 1.0e-8);
  thresh_scf_ = idata->get<double>("thresh_scf", thresh_scf_);
  thresh_overlap_ = idata_->get<double>("thresh_overlap", 1.0e-8);
  ncharge_ = idata->get<int>("charge", 0);
  nele_ = geom_->nele()-ncharge_;

  if (!geom_->magnetism()) {
    hcore_ = make_shared<const RelHcore>(geom_);
    overlap_ = make_shared<const RelOverlap>(geom_);
  } else {
    hcore_ = make_shared<const RelHcore_London>(geom_);
    overlap_ = make_shared<const RelOverlap_London>(geom_);
  }
  init_time.tick_print("1-electron integrals");

  s12_ = overlap_->tildex(thresh_overlap_);

  nneg_ = s12_->mdim()/2;
  assert(s12_->mdim() % 2 == 0);

  if (breit_ && !gaunt_) throw runtime_error("Breit cannot be turned on if Gaunt is off");
}


void Dirac::compute() {
  Timer scftime;
  string indent = "  ";

  shared_ptr<const DistZMatrix> hcore = hcore_->distmatrix();
  shared_ptr<const DistZMatrix> distovl = overlap_->distmatrix();
  shared_ptr<const DistZMatrix> s12 = s12_->distmatrix();
  eig_ = VectorB(hcore->ndim());

  // making initial guess
  shared_ptr<const DistZMatrix> coeff = initial_guess(s12, hcore);
  shared_ptr<const DistZMatrix> aodensity = coeff->form_density_rhf(nele_, nneg_);

  cout << indent << "=== Nuclear Repulsion ===" << endl << indent << endl;
  cout << indent << fixed << setprecision(10) << setw(15) << geom_->nuclear_repulsion() << endl << endl;
  cout << indent << "    * DIIS with orbital gradients will be used." << endl << endl;
  scftime.tick_print("SCF startup");
  cout << endl;
  cout << indent << "=== Dirac RHF iteration (" + geom_->basisfile() + ", " << (geom_->magnetism() ? "RMB" : "RKB") << ") ===" << endl << indent << endl;

  DIIS<DistZMatrix, ZMatrix> diis(5);

  for (int iter = 0; iter != max_iter_; ++iter) {
    Timer ptime(1);

    auto fock = make_shared<DFock>(geom_, hcore_, coeff->matrix()->slice_copy(nneg_, nele_+nneg_), gaunt_, breit_, do_grad_, robust_);

// TODO I have a feeling that the code should not need this, but sometimes there are slight errors. still looking on it.
#if 0
    assert(fock->is_hermitian());
    fock->hermite();
#endif
    // distribute
    shared_ptr<const DistZMatrix> distfock = fock->distmatrix();

    // compute energy here
    const complex<double> prod = aodensity->dot_product(*hcore+*distfock); // identical to Tr(D^+ F)
    if (fabs(prod.imag()) > 1.0e-12) {
      stringstream ss; ss << "imaginary part of energy is nonzero!! Perhaps Fock is not Hermite for some reasons " << setprecision(10) << prod.imag();
//    throw runtime_error(ss.str());
      cout << ss.str() << endl;
    }
    energy_ = 0.5*prod.real() + geom_->nuclear_repulsion();

    auto error_vector = make_shared<const DistZMatrix>(*distfock**aodensity**distovl - *distovl**aodensity**distfock);
    const double error = error_vector->rms();

    ptime.tick_print("Fock build");
    cout << indent << setw(5) << iter << setw(20) << fixed << setprecision(8) << energy_
         << "   " << setw(17) << error << setw(15) << setprecision(2) << scftime.tick() << endl;

    if (error < thresh_scf_ && iter > 0) {
      cout << indent << endl << indent << "  * SCF iteration converged." << endl << endl;
      // when computing gradient, we store half-transform integrals to avoid recomputation
      if (do_grad_) half_ = fock->half();
      break;
    } else if (iter == max_iter_-1) {
      cout << indent << endl << indent << "  * Max iteration reached in SCF." << endl << endl;
      throw runtime_error("Max iteration reached in Dirac--Fock SCF");
    }

    if (iter >= diis_start_) {
      distfock = diis.extrapolate({distfock, error_vector});
      ptime.tick_print("DIIS");
    }

    DistZMatrix intermediate(*coeff % *distfock * *coeff);
    intermediate.diagonalize(eig_);
    coeff = make_shared<DistZMatrix>(*coeff * intermediate);

    aodensity = coeff->form_density_rhf(nele_, nneg_);

  }

  coeff_ = coeff->matrix();

  // print out orbital populations, if needed
  if (idata_->get<bool>("pop", false)) {
    cout << "    * Printing out population analysis to dhf.log" << endl;
    Muffle muf ("dhf.log");
    population_analysis(geom_, coeff_->slice(nneg_, nneg_*2), overlap_, (geom_->magnetism() ? 0 : 1));
  }

}


//Print non dirac sea eigenvalues
void Dirac::print_eig() const {
  const int n = geom_->nbasis();
  for (int i = 0*n; i != 4*n; ++i) cout << setprecision(10) << setw(15) << eig_[i] <<  endl;
}


shared_ptr<const Reference> Dirac::conv_to_ref() const {
  // we store only positive state coefficients
  const size_t npos = coeff_->mdim() - nneg_;
  // coeff is occ, virt, nneg
  shared_ptr<ZMatrix> c = coeff_->clone();
  c->copy_block(0, 0, c->ndim(), npos, coeff_->slice(nneg_, npos));
  c->copy_block(0, npos, c->ndim(), nneg_, coeff_->slice(0, nneg_));
  auto out = make_shared<RelReference>(geom_, c, energy_, nneg_, nele_, 0, npos-nele_, gaunt_, breit_);
  vector<double> eigp(eig_.begin()+nneg_, eig_.end());
  vector<double> eigm(eig_.begin(), eig_.begin()+nneg_);
  VectorB eig(eig_.size());
  copy(eigp.begin(), eigp.end(), eig.begin());
  copy(eigm.begin(), eigm.end(), eig.begin()+eigp.size());
  out->set_eig(eig);
  return out;
}


shared_ptr<const DistZMatrix> Dirac::initial_guess(const shared_ptr<const DistZMatrix> s12, const shared_ptr<const DistZMatrix> hcore) const {
  const int n = geom_->nbasis();
  VectorB eig(hcore->ndim());

  shared_ptr<const DistZMatrix> coeff;
  if (!ref_) {
    // No reference; starting from hcore
    DistZMatrix interm = *s12 % *hcore * *s12;
    interm.diagonalize(eig);
    coeff = make_shared<const DistZMatrix>(*s12 * interm);

  } else if (dynamic_pointer_cast<const RelReference>(ref_)) {
    auto relref = dynamic_pointer_cast<const RelReference>(ref_);

    if (relref->rel()) {
      // Relativistic (4-component) reference
      shared_ptr<ZMatrix> fock = make_shared<DFock>(geom_, hcore_, relref->relcoeff()->slice_copy(0, nele_), gaunt_, breit_, /*store_half*/false, robust_);
      DistZMatrix interm = *s12 % *fock->distmatrix() * *s12;
      interm.diagonalize(eig);
      coeff = make_shared<const DistZMatrix>(*s12 * interm);
    } else {
      // Non-relativistic, GIAO-based reference
      const string typeinfo = geom_->london() ? "GIAO" : "(common origin)";
      assert(geom_->magnetism());
      const int nocc = ref_->nocc();
      shared_ptr<ZMatrix> fock;
      assert(nocc*2 == nele_);
      auto ocoeff = make_shared<ZMatrix>(n*4, 2*nocc);
      ocoeff->add_block(1.0, 0,    0, n, nocc, relref->relcoeff()->slice(0,nocc));
      ocoeff->add_block(1.0, n, nocc, n, nocc, relref->relcoeff()->slice(0,nocc));
      fock = make_shared<DFock>(geom_, hcore_, ocoeff, gaunt_, breit_, /*store_half*/false, robust_);
      DistZMatrix interm = *s12 % *fock->distmatrix() * *s12;
      interm.diagonalize(eig);
      coeff = make_shared<const DistZMatrix>(*s12 * interm);
    }
  } else if (ref_->coeff()->ndim() == n) {
    // Non-relativistic, real reference
    assert(!geom_->magnetism());
    const int nocc = ref_->nocc();
    shared_ptr<ZMatrix> fock;
    if (nocc*2 == nele_) {
      // RHF
      auto ocoeff = make_shared<ZMatrix>(n*4, 2*nocc);
      ocoeff->add_real_block(1.0, 0,    0, n, nocc, ref_->coeff()->slice(0,nocc));
      ocoeff->add_real_block(1.0, n, nocc, n, nocc, ref_->coeff()->slice(0,nocc));
      fock = make_shared<DFock>(geom_, hcore_, ocoeff, gaunt_, breit_, /*store_half*/false, robust_);
    } else if (ref_->noccB() != 0) {
      // UHF & ROHF
      const int nocca = ref_->noccA();
      const int noccb = ref_->noccB();
      assert(nocca+noccb == nele_);
      auto ocoeff = make_shared<ZMatrix>(n*4, nocca+noccb);
      ocoeff->add_real_block(1.0, 0,     0, n, nocca, ref_->coeffA()->slice(0,nocca));
      ocoeff->add_real_block(1.0, n, nocca, n, noccb, ref_->coeffB()->slice(0,noccb));
      fock = make_shared<DFock>(geom_, hcore_, ocoeff, gaunt_, breit_, /*store_half*/false, robust_);
    } else {
      // CASSCF
      auto ocoeff = make_shared<ZMatrix>(n*4, 2*nele_);
      ocoeff->add_real_block(1.0, 0,     0, n, nele_, ref_->coeff()->slice(0,nele_));
      ocoeff->add_real_block(1.0, n, nele_, n, nele_, ref_->coeff()->slice(0,nele_));
      fock = make_shared<DFock>(geom_, hcore_, ocoeff, gaunt_, breit_, /*store_half*/false, robust_);
    }
    DistZMatrix interm = *s12 % *fock->distmatrix() * *s12;
    interm.diagonalize(eig);
    coeff = make_shared<const DistZMatrix>(*s12 * interm);
  } else {
    assert(ref_->coeff()->ndim() == n*4);
    throw logic_error("Invalid Reference provided for Dirac.  (Initial guess not implemented.)");
  }
  return coeff;
}

