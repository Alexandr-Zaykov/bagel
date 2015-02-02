//
// BAGEL - Parallel electron correlation program.
// Filename: asd_ras.h
// Copyright (C) 2013 Shane Parker
//
// Author: Shane Parker <shane.parker@u.northwestern.edu>
// Maintainer: NU theory
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

#ifndef __ASD_ASD_RAS_H
#define __ASD_ASD_RAS_H

#include <src/asd/asd.h>

namespace bagel {

class ASD_RAS : public ASD<RASDvec> {
  public:
    ASD_RAS(const std::shared_ptr<const PTree> input, std::shared_ptr<Dimer> dimer, std::shared_ptr<DimerRAS> cispace);

  private:
    std::tuple<std::shared_ptr<RDM<1>>, std::shared_ptr<RDM<2>>> compute_rdm12_monomer(std::pair<int,int>, std::array<RASDvec,4>&) const override;
    std::tuple<std::shared_ptr<RDM<3>>, std::shared_ptr<RDM<4>>, std::shared_ptr<RDM<4>>> compute_rdm34_monomer(std::pair<int,int>, std::array<RASDvec,4>&) const override;
    std::shared_ptr<RASDvec> form_sigma(std::shared_ptr<const RASDvec> ccvec, std::shared_ptr<const MOFile> jop) const override;
    std::shared_ptr<RASDvec> form_sigma_1e(std::shared_ptr<const RASDvec> ccvec, const double* modata) const override;

    void sigma_aa(std::shared_ptr<const RASCivec> cc, std::shared_ptr<RASCivec> sigma, const double* h1, const double* h2) const;
    void sigma_bb(std::shared_ptr<const RASCivec> cc, std::shared_ptr<RASCivec> sigma, const double* h1, const double* h2) const;
    void sigma_ab(std::shared_ptr<const RASCivec> cc, std::shared_ptr<RASCivec> sigma, const double* h1, const double* h2) const;

    std::tuple<std::shared_ptr<RDM<1>>, std::shared_ptr<RDM<2>>> compute_rdm12_from_civec(std::shared_ptr<const RASCivec>, std::shared_ptr<const RASCivec>) const;
    std::tuple<std::shared_ptr<RDM<1>>, std::shared_ptr<RDM<2>>> compute_rdm12_last_step(std::shared_ptr<const RASDvec>, std::shared_ptr<const RASDvec>, std::shared_ptr<const RASCivec>) const;
    void sigma_2a1(std::shared_ptr<const RASCivec> cc, std::shared_ptr<RASDvec> d) const;
    void sigma_2a2(std::shared_ptr<const RASCivec> cc, std::shared_ptr<RASDvec> d) const;

};

}

#endif
