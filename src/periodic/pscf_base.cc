//
// BAGEL - Parallel electron correlation program.
// Filename: pscf_base.cc
// Copyright (C) 2014 Toru Shiozaki
//
// Author: Hai-Anh Le <anh@u.northwestern.edu>
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


#include <src/periodic/pscf_base.h>
#include <src/util/timer.h>
#include <src/util/math/diis.h>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;
using namespace bagel;

BOOST_CLASS_EXPORT_IMPLEMENT(PSCF_base)

PSCF_base::PSCF_base(const shared_ptr<const PTree> idata, const shared_ptr<const Geometry> geom, const shared_ptr<const Reference> re)
 : Method(idata, geom, re), dodf_(idata->get<bool>("df", true)), dofmm_(idata->get<bool>("cfmm", false)) {

  if (dofmm_) {
    fmm_lmax_   = idata->get<int>("l_max", 10);
    fmm_ws_     = idata->get<int>("ws", 2);
    fmm_extent_ = idata->get<int>("extent", 10);
    fmm_beta_   = idata->get<double>("beta", 1.0);
    fmm_height_ = idata->get<int>("height", 21);
    doewald_    = idata->get<bool>("ewald", false);
    lattice_ = make_shared<const Lattice>(geom, fmm_ws_);
  } else {
    lattice_ = make_shared<const Lattice>(geom, idata->get<int>("extent", 0));
  }

  Timer pscf;

  eig_.resize(lattice_->num_lattice_kvectors());
  for (auto& eigblock : eig_) eigblock = make_shared<VectorB>(geom->nbasis());

  restart_ = idata_->get<bool>("restart", false);
  overlap_ = make_shared<const POverlap>(lattice_);
  koverlap_ = overlap_->ft(lattice_->lattice_vectors(), lattice_->lattice_kvectors());
  pscf.tick_print("Periodic overlap matrix");
  hcore_ = make_shared<const PHcore>(lattice_);
  pscf.tick_print("Periodic hcore matrix");
  if (dofmm_)
    fmm_ = lattice_->form_pfmm(dodf_, fmm_lmax_, fmm_ws_, fmm_extent_, fmm_beta_, fmm_height_, doewald_);

  max_iter_ = idata_->get<int>("maxiter", 100);
  max_iter_ = idata_->get<int>("maxiter_scf", max_iter_);
  diis_start_ = idata_->get<int>("diis_start", 1);
  diis_size_ = idata_->get<int>("diis_size", 5);
  thresh_overlap_ = idata_->get<double>("thresh_overlap", 1.0e-8);
  thresh_scf_ = idata_->get<double>("thresh", 1.0e-8);
  thresh_scf_ = idata_->get<double>("thresh_scf", thresh_scf_);
  string dd = idata_->get<string>("diis", "gradient");

  const int ncharge = idata_->get<int>("charge", 0);
  const int nact    = idata_->get<int>("nact", (geom_->nele() - ncharge) % 2);
  nocc_ = idata_->get<int>("nocc", (geom_->nele() - ncharge + nact) / 2);
  noccB_ = nocc_ - nact;

  if (nocc_ + noccB_ != geom_->nele() - ncharge)
    throw runtime_error("*** nocc and nact for the unit cell are not consistently specified!");

  ktildex_ = koverlap_->tildex(thresh_overlap_);
  pscf.tick_print("Periodic overlap orthogonalization");

  cout << endl;
}
