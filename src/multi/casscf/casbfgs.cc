//
// BAGEL - Parallel electron correlation program.
// Filename: casbfgs.cc
// Copyright (C) 2013 Toru Shiozaki
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


#include <src/multi/casscf/casbfgs.h>
#include <src/multi/casscf/qvec.h>
#include <src/scf/hf/fock.h>
#include <src/util/math/davidson.h>
#include <src/util/math/step_restrict_bfgs.h>
#include <src/util/math/hpw_diis.h>

using namespace std;
using namespace bagel;

void CASBFGS::compute() {

  // equation numbers refer to Chaban, Schmidt and Gordon 1997 TCA 97, 88.

  shared_ptr<SRBFGS<RotFile>> bfgs;

  // ============================
  // macro iteration from here
  // ============================
  Timer timer;

  auto x = make_shared<Matrix>(nbasis_, nbasis_);
  x->unit();
  shared_ptr<const Matrix> xstart;
  vector<double> evals;
  const int limited_memory = idata_->get<int>("limited_memory", 0);

  mute_stdcout();
  for (int iter = 0; iter != max_iter_; ++iter) {

    const shared_ptr<const Coeff> cold = coeff_;
    const shared_ptr<const Matrix> xold = x->copy();

    // first perform CASCI to obtain RDMs
    if (nact_) {
      if (iter) fci_->update(coeff_);
      Timer fci_time(0);
      fci_->compute();
      fci_->compute_rdm12();
      fci_time.tick_print("FCI and RDMs");
    }

    shared_ptr<Matrix> natorb_mat = x->clone();
    natorb_mat->unit();
    if (nact_) {
      // here make a natural orbitals and update coeff_. Closed and virtual orbitals remain canonical. Also, FCI::rdms are updated
      const pair<shared_ptr<Matrix>, VectorB> natorb = fci_->natorb_convert();
      coeff_ = update_coeff(coeff_, natorb.first);
      occup_ = natorb.second;
      if (natocc_) print_natocc();
      natorb_mat->copy_block(nclosed_, nclosed_, nact_, nact_, natorb.first);
    }

    auto sigma = make_shared<RotFile>(nclosed_, nact_, nvirt_);
    sigma->zero();

    // compute one-body operators
    Timer onebody;
    // * preparation
    const MatView ccoeff = coeff_->slice(0, nclosed_);
    // * core Fock operator
    shared_ptr<const Matrix> cfockao = nclosed_ ? make_shared<const Fock<1>>(geom_, hcore_, nullptr, ccoeff, /*store*/false, /*rhf*/true) : hcore_;
    shared_ptr<const Matrix> cfock = make_shared<Matrix>(*coeff_ % *cfockao * *coeff_);
    // * active Fock operator
    // first make a weighted coefficient
    shared_ptr<Matrix> acoeff;
    if (nact_) {
      acoeff = coeff_->slice_copy(nclosed_, nocc_);
      for (int i = 0; i != nact_; ++i)
        blas::scale_n(sqrt(occup_[i]/2.0), acoeff->element_ptr(0, i), acoeff->ndim());
    }
    // then make a AO density matrix
    shared_ptr<const Matrix> afock;
    if (nact_) {
      auto afockao = make_shared<Fock<1>>(geom_, hcore_, nullptr, acoeff, /*store*/false, /*rhf*/true);
      afock = make_shared<Matrix>(*coeff_ % (*afockao - *hcore_) * *coeff_);
    } else {
      afock = cfock->clone();
    }
    // * Q_xr = 2(xs|tu)P_rs,tu (x=general, mo)
    shared_ptr<const Qvec> qxr;
    if (nact_) {
      qxr = make_shared<const Qvec>(coeff_->mdim(), nact_, coeff_, nclosed_, fci_, fci_->rdm2_av());
    }

    // grad(a/i) (eq.4.3a): 4(cfock_ai+afock_ai)
    grad_vc(cfock, afock, sigma);
    // grad(a/t) (eq.4.3b): 2cfock_au gamma_ut + q_at
    grad_va(cfock, qxr, sigma);
    // grad(r/i) (eq.4.3c): 4(cfock_ri+afock_ri) - 2cfock_iu gamma_ur - qxr_ir
    grad_ca(cfock, afock, qxr, sigma);

    // if this is the first time, set up the BFGS solver
    if (iter == 0) {
      // BFGS and DIIS should start at the same time
      shared_ptr<const RotFile> denom = compute_denom(cfock, afock, qxr);
      bfgs = make_shared<SRBFGS<RotFile>>(denom);
    }
    onebody.tick_print("One body operators");

    // get energy
    if (nact_) {
      energy_ = fci_->energy();
      // use state averaged energy to update trust radius
      const double sa_en = blas::average(energy_);
      evals.push_back(sa_en);
    } else {
      const double en = (ccoeff % (*cfockao + *hcore_) * ccoeff).trace() + geom_->nuclear_repulsion();
      energy_ = {en};
      evals.push_back(en);
    }

    // extrapolation using BFGS
    Timer extrap(0);
    cout << " " << endl;
    cout << " -------  Step Restricted BFGS Extrapolation  ------- " << endl;
    *x *= *natorb_mat;
    auto xcopy = x->log(8);
    auto xlog  = make_shared<RotFile>(xcopy, nclosed_, nact_, nvirt_);
    bfgs->check_step(evals, sigma, xlog, /*tight*/false, limited_memory);
    shared_ptr<RotFile> a = bfgs->more_sorensen_extrapolate(sigma, xlog);
    cout << " ---------------------------------------------------- " << endl;
    extrap.tick_print("More-Sorensen/Hebden extrapolation");
    cout << " " << endl;

    // restore the matrix from RotFile
    shared_ptr<const Matrix> amat = a->unpack<Matrix>();
    shared_ptr<Matrix> expa = amat->exp(100);
    expa->purify_unitary();

    // updating coefficients
    coeff_ = make_shared<const Coeff>(*coeff_**expa);
    // for next BFGS extrapolation
    *x *= *expa;

    // synchronization
    mpi__->broadcast(const_pointer_cast<Coeff>(coeff_)->data(), coeff_->size(), 0);

    // setting error of macro iteration
    const double gradient = sigma->rms();

    resume_stdcout();
    print_iteration(iter, 0, 0, energy_, gradient, timer.tick());

    if (gradient < thresh_) {
      rms_grad_ = gradient;
      cout << " " << endl;
      cout << "    * quasi-Newton optimization converged. *   " << endl << endl;
      mute_stdcout();
      break;
    }

    if (iter == max_iter_-1) {
      rms_grad_ = gradient;
      cout << " " << endl;
      if (rms_grad_ > thresh_) cout << "    * The calculation did NOT converge. *    " << endl;
      cout << "    * Max iteration reached during the quasi-Newton optimization. *     " << endl << endl;
    }
    mute_stdcout();
  }
  resume_stdcout();
  // ============================
  // macro iteration to here
  // ============================

  // block diagonalize coeff_ in nclosed and nvirt
  coeff_ = semi_canonical_orb();

  // this is not needed for energy, but for consistency we want to have this...
  // update construct Jop from scratch
  if (nact_) {
    fci_->update(coeff_);
    fci_->compute();
    fci_->compute_rdm12();
  }
}


shared_ptr<const RotFile> CASBFGS::compute_denom(shared_ptr<const Matrix> cfock, shared_ptr<const Matrix> afock, shared_ptr<const Matrix> qxr) const {
  auto out = make_shared<RotFile>(nclosed_, nact_, nvirt_);
  const double tiny = 1.0e-15;

  // ia part (4.7a)
  if (nvirt_ && nclosed_) {
    double* target = out->ptr_vc();
    for (int i = 0; i != nclosed_; ++i) {
      for (int j = 0; j != nvirt_; ++j) {
        *target++ = 4.0*(cfock->element(j+nocc_, j+nocc_)+afock->element(j+nocc_, j+nocc_)) - 4.0*(cfock->element(i,i)+afock->element(i,i));
      }
    }
  }
  // ra part (4.7b)
  if (nvirt_ && nact_) {
    double* target = out->ptr_va();
    for (int i = 0; i != nact_; ++i) {
      if (occup_[i] < tiny) continue;
      for (int j = 0; j != nvirt_; ++j) {
        *target++ = 2.0*occup_[i]*(cfock->element(j+nocc_, j+nocc_)+afock->element(j+nocc_, j+nocc_))
                  - 2.0*occup_[i]*cfock->element(i+nclosed_, i+nclosed_) - 2.0*qxr->element(i+nclosed_, i);
      }
    }
  }
  // it part (4.7c)
  if (nclosed_ && nact_) {
    double* target = out->ptr_ca();
    for (int i = 0; i != nact_; ++i) {
      if (occup_[i] < tiny) continue;
      for (int j = 0; j != nclosed_; ++j) {
        *target++ = 4.0*(cfock->element(i+nclosed_, i+nclosed_)+afock->element(i+nclosed_, i+nclosed_) - cfock->element(j,j) - afock->element(j,j))
                  + 2.0*occup_[i]*(cfock->element(j,j)+afock->element(j,j)) - 2.0*occup_[i]*cfock->element(i+nclosed_, i+nclosed_) - 2.0*qxr->element(i+nclosed_, i);
      }
    }
  }

  const double thresh = 1.0e-8;
  for (int i = 0; i != out->size(); ++i)
    if (fabs(out->data(i)) < thresh) {
      out->data(i) = 1.0e10;
    }
  return out;
}


// grad(a/i) (eq.4.3a): 4(cfock_ai+afock_ai)
void CASBFGS::grad_vc(shared_ptr<const Matrix> cfock, shared_ptr<const Matrix> afock, shared_ptr<RotFile> sigma) const {
  if (!nvirt_ || !nclosed_) return;
  double* target = sigma->ptr_vc();
  for (int i = 0; i != nclosed_; ++i, target += nvirt_) {
    daxpy_(nvirt_, 4.0, cfock->element_ptr(nocc_,i), 1, target, 1);
    daxpy_(nvirt_, 4.0, afock->element_ptr(nocc_,i), 1, target, 1);
  }
}


// grad(a/t) (eq.4.3b): 2cfock_au gamma_ut + q_at
// gamma is assumed to be diagonal
void CASBFGS::grad_va(shared_ptr<const Matrix> cfock, shared_ptr<const Matrix> qxr, shared_ptr<RotFile> sigma) const {
  if (!nvirt_ || !nact_) return;
  double* target = sigma->ptr_va();
  for (int i = 0; i != nact_; ++i, target += nvirt_) {
    daxpy_(nvirt_, 2.0*occup_[i], cfock->element_ptr(nocc_, i+nclosed_), 1, target, 1);
    daxpy_(nvirt_, 2.0, qxr->element_ptr(nocc_, i), 1, target, 1);
  }
}


// grad(r/i) (eq.4.3c): 4(cfock_ri+afock_ri) - 2cfock_iu gamma_ur - qxr_ir
void CASBFGS::grad_ca(shared_ptr<const Matrix> cfock, shared_ptr<const Matrix> afock, shared_ptr<const Matrix> qxr, shared_ptr<RotFile> sigma) const {
  if (!nclosed_ || !nact_) return;
  {
    double* target = sigma->ptr_ca();
    for (int i = 0; i != nact_; ++i, target += nclosed_) {
      daxpy_(nclosed_, 4.0-2.0*occup_[i], cfock->element_ptr(0,nclosed_+i), 1, target, 1);
      daxpy_(nclosed_, 4.0, afock->element_ptr(0,nclosed_+i), 1, target, 1);
      daxpy_(nclosed_, -2.0, qxr->element_ptr(0, i), 1, target, 1);
    }
  }
}

