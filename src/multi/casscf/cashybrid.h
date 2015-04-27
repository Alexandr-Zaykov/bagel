//
// BAGEL - Parallel electron correlation program.
// Filename: casbfgs.h
// Copyright (C) 2014 Toru Shiozaki
//
// Author: Jefferson Bates <jefferson.bates@northwestern.edu>
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


#ifndef __BAGEL_CASSCF_CASHybrid_H
#define __BAGEL_CASSCF_CASHybrid_H

#include <iomanip>
#include <src/ci/fci/fci.h>

namespace bagel {

class CASHybrid : public Method {

  protected:
    int maxiter_switch_;
    double thresh_switch_;
    double thresh_;

    std::vector<double> energy_;

    std::shared_ptr<const Reference> refout_;
    std::shared_ptr<FCI> fci_;

    void common_init() {
      std::cout << "    * Using a hybrid approach to CASSCF *    " << std::endl;

      maxiter_switch_   = idata_->get<int>("maxiter_switch", -1);
      int maxiter_bfgs   = idata_->get<int>("maxiter", 50);
      int maxiter_sci = maxiter_switch_ > 0 ? maxiter_switch_ : maxiter_bfgs;
      thresh_switch_ = idata_->get<double>("thresh_switch", -1.0);

      if (thresh_switch_ > 0.0)
        std::cout << std::setprecision(2) << std::scientific
                  << "    * Quasi-Newton methods will be used if the rms gradient norm becomes smaller than " << thresh_switch_ << " *    " << std::endl;
      std::cout << "    * SuperCI will be used for a maximum of " << maxiter_sci << " iterations *    " << std::endl;
      std::cout << "    * Step-restricted BFGS will then be used for a maximum of " << maxiter_bfgs << " iterations *    " << std::endl << std::endl;
    }


  public:
    CASHybrid(std::shared_ptr<const PTree> idat, std::shared_ptr<const Geometry> geom, std::shared_ptr<const Reference> ref = nullptr)
      : Method(idat, geom, ref) { common_init(); }

    void compute() override;

    std::shared_ptr<const Reference> conv_to_ref() const;

    double energy(const int i) const { return energy_[i]; }
    const std::vector<double>& energy() const { return energy_; }

    std::shared_ptr<FCI> fci() { return fci_; }

    double thresh() const { return thresh_; }
};

}

#endif
