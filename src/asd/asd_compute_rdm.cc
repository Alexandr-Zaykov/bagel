//
// BAGEL - Parallel electron correlation program.
// Filename: asd_compute_rdm.cc
// Copyright (C) 2014 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: NU theory
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

#include <src/asd/asd_base.h>
#include <src/asd/state_tensor.h>
#include <src/smith/prim_op.h>

using namespace std;
using namespace bagel;
using namespace btas;

//***************************************************************************************************************
void
ASD_base::compute_rdm () {
//***************************************************************************************************************
//const int norbA = dimer_->active_refs().first->nact();
//const int norbB = dimer_->active_refs().second->nact();

  // compute transformed gammas (J',J,zeta)
  StateTensor st(adiabats_, subspaces_base());
  st.print();

  // TODO parallelize
  // Loop over both tensors and mupltiply
  const int istate = 0;
  GammaTensor half;
  for (auto& i : *gammatensor_[0]) {
    for (auto& j : st) {
      // if the third index of the gamma tensor is identical to the first one of the state tensor we contract
      auto& ikey = i.first;
      auto& jkey = j.first;
      if (get<0>(jkey) == istate && get<2>(ikey) == get<1>(jkey)) {
        auto tag = make_tuple(get<0>(ikey), get<1>(ikey), get<2>(jkey));
        if (half.exist(tag)) {
          contract(1.0, *i.second, {0,1,2}, j.second, {1,3}, 0.0, *half.get_block(tag), {0,3,2});
        } else {
          auto data = make_shared<Tensor3<double>>(get<1>(ikey).nstates(), get<2>(jkey).nstates(), i.second->extent(2));
          contract(1.0, *i.second, {0,1,2}, j.second, {1,3}, 0.0, *data, {0,3,2});
          half.emplace(tag, data);
        }
      }
    }
  }

  worktensor_ = make_shared<GammaTensor>();
  for (auto& i : half) {
    for (auto& j : st) {
      auto& ikey = i.first;
      auto& jkey = j.first;
      if (get<0>(jkey) == istate && get<1>(ikey) == get<1>(jkey)) {
        auto tag = make_tuple(get<0>(ikey), get<2>(jkey), get<2>(ikey));

        // TODO check if this transformation is necessary
        if (worktensor_->exist(tag)) {
          contract(1.0, *i.second, {0,1,2}, j.second, {0,3}, 1.0, *worktensor_->get_block(tag), {3,1,2});
        } else {
          auto data = make_shared<Tensor3<double>>(get<2>(jkey).nstates(), get<2>(ikey).nstates(), i.second->extent(2));
          contract(1.0, *i.second, {0,1,2}, j.second, {0,3}, 0.0, *data, {3,1,2});
          worktensor_->emplace(tag, data);
        }
      }
    }
  }

/*
  //compute transformed gamma (I',I,nu)
  //first half
  GammaTensor half2;
  for (auto& i : *gammatensor_[1]) {
    for (auto& j : st) {
      auto& ikey = i.first;
      auto& jkey = j.first;
      if (get<0>(jkey) == istate &&  //ground state
          get<1>(ikey) == get<2>(jkey)) { //gamma (nu,[J'],J) == C(0,I',[J'])
        auto tag = make_tuple(get<0>(ikey), get<1>(jkey), get<2>(ikey)); // (nu,I',J)
        if (half2.exist(tag)) {
          contract(1.0, *i.second, {0,1,2}, j.second, {3,0}, 0.0, *half2.get_block(tag), {3,1,2});
        } else {
          auto data = make_shared<Tensor3<double>>(get<1>(jkey).nstates(), get<2>(ikey).nstates(), i.second->extent(2)); // (I',J,nu)
          contract(1.0, *i.second, {0,1,2}, j.second, {3,0}, 0.0, *data, {3,1,2});
          // Gamma(J',J,nu) * C(I',J') -> Gamma'(I',J,nu)
          //       0  1 2       3  0             3  1 2
          half2.emplace(tag, data);
        }
      }
    }
  }

  worktensor2_ = make_shared<GammaTensor>();
  for (auto& i : half2) {
    for (auto& j : st) {
      auto& ikey = i.first;
      auto& jkey = j.first;
      if (get<0>(jkey) == istate && //ground state
          get<2>(ikey) == get<2>(jkey)) { //gamma'(nu,I',[J]) == C(0,I,[J])
        auto tag = make_tuple(get<0>(ikey), get<1>(ikey), get<1>(jkey)); // (nu,I',I)

        // TODO check if this transformation is necessary
        if (worktensor2_->exist(tag)) {
          contract(1.0, *i.second, {0,1,2}, j.second, {3,1}, 1.0, *worktensor2_->get_block(tag), {0,3,2});
        } else {
          auto data = make_shared<Tensor3<double>>(get<1>(ikey).nstates(), get<1>(jkey).nstates(), i.second->extent(2)); // (I',I,nu)
          contract(1.0, *i.second, {0,1,2}, j.second, {3,1}, 0.0, *data, {0,3,2});
          // Gamma'(I',J,nu) * C(I,J) -> Gamma''(I',I,nu)
          //        0  1 2       3 1             0  3 2
          worktensor2_->emplace(tag, data);
        }
      }
    }
  }
*/

  const auto subspaces = subspaces_base();


  // diagonal subspaces
  for (auto& subspace : subspaces) {
    shared_ptr<RDM<1>> r1;
    shared_ptr<RDM<2>> r2;
    tie(r1,r2) = compute_diagonal_block_RDM(subspace);
    if (r1) assert(false); //*onerdm_ += *r1;
    if (r2) *twordm_ += *r2;
  }
  
  // off diagonal subspaces
  for (auto iAB = subspaces.begin(); iAB != subspaces.end(); ++iAB) {
    for (auto jAB = subspaces.begin(); jAB != iAB; ++jAB) {
      shared_ptr<RDM<1>> r1;
      shared_ptr<RDM<2>> r2;
      tie(r1,r2) = couple_blocks_RDM(*jAB, *iAB); //Lower-triangular (i<->j)
      if (r1) *onerdm_ += *r1;
      if (r2) *twordm_ += *r2;
    }
  }
  

//3RDM

  // diagonal subspaces
  for (auto& subspace : subspaces) {
    shared_ptr<RDM<3>> r3;
    shared_ptr<RDM<4>> r4;
    tie(r3,r4) = compute_diagonal_block_RDM34(subspace);
    if (r3) *threerdm_ += *r3;
  }
  
  // off diagonal subspaces
  for (auto iAB = subspaces.begin(); iAB != subspaces.end(); ++iAB) {
    for (auto jAB = subspaces.begin(); jAB != iAB; ++jAB) {
      shared_ptr<RDM<3>> r3;
      shared_ptr<RDM<4>> r4;
      tie(r3,r4) = couple_blocks_RDM34(*jAB, *iAB); //Lower-triangular (i<->j)
      if (r3) *threerdm_ += *r3;
//    if (r4) *fourrdm_ += *r4;
    }
  }
  
  symmetrize_RDM();

  symmetrize_RDM34();

  debug_RDM(); 

}

//int n = 3;
//Matrix K(n,n);
//K.unit();
//btas::CRange<2> range(n*n, 1);
//const MatView gammaview(btas::make_view(range, K.storage()), /*localized*/true);
//
//auto MAT = make_shared<Matrix>( gammaview );
//MAT->print("a",n*n);
//cout << "(nxm)" << MAT->ndim() << MAT->mdim() << endl;

