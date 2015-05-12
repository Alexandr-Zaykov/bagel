//
// BAGEL - Parallel electron correlation program.
// Filename: test_zcasscf.cc
// Copyright (C) 2014 Jefferson E. Bates
//
// Author: Jefferson E. Bates  <jefferson.bates@northwestern.edu>
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


#include <src/multi/zcasscf/zcasscf.h>
#include <src/multi/zcasscf/zsuperci.h>
#include <src/multi/zcasscf/zcasbfgs.h>

double relcas_energy(std::string inp) {

  auto ofs = std::make_shared<std::ofstream>(inp + ".testout", std::ios::trunc);
  std::streambuf* backup_stream = std::cout.rdbuf(ofs->rdbuf());

  // a bit ugly to hardwire an input file, but anyway...
  std::string filename = location__ + inp + ".json";
  auto idata = std::make_shared<const PTree>(filename);
  auto keys = idata->get_child("bagel");
  std::shared_ptr<Geometry> geom;
  std::shared_ptr<const Reference> ref;
  double energy = 0.0;

  for (auto& itree : *keys) {
    const std::string method = to_lower(itree->get<std::string>("title", ""));

#ifndef DISABLE_SERIALIZATION
    if (itree->get<bool>("load_ref", false)) {
      const std::string name = itree->get<std::string>("ref_in", "");
      assert (name != "");
      IArchive archive(name);
      std::shared_ptr<Reference> ptr;
      archive >> ptr;
      ref = std::shared_ptr<Reference>(ptr);
    }
#endif

    if (method == "molecule") {
      geom = geom ? std::make_shared<Geometry>(*geom, itree) : std::make_shared<Geometry>(itree);
      if (itree->get<bool>("restart", false))
        ref.reset();
      if (ref) ref = ref->project_coeff(geom);

    } else if (method == "zcasscf") {
      std::string algorithm = itree->get<std::string>("algorithm", "");
      if (algorithm == "superci" || algorithm == "") {
        auto zcas = std::make_shared<ZSuperCI>(itree, geom, ref);
        zcas->compute();
        ref = zcas->conv_to_ref();
        energy = ref->energy();
      } else if (algorithm == "bfgs") {
        auto zcas = std::make_shared<ZCASBFGS>(itree, geom, ref);
        zcas->compute();
        ref = zcas->conv_to_ref();
        energy = ref->energy();
      }
    }
#ifndef DISABLE_SERIALIZATION
    if (itree->get<bool>("save_ref", false)) {
      const std::string name = itree->get<std::string>("ref_out", "");
      assert (name != "");
      OArchive archive(name);
      archive << ref;
    }
#endif
  }
  std::cout.rdbuf(backup_stream);
  return energy;
}

BOOST_AUTO_TEST_SUITE(TEST_RELCAS)

BOOST_AUTO_TEST_CASE(ZCASSCF) {
  BOOST_CHECK(compare(relcas_energy("h2_qzvpp_superci_coulomb"), -1.01931815));
  BOOST_CHECK(compare(relcas_energy("hf_tzvpp_superci_coulomb"), -100.03016820));
  BOOST_CHECK(compare(relcas_energy("he_tzvpp_bfgs_coulomb"),    -2.875647885));
  BOOST_CHECK(compare(relcas_energy("nh_tzvpp_triplet_gaunt"),   -55.00281016));
  BOOST_CHECK(compare(relcas_energy("o2_svp_triplet_breit"),     -149.56647468));
#ifndef DISABLE_SERIALIZATION
//BOOST_CHECK(compare(relcas_energy("hf_tzvpp_bfgs_saveref"), -100.03016820));
#endif
}

BOOST_AUTO_TEST_SUITE_END()
