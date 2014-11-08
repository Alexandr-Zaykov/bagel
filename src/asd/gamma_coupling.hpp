//
// BAGEL - Parallel electron correlation program.
// Filename: gamma_coupling.hpp
// Copyright (C) 2013 Toru Shiozaki
//
// Author: Shane Parker <shane.parker@u.northwestern.edu>
// Maintainer: Shiozaki Group
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

#ifdef ASD_HEADERS

#ifndef __SRC_ASD_GAMMA_COUPLING_HPP
#define __SRC_ASD_GAMMA_COUPLING_HPP

template <class VecType, int N>
void GammaForest<VecType,N>::couple_blocks(const DimerSubspace<VecType>& AB, const DimerSubspace<VecType>& ApBp) {
  static_assert(N == 2, "the following assumes N == 2");
  Coupling term_type = coupling_type_RDM34(AB, ApBp);

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
        std::cout << "DIAGONAL" << std::endl;
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha}); //a'a
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta});   //b'b
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha}); //a'a
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta});   //b'b
//ADDED
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'a'aa
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta});  //b'b'bb
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha}); //a'b'ba

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'a'aa
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta});  //b'b'bb
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha}); //a'b'ba
//4RDM
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'a'a'aaa
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha}); //a'b'a'aba
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});  //b'a'b'bab
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta});  //b'b'b'bbb

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'a'a'aaa
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha}); //a'b'a'aba
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});  //b'a'b'bab
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta});  //b'b'b'bbb
//END ADDED
        break;

      case Coupling::aET : // Alpha ET
        std::cout << "Alpha-ET" << std::endl;
        // One-body aET
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateAlpha});

        //Two-body aET, type 1
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'aa
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta}); //b'ab

        //Two-body aET, type 2
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha}); //a'a'a
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta}); //a'b'b
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateAlpha});
//ADDED
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'a'a'aa
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta}); //a'b'a'ab
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta}); //a'b'b'bb

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'a'aaa
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'b'baa
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateAlpha}); //b'b'bba
//4RDM
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'a'a'a'aaa
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta}); //a'b'a'a'aab
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta}); //a'b'b'a'abb
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta}); //a'b'b'b'bbb

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha});//a'a'a'aaaa
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha});//a'a'b'baaa
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha});//a'b'b'bbaa
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha});//b'b'b'bbba
//END ADDED
        break;

      case Coupling::bET : // Beta ET
        std::cout << "Beta-ET" << std::endl;
        // One-body bET
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateBeta});
        //Two-body bET, type 1
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateAlpha});//a'ba
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta});//b'bb

        //Two-body aET, type 2
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha});//b'a'a
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta});//b'b'b
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateBeta});
//ADDED
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //b'a'a'aa
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta}); //b'b'a'ab
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta}); //b'b'b'bb

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta}); //a'a'aab
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta}); //a'b'bab
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta}); //b'b'bbb
//4RDM
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //b'a'a'a'aaa
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});  //b'b'a'a'aab
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta});  //b'b'b'a'abb
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta});  //b'b'b'b'bbb

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});//a'a'a'aaab
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});//a'a'b'baab
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});//a'b'b'bbab
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta});//b'b'b'bbbb
//END ADDED
        break;

      case Coupling::abFlip :
        std::cout << "AB-Flip" << std::endl;
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::AnnihilateAlpha}); //b'a
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateBeta}); //a'b
//ADDED
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); // a'b'aa
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta}); //b'b'ab

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta}); //a'a'ab
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta}); //b'a'bb

        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); // b'a'a'aaa
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});  // b'b'a'aab
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta});  // b'b'b'abb

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});//a'a'a'aab
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});//a'b'a'bab
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta});//b'b'a'bbb
//END ADDED
        break;

      case Coupling::abET :
        std::cout << "AB-ET" << std::endl;
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta}); //a'b'
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta}); //ab
//ADDED
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha}); //b'a'a'a
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateAlpha, GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta}); //b'a'b'b

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateAlpha}); //a'aba
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateAlpha}); //b'bba
//4RDM
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'b'a'a'aa
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});  //a'b'b'a'ab
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta});  //a'b'b'b'bb

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateAlpha});//a'a'aaba
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateAlpha});//a'b'baba
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateAlpha});//b'b'bbba
//END ADDED
        break;

      case Coupling::aaET :
        std::cout << "AA-ET" << std::endl;
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha}); //a'a'
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //aa
//ADDED
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha}); //a'a'a'a
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta});   //a'a'b'b

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'aaa
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //b'baa
//4RDM
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'a'a'a'aa
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta}); //a'a'b'a'ab
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta}); //a'a'b'b'bb

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'a'aaaa
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'b'baaa
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //b'b'bbaa
//END ADDED
        break;

      case Coupling::bbET :
        std::cout << "BB-ET" << std::endl;
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta});
//ADDED
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta}); //b'b'b'b
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha});//b'b'a'a

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta}); //a'abb
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta}); //b'bbb
//4RDM
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //b'b'a'a'aa
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta}); //b'b'b'a'ab
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta}); //b'b'b'b'bb

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta}); //a'a'aabb
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta}); //a'b'babb
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta}); //b'b'bbbb
//END ADDED
        break;

//ADDED
      case Coupling::aaaET :
        std::cout << "AAA-ET" << std::endl;
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha}); // a'a'a'
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); // aaa
        break;

      case Coupling::bbbET :
        std::cout << "BBB-ET" << std::endl;
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::CreateBeta}); // b'b'b'
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta}); // bbb
        break;

      case Coupling::aabET :
        std::cout << "AAB-ET" << std::endl;
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});
        break;

      case Coupling::abbET :
        std::cout << "ABB-ET" << std::endl;
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta, GammaSQ::CreateBeta});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta});
        break;

      case Coupling::bETflp :
        std::cout << "B-ET + Flip" << std::endl;
//3RDM
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::AnnihilateAlpha});
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta});
//4RDM
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //b'b'a'aa
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateBeta, GammaSQ::CreateBeta, GammaSQ::CreateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta});  //b'b'b'ab

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta}); //a'a'abb
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::CreateAlpha, GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta}); //b'a'bbb
//END ADDED
        break;

      case Coupling::aETflp :
        std::cout << "A-ET + Flip" << std::endl;
//3RDM
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateBeta}); //a'a'b
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //b'aa
//4RDM
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateAlpha}); //a'a'a'ba
        this->insert<0>(bra_A, bra_tags[0], ket_A, ket_tags[0], {GammaSQ::CreateAlpha, GammaSQ::CreateAlpha, GammaSQ::CreateBeta,  GammaSQ::AnnihilateBeta, GammaSQ::AnnihilateBeta});  //a'a'b'bb

        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateAlpha, GammaSQ::CreateBeta, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //a'b'aaa
        this->insert<1>(bra_B, bra_tags[1], ket_B, ket_tags[1], {GammaSQ::CreateBeta,  GammaSQ::CreateBeta, GammaSQ::AnnihilateBeta,  GammaSQ::AnnihilateAlpha, GammaSQ::AnnihilateAlpha}); //b'b'baa
//END ADDED
        break;

      default :
        assert(false); break; // Control should never reach here
    }
  }
}

#endif

#endif
