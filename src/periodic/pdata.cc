//
// BAGEL - Parallel electron correlation program.
// Filename: pdata.cc
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

#include <src/periodic/pdata.h>

using namespace std;
using namespace bagel;

BOOST_CLASS_EXPORT_IMPLEMENT(PData)

PData::PData(const int bsize, const int nblock, const bool serial) : blocksize_(bsize), nblock_(nblock) {

  pdata_.resize(nblock);
  for (int i = 0; i != nblock; ++i) {
    auto block = make_shared<ZMatrix>(bsize, bsize, serial);
    block->zero();
    pdata_[i] = block;
  }
}


PData::PData(const PData& o) : blocksize_(o.blocksize()), nblock_(o.nblock()) {

  pdata_.resize(nblock_);
  for (int i = 0; i != nblock_; ++i)
    pdata_[i] = o.pdata(i);
}


PData::PData(std::vector<std::shared_ptr<const ZMatrix>> v) : blocksize_(v.front()->ndim()), nblock_(v.size()) {

  assert(v.front()->ndim() == v.front()->mdim());
  pdata_.resize(nblock_);
  for (int i = 0; i != nblock_; ++i)
    pdata_[i] = make_shared<ZMatrix>(*v[i]);
}


PData::PData(const PData& o, shared_ptr<const ZMatrix> b) : blocksize_(b->ndim()), nblock_(o.nblock() + 1) {

  assert(b->ndim() == b->mdim());
  assert(o.pdata().front()->ndim() == b->ndim());
  pdata_.resize(nblock_);
  for (int i = 0; i != nblock_ - 1; ++i)
    pdata_[i] = o.pdata(i);

  pdata_[nblock_-1] = make_shared<ZMatrix>(*b);
}


#if 1
PData PData::operator+(const PData& o) const {

  PData out(*this);
  assert(nblock_ == o.nblock() && blocksize_ == o.blocksize());

  for (int i = 0; i != nblock_; ++i)
    *(out[i]) += *o.pdata(i);

  return out;
}


PData& PData::operator+=(const PData& o) {

  assert(nblock_ == o.nblock() && blocksize_ == o.blocksize());

  for (int i = 0; i != nblock_; ++i)
    *(pdata_[i]) += *o.pdata(i);

  return *this;
}
#endif

PData& PData::scale(const double a) {

  for (int i = 0; i != nblock_; ++i)
    pdata_[i]->scale(a);

  return *this;
}


void PData::zero() { for (auto& block : pdata_) block->zero(); }


void PData::allreduce() { for (auto& block : pdata_) block->allreduce(); }


void PData::fill_upper_conjg() { for (auto&block : pdata_) block->fill_upper_conjg(); }


void PData::print(const string tag, const int size) const {
  pdata_.front()->print(tag, size); cout << endl;
  for (auto iblock = pdata_.begin() + 1; iblock != pdata_.end(); ++iblock) {
    (*iblock)->print("", size);
    cout << endl;
  }
}


void PData::print_real_part(const string tag, const int size) const {
  assert(pdata_.front()->get_imag_part()->rms() < 1e-10);
  pdata_.front()->get_real_part()->print(tag, size);
  cout << endl;
  for (auto iblock = pdata_.begin() + 1; iblock != pdata_.end(); ++iblock) {
    assert((*iblock)->get_imag_part()->rms() < 1e-10);
    (*iblock)->get_real_part()->print("", size);
    cout << endl;
  }
}


shared_ptr<const PData> PData::ft(const vector<array<double, 3>> gvector, const vector<array<double, 3>> kvector) const {

  PData out(blocksize_, kvector.size());

  int k = 0;
  for (auto& kvec : kvector) {
    auto kblock = make_shared<ZMatrix>(blocksize_, blocksize_);
    kblock->zero();
    int g = 0;
    for (auto& gvec : gvector) {
      shared_ptr<const ZMatrix> gblock = pdata(g);
      assert (gblock->get_imag_part()->rms() < 1e-10); // gblock should be real
      const double exponent = gvec[0]* kvec[0] + gvec[1] * kvec[1] + gvec[2] * kvec[2];
      const double real = cos(exponent);
      const double imag = sin(exponent);
      auto tmp1 = make_shared<const ZMatrix>(*(gblock->get_real_part()), complex<double>(real, 0.0));
      auto tmp2 = make_shared<const ZMatrix>(*(gblock->get_real_part()), complex<double>(0.0, imag));
      *kblock += *tmp1 + *tmp2;
      ++g;
    }
    out[k] = kblock;
    ++k;
  }

  return make_shared<const PData>(out);
}


shared_ptr<const PData> PData::ift(const vector<array<double, 3>> gvector, const vector<array<double, 3>> kvector) const {

  PData out(blocksize_, gvector.size());

  int g = 0;
  for (auto& gvec : gvector) {
    auto gblock = make_shared<ZMatrix>(blocksize_, blocksize_);
    gblock->zero();
    int k = 0;
    for (auto& kvec : kvector) {
      shared_ptr<const ZMatrix> kblock = pdata(k);
      const double exponent = -gvec[0] * kvec[0] - gvec[1] * kvec[1] - gvec[2] * kvec[2];
      const double real = cos(exponent);
      const double imag = sin(exponent);
      auto tmp1 = make_shared<const ZMatrix>(*(kblock->get_real_part()), complex<double>(real, 0.0));
      auto tmp2 = make_shared<const ZMatrix>(*(kblock->get_real_part()), complex<double>(0.0, imag));
      auto tmp3 = make_shared<const ZMatrix>(*(kblock->get_imag_part()), complex<double>(-imag, 0.0));
      auto tmp4 = make_shared<const ZMatrix>(*(kblock->get_imag_part()), complex<double>(0.0, real));
      *gblock += *tmp1 + *tmp2 + *tmp3 + *tmp4;
      ++k;
    }
    out[g] = gblock;
    out[g]->scale(1.0/kvector.size());
    ++g;
  }

  return make_shared<const PData>(out);
}

shared_ptr<PData> PData::tildex(const double thresh_overlap) const {

  auto out = make_shared<PData>(blocksize_, nblock_);
  for (int i = 0; i != nblock_; ++i)
    (*out)[i] = pdata_[i]->tildex(thresh_overlap);

  return out;
}
