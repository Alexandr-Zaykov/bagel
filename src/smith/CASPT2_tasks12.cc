//
// BAGEL - Parallel electron correlation program.
// Filename: CASPT2_tasks12.cc
// Copyright (C) 2014 Shiozaki group
//
// Author: Shiozaki group <shiozaki@northwestern.edu>
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

#include <bagel_config.h>
#ifdef COMPILE_SMITH

#include <src/smith/CASPT2_tasks12.h>

using namespace std;
using namespace bagel;
using namespace bagel::SMITH;
using namespace bagel::SMITH::CASPT2;

void Task550::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  ta1_->init();
  madness::World::get_default().gop.fence();
  (*ta0_)("a3, c2, a1, x0") += (*ta1_)("x1, x0") * (*ta2_)("x1, a3, c2, a1");
  madness::World::get_default().gop.fence();
}

void Task551::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x1, a3, c2, a1") += (*ta1_)("x1, a3, c2, a1") * (-1)
     + (*ta1_)("x1, a1, c2, a3") * 2;
  madness::World::get_default().gop.fence();
}

void Task552::compute_() {
  madness::World::get_default().gop.fence();
  (*ta0_)("x1, a2, x0, a1") += (*ta1_)("a1, a2, x0, x1");
  madness::World::get_default().gop.fence();
}

void Task553::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  ta1_->init();
  madness::World::get_default().gop.fence();
  (*ta0_)("a1, a2, x0, x1") += (*ta1_)("x3, x0, x2, x1") * (*ta2_)("x3, a1, x2, a2") * 2;
  madness::World::get_default().gop.fence();
}

void Task555::compute_() {
  madness::World::get_default().gop.fence();
  (*ta0_)("ci0") += (*ta1_)("ci0");
  madness::World::get_default().gop.fence();
}

void Task556::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  ta1_->init();
  madness::World::get_default().gop.fence();
  (*ta0_)("ci0") += (*ta1_)("ci0, x0, x5, x1, x4") * (*ta2_)("x1, x0, x5, x4");
  madness::World::get_default().gop.fence();
}

void Task557::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x1, x0, x5, x4") += (*ta1_)("c1, x5, c2, x4") * (*ta1_)("c1, x0, c2, x1") * 4;
  madness::World::get_default().gop.fence();
}

void Task558::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  ta1_->init();
  madness::World::get_default().gop.fence();
  (*ta0_)("ci0") += (*ta1_)("ci0, x0, x3, x1, x2") * (*ta2_)("x1, x0, x3, x2");
  madness::World::get_default().gop.fence();
}

void Task559::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x1, x0, x3, x2") += (*ta1_)("c1, x3, c3, x2") * (*ta2_)("x1, x0, c1, c3");
  madness::World::get_default().gop.fence();
}

void Task560::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x1, x0, c1, c3") += (*ta1_)("c2, c3") * (*ta2_)("c1, x0, c2, x1") * (-8);
  madness::World::get_default().gop.fence();
}

void Task561::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x1, x0, x3, x2") += (*ta1_)("c1, x3, c2, x2") * (*ta1_)("c1, x0, c2, x1") * e0_ * (-4);
  madness::World::get_default().gop.fence();
}

void Task562::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x1, x0, x3, x2") += (*ta1_)("c1, x3, c2, x2") * (*ta2_)("c1, x0, c2, x1") * 2;
  madness::World::get_default().gop.fence();
}

void Task563::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x1, x0, x3, x2") += (*ta1_)("c1, x0, c2, x1") * (*ta2_)("c1, x3, c2, x2") * 2;
  madness::World::get_default().gop.fence();
}

void Task564::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  ta1_->init();
  madness::World::get_default().gop.fence();
  (*ta0_)("ci0") += (*ta1_)("ci0, x5, x4, x0, x3, x1, x2") * (*ta2_)("x1, x0, x2, x5, x4, x3");
  madness::World::get_default().gop.fence();
}

void Task565::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x1, x0, x2, x5, x4, x3") += (*ta1_)("x5, x4, c1, x3") * (*ta2_)("x1, x0, c1, x2");
  madness::World::get_default().gop.fence();
}

void Task566::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x1, x0, c1, x2") += (*ta1_)("c2, x2") * (*ta2_)("c1, x0, c2, x1") * 4;
  madness::World::get_default().gop.fence();
}

void Task567::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  ta1_->init();
  madness::World::get_default().gop.fence();
  (*ta0_)("ci0") += (*ta1_)("ci0, x1, x3, x0, x2") * (*ta2_)("x2, x3, x1, x0");
  madness::World::get_default().gop.fence();
}

void Task568::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x2, x3, x1, x0") += (*ta1_)("c1, x0, c2, x1") * (*ta2_)("x2, c1, c2, x3");
  madness::World::get_default().gop.fence();
}

void Task569::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x2, c1, c2, x3") += (*ta1_)("c1, a3, c2, x3") * (*ta2_)("a3, x2") * (-4);
  madness::World::get_default().gop.fence();
}

void Task570::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x2, x3, x1, x0") += (*ta1_)("c1, x3, c3, x2") * (*ta2_)("x0, c3, c1, x1");
  madness::World::get_default().gop.fence();
}

void Task571::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x0, c3, c1, x1") += (*ta1_)("x1, a2") * (*ta2_)("c1, a2, c3, x0") * (-4);
  madness::World::get_default().gop.fence();
}

void Task572::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  ta1_->init();
  madness::World::get_default().gop.fence();
  (*ta0_)("ci0") += (*ta1_)("ci0, x2, x5, x3, x4, x1, x0") * (*ta2_)("x3, x5, x4, x2, x1, x0");
  madness::World::get_default().gop.fence();
}

void Task573::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x3, x5, x4, x2, x1, x0") += (*ta1_)("x0, x1, c1, x2") * (*ta2_)("x3, c1, x5, x4");
  madness::World::get_default().gop.fence();
}

void Task574::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x3, c1, x5, x4") += (*ta1_)("c1, x5, c2, x4") * (*ta2_)("x3, c2") * 4;
  madness::World::get_default().gop.fence();
}

void Task575::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  ta1_->init();
  madness::World::get_default().gop.fence();
  (*ta0_)("ci0") += (*ta1_)("ci0, x7, x6, x2, x5, x1, x0") * (*ta2_)("x2, x1, x0, x7, x6, x5");
  madness::World::get_default().gop.fence();
}

void Task576::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x2, x1, x0, x7, x6, x5") += (*ta1_)("x7, x6, c1, x5") * (*ta1_)("x0, x1, c1, x2") * 2;
  madness::World::get_default().gop.fence();
}

void Task577::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  ta1_->init();
  madness::World::get_default().gop.fence();
  (*ta0_)("ci0") += (*ta1_)("ci0, x5, x4, x2, x3, x1, x0") * (*ta2_)("x2, x1, x0, x5, x4, x3");
  madness::World::get_default().gop.fence();
}

void Task578::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x2, x1, x0, x5, x4, x3") += (*ta1_)("x5, x4, c2, x3") * (*ta2_)("x2, x1, x0, c2");
  madness::World::get_default().gop.fence();
}

void Task579::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x2, x1, x0, c2") += (*ta1_)("c1, c2") * (*ta2_)("x0, x1, c1, x2") * (-2);
  madness::World::get_default().gop.fence();
}

void Task580::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x2, x1, x0, x5, x4, x3") += (*ta1_)("x0, x1, c1, x2") * (*ta2_)("x3, c1, x5, x4");
  madness::World::get_default().gop.fence();
}

void Task581::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x3, c1, x5, x4") += (*ta1_)("c1, a2, x5, x4") * (*ta2_)("a2, x3") * 2;
  madness::World::get_default().gop.fence();
}

void Task582::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x2, x1, x0, x5, x4, x3") += (*ta1_)("x5, x4, c1, x3") * (*ta2_)("x1, x0, c1, x2");
  madness::World::get_default().gop.fence();
}

void Task583::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x1, x0, c1, x2") += (*ta1_)("x0, x1, c1, x2") * e0_ * (-2);
  madness::World::get_default().gop.fence();
}

void Task584::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x1, x0, c1, x2") += (*ta1_)("x2, a2") * (*ta2_)("c1, a2, x0, x1") * 2;
  madness::World::get_default().gop.fence();
}

void Task585::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x2, x1, x0, x5, x4, x3") += (*ta1_)("x5, x4, c1, x3") * (*ta2_)("x0, x1, c1, x2");
  madness::World::get_default().gop.fence();
}

void Task586::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x2, x1, x0, x5, x4, x3") += (*ta1_)("x0, x1, c1, x2") * (*ta2_)("x5, x4, c1, x3");
  madness::World::get_default().gop.fence();
}

void Task587::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  ta1_->init();
  madness::World::get_default().gop.fence();
  (*ta0_)("ci0") += (*ta1_)("ci0, x2, x3, x1, x0") * (*ta2_)("x3, x2, x1, x0");
  madness::World::get_default().gop.fence();
}

void Task588::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x3, x2, x1, x0") += (*ta1_)("x0, x1, c1, x2") * (*ta2_)("c1, x3");
  madness::World::get_default().gop.fence();
}

void Task589::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("c1, x3") += (*ta1_)("c2, a3, c1, x3") * (*ta2_)("a3, c2") * 4;
  madness::World::get_default().gop.fence();
}

void Task590::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("c1, x3") += (*ta1_)("c1, a3, c2, x3") * (*ta2_)("a3, c2") * (-2);
  madness::World::get_default().gop.fence();
}

void Task591::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x3, x2, x1, x0") += (*ta1_)("x0, a1, c2, x1") * (*ta2_)("x2, c2, a1, x3");
  madness::World::get_default().gop.fence();
}

void Task592::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x2, c2, a1, x3") += (*ta1_)("c2, a1, c3, x3") * (*ta2_)("x2, c3") * (-2);
  madness::World::get_default().gop.fence();
}

void Task593::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x3, x2, x1, x0") += (*ta1_)("c1, a2, x0, x1") * (*ta2_)("x2, a2, c1, x3");
  madness::World::get_default().gop.fence();
}

void Task594::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x2, a2, c1, x3") += (*ta1_)("c3, a2, c1, x3") * (*ta2_)("x2, c3") * (-2);
  madness::World::get_default().gop.fence();
}

void Task595::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x2, a2, c1, x3") += (*ta1_)("c1, a2, c3, x3") * (*ta2_)("x2, c3") * 4;
  madness::World::get_default().gop.fence();
}

void Task596::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x3, x2, x1, x0") += (*ta1_)("c1, x3, x2, a2") * (*ta2_)("c1, a2, x0, x1");
  madness::World::get_default().gop.fence();
}

void Task597::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x3, x2, x1, x0") += (*ta1_)("c1, x3") * (*ta2_)("x0, x1, c1, x2") * 2;
  madness::World::get_default().gop.fence();
}

void Task598::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  ta1_->init();
  madness::World::get_default().gop.fence();
  (*ta0_)("ci0") += (*ta1_)("ci0, x5, x3, x2, x4, x1, x0") * (*ta2_)("x3, x5, x4, x2, x1, x0");
  madness::World::get_default().gop.fence();
}

void Task599::compute_() {
  if (!ta0_->initialized())
    ta0_->fill_local(0.0);
  madness::World::get_default().gop.fence();
  (*ta0_)("x3, x5, x4, x2, x1, x0") += (*ta1_)("x0, x1, c1, x2") * (*ta2_)("x3, x5, c1, x4");
  madness::World::get_default().gop.fence();
}

#endif
