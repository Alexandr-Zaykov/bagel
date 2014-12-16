//
// BAGEL - Parallel electron correlation program.
// Filename: breitint.h
// Copyright (C) 2012 Toru Shiozaki
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


#ifndef __SRC_REL_BREITINT_H
#define __SRC_REL_BREITINT_H

#include <src/molecule/molecule.h>
#include <src/mat1e/matrix1earray.h>

namespace bagel {

class BreitInt : public Matrix1eArray<6> {
  protected:
    std::vector<std::pair<const int, const int>> index_;

    void init(std::shared_ptr<const Molecule>) override;
    void computebatch(const std::array<std::shared_ptr<const Shell>,2>&, const int, const int, std::shared_ptr<const Molecule>) override;

  public:
    BreitInt(const std::shared_ptr<const Molecule>);

    void print(const std::string name) const override { Matrix1eArray<6>::print(name.empty() ? "Breit" : name); }

    std::pair<const int, const int> index(const int i) const { return index_[i]; }
    std::vector<std::pair<const int, const int>> index() const { return index_; }

    /// returns if block k is not diagonal
    bool not_diagonal(const int k) const { assert(k >= 0); assert(k <= index_.size()); return index_[k].first != index_[k].second; }

};

}

#endif

