//
// BAGEL - Parallel electron correlation program.
// Filename: cphf.h
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


#ifndef __SRC_GRAD_CPHF_H
#define __SRC_GRAD_CPHF_H

#include <src/util/math/linearRM.h>
#include <src/wfn/reference.h>

namespace bagel {

class CPHF {
  protected:
    std::shared_ptr<LinearRM<Matrix>> solver_;
    std::shared_ptr<const Matrix> grad_;
    VectorB eig_;
    std::shared_ptr<const DFHalfDist> halfjj_;
    std::shared_ptr<const Reference> ref_;
    std::shared_ptr<const Geometry> geom_;

  public:
    CPHF(const std::shared_ptr<const Matrix> grad, const VectorB& eig,
         const std::shared_ptr<const DFHalfDist> half, const std::shared_ptr<const Reference> g);

    std::shared_ptr<Matrix> solve(const double thresh, const int maxiter = 100);

};

}

#endif

