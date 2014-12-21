//
// BAGEL - Parallel electron correlation program.
// Filename: smith.h
// Copyright (C) 2013 Matthew MacLeod
//
// Author: Matthew K. MacLeod <matthew.macleod@northwestern.edu>
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


#ifndef __SRC_SMITH_SMITH_H
#define __SRC_SMITH_SMITH_H

#include <src/smith/spinfreebase.h>
#include <stddef.h>
#include <map>
#include <memory>
#include <src/wfn/method.h>
#include <src/wfn/reference.h>

namespace bagel {

class Smith : public Method {
  protected:
    std::shared_ptr<SMITH::SpinFreeMethod<SMITH::Storage_Incore>> algo_;

    // correlated density matrices
    // second order density matrix
    std::shared_ptr<const Matrix> dm1_;
    // first order density matrices
    std::shared_ptr<const Matrix> dm11_;
    std::shared_ptr<const Matrix> dm2_;
    // correction <1|1>
    double wf1norm_;
    // ci derivative
    std::shared_ptr<const Civec> cider_;

    std::shared_ptr<const Coeff> coeff_;



  public:
    Smith(std::shared_ptr<const PTree>, std::shared_ptr<const Geometry>, std::shared_ptr<const Reference>);

    void compute() override;

    std::shared_ptr<const Reference> conv_to_ref() const override { return std::shared_ptr<const Reference>(); }

    std::shared_ptr<const Matrix> dm1() const { return dm1_; }
    std::shared_ptr<const Matrix> dm11() const { return dm11_; }
    std::shared_ptr<const Matrix> dm2() const { return dm2_; }
    double wf1norm() const { return wf1norm_; }
    std::shared_ptr<const Civec> cideriv() const { return cider_; }
    std::shared_ptr<const Coeff> coeff() const { return coeff_; }

    std::shared_ptr<const SMITH::SpinFreeMethod<SMITH::Storage_Incore>> algo() const { return algo_; }

};

}
#endif
