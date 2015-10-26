//
// BAGEL - Parallel electron correlation program.
// Filename: angmom_london.h
// Copyright (C) 2015 Toru Shiozaki
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


#ifndef __SRC_MAT1E_GIAO_ANGMOM_LONDON_H
#define __SRC_MAT1E_GIAO_ANGMOM_LONDON_H

#include <src/wfn/geometry.h>

namespace bagel {

class AngMom_London {
  protected:
    std::shared_ptr<const Geometry> geom_;
    std::array<double,3> mcoord_;

  public:
    AngMom_London(std::shared_ptr<const Geometry> geom, std::array<double,3> mcoord);

    std::array<std::shared_ptr<ZMatrix>,3> compute() const;
};

}

#endif
