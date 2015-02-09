//
// BAGEL - Parallel electron correlation program.
// Filename: multi/rasscf/rasscf.cc
// Copyright (C) 2015 Toru Shiozaki
//
// Author: Inkoo Kim <inkoo.kim@northwestern.edu>
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


#include <fstream>
#include <src/scf/hf/fock.h>
#include <src/multi/rasscf/rasscf.h>
#include <src/multi/casscf/qvec.h>

using namespace std;
using namespace bagel;


RASSCF::RASSCF(std::shared_ptr<const PTree> idat, const shared_ptr<const Geometry> geom, const shared_ptr<const Reference> re)
  : Method(idat, geom, re), hcore_(make_shared<Hcore>(geom)) {

  if (!ref_) {
    auto scf = make_shared<RHF>(idat, geom);
    scf->compute();
    ref_ = scf->conv_to_ref();
  }

  common_init();

}


void RASSCF::common_init() {
  // at the moment I only care about C1 symmetry, with dynamics in mind
  if (geom_->nirrep() > 1) throw runtime_error("RASSCF: C1 only at the moment.");
  print_header();

  const shared_ptr<const PTree> iactive = idata_->get_child_optional("active");
  if (iactive) {
    set<int> active_indices;
    // Subtracting one so that orbitals are input in 1-based format but are stored in C format (0-based)
    for (auto& i : *iactive) active_indices.insert(lexical_cast<int>(i->data()) - 1);
    ref_ = ref_->set_active(active_indices);
    cout << " " << endl;
    cout << "    ==== Active orbitals : ===== " << endl;
    for (auto& i : active_indices) cout << "         Orbital " << i+1 << endl;
    cout << "    ============================ " << endl << endl;
  }

  // first set coefficient
  coeff_ = ref_->coeff();
  if (geom_->nbasis() != coeff_->mdim()) {
    Overlap ovl(geom_);
    shared_ptr<const Matrix> tildex = ovl.tildex();

    Matrix c(coeff_->ndim(), tildex->mdim());
    c.copy_block(0, 0, coeff_->ndim(), coeff_->mdim(), coeff_);

    shared_ptr<const Matrix> trans = get<0>((*tildex % ovl * *coeff_).svd());
    c.copy_block(0, coeff_->mdim(), coeff_->ndim(), tildex->mdim()-coeff_->mdim(), *tildex * trans->slice(coeff_->mdim(), tildex->mdim()));
    coeff_ = make_shared<Coeff>(move(c));

#ifndef NDEBUG
    Matrix unit(coeff_->mdim(), coeff_->mdim()); unit.unit();
    assert((*coeff_ % ovl * *coeff_ - unit).rms() < 1.0e-10);
#endif
  }

  // get maxiter from the input
  max_iter_ = idata_->get<int>("maxiter", 50);
  // get maxiter from the input
  max_micro_iter_ = idata_->get<int>("maxiter_micro", 100);
  // get nstate from the input
  nstate_ = idata_->get<int>("nstate", 1);
  // get thresh (for macro iteration) from the input
  thresh_ = idata_->get<double>("thresh", 1.0e-8);
  // get thresh (for micro iteration) from the input
  thresh_micro_ = idata_->get<double>("thresh_micro", 5.0e-6);
  // option for printing natural orbitals
  natocc_ = idata_->get<bool>("natocc",false);

  // nocc from the input. If not present, full valence active space is generated.
  nact_ = idata_->get<int>("nact", 0);
  nact_ = idata_->get<int>("nact_cas", nact_);

  // nclosed from the input. If not present, full core space is generated.
  nclosed_ = idata_->get<int>("nclosed", -1);
  if (nclosed_ < -1) {
    throw runtime_error("It appears that nclosed < 0. Check nocc value.");
  } else if (nclosed_ == -1) {
    cout << "    * full core space generated for nclosed." << endl;
    nclosed_ = geom_->num_count_ncore_only() / 2;
  }
  nocc_ = nclosed_ + nact_;

  nbasis_ = coeff_->mdim();
  nvirt_ = nbasis_ - nocc_;
  if (nvirt_ < 0) throw runtime_error("It appears that nvirt < 0. Check the nocc value");

  cout << "    * nstate   : " << setw(6) << nstate_ << endl;
  cout << "    * nclosed  : " << setw(6) << nclosed_ << endl;
  cout << "    * nact     : " << setw(6) << nact_ << endl;
  cout << "    * nvirt    : " << setw(6) << nvirt_ << endl;

  const int idel = geom_->nbasis() - nbasis_;
  if (idel)
    cout << "      Due to linear dependency, " << idel << (idel==1 ? " function is" : " functions are") << " omitted" << endl;


  // RASSCF methods should have RASCI member. Inserting "ncore" and "norb" keyword for closed and total orbitals.
  mute_stdcout();
  if (nact_) {
//  auto idata = make_shared<PTree>(*idata_);
//  idata->erase("active");
    cout << "let's call rasci" << endl;
    auto idata = idata_->get_child_optional("ras") ? idata_->get_child_optional("ras") : make_shared<PTree>();
  //rasci_ = make_shared<RASCI>(idata, geom_, ref_, nclosed_, nact_); // nstate does not need to be specified as it is in idata_...
    rasci_ = make_shared<RASCI>(idata, geom_, ref_); 
    ras_ = rasci_->rasarray();
  }
  resume_stdcout();


  schwarz_ = geom_->schwarz();

  cout <<  "  === RASSCF iteration (" + geom_->basisfile() + ") ===" << endl << endl;

}


RASSCF::~RASSCF() {

}


void RASSCF::print_header() const {
  cout << "  ---------------------------" << endl;
  cout << "      RASSCF calculation     " << endl;
  cout << "  ---------------------------" << endl << endl;
}

void RASSCF::print_iteration(int iter, int miter, int tcount, const vector<double> energy, const double error, const double time) const {
  if (energy.size() != 1 && iter) cout << endl;

  int i = 0;
  for (auto& e : energy) {
    cout << "  " << setw(5) << iter << setw(3) << i << setw(4) << miter << setw(4) << tcount
                 << setw(16) << fixed << setprecision(8) << e << "   "
                 << setw(10) << scientific << setprecision(2) << (i==0 ? error : 0.0) << fixed << setw(10) << setprecision(2)
                 << time << endl;
    ++i;
  }
}

static streambuf* backup_stream_;
static ofstream* ofs_;

void RASSCF::mute_stdcout() {
  ofstream* ofs(new ofstream("casscf.log",(backup_stream_ ? ios::app : ios::trunc)));
  ofs_ = ofs;
  backup_stream_ = cout.rdbuf(ofs->rdbuf());
}


void RASSCF::resume_stdcout() {
  cout.rdbuf(backup_stream_);
  delete ofs_;
}


shared_ptr<Matrix> RASSCF::ao_rdm1(shared_ptr<const RDM<1>> rdm1, const bool inactive_only) const {
  // first make 1RDM in MO
  const size_t nmobasis = coeff_->mdim();
  auto mo_rdm1 = make_shared<Matrix>(nmobasis, nmobasis);
  for (int i = 0; i != nclosed_; ++i) mo_rdm1->element(i,i) = 2.0;
  if (!inactive_only) {
    for (int i = 0; i != nact_; ++i) {
      for (int j = 0; j != nact_; ++j) {
        mo_rdm1->element(nclosed_+j, nclosed_+i) = rdm1->element(j,i);
      }
    }
  }
  // transform into AO basis
  return make_shared<Matrix>(*coeff_ * *mo_rdm1 ^ *coeff_);
}


#if 0
void RASSCF::one_body_operators(shared_ptr<Matrix>& f, shared_ptr<Matrix>& fact, shared_ptr<Matrix>& factp, shared_ptr<Matrix>& gaa,
                                shared_ptr<RASRotFile>& d, const bool superci) const {

  shared_ptr<Matrix> finact;

  // get quantity Q_xr = 2(xs|tu)P_rs,tu (x=general)
  // note: this should be after natorb transformation.
  auto qxr = make_shared<Qvec>(coeff_->mdim(), nact_, coeff_, nclosed_, fci_, fci_->rdm2_av());

  {
    // Fock operators
    // make a matrix that contains rdm1_av
    auto rdm1mat = make_shared<Matrix>(nact_, nact_);
    copy_n(fci_->rdm1_av()->data(), rdm1mat->size(), rdm1mat->data());
  rdm1mat->print("1RDM (natural)", nact_);
    rdm1mat->sqrt();
    rdm1mat->scale(1.0/sqrt(2.0));
  rdm1mat->print("1RDM (scaled)", nact_);
    auto acoeff = coeff_->slice(nclosed_, nclosed_+nact_);

    finact = make_shared<Matrix>(*coeff_ % *fci_->jop()->core_fock() * *coeff_);
    auto fact_ao = make_shared<Fock<1>>(geom_, hcore_->clone(), nullptr, acoeff * *rdm1mat, false, /*rhf*/true);
    f = make_shared<Matrix>(*finact + *coeff_% *fact_ao * *coeff_);
  }
  fci_->jop()->core_fock()->print("Core Fock matrix", f->ndim());
  finact->print("F_inact matrix", f->ndim());
  f->print("F matrix", f->ndim());
  hcore_->clone()->print("hcore matrix", hcore_->ndim());
  {
    // active-x Fock operator Dts finact_sx + Qtx
    fact = qxr->copy();// nbasis_ runs first
    for (int i = 0; i != nact_; ++i)
      daxpy_(nbasis_, occup_(i), finact->element_ptr(0,nclosed_+i), 1, fact->data()+i*nbasis_, 1);
  }
  for (int i = 0; i != nbasis_; ++i) {
    cout << " Fact(" << i << "th row) = ";
    for (int j = 0; j != nact_; ++j) {
      cout << fact->element(i,j) << " ";
    }
    cout << endl;
  }    
  {
    // active Fock' operator (Fts+Fst) / (ns+nt)
    factp = make_shared<Matrix>(nact_, nact_);
    for (int i = 0; i != nact_; ++i)
      for (int j = 0; j != nact_; ++j) {
#if 1
        if (occup_(i)+occup_(j) > roccup_thresh)
          factp->element(j,i) = (fact->element(j+nclosed_,i)+fact->element(i+nclosed_,j)) / (occup_(i)+occup_(j));
        else
          factp->element(j,i) = 0.0;
#else
        factp->element(j,i) = (fact->element(j+nclosed_,i)+fact->element(i+nclosed_,j)) *0.5;
#endif
      }
  }

  // G matrix (active-active) Drs,tu Factp_tu - delta_rs nr sum_v Factp_vv
  gaa = factp->clone();
  dgemv_("N", nact_*nact_, nact_*nact_, 1.0, fci_->rdm2_av()->data(), nact_*nact_, factp->data(), 1, 0.0, gaa->data(), 1);
  double p = 0.0;
  for (int i = 0; i != nact_; ++i) p += occup_(i) * factp->element(i,i);
  for (int i = 0; i != nact_; ++i) gaa->element(i,i) -= occup_(i) * p;

  // denominator
  auto denom = make_shared<RASRotFile>(nclosed_, nact_, nvirt_);
  fill_n(denom->data(), denom->size(), 1.0e100);

  double* target = denom->ptr_va();
  for (int i = 0; i != nact_; ++i) {
    if (occup_(i) > roccup_thresh) {
      for (int j = 0; j != nvirt_; ++j, ++target)
        *target = (gaa->element(i,i) + occup_(i)*f->element(j+nocc_, j+nocc_)) / (superci ? occup_(i) : 1.0);
    } else {
      for (int j = 0; j != nvirt_; ++j, ++target)
        *target = 1.0/roccup_thresh;
    }
  }

  target = denom->ptr_vc();
  for (int i = 0; i != nclosed_; ++i)
    for (int j = 0; j != nvirt_; ++j, ++target)
      *target = (f->element(j+nocc_, j+nocc_) - f->element(i, i)) / (superci ? 2.0 : 1.0);

  target = denom->ptr_ca();
  for (int i = 0; i != nact_; ++i) {
    if (2.0-occup_(i) > roccup_thresh) {
      for (int j = 0; j != nclosed_; ++j, ++target)
        *target = ((f->element(nclosed_+i,nclosed_+i)*2.0-fact->element(i+nclosed_,i)) - f->element(j, j)*(2.0-occup_(i))) / (superci ? 2.0-occup_(i) : 1.0);
    } else {
      for (int j = 0; j != nclosed_; ++j, ++target)
        *target = 1.0/roccup_thresh;
    }
  }
  d = denom;
}
#endif

shared_ptr<const Coeff> RASSCF::update_coeff(const shared_ptr<const Matrix> cold, shared_ptr<const Matrix> mat) const {
  auto cnew = make_shared<Coeff>(*cold);
  int nbas = cold->ndim();
  assert(nbas == geom_->nbasis());
  dgemm_("N", "N", nbas, nact_, nact_, 1.0, cold->data()+nbas*nclosed_, nbas, mat->data(), nact_,
                   0.0, cnew->data()+nbas*nclosed_, nbas);
  return cnew;
}


#if 0
shared_ptr<Matrix> RASSCF::form_natural_orbs() {
  // here make a natural orbitals and update the coefficients
  // this effectively updates 1,2RDM and integrals
  const pair<shared_ptr<Matrix>, VectorB> natorb = fci_->natorb_convert();
  // new coefficients
  coeff_ = update_coeff(coeff_, natorb.first);
  // occupation number of the natural orbitals
  occup_ = natorb.second;
  if (natocc_) print_natocc();
  return natorb.first;
}
#endif

#if 0
shared_ptr<const Coeff> RASSCF::semi_canonical_orb() const {
  auto rdm1mat = make_shared<Matrix>(nact_, nact_);
  copy_n(fci_->rdm1_av()->data(), rdm1mat->size(), rdm1mat->data());
  rdm1mat->sqrt();
  rdm1mat->scale(1.0/sqrt(2.0));
  auto ocoeff = coeff_->slice(0, nclosed_);
  auto acoeff = coeff_->slice(nclosed_, nocc_);
  auto vcoeff = coeff_->slice(nocc_, nbasis_);

  VectorB eig(coeff_->mdim());
  Fock<1> fock(geom_, fci_->jop()->core_fock(), nullptr, acoeff * *rdm1mat, false, /*rhf*/true);
  Matrix trans(nbasis_, nbasis_);
  trans.unit();
  if (nclosed_) {
    Matrix ofock = ocoeff % fock * ocoeff;
    ofock.diagonalize(eig);
    trans.copy_block(0, 0, nclosed_, nclosed_, ofock);
  }
  Matrix vfock = vcoeff % fock * vcoeff;
  vfock.diagonalize(eig);
  trans.copy_block(nocc_, nocc_, nvirt_, nvirt_, vfock);
  return make_shared<Coeff>(*coeff_ * trans);
}
#endif

shared_ptr<const Reference> RASSCF::conv_to_ref() const {
  shared_ptr<Reference> out;
  assert(false);
}
#if 0
  if (nact_) {
    out = make_shared<Reference>(geom_, coeff_, nclosed_, nact_, nvirt_, energy_av(),
                                 rasci_->rdm1(), rasci_->rdm2(), rasci_->rdm1_av(), rasci_->rdm2_av(), rasci_->conv_to_ciwfn());
    // TODO
    // compute one-body operators
    shared_ptr<Matrix> f;
    shared_ptr<Matrix> fact, factp, gaa;
    shared_ptr<RASRotFile>  denom;
    one_body_operators(f, fact, factp, gaa, denom);
    if (natocc_) print_natocc();

    *f *= 2.0;

    for (int i = 0; i != nbasis_; ++i) {
      for (int j = 0; j != nbasis_; ++j) {
        if (i < nocc_ && j < nocc_) continue;
        f->element(j,i) = 0.0;
      }
    }
    for (int j = 0; j != nact_; ++j) {
      for (int i = 0; i != nocc_; ++i) {
        f->element(i,j+nclosed_) = fact->element(i,j);
      }
    }

    auto erdm = make_shared<Matrix>(*coeff_ * *f ^ *coeff_);

    out->set_erdm1(erdm);
    out->set_nstate(nstate_);
  } else {
    out = make_shared<Reference>(geom_, coeff_, nclosed_, nact_, nvirt_, energy_av());
  }
  return out;
}
#endif

#if 0
void RASSCF::print_natocc() const {
  assert(occup_.size() > 0);
  cout << " " << endl;
  cout << "  ========       state-averaged       ======== " << endl;
  cout << "  ======== natural occupation numbers ======== " << endl;
  for (int i=0; i!=occup_.size(); ++i)
    cout << setprecision(4) << "   Orbital " << i << " : " << occup_[i] << endl;
  cout << "  ============================================ " << endl;
}
#endif
