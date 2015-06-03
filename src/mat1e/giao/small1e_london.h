//
// BAGEL - Parallel electron correlation program.
// Filename: small1e_london.h
// Copyright (C) 2014 Toru Shiozaki
//
// Author: Ryan D. Reynolds <RyanDReynolds@u.northwestern.edu>
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


#ifndef __SRC_LONDON_SMALL1E_LONDON_H
#define __SRC_LONDON_SMALL1E_LONDON_H

#include <src/util/math/zmatrix.h>
#include <src/integral/smallints1e_london.h>
#include <src/integral/comprys/complexnaibatch.h>
#include <src/mat1e/matrix1earray.h>

namespace bagel {

template <typename Batch>
class Small1e_London : public Matrix1eArray<4*Batch::Nblocks(), ZMatrix> {
  protected:
    void init(std::shared_ptr<const Molecule> mol) override {
      std::list<std::shared_ptr<const Shell>> shells;
      for (auto& i : mol->atoms())
        shells.insert(shells.end(), i->shells().begin(), i->shells().end());

      const size_t nshell = accumulate(mol->atoms().begin(), mol->atoms().end(), 0, [](int r, std::shared_ptr<const Atom> p) { return r+p->nshell(); });
      TaskQueue<Matrix1eArrayTask<4*Batch::Nblocks(), ZMatrix>> task(nshell*(nshell+1)/2);

      int o0 = 0;
      int u = 0;
      for (auto& a0 : shells) {
        int o1 = 0;
        for (auto& a1 : shells) {
          if (u++ % mpi__->size() == mpi__->rank()) {
            std::array<std::shared_ptr<const Shell>,2> input = {{a1, a0}};
            task.emplace_back(input, o0, o1, mol, this);
          }
          o1 += a1->nbasis();
        }
        o0 += a0->nbasis();
      }
      task.compute();
      for (auto& i : this->matrices_) i->allreduce();
    }

    void computebatch(const std::array<std::shared_ptr<const Shell>,2>& input, const int offsetb0, const int offsetb1, std::shared_ptr<const Molecule> mol) override {
      // input = [b1, b0]
      assert(input.size() == 2);
      const int dimb1 = input[0]->nbasis();
      const int dimb0 = input[1]->nbasis();
      SmallInts1e_London<Batch, std::shared_ptr<const Molecule>> batch(input, mol);
      batch.compute();

      for (int i = 0; i != this->Nblocks(); ++i)
        this->matrices_[i]->copy_block(offsetb1, offsetb0, dimb1, dimb0, batch[i]);
    }

  public:
    Small1e_London(const std::shared_ptr<const Molecule> mol) : Matrix1eArray<4*Batch::Nblocks(), ZMatrix>(mol) {
      init(mol);
    }

    void print(const std::string name = "") const override {
      Matrix1eArray<4*Batch::Nblocks(), ZMatrix>::print(name.empty() ? "Small1e" : name);
    }
};

template<> void Small1e_London<ComplexNAIBatch>::computebatch(const std::array<std::shared_ptr<const Shell>,2>& input, const int offsetb0, const int offsetb1, std::shared_ptr<const Molecule>);
template<> void Small1e_London<ComplexERIBatch>::computebatch(const std::array<std::shared_ptr<const Shell>,2>& input, const int offsetb0, const int offsetb1, std::shared_ptr<const Molecule>);

}

#endif
