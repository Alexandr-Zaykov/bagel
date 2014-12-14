//
// BAGEL - Parallel electron correlation program.
// Filename: pdfdist.cc
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

#include <src/periodic/pdfdist.h>
#include <src/periodic/pdfinttask.h>

using namespace bagel;
using namespace std;

PDFDist::PDFDist(const vector<array<double, 3>>& L, const int nbas, const int naux,
                 const vector<shared_ptr<const Atom>>& atoms0,
                 const vector<shared_ptr<const Atom>>& aux_atoms, const shared_ptr<const Geometry> cell0,
                 const double thresh, const bool serial, const shared_ptr<Matrix> data2)
  : lattice_vectors_(L), nbasis_(nbas), naux_(naux), serial_(serial), data2_(data2) {

  // prepare to compute 1- and 2-index ints
  vector<shared_ptr<const Shell>> ashell, b0shell;
  for (auto& i : atoms0)    b0shell.insert(b0shell.end(), i->shells().begin(), i->shells().end());
  for (auto& i : aux_atoms)  ashell.insert(ashell.end(),  i->shells().begin(), i->shells().end());

  // compute auxiliary charge data1_ then projection matrix
  compute_aux_charge(ashell);

  // 2-index integrals (i|j_L)^{-1} (sum over L)
  if (data2)
    data2_ = data2;
  else
    pcompute_2index(ashell, thresh);

  // form PDFDist_ints for every cell
  dfdist_.resize(L.size());
  Timer time;
  for (int i = 0; i != L.size(); ++i) {
    vector<shared_ptr<const Atom>> atoms1(atoms0.size());
    int iat = 0;
    for (auto& atom : atoms0) {
      atoms1[iat] = make_shared<const Atom>(*atom, lattice_vectors_[i]);
      ++iat;
    }
    dfdist_[i] = make_shared<PDFDist_ints>(L, nbas, naux, atoms0, atoms1, aux_atoms, cell0, thresh, projector_, data1_);
  }
  time.tick_print("3-index and overlap integrals");
}


void PDFDist::compute_aux_charge(const vector<shared_ptr<const Shell>>& ashell) {

  Timer time;
  TaskQueue<PDFIntTask_aux> tasks(ashell.size());
  data1_ = make_shared<VectorB>(naux_);

  // <a|.>
  auto i1 = make_shared<const Shell>(ashell.front()->spherical());
  int j0 = 0;
  for (auto& i0 : ashell) {
    tasks.emplace_back(array<shared_ptr<const Shell>,2>{{i1, i0}}, j0, data1_);
    j0 += i0->nbasis();
  }

  tasks.compute();

  if (!serial_)
    data1_->allreduce();

  time.tick_print("aux charge integrals");

  const double q = data1_->rms() * naux_;
  for (auto& idata : *data1_) idata /= q;

  // P_{ij} = <i|.><.|j>
  Matrix p(naux_, naux_);
  dger_(naux_, naux_, 1.0, data1_->data(), 1, data1_->data(), 1, p.data(), naux_);
  projector_ = make_shared<const Matrix>(move(p));
}


void PDFDist::pcompute_2index(const vector<shared_ptr<const Shell>>& ashell, const double throverlap) {

  Timer time;

  TaskQueue<PDFIntTask_2index> tasks(ashell.size() * ashell.size() * ncell());

  // (a|bL)
  auto b3 = make_shared<const Shell>(ashell.front()->spherical());
  data2_ = make_shared<Matrix>(naux_, naux_, serial_);    data2_->zero();
  eta_   = make_shared<Matrix>(naux_, naux_, serial_);    eta_->zero();

  correction_   = make_shared<Matrix>(naux_, naux_, serial_);    correction_->zero();

  vector<shared_ptr<Matrix>> data2_at(ncell());
  for (auto& idat : data2_at) idat = make_shared<Matrix>(naux_, naux_, serial_);

  int o0 = 0;
  for (auto& b0 : ashell) {
    int n = 0;
    for (auto& L : lattice_vectors_) {
      int o1 = 0;
      for (auto& b1 : ashell) {
        auto b11 = make_shared<const Shell>(*(b1->move_atom(L)));
        tasks.emplace_back(array<shared_ptr<const Shell>,4>{{b11, b3, b0, b3}}, array<int,2>{{o0, o1}}, data2_at[n]);
        o1 += b1->nbasis();
      }
      ++n;
    }
    o0 += b0->nbasis();
  }

  time.tick_print("2-index integrals prep");
  tasks.compute();

  if (!serial_)
    for (auto& dat : data2_at)
      dat->allreduce();

  // now project and sum
  if (!projector_)
    throw logic_error("failed attempt to project data2_ before computing the projection matrix");

  // P_C = 1 - P
  auto projectorC = make_shared<Matrix>(naux_, naux_, serial_);
  projectorC->unit();
  *projectorC -= *projector_;

  for (int i = 0; i != ncell(); ++i) {
    *data2_ += *projectorC * *data2_at[i] * *projectorC;
//    *eta_ += *projectorC * *data2_at[i];
    dgemm_("N", "N", naux_, naux_, naux_, 1.0, projectorC->data(), naux_, data2_at[i]->data(), naux_, 0.0, eta_->data(), naux_);
    *correction_ += *data2_at[i];
  }

  cout << setprecision(9) << "    eta    " << eta_->rms()                 //DEBUG
                          << "   dat2    " << data2_->rms()               //DEBUG
                          << "    corr   " << correction_->rms() << endl; //DEBUG

  time.tick_print("2-index integrals");

  // make data2_ positive definite
  *data2_ += *projector_;

  data2_->inverse(); //TODO: linear dependency can be a problem!
  // use data2_ within node
  data2_->localize();
  time.tick_print("computing inverse");
}
