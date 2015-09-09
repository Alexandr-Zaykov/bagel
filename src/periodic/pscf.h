//
// BAGEL - Parallel electron correlation program.
// Filename: pscf.h
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

#ifndef __BAGEL_SRC_PERIODIC_PSCF_H
#define __BAGEL_SRC_PERIODIC_PSCF_H

#include <src/periodic/pscf_base.h>
#include <src/periodic/lattice.h>
#include <src/periodic/phcore.h>
#include <src/periodic/pfock.h>

namespace bagel {

class PSCF : public PSCF_base {
  protected:
    bool dodf_;
    bool dofmm_;

  private:
    // serialization
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive& ar, const unsigned int) {
      ar << boost::serialization::base_object<PSCF_base>(*this);
      ar << dodf_;
    }

  public:
    PSCF() { }
    PSCF(const std::shared_ptr<const PTree> idata_, const std::shared_ptr<const Geometry> geom,
         const std::shared_ptr<const Reference> re = nullptr);
    virtual ~PSCF() { }

    void compute() override;
    bool dodf() const { return dodf_; }

    std::shared_ptr<const Reference> conv_to_ref() const override { return nullptr; }
};

}

#include <src/util/archive.h>
BOOST_CLASS_EXPORT_KEY(bagel::PSCF)

#endif

