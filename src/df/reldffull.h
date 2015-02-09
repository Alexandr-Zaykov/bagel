//
// BAGEL - Parallel electron correlation program.
// Filename: reldffull.h
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

#ifndef __SRC_DF_RELDFFULL_H
#define __SRC_DF_RELDFFULL_H

#include <src/df/reldf.h>
#include <src/df/reldfhalf.h>

namespace bagel {

class RelDF;
class RelDFHalf;

class RelDFFull : public RelDFBase {
  protected:
    std::array<std::shared_ptr<DFFullDist>,2> dffull_;

  public:
    RelDFFull(std::shared_ptr<const RelDFHalf>, std::array<std::shared_ptr<const Matrix>,4>, std::array<std::shared_ptr<const Matrix>,4>);
    RelDFFull(std::array<std::shared_ptr<DFFullDist>,2> a, std::pair<int,int> cartesian, std::vector<std::shared_ptr<const SpinorInfo>> basis);
    RelDFFull(const RelDFFull& o);

    std::array<std::shared_ptr<DFFullDist>, 2> get_data() const { return dffull_; }
    std::shared_ptr<DFFullDist> get_real() const { return dffull_[0]; }
    std::shared_ptr<DFFullDist> get_imag() const { return dffull_[1]; }

    bool matches(std::shared_ptr<const RelDFFull>) const { return true; }

    std::shared_ptr<RelDFFull> copy() const { return std::make_shared<RelDFFull>(*this); }
    std::shared_ptr<RelDFFull> clone() const;
    std::shared_ptr<RelDFFull> apply_J() const;
    std::shared_ptr<RelDFFull> apply_JJ() const;

    // zaxpy
    void ax_plus_y(std::complex<double> a, std::shared_ptr<const RelDFFull> o) { ax_plus_y(a, *o); }
    void ax_plus_y(std::complex<double> a, const RelDFFull& o);
    void scale(std::complex<double> a);

    RelDFFull& operator+=(const RelDFFull& o) { ax_plus_y(1.0, o); return *this; }
    RelDFFull& operator-=(const RelDFFull& o) { ax_plus_y(-1.0, o); return *this; }

    std::complex<double> fac() const { assert(basis_.size() == 1); return basis_[0]->fac(cartesian_); }

    std::shared_ptr<Matrix> form_aux_2index_real() const {
      std::shared_ptr<Matrix> out = dffull_[0]->form_aux_2index(dffull_[0], 1.0);
      *out += *dffull_[1]->form_aux_2index(dffull_[1], 1.0); // positive, due to complex conjugate
      return out;
    }

    std::list<std::shared_ptr<RelDFHalfB>> back_transform(std::array<std::shared_ptr<const Matrix>,4>,
                                                          std::array<std::shared_ptr<const Matrix>,4>) const;
    std::shared_ptr<ZMatrix> form_4index(std::shared_ptr<const RelDFFull>, const double fac) const;
    std::shared_ptr<ZMatrix> form_2index(std::shared_ptr<const RelDFFull>, const double fac, const bool conjugate_left = true) const;
    std::shared_ptr<ZMatrix> form_4index_1fixed(std::shared_ptr<const RelDFFull>, const double fac, const int i) const;

    std::shared_ptr<RelDFFull> apply_2rdm(std::shared_ptr<const ZRDM<2>>) const;
};

}

#endif
