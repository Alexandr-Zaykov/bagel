//
// BAGEL - Parallel electron correlation program.
// Filename: test_main.cc
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


#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Suites

#include <stddef.h>
#include <array>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <boost/test/unit_test.hpp>
#include <src/util/parallel/resources.h>
#include <src/util/parallel/mpi_interface.h>
#include <src/util/input/input.h>

using namespace bagel;

Resources b(8);
Resources* bagel::resources__ = &b;
static MPI_Interface c;
MPI_Interface* bagel::mpi__ = &c;

static double THRESH = 1.0e-8;

bool compare(const double a, const double b, const double thr = THRESH) { return fabs(a-b) < thr; };

template<class T>
bool compare(const T a, const T b, const double thr = THRESH) {
 if (a.size() != b.size()) throw std::logic_error("comparing vectors with different sizes");
 bool out = true;
 for (auto i = a.begin(), j = b.begin(); i != a.end(); ++i, ++j) out &= compare(*i, *j, thr);
 return out;
}

#include <src/scf/test_scf.cc>

#include <src/ks/test_ks.cc>

#include <src/rel/test_rel.cc>

#include <src/prop/test_prop.cc>

#include <src/pt2/mp2/test_mp2.cc>

#include <src/casscf/test_casscf.cc>

#include <src/ci/fci/test_fci.cc>

//#include <src/ci/zfci/test_zfci.cc>

//#include <src/zcasscf/test_zcasscf.cc>

#include <src/ci/ras/test_ras.cc>

#include <src/pt2/nevpt2/test_nevpt2.cc>

#include <src/opt/test_opt.cc>

#include <src/util/io/test_molden.cc>

#include <src/molecule/test_localize.cc>

#include <src/asd/test_asd.cc>

#include <src/asd/dmrg/test_asd_dmrg.cc>

#include <src/london/test_london.cc>
