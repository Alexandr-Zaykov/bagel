//
// BAGEL - Parallel electron correlation program.
// Filename: supercimicro.h
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

#ifndef __SRC_ASDSCF_SUPERCIMICRO_H
#define __SRC_ASDSCF_SUPERCIMICRO_H

#include <src/asdscf/asdscf.h>

namespace bagel {

class ASDSuperCIMicro {
  protected:
    std::shared_ptr<const ASDSCF> casscf_;

    // input
    std::shared_ptr<const ASDRotFile> grad_;
    std::shared_ptr<const ASDRotFile> denom_;

    std::shared_ptr<const Matrix> fock_;
    std::shared_ptr<const Matrix> fockact_;
    std::shared_ptr<const Matrix> fockactp_;
    std::shared_ptr<const Matrix> gaa_;

    // results will be set to here
    std::shared_ptr<const ASDRotFile> cc_;

    std::shared_ptr<ASDRotFile> form_sigma(std::shared_ptr<const ASDRotFile> cc) const;

  private:
    void sigma_at_at_(std::shared_ptr<const ASDRotFile>, std::shared_ptr<ASDRotFile>) const;
    void sigma_ai_ai_(std::shared_ptr<const ASDRotFile>, std::shared_ptr<ASDRotFile>) const;
    void sigma_at_ai_(std::shared_ptr<const ASDRotFile>, std::shared_ptr<ASDRotFile>) const;
    void sigma_ai_ti_(std::shared_ptr<const ASDRotFile>, std::shared_ptr<ASDRotFile>) const;
    void sigma_ti_ti_(std::shared_ptr<const ASDRotFile>, std::shared_ptr<ASDRotFile>) const;

  public:
    ASDSuperCIMicro(std::shared_ptr<const ASDSCF> cas, std::shared_ptr<const ASDRotFile> g, std::shared_ptr<const ASDRotFile> d,
                 std::shared_ptr<const Matrix> f1, std::shared_ptr<const Matrix> f2, std::shared_ptr<const Matrix> f3, std::shared_ptr<const Matrix> ga)
      : casscf_(cas), grad_(g), denom_(d), fock_(f1), fockact_(f2), fockactp_(f3), gaa_(ga) { }

    void compute();

    std::shared_ptr<const ASDRotFile> cc() const { assert(cc_); return cc_; }
};

}

#endif
