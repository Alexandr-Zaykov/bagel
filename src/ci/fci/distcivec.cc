//
// BAGEL - Brilliantly Advanced General Electronic Structure Library
// Filename: distcivec.cc
// Copyright (C) 2013 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the BAGEL package.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <bagel_config.h>
#ifdef HAVE_GA
#include <ga.h>
#include <src/ci/fci/civec.h>
#include <src/util/math/algo.h>
#include <src/util/parallel/distqueue.h>
#include <src/util/parallel/mpi_interface.h>

using namespace std;
using namespace bagel;


template<typename DataType>
void GA_Task<DataType>::wait() {
  NGA_NbWait(&tag);
}


template<typename DataType>
DistCivector<DataType>::DistCivector(shared_ptr<const Determinants> det) : det_(det), lena_(det->lena()), lenb_(det->lenb()), dist_(lena_, mpi__->size()) {
  // create GA
  auto type = is_same<double,DataType>::value ? MT_C_DBL : MT_C_DCPL;
  auto atable = dist_.atable();
  int64_t tsize = 0;
  for (auto& i : atable) {
    blocks_.push_back(i.first);
    tsize += i.first;
  }
  blocks_.push_back(tsize);

  int64_t nblocks = blocks_.size() - 1;
  assert(nblocks <= mpi__->size());
  ga_ = NGA_Create_irreg64(type, 1, &blocks_.back(), const_cast<char*>(""), &nblocks, blocks_.data());

#ifndef NDEBUG
  int64_t offset = astart_*lenb_;
  int64_t last   = astart_*lenb_ + size() - 1;
  assert(NGA_Nodeid() == NGA_Locate64(ga_, &offset));
  assert(NGA_Nodeid() == NGA_Locate64(ga_, &last));
#endif
}


template<typename DataType>
DistCivector<DataType>& DistCivector<DataType>::operator=(const DistCivector<DataType>& o) {
  GA_Copy(o.ga_, ga_);
}


template<typename DataType>
unique_ptr<DataType[]> DistCivector<DataType>::local() const {
  unique_ptr<DataType[]> out(new DataType[size()]);
  int64_t start = astart_*lenb_;
  int64_t last  = start+size()-1;
  int64_t ld  = size();
  assert(is_local(start));
  NGA_Get64(ga_, &start, &last, out.get(), &ld);
  return move(out);
}


template<typename DataType>
bool DistCivector<DataType>::is_local(const size_t a) const {
  const size_t lo = a*lenb_;
  int64_t nodeid = NGA_Nodeid();
  return (nodeid+1 < blocks_.size()) && (lo >= blocks_[nodeid] && lo < blocks_[nodeid+1]);
}


template<typename DataType>
void DistCivector<DataType>::set_local(const size_t la, const size_t lb, const DataType a) {
  int64_t element = astart_*lenb_ + lb + la*lenb_;
  int64_t one = 1;
  NGA_Put64(ga_, &element, &element, const_cast<DataType*>(&a), &one);
}


template<typename DataType>
std::shared_ptr<GA_Task<DataType>> DistCivector<DataType>::accumulate_bstring_buf(std::unique_ptr<DataType[]>&& buf, const size_t a) {
  int64_t offset = a*lenb_;
  int64_t last = offset + lenb_ - 1;
  int64_t size = lenb_;
  DataType one = 1.0;
  ga_nbhdl_t handle;
  // non-blocking accumulate call
  NGA_NbAcc64(ga_, &offset, &last, buf.get(), &size, &one, &handle);
  return make_shared<GA_Task<DataType>>(handle, std::move(buf));
}


template<typename DataType>
std::shared_ptr<GA_Task<DataType>> DistCivector<DataType>::get_bstring_buf(DataType* buf, const size_t a) const {
  int64_t offset = a*lenb_;
  int64_t last = offset + lenb_ - 1;
  int64_t size = lenb_;
  ga_nbhdl_t handle;
  // non-blocking get acall
  NGA_NbGet64(ga_, &offset, &last, buf, &size, &handle);
  return make_shared<GA_Task<DataType>>(handle);
}


template<typename DataType>
void DistCivector<DataType>::local_accumulate(const DataType a, const unique_ptr<DataType[]>& buf) {
  // asusming that the size is identical to local tile
  int64_t start = astart_ * lenb_;
  int64_t last = start + size() - 1;
  int64_t ld = size();
  NGA_Acc64(ga_, &start, &last, const_cast<DataType*>(buf.get()), &ld, const_cast<DataType*>(&a));
}


template<typename DataType>
void DistCivector<DataType>::zero() {
  GA_Zero(ga_);
}


template<>
double DistCivector<double>::dot_product(const DistCivector<double>& o) const {
  return GA_Ddot(ga_, o.ga_);
}


template<>
complex<double> DistCivector<complex<double>>::dot_product(const DistCivector<complex<double>>& o) const {
  assert(size() == o.size());
  // GA_Zdot is zdotu, so we have to compute this manually
  int64_t start = astart_ * lenb_;
  int64_t ld  = size();
  int64_t last  = start + ld - 1;
  unique_ptr<complex<double>[]> a(new complex<double>[size()]);
  unique_ptr<complex<double>[]> b(new complex<double>[size()]);
  NGA_Get64(  ga_, &start, &last, a.get(), &ld);
  NGA_Get64(o.ga_, &start, &last, b.get(), &ld);
  complex<double> sum = blas::dot_product(a.get(), ld, b.get());
  mpi__->allreduce(&sum, 1);
  return sum;
}


template<typename DataType>
void DistCivector<DataType>::scale(const DataType a) {
  GA_Scale(ga_, const_cast<DataType*>(&a));
}


template<typename DataType>
void DistCivector<DataType>::ax_plus_y(const DataType a, const DistCivector<DataType>& o) {
  assert(size() == o.size());
  DataType one = 1.0;
  GA_Add(const_cast<DataType*>(&a), o.ga_, &one, ga_, ga_);
}


template<typename DataType>
shared_ptr<DistCivector<DataType>> DistCivector<DataType>::transpose() const {
  auto out = make_shared<DistCivector<DataType>>(det_->transpose());
  GA_Transpose(ga_, out->ga_);
  return out;
}


#if 0
namespace bagel { namespace DFCI {
  class SpinTask {
    protected:
      const bitset<nbit__> abit_;
      double* target_;
      const DistCivector<double>* cc_;

      unique_ptr<double[]> buf_;
      vector<int> requests_;

    public:
      SpinTask(const bitset<nbit__> a, double* t, const DistCivector<double>* c) : abit_(a), target_(t), cc_(c) {
        shared_ptr<const Determinants> det = c->det();
        const size_t lb = det->lenb();
        const int norb = det->norb();

        buf_ = unique_ptr<double[]>(new double[lb * abit_.count() * (norb - abit_.count() + 1)]);

        for (int i = 0, k = 0; i < norb; ++i) {
          for (int j = 0; j < norb; ++j) {
            if ( abit_[j] ) {
              bitset<nbit__> tarbit = abit_; tarbit.reset(j);
              if ( !tarbit[i] ) {
                tarbit.set(i);
                const int l = cc_->get_bstring_buf(buf_.get() + lb*k++, det->lexical<0>(tarbit));
                if (l >= 0) requests_.push_back(l);
              }
            }
          }
        }
      }

      bool test() {
        bool out = true;
        for (auto i = requests_.begin(); i != requests_.end(); ) {
          if (mpi__->test(*i)) {
            i = requests_.erase(i);
          }
          else {
            ++i;
            out = false;
          }
        }
        return out;
      }

      void compute() {
        shared_ptr<const Determinants> det = cc_->det();

        const size_t lb = det->lenb();
        const int norb = det->norb();

        for (int i = 0, k = 0; i < norb; ++i) {
          for (int j = 0; j < norb; ++j) {
            if ( abit_[j] ) {
              bitset<nbit__> tarbit = abit_; tarbit.reset(j);
              if ( !tarbit[i] ) {
                const double* source = buf_.get() + lb*k++;
                const int aphase = det->sign(abit_, i, j);
                for (auto& iter : det->phib(j, i))
                  target_[iter.target] -= static_cast<double>(aphase * iter.sign) * source[iter.source];
              }
            }
          }
        }
      }
  };
} }

template <>
shared_ptr<DistCivector<double>> DistCivector<double>::spin() const {
  const size_t lb = det_->lenb();
  auto out = this->clone();

#ifndef USE_SERVER_THREAD
  DistQueue<DFCI::SpinTask, const DistCivector<double>*> dq(this);
#else
  DistQueue<DFCI::SpinTask> dq;
#endif

  for (size_t ia = astart_; ia < aend_; ++ia)
    dq.emplace_and_compute(det_->string_bits_a(ia), out->local() + (ia - astart_) * lb, this);

  const double sz = 0.5*static_cast<double>(det_->nspin());
  out->ax_plus_y(sz*sz + sz + static_cast<double>(det_->neleb()), *this);

  dq.finish();

  this->terminate_mpi_recv();
  return out;
}

template<>
void DistCivector<double>::spin_decontaminate(const double thresh) {
  const int nspin = det_->nspin();
  const int max_spin = det_->nelea() + det_->neleb();

  const double pure_expectation = static_cast<double>(nspin * (nspin + 2)) * 0.25;

  shared_ptr<DistCivector<double>> S2 = spin();
  double actual_expectation = dot_product(*S2);

  int k = nspin + 2;
  while( fabs(actual_expectation - pure_expectation) > thresh ) {
    if ( k > max_spin ) { this->print(0.05); throw runtime_error("Spin decontamination failed."); }

    const double factor = -4.0/(static_cast<double>(k*(k+2)));
    ax_plus_y(factor, *S2);

    const double norm = this->norm();
    const double rescale = (norm*norm > 1.0e-60) ? 1.0/norm : 0.0;
    scale(rescale);

    S2 = spin();
    actual_expectation = dot_product(*S2);

    k += 2;
  }
}

namespace bagel { namespace DFCI {
  class LowerTask {
    protected:
      const bitset<nbit__> abit_;
      double* target_;
      const DistCivector<double>* cc_;
      shared_ptr<const Determinants> tdet_;

      unique_ptr<double[]> buf_;
      vector<int> requests_;

    public:
      LowerTask(const bitset<nbit__> a, double* t, const DistCivector<double>* c, shared_ptr<const Determinants> td) : abit_(a), target_(t), cc_(c), tdet_(td) {
        shared_ptr<const Determinants> det = c->det();
        const size_t lbs = det->lenb();
        const int norb = det->norb();

        buf_ = unique_ptr<double[]>(new double[lbs * (norb - abit_.count())]);

        for (int i = 0, k = 0; i < norb; ++i) {
          if (!abit_[i]) {
            bitset<nbit__> sourcebit = abit_; sourcebit.set(i);
            const int l = cc_->get_bstring_buf(buf_.get() + lbs*k++, det->lexical<0>(sourcebit));
            if (l >= 0) requests_.push_back(l);
          }
        }
      }

      bool test() {
        bool out = true;
        for (auto i = requests_.begin(); i != requests_.end(); ) {
          if (mpi__->test(*i)) {
            i = requests_.erase(i);
          }
          else {
            ++i;
            out = false;
          }
        }
        return out;
      }

      void compute() {
        shared_ptr<const Determinants> sdet = cc_->det();

        const size_t lbs = sdet->lenb();
        const int norb = sdet->norb();

        for (int i = 0, k = 0; i < norb; ++i) {
          if (abit_[i]) continue;
          bitset<nbit__> sabit = abit_; sabit.set(i);
          const int aphase = sdet->sign<0>(sabit, i);
          const double* sourcedata = buf_.get() + lbs * k++;
          double* targetdata = target_;
          for (auto& bbit : tdet_->string_bits_b()) {
            if ( bbit[i] ) {
              bitset<nbit__> sbbit = bbit; sbbit.reset(i);
              const int bphase = -sdet->sign<1>(sbbit, i);
              *targetdata += static_cast<double>(aphase * bphase) * sourcedata[sdet->lexical<1>(sbbit)];
            }
            ++targetdata;
          }
        }
      }
  };
} }

template<>
shared_ptr<DistCivector<double>> DistCivector<double>::spin_lower(shared_ptr<const Determinants> tdet) const {
  if (!tdet) tdet = make_shared<Determinants>(det_->norb(), det_->nelea()-1, det_->neleb()+1, det_->compress(), true);
  assert( (tdet->nelea() == det_->nelea()-1) && (tdet->neleb() == det_->neleb()+1) );

  auto out = make_shared<DistCivector<double>>(tdet);

  const size_t tastart = out->astart();
  const size_t taend = out->aend();
  const size_t lbt = tdet->lenb();

#ifndef USE_SERVER_THREAD
  DistQueue<DFCI::LowerTask, const DistCivector<double>*> dq(this);
#else
  DistQueue<DFCI::LowerTask> dq;
#endif

  for (size_t ia = tastart; ia < taend; ++ia)
    dq.emplace_and_compute(tdet->string_bits_a(ia), out->local() + (ia - tastart) * lbt, this, tdet);

  dq.finish();

  this->terminate_mpi_recv();
  return out;
}

namespace bagel { namespace DFCI {
  class RaiseTask {
    protected:
      const bitset<nbit__> abit_;
      double* target_;
      const DistCivector<double>* cc_;
      shared_ptr<const Determinants> tdet_;

      unique_ptr<double[]> buf_;
      vector<int> requests_;

    public:
      RaiseTask(const bitset<nbit__> a, double* t, const DistCivector<double>* c, shared_ptr<const Determinants> td) : abit_(a), target_(t), cc_(c), tdet_(td) {
        shared_ptr<const Determinants> det = c->det();
        const size_t lbs = det->lenb();
        const int norb = det->norb();

        buf_ = unique_ptr<double[]>(new double[lbs * abit_.count()]);

        for (int i = 0, k = 0; i < norb; ++i) {
          if ( abit_[i] ) {
            bitset<nbit__> sourcebit = abit_; sourcebit.reset(i);
            const int l = cc_->get_bstring_buf(buf_.get() + lbs*k++, det->lexical<0>(sourcebit));
            if (l >= 0) requests_.push_back(l);
          }
        }
      }

      bool test() {
        bool out = true;
        for (auto i = requests_.begin(); i != requests_.end(); ) {
          if (mpi__->test(*i)) {
            i = requests_.erase(i);
          }
          else {
            ++i;
            out = false;
          }
        }
        return out;
      }

      void compute() {
        shared_ptr<const Determinants> sdet = cc_->det();

        const size_t lbs = sdet->lenb();
        const int norb = sdet->norb();

        for (int i = 0, k = 0; i < norb; ++i) {
          if ( !abit_[i] ) continue;
          bitset<nbit__> sabit = abit_; sabit.reset(i);
          const int aphase = sdet->sign<0>(sabit, i);
          const double* sourcedata = buf_.get() + lbs * k++;
          double* targetdata = target_;
          for (auto& bbit : tdet_->string_bits_b()) {
            if ( !bbit[i] ) {
              bitset<nbit__> sbbit = bbit; sbbit.set(i);
              const int bphase = sdet->sign<1>(sbbit, i);
              *targetdata += static_cast<double>(aphase * bphase) * sourcedata[sdet->lexical<1>(sbbit)];
            }
            ++targetdata;
          }
        }
      }
  };
} }

template<>
shared_ptr<DistCivector<double>> DistCivector<double>::spin_raise(shared_ptr<const Determinants> tdet) const {
  if (!tdet) tdet = make_shared<Determinants>(det_->norb(), det_->nelea()+1, det_->neleb()-1, det_->compress(), true);
  assert( (tdet->nelea() == det_->nelea()+1) && (tdet->neleb() == det_->neleb()-1) );

  auto out = make_shared<DistCivector<double>>(tdet);

  const size_t tastart = out->astart();
  const size_t taend = out->aend();
  const size_t lbt = out->lenb();

#ifndef USE_SERVER_THREAD
  DistQueue<DFCI::RaiseTask, const DistCivector<double>*> dq(this);
#else
  DistQueue<DFCI::RaiseTask> dq;
#endif

  for (size_t ia = tastart; ia < taend; ++ia)
    dq.emplace_and_compute(tdet->string_bits_a(ia), out->local() + (ia - tastart) * lbt, this, tdet);

  dq.finish();

  this->terminate_mpi_recv();
  return out;
}

template<>
shared_ptr<DistCivector<double>> DistCivector<double>::apply(const int orbital, const bool action, const bool spin) const {
  // action: true -> create; false -> annihilate
  // spin:   true -> alpha;  false -> beta
  shared_ptr<const Determinants> sdet = this->det();

  const size_t lbs = sdet->lenb();

  shared_ptr<DistCivector<double>> out;

  if (spin) {
    shared_ptr<const Determinants> tdet = ( action ? sdet->addalpha() : sdet->remalpha() );
    out = make_shared<DistCivector<double>>(tdet);
    assert( lbs == tdet->lenb() );


    const size_t tastart = out->astart();
    const size_t taend = out->aend();
    const size_t lbt = tdet->lenb();

    list<tuple<int, double*, int>> requests;

    for (auto& iter : ( action ? sdet->phiupa(orbital) : sdet->phidowna(orbital) )) {
      const size_t targ = iter.target;
      if ( tastart <= targ && taend > targ ) {
        double* dest = out->local() + (targ - tastart) * lbt;
        const int req = get_bstring_buf( dest, iter.source );
        if (req >= 0) {
          requests.emplace_back(req, dest, iter.sign);
          //this->flush();
        }
        else {
          const int sign = iter.sign;
          for_each(dest, dest + lbt, [&sign] (double& t) { t *= sign; });
        }
      }
    }

#ifndef USE_SERVER_THREAD
    this->flush();
#endif

    bool done;
    do {
      done = true;
      for (auto i = requests.begin(); i != requests.end(); ) {
        if (mpi__->test(get<0>(*i))) {
          double* dest = get<1>(*i);
          const int sign = get<2>(*i);
          for_each(dest, dest + lbt, [&sign] (double& t) { t *= sign; });
          i = requests.erase(i);
        }
        else {
          done = false;
          ++i;
        }
      }
#ifndef USE_SERVER_THREAD
      size_t d = done ? 0 : 1;
      mpi__->soft_allreduce(&d, 1);
      done = (d == 0);
      if (!done) this->flush();
#endif
      if (!done) this_thread::sleep_for(sleeptime__);
    } while ( !done );

    this->terminate_mpi_recv();
  }
  else { // This case requires no communication
    shared_ptr<const Determinants> tdet = ( action ? sdet->addbeta() : sdet->rembeta() );
    out = make_shared<DistCivector<double>>(tdet);
    assert( sdet->lena() == tdet->lena() );
    const size_t lbt = tdet->lenb();

    for (size_t ia = astart_; ia < aend_; ++ia) {
      const double* source_base = this->local() + (ia - astart_) * lbs;
      double* target_base = out->local() + (ia - astart_) * lbt;
      for (auto& iter : ( action ? sdet->phiupb(orbital) : sdet->phidownb(orbital) )) {
        const double sign = static_cast<double>(iter.sign);
        target_base[iter.target] += sign * source_base[iter.source];
      }
    }
  }

  return out;
}

template<>
shared_ptr<DistDvec> DistDvec::apply(const int orbital, const bool action, const bool spin) const {
  // action: true -> create; false -> annihilate
  // spin:   true -> alpha;  false -> beta
#if 1
  vector<CiPtr> out;
  for (auto& i : dvec_) out.push_back(i->apply(orbital, action, spin));
  return make_shared<DistDvec>(out);
#else
  shared_ptr<const Determinants> sdet = this->det();
  const int norb = sdet->norb();
  const int nstate = ij();

  const size_t las = sdet->lena();
  const size_t lbs = sdet->lenb();

  shared_ptr<DistDvec> out;

  if (spin) {
    shared_ptr<const Determinants> tdet = ( action ? sdet->addalpha() : sdet->remalpha() );
    out = make_shared<DistDvec>(tdet, ij());
    assert( lbs == tdet->lenb() );

    const size_t tastart = out->dvec().front()->astart();
    const size_t taend = out->dvec().front()->aend();
    const size_t lbt = tdet->lenb();

    list<tuple<int, double*, int>> requests;

    for (int ist = 0; ist < nstate; ++ist) {
      shared_ptr<const DistCivec> cc = data(ist);
      cc->init_mpi_recv();
      for (auto& iter : ( action ? sdet->phiupa(orbital) : sdet->phidowna(orbital) )) {
        const size_t targ = iter.target;
        if ( tastart <= targ && taend > targ ) {
          double* dest = out->data(ist)->local() + (targ - tastart) * lbt;
          const int req = cc->get_bstring_buf( dest, iter.source );
          if (req >= 0) {
            requests.emplace_back(req, dest, iter.sign);
          }
          else {
            const int sign = iter.sign;
            for_each(dest, dest + lbt, [&sign] (double& t) { t *= sign; });
          }
        }
      }
      for (int jst = 0; jst <= ist; ++jst) data(jst)->flush();
    }

    bool done;
    do {
      done = true;
      for (auto i = requests.begin(); i != requests.end(); ) {
        if (mpi__->test(get<0>(*i))) {
          double* dest = get<1>(*i);
          const int sign = get<2>(*i);
          for_each(dest, dest + lbt, [&sign] (double& t) { t *= sign; });
          i = requests.erase(i);
        }
        else {
          done = false;
          ++i;
        }
      }
#ifndef USE_SERVER_THREAD
      size_t d = done ? 0 : 1;
      mpi__->soft_allreduce(&d, 1);
      done = (d == 0);
      if (!done) { for (int ist = 0; ist < nstate; ++ist) data(ist)->flush(); }
#endif
      if (!done) this_thread::sleep_for(sleeptime__);
    } while ( !done );

    for (int ist = 0; ist < nstate; ++ist) data(ist)->terminate_mpi_recv();
  }
  else { // This case requires no communication
    shared_ptr<const Determinants> tdet = ( action ? sdet->addbeta() : sdet->rembeta() );
    assert( sdet->lena() == tdet->lena() );
    out = make_shared<DistDvec>(tdet, ij());
    const size_t lbt = tdet->lenb();

    const size_t astart = out->data(0)->astart();
    const size_t aend = out->data(0)->aend();

    for (int ist = 0; ist < nstate; ++ist) {
      for (size_t ia = astart; ia < aend; ++ia) {
        const double* source_base = this->data(ist)->local() + (ia - astart) * lbs;
        double* target_base = out->data(ist)->local() + (ia - astart) * lbt;
        for (auto& iter : ( action ? sdet->phiupb(orbital) : sdet->phidownb(orbital) )) {
          const double sign = static_cast<double>(iter.sign);
          target_base[iter.target] += sign * source_base[iter.source];
        }
      }
    }
  }

  return out;
#endif
}
#endif

template class bagel::GA_Task<double>;
template class bagel::GA_Task<std::complex<double>>;
template class bagel::DistCivector<double>;
template class bagel::DistCivector<complex<double>>;

#endif // end of HAVE_GA
