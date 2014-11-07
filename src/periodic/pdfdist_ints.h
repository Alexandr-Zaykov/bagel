//
// BAGEL - Parallel electron correlation program.
// Filename: pdfdist_ints.h
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

#ifndef __SRC_PERIODIC_PDFDIST_INTS_H
#define __SRC_PERIODIC_PDFDIST_INTS_H

#include <src/df/df.h>
#include <src/wfn/geometry.h>

namespace bagel {

class PDFDist_ints : public DFDist {
  protected:
    /// lattice vectors in direct space
    std::vector<std::array<double, 3>> lattice_vectors_;

    int nbasis_;
    std::shared_ptr<const Matrix> projector_;

    /// auxiliary charge ints <a|.>
    std::shared_ptr<const VectorB> data1_;

    /// 3-index integrals (r sL'|iL) for each L' (sum over all lattice vectors L)
    std::vector<std::shared_ptr<DFBlock>> data3_in_cell_;
    void pcompute_3index(const std::vector<std::shared_ptr<const Shell>>& ashell,   /*aux   */
                         const std::vector<std::shared_ptr<const Shell>>& b0shell,  /*cell 0*/
                         const std::vector<std::shared_ptr<const Shell>>& bgshell); /*cell g*/

    /// charged part of coeff
    std::shared_ptr<btas::Tensor3<double>> coeffC_;
    void compute_charged_coeff(const std::vector<std::shared_ptr<const Shell>>& b0shell,
                               const std::vector<std::shared_ptr<const Shell>>& bgshell);

    /// NAI integrals
    void pcompute_NAI(const std::vector<std::shared_ptr<const Shell>>& b0shell,
                      const std::vector<std::shared_ptr<const Shell>>& bgshell,
                      const std::shared_ptr<const Geometry> cell0);
    std::vector<std::shared_ptr<Matrix>> nai_in_cell_;

  public:
    PDFDist_ints(const std::vector<std::array<double, 3>>& lattice_vectors, const int nbas, const int naux,
                 const std::vector<std::shared_ptr<const Atom>>& atoms_c0,
                 const std::vector<std::shared_ptr<const Atom>>& atoms_cg,
                 const std::vector<std::shared_ptr<const Atom>>& aux_atoms, const std::shared_ptr<const Geometry> cell0,
                 const double thr, const std::shared_ptr<const Matrix> projector = nullptr,
                 const std::shared_ptr<const VectorB> data1 = nullptr);

    std::vector<std::array<double, 3>> lattice_vectors() const { return lattice_vectors_; }
    int ncell() const { return lattice_vectors_.size(); }

    std::shared_ptr<const VectorB> data1() const { return data1_; }

    std::vector<std::shared_ptr<DFBlock>> data3_in_cell() const { return data3_in_cell_; }
    std::shared_ptr<DFBlock> data3_in_cell(const int i) const { return data3_in_cell_[i]; }

    std::shared_ptr<btas::Tensor3<double>> coeffC() const { return coeffC_; }

    std::vector<std::shared_ptr<Matrix>> nai_in_cell() const { return nai_in_cell_; }
    std::shared_ptr<Matrix> nai_in_cell(const int i) const { return nai_in_cell_[i]; }
};

}

#endif
