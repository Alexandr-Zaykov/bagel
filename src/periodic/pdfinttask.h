//
// BAGEL - Parallel electron correlation program.
// Filename: pdfinttask.h
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

#ifndef __SRC_PERIODIC_PDFINTTASK_H
#define __SRC_PERIODIC_PDFINTTASK_H

#include <src/df/dfblock.h>
#include <src/molecule/shell.h>
#include <src/integral/rys/eribatch.h>
#include <src/integral/os/overlapbatch.h>

namespace bagel {

class PDFIntTask_aux {
  protected:
    std::array<std::shared_ptr<const Shell>,2> shell_;
    int offset_;
    std::shared_ptr<VectorB> pdf_;

  public:
    // <i|.>
    PDFIntTask_aux(std::array<std::shared_ptr<const Shell>,2>&& sh, int offset, std::shared_ptr<VectorB> pdf)
     : shell_(sh), offset_(offset), pdf_(pdf) { }

    void compute() {

      auto overlap = std::make_shared<OverlapBatch>(shell_);
      const double* odata = overlap->data();
      double* const data = pdf_->data();

      for (int j0 = offset_; j0 != offset_ + shell_[1]->nbasis(); ++j0, ++odata)
        data[j0] = *odata;
    }
};


class PDFIntTask_coeff {
  protected:
    std::array<std::shared_ptr<const Shell>,2> shell_;
    std::array<int,2> offset_;
    std::shared_ptr<DFBlock> coeffC_;
    std::shared_ptr<const VectorB> data1_;

  public:
    // <r|sL>
    PDFIntTask_coeff(std::array<std::shared_ptr<const Shell>,2>&& sh, std::array<int,2>&& offset,
                     std::shared_ptr<DFBlock>& coeffC, const std::shared_ptr<const VectorB>& data1)
     : shell_(sh), offset_(offset), coeffC_(coeffC), data1_(data1) { }

    void compute() {

      auto overlap = std::make_shared<OverlapBatch>(shell_);
      const double* odata = overlap->data();

      const size_t naux = coeffC_->asize();
      const size_t nbas = coeffC_->b1size();
      const double q = data1_->rms() * naux;

      double* const coeff = coeffC_->data();
      for (int j0 = offset_[0]; j0 != offset_[0] + shell_[1]->nbasis(); ++j0)
        for (int j1 = offset_[1]; j1 != offset_[1] + shell_[0]->nbasis(); ++j1, ++odata)
          for (int a = 0; a != naux; ++a) {
            coeff[a + naux * (j1 + nbas * j0)] = *odata * (*data1_)(a) / q;
          }
    }
};


class PDFIntTask_2index {
  protected:
    std::array<std::shared_ptr<const Shell>,4> shell_;
    std::array<int,2> offset_;
    std::shared_ptr<Matrix> data2_;

    std::shared_ptr<ERIBatch> compute_batch(const std::array<std::shared_ptr<const Shell>,4>& input) const {
      auto eribatch = std::make_shared<ERIBatch>(input, 2.0);
      eribatch->compute();
      return eribatch;
    }

  public:
    // (i.|jL.) sum over L
    PDFIntTask_2index(std::array<std::shared_ptr<const Shell>,4>&& sh, std::array<int,2>&& offset, std::shared_ptr<Matrix>& data2)
     : shell_(sh), offset_(offset), data2_(data2) { }

    void compute() {

      std::shared_ptr<ERIBatch> eribatch = compute_batch(shell_);
      const double* eridata = eribatch->data();

      const size_t naux = data2_->ndim();

      assert(offset_.size() == 2);
      double* const data = data2_->data();
      for (int j0 = offset_[0]; j0 != offset_[0] + shell_[2]->nbasis(); ++j0)
        for (int j1 = offset_[1]; j1 != offset_[1] + shell_[0]->nbasis(); ++j1, ++eridata)
          data[j0+j1*naux] = *eridata;
    }
};


class PDFIntTask_3index {
  protected:
    const std::array<std::shared_ptr<const Shell>,4> shell_;
    const std::array<int,3> offset_;
    std::shared_ptr<DFBlock> dfblock_;

    std::shared_ptr<ERIBatch> compute_batch(const std::array<std::shared_ptr<const Shell>,4>& input) const {
      auto eribatch = std::make_shared<ERIBatch>(input, 2.0);
      eribatch->compute();
      return eribatch;
    }

  public:
    // (r sL'|iL .) sum over L
    PDFIntTask_3index(std::array<std::shared_ptr<const Shell>,4>&& shells, std::array<int,3>&& offset, std::shared_ptr<DFBlock>& df)
     : shell_(shells), offset_(offset), dfblock_(df) { };

    void compute() {

      std::shared_ptr<ERIBatch> eribatch = compute_batch(shell_);

      assert(dfblock_->b1size() == dfblock_->b2size());
      assert(offset_.size() == 3);
      const size_t nbin = dfblock_->b1size();
      const size_t naux = dfblock_->asize();
      const double* eridata = eribatch->data();

      double* const data = dfblock_->data();
      for (int j0 = offset_[0]; j0 != offset_[0] + shell_[3]->nbasis(); ++j0)
        for (int j1 = offset_[1]; j1 != offset_[1] + shell_[2]->nbasis(); ++j1)
          for (int a = offset_[2]; a != offset_[2] + shell_[1]->nbasis(); ++a, ++eridata)
            data[a + naux * (j1 + nbin * j0)] = *eridata;
    }
};


}

#endif
