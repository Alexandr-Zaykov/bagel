//
// BAGEL - Parallel electron correlation program.
// Filename: spinfreebase.cc
// Copyright (C) 2014 Toru Shiozaki
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

#include <src/smith/moint.h>
#include <src/smith/spinfreebase.h>
#include <src/smith/smith_util.h>

using namespace std;
using namespace bagel;
using namespace bagel::SMITH;

template<typename DataType>
SpinFreeMethod<DataType>::SpinFreeMethod(shared_ptr<const SMITH_Info<DataType>> inf) : info_(inf) {
  static_assert(is_same<DataType,double>::value or is_same<DataType,complex<double>>::value,
                "illegal DataType for SpinFreeMethod");

  Timer timer;
  const int max = info_->maxtile();
  if (info_->ncore() > info_->nclosed())
    throw runtime_error("frozen core has been specified but there are not enough closed orbitals");

  // TODO we need to update the following. Probably it is a good idea to generalize IndexRange class

  closed_ = IndexRange(info_->nclosed()-info_->ncore(), max, 0, info_->ncore());
  if (is_same<DataType,complex<double>>::value)
    closed_.merge(IndexRange(info_->nclosed()-info_->ncore(), max, closed_.nblock(), info_->ncore()*2+closed_.size(), info_->ncore()));

  active_ = IndexRange(info_->nact(), min(max,10), closed_.nblock(), info_->ncore()+closed_.size());
  if (is_same<DataType,complex<double>>::value)
    active_.merge(IndexRange(info_->nact(), min(max,10), closed_.nblock()+active_.nblock(), info_->ncore()*2+closed_.size()+active_.size(),
                                                                                            info_->ncore()*2+closed_.size()));

  virt_ = IndexRange(info_->nvirt(), max, closed_.nblock()+active_.nblock(), info_->ncore()*2+closed_.size()+active_.size());
  if (is_same<DataType,complex<double>>::value)
    virt_.merge(IndexRange(info_->nvirt(), max, closed_.nblock()+active_.nblock()+virt_.nblock(), info_->ncore()*2+closed_.size()+active_.size()+virt_.size(),
                                                                                                  info_->ncore()*2+closed_.size()+active_.size()));

  all_    = closed_; all_.merge(active_); all_.merge(virt_);

  rclosed_ = make_shared<const IndexRange>(closed_);
  ractive_ = make_shared<const IndexRange>(active_);
  rvirt_   = make_shared<const IndexRange>(virt_);

  if (info_->ciwfn() && info_->grad()) {
    // length of the ci expansion
    const size_t ci_size = info_->ref()->civectors()->data(info_->target())->size();
    ci_ = IndexRange(ci_size, max);
    rci_ = make_shared<const IndexRange>(ci_);
  }

  // f1 tensor.
  {
    MOFock<DataType> fock(info_, {all_, all_});
    f1_ = fock.tensor();
    h1_ = fock.h1();
    core_energy_ = fock.core_energy();
    // canonical orbitals within closed and virtual subspaces
    coeff_ = fock.coeff();
  }

  // v2 tensor.
  {
    IndexRange occ(closed_);  occ.merge(active_);
    IndexRange virt(active_); virt.merge(virt_);

    // in the case of MRCI, we need to include all sectors
    if (to_upper(info_->method()) == "MRCI") {
      occ = all_;
      virt = all_;
    }
    K2ext<DataType> v2k(info_, coeff_, {occ, virt, occ, virt});
    v2_ = v2k.tensor();
  }
  timer.tick_print("MO integral evaluation");

  // rdms.
  if (info_->ciwfn()) {
    auto fockact = make_shared<MatType>(info_->nact(), info_->nact());
    for (auto& i1 : active_)
      for (auto& i0 : active_)
        fockact->copy_block(i0.offset()-info_->nclosed(), i1.offset()-info_->nclosed(), i0.size(), i1.size(), f1_->get_block(i0, i1).get());

    feed_rdm_denom(fockact);
    timer.tick_print("RDM + denominator evaluation");

    // rdm ci derivatives. Only for gradient computations
    if (info_->grad()) {
      feed_rdm_deriv(fockact);
      timer.tick_print("RDM derivative evaluation");
    }
  }

  // set e0
  e0_ = compute_e0();
}


template<typename DataType>
void SpinFreeMethod<DataType>::feed_rdm_denom(shared_ptr<const MatType> fockact) {
  const int nclo = info_->nclosed();
  const int nstates = info_->ciwfn()->nstates();
  rdm0all_ = make_shared<Vec<Tensor_<DataType>>>();
  rdm1all_ = make_shared<Vec<Tensor_<DataType>>>();
  rdm2all_ = make_shared<Vec<Tensor_<DataType>>>();
  rdm3all_ = make_shared<Vec<Tensor_<DataType>>>();
  rdm4all_ = make_shared<Vec<Tensor_<DataType>>>();

  auto denom = make_shared<Denom<DataType>>(fockact, nstates, /*thresh*/1.0e-9);

  // TODO this can be reduced to half by bra-ket symmetry
  for (int ist = 0; ist != nstates; ++ist) {
    for (int jst = 0; jst != nstates; ++jst) {

      auto rdm0t = make_shared<Tensor_<DataType>>(vector<IndexRange>());
      auto rdm1t = make_shared<Tensor_<DataType>>(vector<IndexRange>(2,active_));
      auto rdm2t = make_shared<Tensor_<DataType>>(vector<IndexRange>(4,active_));
      auto rdm3t = make_shared<Tensor_<DataType>>(vector<IndexRange>(6,active_));
      auto rdm4t = make_shared<Tensor_<DataType>>(vector<IndexRange>(8,active_));

      shared_ptr<const RDM<1,DataType>> rdm1;
      shared_ptr<const RDM<2,DataType>> rdm2;
      shared_ptr<const RDM<3,DataType>> rdm3;
      shared_ptr<const RDM<4,DataType>> rdm4;
      tie(rdm1, rdm2) = info_->rdm12(jst, ist);
      tie(rdm3, rdm4) = info_->rdm34(jst, ist);

      unique_ptr<DataType[]> data0(new DataType[1]);
      data0[0] = jst == ist ? 1.0 : 0.0;
      rdm0t->put_block(data0);
      fill_block<2,DataType>(rdm1t, rdm1, vector<int>(2,nclo), vector<IndexRange>(2,active_));
      fill_block<4,DataType>(rdm2t, rdm2, vector<int>(4,nclo), vector<IndexRange>(4,active_));
      fill_block<6,DataType>(rdm3t, rdm3, vector<int>(6,nclo), vector<IndexRange>(6,active_));
      fill_block<8,DataType>(rdm4t, rdm4, vector<int>(8,nclo), vector<IndexRange>(8,active_));

      rdm0all_->emplace(jst, ist, rdm0t);
      rdm1all_->emplace(jst, ist, rdm1t);
      rdm2all_->emplace(jst, ist, rdm2t);
      rdm3all_->emplace(jst, ist, rdm3t);
      rdm4all_->emplace(jst, ist, rdm4t);

      denom->append(jst, ist, rdm1, rdm2, rdm3, rdm4);
    }
  }
  denom->compute();
  denom_ = denom;
}


template<>
void SpinFreeMethod<double>::feed_rdm_deriv(shared_ptr<const MatType> fockact) {
  using DataType = double;
  shared_ptr<Dvec> rdm0d = make_shared<Dvec>(info_->ref()->civectors()->data(info_->target()), 1);
  shared_ptr<Dvec> rdm1d = info_->ref()->rdm1deriv(info_->target());
  shared_ptr<Dvec> rdm2d = info_->ref()->rdm2deriv(info_->target());
  shared_ptr<Dvec> rdm3d, rdm4d;
  // RDM4 is contracted a priori by the Fock operator
  tie(rdm3d, rdm4d) = info_->ref()->rdm34deriv(info_->target(), fockact);
  assert(rdm3d->ij() == rdm4d->ij());

  vector<IndexRange> o1 = {ci_};
  vector<IndexRange> o3 = {ci_, active_, active_};
  vector<IndexRange> o5 = {ci_, active_, active_, active_, active_};
  vector<IndexRange> o7 = {ci_, active_, active_, active_, active_, active_, active_};
  rdm0deriv_ = make_shared<Tensor_<DataType>>(o1);
  rdm1deriv_ = make_shared<Tensor_<DataType>>(o3);
  rdm2deriv_ = make_shared<Tensor_<DataType>>(o5);
  rdm3deriv_ = make_shared<Tensor_<DataType>>(o7);
  rdm4deriv_ = make_shared<Tensor_<DataType>>(o7);

  const int nclo = info_->nclosed();
  vector<int> inpoff1(1,0);
  vector<int> inpoff3(2,nclo); inpoff3.push_back(0);
  vector<int> inpoff5(4,nclo); inpoff5.push_back(0);
  vector<int> inpoff7(6,nclo); inpoff7.push_back(0);

  const int nact = info_->nact();
  const btas::CRange<1> range1(rdm1d->extent(0)*rdm1d->extent(1));
  const btas::CRange<3> range3(rdm1d->extent(0)*rdm1d->extent(1), nact, nact);
  const btas::CRange<5> range5(rdm2d->extent(0)*rdm2d->extent(1), nact, nact, nact, nact);
  const btas::CRange<7> range7(rdm3d->extent(0)*rdm3d->extent(1), nact, nact, nact, nact, nact, nact);

  rdm0d->resize(range1);
  rdm1d->resize(range3);
  rdm2d->resize(range5);
  rdm3d->resize(range7);
  rdm4d->resize(range7);
  fill_block<1,DataType>(rdm0deriv_, rdm0d, inpoff1, o1);
  fill_block<3,DataType>(rdm1deriv_, rdm1d, inpoff3, o3);
  fill_block<5,DataType>(rdm2deriv_, rdm2d, inpoff5, o5);
  fill_block<7,DataType>(rdm3deriv_, rdm3d, inpoff7, o7);
  fill_block<7,DataType>(rdm4deriv_, rdm4d, inpoff7, o7);
}


template<>
void SpinFreeMethod<complex<double>>::feed_rdm_deriv(shared_ptr<const MatType> fockact) {
  throw logic_error("SpinFreeMethod::feed_rdm_deriv is not implemented for relativistic cases.");
}


template<typename DataType>
void SpinFreeMethod<DataType>::set_rdm(const int ist, const int jst) {
  // ist is bra, jst is ket.
  // CAREFUL! the following is due to SMITH's convention (i.e., index are reversed)

  // TODO is this OK for complex cases?
  rdm0_ = rdm0all_->at(jst, ist);
  rdm1_ = rdm1all_->at(jst, ist);
  rdm2_ = rdm2all_->at(jst, ist);
  rdm3_ = rdm3all_->at(jst, ist);
  rdm4_ = rdm4all_->at(jst, ist);
}


template<typename DataType>
void SpinFreeMethod<DataType>::print_iteration() {
  cout << "      ---- iteration ----" << endl << endl;
}


template<typename DataType>
void SpinFreeMethod<DataType>::print_iteration(const int i, const double en, const double err, const double tim, const int ist) {
  cout << "     " << setw(4) << i;
  if (ist >= 0)
    cout << setw(4) << ist;
  cout << setw(15) << fixed << setprecision(8) << en << setw(15) << fixed << setprecision(8) << err
                                                     << setw(10) << fixed << setprecision(2) << tim << endl;
}


template<typename DataType>
void SpinFreeMethod<DataType>::print_iteration(const bool noconv) {
  cout << endl << "      -------------------" << endl;
  if (noconv) cout << "      *** Convergence not reached ***" << endl;
  cout << endl;
}


template<typename DataType>
double SpinFreeMethod<DataType>::compute_e0() {
  assert(!!f1_);
  const size_t nstates = info_->ciwfn()->nstates();
  double sum = 0.0;
  for (int ist = 0; ist != nstates; ++ist) {
    set_rdm(ist, ist);
    assert(!!rdm1_);
    for (auto& i1 : active_) {
      for (auto& i0 : active_) {
        const size_t size = i0.size() * i1.size();
        unique_ptr<DataType[]> fdata = f1_->get_block(i0, i1);
        unique_ptr<DataType[]> rdata = rdm1_->get_block(i0, i1);
        sum += detail::real(blas::dot_product(fdata.get(), size, rdata.get()));
      }
    }
  }
  sum /= nstates;
  cout << "    - Zeroth order energy: " << setw(20) << setprecision(10) << sum << endl;
  return sum;
}


// local function to compress the following
template<typename DataType>
void SpinFreeMethod<DataType>::loop_over(function<void(const Index&, const Index&, const Index&, const Index&)> func) const {
  for (auto& i3 : virt_)
    for (auto& i2 : closed_)
      for (auto& i1 : virt_)
        for (auto& i0 : closed_)
          func(i0, i1, i2, i3);
  for (auto& i2 : active_)
    for (auto& i0 : active_)
      for (auto& i3 : virt_)
        for (auto& i1 : virt_)
          func(i0, i1, i2, i3);
  for (auto& i0 : active_)
    for (auto& i3 : virt_)
      for (auto& i2 : closed_)
        for (auto& i1 : virt_)
          func(i0, i1, i2, i3);
  for (auto& i3 : active_)
    for (auto& i2 : closed_)
      for (auto& i1 : virt_)
        for (auto& i0 : closed_)
          func(i0, i1, i2, i3);
  for (auto& i3 : active_)
    for (auto& i1 : active_)
      for (auto& i2 : closed_)
        for (auto& i0 : closed_)
          func(i0, i1, i2, i3);
  for (auto& i3 : active_)
    for (auto& i2 : active_)
      for (auto& i1 : virt_)
        for (auto& i0 : closed_) {
          func(i0, i1, i2, i3);
          func(i2, i1, i0, i3);
        }
  for (auto& i3 : active_)
    for (auto& i2 : active_)
      for (auto& i0 : active_)
        for (auto& i1 : virt_)
          func(i0, i1, i2, i3);
  for (auto& i3 : active_)
    for (auto& i1 : active_)
      for (auto& i0 : active_)
        for (auto& i2 : closed_)
          func(i0, i1, i2, i3);
}


template<typename DataType>
shared_ptr<Tensor_<DataType>> SpinFreeMethod<DataType>::init_amplitude() const {
  shared_ptr<Tensor_<DataType>> out = v2_->clone();
  auto put = [this, &out](const Index& i0, const Index& i1, const Index& i2, const Index& i3) {
    const size_t size = v2_->get_size_alloc(i0, i1, i2, i3);
    unique_ptr<DataType[]> buf(new DataType[size]);
    fill_n(buf.get(), size, 0.0);
    out->put_block(buf, i0, i1, i2, i3);
  };
  loop_over(put);
  return out;
}


template<typename DataType>
shared_ptr<Tensor_<DataType>> SpinFreeMethod<DataType>::init_residual() const {
  shared_ptr<Tensor_<DataType>> out = v2_->clone();
  auto put = [this, &out](const Index& i0, const Index& i1, const Index& i2, const Index& i3) {
    const size_t size = v2_->get_size_alloc(i2, i3, i0, i1);
    unique_ptr<DataType[]> buf(new DataType[size]);
    fill_n(buf.get(), size, 0.0);
    out->put_block(buf, i2, i3, i0, i1);
  };
  loop_over(put);
  return out;
}


template<typename DataType>
DataType SpinFreeMethod<DataType>::dot_product_transpose(shared_ptr<const MultiTensor_<DataType>> r, shared_ptr<const MultiTensor_<DataType>> t2) const {
  assert(r->nref() == t2->nref());
  DataType out = 0.0;
  for (int i = 0; i != r->nref(); ++i)
    out += r->fac(i) * t2->fac(i)
         + dot_product_transpose(r->at(i), t2->at(i));
  return out;
}


template<typename DataType>
DataType SpinFreeMethod<DataType>::dot_product_transpose(shared_ptr<const Tensor_<DataType>> r, shared_ptr<const Tensor_<DataType>> t2) const {
  DataType out = 0.0;
  auto prod = [this, &r, &t2, &out](const Index& i0, const Index& i1, const Index& i2, const Index& i3) {
    const size_t size = r->get_size_alloc(i2, i3, i0, i1);
    if (size != 0) {
      unique_ptr<DataType[]> tmp0 = t2->get_block(i0, i1, i2, i3);
      unique_ptr<DataType[]> tmp1(new DataType[size]);
      sort_indices<2,3,0,1,0,1,1,1>(tmp0.get(), tmp1.get(), i0.size(), i1.size(), i2.size(), i3.size());

      out += blas::dot_product(tmp1.get(), size, r->get_block(i2, i3, i0, i1).get());
    }
  };
  loop_over(prod);
  return out;
}


template<typename DataType>
void SpinFreeMethod<DataType>::diagonal(shared_ptr<Tensor_<DataType>> r, shared_ptr<const Tensor_<DataType>> t) const {
  assert(to_upper(info_->method()) == "CASPT2");
  if (!is_same<DataType,double>::value)
    throw logic_error("SpinFreeMethod<DataType>::diagonal is only correct for non-rel spin-adapted cases ");

  for (auto& i3 : virt_) {
    for (auto& i2 : closed_) {
      for (auto& i1 : virt_) {
        for (auto& i0 : closed_) {
          // if this block is not included in the current wave function, skip it
          if (!r->get_size_alloc(i0, i1, i2, i3)) continue;
          unique_ptr<DataType[]>       data0 = t->get_block(i0, i1, i2, i3);
          const unique_ptr<DataType[]> data1 = t->get_block(i0, i3, i2, i1);

          sort_indices<0,3,2,1,8,1,-4,1>(data1, data0, i0.size(), i3.size(), i2.size(), i1.size());
          size_t iall = 0;
          for (int j3 = i3.offset(); j3 != i3.offset()+i3.size(); ++j3)
            for (int j2 = i2.offset(); j2 != i2.offset()+i2.size(); ++j2)
              for (int j1 = i1.offset(); j1 != i1.offset()+i1.size(); ++j1)
                for (int j0 = i0.offset(); j0 != i0.offset()+i0.size(); ++j0, ++iall)
                  // note that e0 is cancelled by another term
                  data0[iall] *= -(eig_[j0] + eig_[j2] - eig_[j3] - eig_[j1]);
          r->add_block(data0, i0, i1, i2, i3);
        }
      }
    }
  }
}

#define SPINFREEMETHOD_DETAIL
#include <src/smith/spinfreebase_update.cpp>
#undef SPINFREEMETHOD_DETAIL

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// explict instantiation at the end of the file
template class SpinFreeMethod<double>;
template class SpinFreeMethod<complex<double>>;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
