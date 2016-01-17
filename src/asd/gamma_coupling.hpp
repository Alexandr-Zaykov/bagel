//
// BAGEL - Brilliantly Advanced General Electronic Structure Library
// Filename: gamma_coupling.hpp
// Copyright (C) 2013 Toru Shiozaki
//
// Author: Shane Parker <shane.parker@u.northwestern.edu>
// Maintainer: Shiozaki Group
//
// This file is part of the BAGEL package.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifdef ASD_HEADERS

#ifndef __SRC_ASD_GAMMA_COUPLING_HPP
#define __SRC_ASD_GAMMA_COUPLING_HPP

template <class VecType, int N>
void GammaForest<VecType,N>::couple_blocks(const DimerSubspace<VecType>& AB, const DimerSubspace<VecType>& ApBp) {
  static_assert(N == 2, "the following assumes N == 2");
  Coupling term_type = coupling_type(AB, ApBp);

  auto* space1 = &AB;   // bra
  auto* space2 = &ApBp; // ket

  bool flip = (static_cast<int>(term_type) < 0);

  if (flip) {
    term_type = Coupling(-1*static_cast<int>(term_type));
    std::swap(space1,space2);
  }

  // Throughout, I'm going to consider space1 to be the bra state, space2 to be the ket state
  if (term_type != Coupling::none) {
    const std::array<int,2> bra_tags {{space1->template tag<0>(), space1->template tag<1>()}};
    const std::array<int,2> ket_tags {{space2->template tag<0>(), space2->template tag<1>()}};

    std::shared_ptr<const VecType> bra_A = space1->template ci<0>();
    std::shared_ptr<const VecType> bra_B = space1->template ci<1>();

    std::shared_ptr<const VecType> ket_A = space2->template ci<0>();
    std::shared_ptr<const VecType> ket_B = space2->template ci<1>();

    switch(term_type) {
      case Coupling::none :
        assert(false); // Control should never be able to reach here
        break;
      case Coupling::diagonal :
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha});
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta});
        break;
      case Coupling::aET : // Alpha ET
        // One-body aET
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateAlpha});

        //Two-body aET, type 1
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});

        //Two-body aET, type 2
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha});
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateAlpha});
        break;
      case Coupling::bET : // Beta ET
        // One-body bET
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateBeta});
        //Two-body bET, type 1
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateAlpha});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta});

        //Two-body aET, type 2
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha});
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateBeta});
        break;
      case Coupling::abFlip :
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::AnnihilateAlpha});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateBeta});
        break;
      case Coupling::abET :
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});
        break;
      case Coupling::aaET :
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha});
        break;
      case Coupling::bbET :
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta});
        break;
      default :
        assert(false); break; // Control should never reach here
    }
  }
}

#endif

#endif
