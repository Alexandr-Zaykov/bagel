//
// BAGEL - Parallel electron correlation program.
// Filename: CAS_test_tasks.h
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


#ifndef __SRC_SMITH_CAS_test_TASKS_H
#define __SRC_SMITH_CAS_test_TASKS_H

#include <memory>
#include <algorithm>
#include <src/smith/indexrange.h>
#include <src/smith/tensor.h>
#include <src/smith/task.h>
#include <src/smith/subtask.h>
#include <src/smith/storage.h>
#include <vector>

namespace bagel {
namespace SMITH {
namespace CAS_test{

class Task0 : public Task {
  protected:
    std::shared_ptr<Tensor> r_;
    IndexRange closed_;
    IndexRange active_;
    IndexRange virt_;

    void compute_() {
      r_->zero();
    };

  public:
    Task0(std::vector<std::shared_ptr<Tensor>> t) : Task() {
      r_ =  t[0];
    };
    ~Task0() {};
};


class Task1 : public Task {  // associated with gamma
  protected:
    class Task_local : public SubTask<6,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,6>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<6,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index x1 = b(0);
          const Index x4 = b(1);
          const Index x0 = b(2);
          const Index x5 = b(3);
          const Index x2 = b(4);
          const Index x3 = b(5);
          // std::shared_ptr<Tensor > Gamma0;
          // std::shared_ptr<Tensor > rdm3;
          // std::shared_ptr<Tensor > f1;

          // tensor label: Gamma0
          std::unique_ptr<double[]> odata = out()->move_block(x5, x0, x4, x1);
          // associated with merged
          std::unique_ptr<double[]> fdata = in(1)->get_block(x3, x2);
          {
            std::unique_ptr<double[]> i0data = in(0)->get_block(x5, x0, x4, x1, x3, x2);
            for (int i2 = 0; i2 != x2.size(); ++i2) {
              for (int i3 = 0; i3 != x3.size(); ++i3) {
                for (int i1 = 0; i1 != x1.size(); ++i1) {
                  for (int i4 = 0; i4 != x4.size(); ++i4) {
                    for (int i0 = 0; i0 != x0.size(); ++i0) {
                      for (int i5 = 0; i5 != x5.size(); ++i5) {
                        odata[i5+x5.size()*(i0+x0.size()*(i4+x4.size()*(i1)))]
                          += (1.0) * i0data[i5+x5.size()*(i0+x0.size()*(i4+x4.size()*(i1+x1.size()*(i3+x3.size()*(i2)))))] * fdata[i3+x3.size()*(i2)];
                      }
                    }
                  }
                }
              }
            }
          }
          out()->put_block(odata, x5, x0, x4, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task1(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
          for (auto& x1 : *range[1])
            for (auto& x4 : *range[1])
              for (auto& x0 : *range[1])
                for (auto& x5 : *range[1])
                  for (auto& x2 : *range[1])
                    for (auto& x3 : *range[1])
                      subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,6>{{x5, x0, x4, x1, x3, x2}}, in, t[0], range)));
    };
    ~Task1() {};
};


class Task2 : public Task {  // associated with gamma
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index x1 = b(0);
          const Index x2 = b(1);
          const Index x0 = b(2);
          const Index x3 = b(3);
          // std::shared_ptr<Tensor > Gamma2;
          // std::shared_ptr<Tensor > rdm2;

          // tensor label: Gamma2
          std::unique_ptr<double[]> odata = out()->move_block(x3, x0, x2, x1);
          {
            std::unique_ptr<double[]> i0data = in(0)->get_block(x3, x0, x2, x1);
            sort_indices<0,1,2,3,1,1,1,1>(i0data, odata, x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task2(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
          for (auto& x1 : *range[1])
            for (auto& x2 : *range[1])
              for (auto& x0 : *range[1])
                for (auto& x3 : *range[1])
                  subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x3, x0, x2, x1}}, in, t[0], range)));
    };
    ~Task2() {};
};


class Task3 : public Task {  // associated with gamma
  protected:
    class Task_local : public SubTask<6,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,6>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<6,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index x2 = b(0);
          const Index x3 = b(1);
          const Index x1 = b(2);
          const Index x4 = b(3);
          const Index x0 = b(4);
          const Index x5 = b(5);
          // std::shared_ptr<Tensor > Gamma9;
          // std::shared_ptr<Tensor > rdm3;

          // tensor label: Gamma9
          std::unique_ptr<double[]> odata = out()->move_block(x5, x0, x4, x1, x3, x2);
          {
            std::unique_ptr<double[]> i0data = in(0)->get_block(x5, x0, x4, x1, x3, x2);
            sort_indices<0,1,2,3,4,5,1,1,1,1>(i0data, odata, x5.size(), x0.size(), x4.size(), x1.size(), x3.size(), x2.size());
          }
          out()->put_block(odata, x5, x0, x4, x1, x3, x2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task3(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
          for (auto& x2 : *range[1])
            for (auto& x3 : *range[1])
              for (auto& x1 : *range[1])
                for (auto& x4 : *range[1])
                  for (auto& x0 : *range[1])
                    for (auto& x5 : *range[1])
                      subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,6>{{x5, x0, x4, x1, x3, x2}}, in, t[0], range)));
    };
    ~Task3() {};
};


class Task4 : public Task {  // associated with gamma
  protected:
    class Task_local : public SubTask<7,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,7>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<7,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x5 = b(1);
          const Index x0 = b(2);
          const Index x4 = b(3);
          const Index x1 = b(4);
          const Index x3 = b(5);
          const Index x2 = b(6);
          // std::shared_ptr<Tensor > Gamma12;
          // std::shared_ptr<Tensor > rdm3I0;
          // std::shared_ptr<Tensor > f1;

          // tensor label: Gamma12
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x5, x0, x4, x1);
          // associated with merged
          std::unique_ptr<double[]> fdata = in(1)->get_block(x3, x2);
          {
            std::unique_ptr<double[]> i0data = in(0)->get_block(ci0, x5, x0, x4, x1, x3, x2);
            for (int ix2 = 0; ix2 != x2.size(); ++ix2) {
              for (int ix3 = 0; ix3 != x3.size(); ++ix3) {
                for (int ix1 = 0; ix1 != x1.size(); ++ix1) {
                  for (int ix4 = 0; ix4 != x4.size(); ++ix4) {
                    for (int ix0 = 0; ix0 != x0.size(); ++ix0) {
                      for (int ix5 = 0; ix5 != x5.size(); ++ix5) {
                        for (int ici0 = 0; ici0 != ci0.size(); ++ici0) {
                          odata[ici0+ci0.size()*(ix5+x5.size()*(ix0+x0.size()*(ix4+x4.size()*(ix1))))]
                            += (1.0) * i0data[ici0+ci0.size()*(ix5+x5.size()*(ix0+x0.size()*(ix4+x4.size()*(ix1+x1.size()*(ix3+x3.size()*(ix2))))))] * fdata[ix3+x3.size()*(ix2)];
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          out()->put_block(odata, ci0, x5, x0, x4, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task4(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,4> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock()*range[1]->nblock()*range[1]->nblock());
          for (auto& x1 : *range[1])
            for (auto& x4 : *range[1])
              for (auto& x0 : *range[1])
                for (auto& x5 : *range[1])
                  for (auto& ci0 : *range[3])
                    for (auto& x2 : *range[1])
                      for (auto& x3 : *range[1])
                        subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,7>{{ci0, x5, x0, x4, x1, x3, x2}}, in, t[0], range)));
    };
    ~Task4() {};
};


class Task5 : public Task {  // associated with gamma
  protected:
    class Task_local : public SubTask<5,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x3 = b(1);
          const Index x0 = b(2);
          const Index x2 = b(3);
          const Index x1 = b(4);
          // std::shared_ptr<Tensor > Gamma13;
          // std::shared_ptr<Tensor > rdm2I0;

          // tensor label: Gamma13
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x3, x0, x2, x1);
          {
            std::unique_ptr<double[]> i0data = in(0)->get_block(ci0, x3, x0, x2, x1);
            sort_indices<0,1,2,3,4,1,1,1,1>(i0data, odata, ci0.size(), x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, ci0, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task5(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,4> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
          for (auto& x1 : *range[1])
            for (auto& x2 : *range[1])
              for (auto& x0 : *range[1])
                for (auto& x3 : *range[1])
                  for (auto& ci0 : *range[3])
                    subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x3, x0, x2, x1}}, in, t[0], range)));
    };
    ~Task5() {};
};


class Task6 : public Task {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,1>(block, in, out), range_(ran) { }

        void compute() override {
          const Index x1 = b(0);
          const Index a2 = b(1);
          const Index x0 = b(2);
          const Index a1 = b(3);

          // tensor label: r
          std::unique_ptr<double[]> odata = out()->move_block(x1, a2, x0, a1);
          {
            // tensor label: I0
            std::unique_ptr<double[]> i0data = in(0)->get_block(x0, x1, a1, a2);
            sort_indices<1,3,0,2,1,1,1,1>(i0data, odata, x0.size(), x1.size(), a1.size(), a2.size());
          }
          out()->put_block(odata, x1, a2, x0, a1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task6(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[2]->nblock()*range[1]->nblock()*range[2]->nblock()*range[1]->nblock());
      for (auto& a1 : *range[2])
        for (auto& x0 : *range[1])
          for (auto& a2 : *range[2])
            for (auto& x1 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x1, a2, x0, a1}}, in, t[0], range)));
    };
    ~Task6() {};
};


class Task7 : public Task {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }

        void compute() override {
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index a1 = b(2);
          const Index a2 = b(3);

          // tensor label: I0
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, a1, a2), 0.0);

          for (auto& x5 : *range_[1]) {
            for (auto& x4 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x5, a1, x4, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x5, a1, x4, a2)]);
              sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, x5.size(), a1.size(), x4.size(), a2.size());

              // tensor label: I1
              std::unique_ptr<double[]> i1data = in(1)->get_block(x5, x0, x4, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x5, x0, x4, x1)]);
              sort_indices<0,2,1,3,0,1,1,1>(i1data, i1data_sorted, x5.size(), x0.size(), x4.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), x0.size()*x1.size(), x5.size()*x4.size(),
                     1.0, i0data_sorted, x5.size()*x4.size(), i1data_sorted, x5.size()*x4.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), x0.size(), x1.size());
          out()->put_block(odata, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task7(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, x1, a1, a2}}, in, t[0], range)));
    };
    ~Task7() {};
};



class Task8 : public Task {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,1>(block, in, out), range_(ran) { }

        void compute() override {
          const Index x5 = b(0);
          const Index x0 = b(1);
          const Index x4 = b(2);
          const Index x1 = b(3);

          // tensor label: I1
          std::unique_ptr<double[]> odata = out()->move_block(x5, x0, x4, x1);
          {
            // tensor label: Gamma0
            std::unique_ptr<double[]> i0data = in(0)->get_block(x5, x0, x4, x1);
            sort_indices<0,1,2,3,1,1,2,1>(i0data, odata, x5.size(), x0.size(), x4.size(), x1.size());
          }
          out()->put_block(odata, x5, x0, x4, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task8(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x4 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x5 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x5, x0, x4, x1}}, in, t[0], range)));
    };
    ~Task8() {};
};


class Task9 : public Task {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }

        void compute() override {
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index a1 = b(2);
          const Index a2 = b(3);

          // tensor label: I0
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, a1, a2), 0.0);

          for (auto& x3 : *range_[1]) {
            for (auto& x2 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x3, a1, x2, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x3, a1, x2, a2)]);
              sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), x2.size(), a2.size());

              // tensor label: I6
              std::unique_ptr<double[]> i1data = in(1)->get_block(x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x3, x0, x2, x1)]);
              sort_indices<0,2,1,3,0,1,1,1>(i1data, i1data_sorted, x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), x0.size()*x1.size(), x3.size()*x2.size(),
                     1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), x0.size(), x1.size());
          out()->put_block(odata, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task9(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, x1, a1, a2}}, in, t[0], range)));
    };
    ~Task9() {};
};


class Task10 : public Task {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        const double e0_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran, const double e)
          : SubTask<4,1>(block, in, out), range_(ran), e0_(e) { }

        void compute() override {
          const Index x3 = b(0);
          const Index x0 = b(1);
          const Index x2 = b(2);
          const Index x1 = b(3);

          // tensor label: I6
          std::unique_ptr<double[]> odata = out()->move_block(x3, x0, x2, x1);
          {
            // tensor label: Gamma2
            std::unique_ptr<double[]> i0data = in(0)->get_block(x3, x0, x2, x1);
            dscal_(x3.size()*x0.size()*x2.size()*x1.size(), e0_, i0data.get(), 1);
            sort_indices<0,1,2,3,1,1,-2,1>(i0data, odata, x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task10(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range, const double e) : Task() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x3, x0, x2, x1}}, in, t[0], range, e)));
    };
    ~Task10() {};
};


class Task11 : public Task {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }

        void compute() override {
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index a1 = b(2);
          const Index a2 = b(3);

          // tensor label: I0
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, a1, a2), 0.0);

          for (auto& x3 : *range_[1]) {
            for (auto& x2 : *range_[1]) {
              // tensor label: v2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x3, a1, x2, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x3, a1, x2, a2)]);
              sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), x2.size(), a2.size());

              // tensor label: I8
              std::unique_ptr<double[]> i1data = in(1)->get_block(x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x3, x0, x2, x1)]);
              sort_indices<0,2,1,3,0,1,1,1>(i1data, i1data_sorted, x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), x0.size()*x1.size(), x3.size()*x2.size(),
                     1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), x0.size(), x1.size());
          out()->put_block(odata, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task11(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, x1, a1, a2}}, in, t[0], range)));
    };
    ~Task11() {};
};


class Task12 : public Task {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,1>(block, in, out), range_(ran) { }

        void compute() override {
          const Index x3 = b(0);
          const Index x0 = b(1);
          const Index x2 = b(2);
          const Index x1 = b(3);

          // tensor label: I8
          std::unique_ptr<double[]> odata = out()->move_block(x3, x0, x2, x1);
          {
            // tensor label: Gamma2
            std::unique_ptr<double[]> i0data = in(0)->get_block(x3, x0, x2, x1);
            sort_indices<0,1,2,3,1,1,2,1>(i0data, odata, x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task12(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x3, x0, x2, x1}}, in, t[0], range)));
    };
    ~Task12() {};
};


class Task13 : public Task {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,1>(block, in, out), range_(ran) { }

        void compute() override {
          const Index x1 = b(0);
          const Index a2 = b(1);
          const Index x0 = b(2);
          const Index a1 = b(3);

          // tensor label: r
          std::unique_ptr<double[]> odata = out()->move_block(x1, a2, x0, a1);
          {
            // tensor label: I2
            std::unique_ptr<double[]> i0data = in(0)->get_block(x0, x1, a1, a2);
            sort_indices<1,3,0,2,1,1,1,1>(i0data, odata, x0.size(), x1.size(), a1.size(), a2.size());
          }
          {
            // tensor label: I2
            std::unique_ptr<double[]> i0data = in(0)->get_block(x1, x0, a2, a1);
            sort_indices<0,2,1,3,1,1,1,1>(i0data, odata, x1.size(), x0.size(), a2.size(), a1.size());
          }
          out()->put_block(odata, x1, a2, x0, a1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task13(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[2]->nblock()*range[1]->nblock()*range[2]->nblock()*range[1]->nblock());
      for (auto& a1 : *range[2])
        for (auto& x0 : *range[1])
          for (auto& a2 : *range[2])
            for (auto& x1 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x1, a2, x0, a1}}, in, t[0], range)));
    };
    ~Task13() {};
};


class Task14 : public Task {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }

        void compute() override {
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index a1 = b(2);
          const Index a2 = b(3);

          // tensor label: I2
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, a1, a2), 0.0);

          for (auto& a3 : *range_[2]) {
            // tensor label: f1
            std::unique_ptr<double[]> i0data = in(0)->get_block(a3, a2);
            std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(a3, a2)]);
            sort_indices<0,1,0,1,1,1>(i0data, i0data_sorted, a3.size(), a2.size());

            // tensor label: I3
            std::unique_ptr<double[]> i1data = in(1)->get_block(x0, x1, a1, a3);
            std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x0, x1, a1, a3)]);
            sort_indices<3,0,1,2,0,1,1,1>(i1data, i1data_sorted, x0.size(), x1.size(), a1.size(), a3.size());

            dgemm_("T", "N", a2.size(), x0.size()*x1.size()*a1.size(), a3.size(),
                   1.0, i0data_sorted, a3.size(), i1data_sorted, a3.size(),
                   1.0, odata_sorted, a2.size());
          }

          sort_indices<1,2,3,0,1,1,1,1>(odata_sorted, odata, a2.size(), x0.size(), x1.size(), a1.size());
          out()->put_block(odata, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task14(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, x1, a1, a2}}, in, t[0], range)));
    };
    ~Task14() {};
};


class Task15 : public Task {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }

        void compute() override {
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index a1 = b(2);
          const Index a3 = b(3);

          // tensor label: I3
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, a1, a3);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, a1, a3)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, a1, a3), 0.0);

          for (auto& x3 : *range_[1]) {
            for (auto& x2 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x3, a1, x2, a3);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x3, a1, x2, a3)]);
              sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), x2.size(), a3.size());

              // tensor label: I4
              std::unique_ptr<double[]> i1data = in(1)->get_block(x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x3, x0, x2, x1)]);
              sort_indices<0,2,1,3,0,1,1,1>(i1data, i1data_sorted, x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a3.size(), x0.size()*x1.size(), x3.size()*x2.size(),
                     1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
                     1.0, odata_sorted, a1.size()*a3.size());
            }
          }

          sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a3.size(), x0.size(), x1.size());
          out()->put_block(odata, x0, x1, a1, a3);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task15(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a3 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, x1, a1, a3}}, in, t[0], range)));
    };
    ~Task15() {};
};


class Task16 : public Task {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,1>(block, in, out), range_(ran) { }

        void compute() override {
          const Index x3 = b(0);
          const Index x0 = b(1);
          const Index x2 = b(2);
          const Index x1 = b(3);

          // tensor label: I4
          std::unique_ptr<double[]> odata = out()->move_block(x3, x0, x2, x1);
          {
            // tensor label: Gamma2
            std::unique_ptr<double[]> i0data = in(0)->get_block(x3, x0, x2, x1);
            sort_indices<0,1,2,3,1,1,2,1>(i0data, odata, x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) i->compute();
    }

  public:
    Task16(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : Task() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x3, x0, x2, x1}}, in, t[0], range)));
    };
    ~Task16() {};
};


class Task17 : public EnergyTask {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        double energy_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }

        double energy() const { return energy_; }

        void compute() override {
          energy_ = 0.0;
          const Index x0 = b(0);
          const Index a1 = b(1);
          const Index x1 = b(2);
          const Index a2 = b(3);

          // tensor label: t2
          std::unique_ptr<double[]> i0data = in(0)->get_block(x0, a1, x1, a2);
          std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x0, a1, x1, a2)]);
          sort_indices<3,2,1,0,0,1,1,1>(i0data, i0data_sorted, x0.size(), a1.size(), x1.size(), a2.size());

          // tensor label: I10
          std::unique_ptr<double[]> i1data = in(1)->get_block(x0, x1, a1, a2);
          std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x0, x1, a1, a2)]);
          sort_indices<3,1,2,0,0,1,1,1>(i1data, i1data_sorted, x0.size(), x1.size(), a1.size(), a2.size());

          energy_ += ddot_(x0.size()*x1.size()*a1.size()*a2.size(), i0data_sorted, 1, i1data_sorted, 1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      this->energy_ = 0.0;
      for (auto& i : subtasks_) {
        i->compute();
        this->energy_ += i->energy();
      }
    }

  public:
    Task17(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : EnergyTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[1]->nblock()*range[2]->nblock()*range[1]->nblock());
      for (auto& a2 : *range[2])
        for (auto& x1 : *range[1])
          for (auto& a1 : *range[2])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, a1, x1, a2}}, in, t[0], range)));
    };
    ~Task17() {};
};


class Task18 : public EnergyTask {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        double energy_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }

        double energy() const { return energy_; }

        void compute() override {
          energy_ = 0.0;
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index a1 = b(2);
          const Index a2 = b(3);

          // tensor label: I10
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, a1, a2), 0.0);

          for (auto& x5 : *range_[1]) {
            for (auto& x4 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x5, a1, x4, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x5, a1, x4, a2)]);
              sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, x5.size(), a1.size(), x4.size(), a2.size());

              // tensor label: I11
              std::unique_ptr<double[]> i1data = in(1)->get_block(x5, x0, x4, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x5, x0, x4, x1)]);
              sort_indices<0,2,1,3,0,1,1,1>(i1data, i1data_sorted, x5.size(), x0.size(), x4.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), x0.size()*x1.size(), x5.size()*x4.size(),
                     1.0, i0data_sorted, x5.size()*x4.size(), i1data_sorted, x5.size()*x4.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), x0.size(), x1.size());
          out()->put_block(odata, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      this->energy_ = 0.0;
      for (auto& i : subtasks_) {
        i->compute();
        this->energy_ += i->energy();
      }
    }

  public:
    Task18(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : EnergyTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, x1, a1, a2}}, in, t[0], range)));
    };
    ~Task18() {};
};


class Task19 : public EnergyTask {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        double energy_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,1>(block, in, out), range_(ran) { }

        double energy() const { return energy_; }

        void compute() override {
          energy_ = 0.0;
          const Index x5 = b(0);
          const Index x0 = b(1);
          const Index x4 = b(2);
          const Index x1 = b(3);

          // tensor label: I11
          std::unique_ptr<double[]> odata = out()->move_block(x5, x0, x4, x1);
          {
            // tensor label: Gamma0
            std::unique_ptr<double[]> i0data = in(0)->get_block(x5, x0, x4, x1);
            sort_indices<0,1,2,3,1,1,1,2>(i0data, odata, x5.size(), x0.size(), x4.size(), x1.size());
          }
          out()->put_block(odata, x5, x0, x4, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      this->energy_ = 0.0;
      for (auto& i : subtasks_) {
        i->compute();
        this->energy_ += i->energy();
      }
    }

  public:
    Task19(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : EnergyTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x4 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x5 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x5, x0, x4, x1}}, in, t[0], range)));
    };
    ~Task19() {};
};


class Task20 : public EnergyTask {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        double energy_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }

        double energy() const { return energy_; }

        void compute() override {
          energy_ = 0.0;
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index a1 = b(2);
          const Index a2 = b(3);

          // tensor label: I10
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, a1, a2), 0.0);

          for (auto& a3 : *range_[2]) {
            // tensor label: f1
            std::unique_ptr<double[]> i0data = in(0)->get_block(a3, a2);
            std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(a3, a2)]);
            sort_indices<0,1,0,1,1,1>(i0data, i0data_sorted, a3.size(), a2.size());

            // tensor label: I14
            std::unique_ptr<double[]> i1data = in(1)->get_block(x0, x1, a1, a3);
            std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x0, x1, a1, a3)]);
            sort_indices<3,0,1,2,0,1,1,1>(i1data, i1data_sorted, x0.size(), x1.size(), a1.size(), a3.size());

            dgemm_("T", "N", a2.size(), x0.size()*x1.size()*a1.size(), a3.size(),
                   1.0, i0data_sorted, a3.size(), i1data_sorted, a3.size(),
                   1.0, odata_sorted, a2.size());
          }

          sort_indices<1,2,3,0,1,1,1,1>(odata_sorted, odata, a2.size(), x0.size(), x1.size(), a1.size());
          out()->put_block(odata, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      this->energy_ = 0.0;
      for (auto& i : subtasks_) {
        i->compute();
        this->energy_ += i->energy();
      }
    }

  public:
    Task20(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : EnergyTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, x1, a1, a2}}, in, t[0], range)));
    };
    ~Task20() {};
};


class Task21 : public EnergyTask {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        double energy_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }

        double energy() const { return energy_; }

        void compute() override {
          energy_ = 0.0;
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index a1 = b(2);
          const Index a3 = b(3);

          // tensor label: I14
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, a1, a3);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, a1, a3)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, a1, a3), 0.0);

          for (auto& x3 : *range_[1]) {
            for (auto& x2 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x3, a1, x2, a3);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x3, a1, x2, a3)]);
              sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), x2.size(), a3.size());

              // tensor label: I15
              std::unique_ptr<double[]> i1data = in(1)->get_block(x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x3, x0, x2, x1)]);
              sort_indices<0,2,1,3,0,1,1,1>(i1data, i1data_sorted, x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a3.size(), x0.size()*x1.size(), x3.size()*x2.size(),
                     1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
                     1.0, odata_sorted, a1.size()*a3.size());
            }
          }

          sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a3.size(), x0.size(), x1.size());
          out()->put_block(odata, x0, x1, a1, a3);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      this->energy_ = 0.0;
      for (auto& i : subtasks_) {
        i->compute();
        this->energy_ += i->energy();
      }
    }

  public:
    Task21(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : EnergyTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a3 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, x1, a1, a3}}, in, t[0], range)));
    };
    ~Task21() {};
};


class Task22 : public EnergyTask {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        double energy_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,1>(block, in, out), range_(ran) { }

        double energy() const { return energy_; }

        void compute() override {
          energy_ = 0.0;
          const Index x3 = b(0);
          const Index x0 = b(1);
          const Index x2 = b(2);
          const Index x1 = b(3);

          // tensor label: I15
          std::unique_ptr<double[]> odata = out()->move_block(x3, x0, x2, x1);
          {
            // tensor label: Gamma2
            std::unique_ptr<double[]> i0data = in(0)->get_block(x3, x0, x2, x1);
            sort_indices<0,1,2,3,1,1,1,1>(i0data, odata, x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      this->energy_ = 0.0;
      for (auto& i : subtasks_) {
        i->compute();
        this->energy_ += i->energy();
      }
    }

  public:
    Task22(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : EnergyTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x3, x0, x2, x1}}, in, t[0], range)));
    };
    ~Task22() {};
};


class Task23 : public EnergyTask {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        double energy_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }

        double energy() const { return energy_; }

        void compute() override {
          energy_ = 0.0;
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index a1 = b(2);
          const Index a2 = b(3);

          // tensor label: I10
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, a1, a2), 0.0);

          for (auto& x3 : *range_[1]) {
            for (auto& x2 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x3, a1, x2, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x3, a1, x2, a2)]);
              sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), x2.size(), a2.size());

              // tensor label: I18
              std::unique_ptr<double[]> i1data = in(1)->get_block(x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x3, x0, x2, x1)]);
              sort_indices<0,2,1,3,0,1,1,1>(i1data, i1data_sorted, x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), x0.size()*x1.size(), x3.size()*x2.size(),
                     1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), x0.size(), x1.size());
          out()->put_block(odata, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      this->energy_ = 0.0;
      for (auto& i : subtasks_) {
        i->compute();
        this->energy_ += i->energy();
      }
    }

  public:
    Task23(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : EnergyTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, x1, a1, a2}}, in, t[0], range)));
    };
    ~Task23() {};
};


class Task24 : public EnergyTask {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        double energy_;
        double e0_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran, const double e)
          : SubTask<4,1>(block, in, out), range_(ran), e0_(e) { }

        double energy() const { return energy_; }

        void compute() override {
          energy_ = 0.0;
          const Index x3 = b(0);
          const Index x0 = b(1);
          const Index x2 = b(2);
          const Index x1 = b(3);

          // tensor label: I18
          std::unique_ptr<double[]> odata = out()->move_block(x3, x0, x2, x1);
          {
            // tensor label: Gamma2
            std::unique_ptr<double[]> i0data = in(0)->get_block(x3, x0, x2, x1);
            dscal_(x3.size()*x0.size()*x2.size()*x1.size(), e0_, i0data.get(), 1);
            sort_indices<0,1,2,3,1,1,-1,2>(i0data, odata, x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      this->energy_ = 0.0;
      for (auto& i : subtasks_) {
        i->compute();
        this->energy_ += i->energy();
      }
    }

  public:
    Task24(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range, const double e) : EnergyTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x3, x0, x2, x1}}, in, t[0], range, e)));
    };
    ~Task24() {};
};


class Task25 : public EnergyTask {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        double energy_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }

        double energy() const { return energy_; }

        void compute() override {
          energy_ = 0.0;
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index a1 = b(2);
          const Index a2 = b(3);

          // tensor label: I10
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, a1, a2), 0.0);

          for (auto& x3 : *range_[1]) {
            for (auto& x2 : *range_[1]) {
              // tensor label: v2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x3, a1, x2, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x3, a1, x2, a2)]);
              sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), x2.size(), a2.size());

              // tensor label: I21
              std::unique_ptr<double[]> i1data = in(1)->get_block(x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x3, x0, x2, x1)]);
              sort_indices<0,2,1,3,0,1,1,1>(i1data, i1data_sorted, x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), x0.size()*x1.size(), x3.size()*x2.size(),
                     1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), x0.size(), x1.size());
          out()->put_block(odata, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      this->energy_ = 0.0;
      for (auto& i : subtasks_) {
        i->compute();
        this->energy_ += i->energy();
      }
    }

  public:
    Task25(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : EnergyTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, x1, a1, a2}}, in, t[0], range)));
    };
    ~Task25() {};
};


class Task26 : public EnergyTask {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        double energy_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,1>(block, in, out), range_(ran) { }

        double energy() const { return energy_; }

        void compute() override {
          energy_ = 0.0;
          const Index x3 = b(0);
          const Index x0 = b(1);
          const Index x2 = b(2);
          const Index x1 = b(3);

          // tensor label: I21
          std::unique_ptr<double[]> odata = out()->move_block(x3, x0, x2, x1);
          {
            // tensor label: Gamma2
            std::unique_ptr<double[]> i0data = in(0)->get_block(x3, x0, x2, x1);
            sort_indices<0,1,2,3,1,1,1,1>(i0data, odata, x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      this->energy_ = 0.0;
      for (auto& i : subtasks_) {
        i->compute();
        this->energy_ += i->energy();
      }
    }

  public:
    Task26(std::vector<std::shared_ptr<Tensor>> t,  std::array<std::shared_ptr<const IndexRange>,3> range) : EnergyTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x3, x0, x2, x1}}, in, t[0], range)));
    };
    ~Task26() {};
};


class Task27 : public CorrectionTask {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        double correction_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }

        double correction() const { return correction_; }

        void compute() override {
          correction_ = 0.0;
          const Index x0 = b(0);
          const Index a1 = b(1);
          const Index x1 = b(2);
          const Index a2 = b(3);

          // tensor label: t2
          std::unique_ptr<double[]> i0data = in(0)->get_block(x0, a1, x1, a2);
          std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x0, a1, x1, a2)]);
          sort_indices<3,2,1,0,0,1,1,1>(i0data, i0data_sorted, x0.size(), a1.size(), x1.size(), a2.size());

          // tensor label: I23
          std::unique_ptr<double[]> i1data = in(1)->get_block(x0, x1, a1, a2);
          std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x0, x1, a1, a2)]);
          sort_indices<3,1,2,0,0,1,1,1>(i1data, i1data_sorted, x0.size(), x1.size(), a1.size(), a2.size());

          correction_ += ddot_(x0.size()*x1.size()*a1.size()*a2.size(), i0data_sorted, 1, i1data_sorted, 1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      this->correction_ = 0.0;
      for (auto& i : subtasks_) {
        i->compute();
        this->correction_ += i->correction();
      }
    }

  public:
    Task27(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : CorrectionTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[1]->nblock()*range[2]->nblock()*range[1]->nblock());
      for (auto& a2 : *range[2])
        for (auto& x1 : *range[1])
          for (auto& a1 : *range[2])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, a1, x1, a2}}, in, t[0], range)));
    }
    ~Task27() {}
};


class Task28 : public CorrectionTask {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        double correction_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }

        double correction() const { return correction_; }

        void compute() override {
          correction_ = 0.0;
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index a1 = b(2);
          const Index a2 = b(3);

          // tensor label: I23
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, a1, a2), 0.0);

          for (auto& x3 : *range_[1]) {
            for (auto& x2 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x3, a1, x2, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x3, a1, x2, a2)]);
              sort_indices<0,2,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), x2.size(), a2.size());

              // tensor label: I24
              std::unique_ptr<double[]> i1data = in(1)->get_block(x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x3, x0, x2, x1)]);
              sort_indices<0,2,1,3,0,1,1,1>(i1data, i1data_sorted, x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), x0.size()*x1.size(), x3.size()*x2.size(),
                     1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), x0.size(), x1.size());
          out()->put_block(odata, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      this->correction_ = 0.0;
      for (auto& i : subtasks_) {
        i->compute();
        this->correction_ += i->correction();
      }
    }

  public:
    Task28(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : CorrectionTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, x1, a1, a2}}, in, t[0], range)));
    }
    ~Task28() {}
};


class Task29 : public CorrectionTask {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }
        double correction_;

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,1>(block, in, out), range_(ran) { }

        double correction() const { return correction_; }

        void compute() override {
          correction_ = 0.0;
          const Index x3 = b(0);
          const Index x0 = b(1);
          const Index x2 = b(2);
          const Index x1 = b(3);

          // tensor label: I24
          std::unique_ptr<double[]> odata = out()->move_block(x3, x0, x2, x1);
          {
            // tensor label: Gamma2
            std::unique_ptr<double[]> i0data = in(0)->get_block(x3, x0, x2, x1);
            sort_indices<0,1,2,3,1,1,1,2>(i0data, odata, x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      this->correction_ = 0.0;
      for (auto& i : subtasks_) {
        i->compute();
        this->correction_ += i->correction();
      }
    }

  public:
    Task29(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : CorrectionTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x3, x0, x2, x1}}, in, t[0], range)));
    }
    ~Task29() {}
};


class Task30 : public DensityTask {
  protected:
    std::shared_ptr<Tensor> d_;
    IndexRange closed_;
    IndexRange active_;
    IndexRange virt_;

    void compute_() {
      d_->zero();
    };

  public:
    Task30(std::vector<std::shared_ptr<Tensor>> t) : DensityTask() {
      d_ =  t[0];
    };
    ~Task30() {};
};


class Task31 : public DensityTask {
  protected:
    class Task_local : public SubTask<2,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,2>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<2,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index x2 = b(0);
          const Index x3 = b(1);

          // tensor label: den2
          std::unique_ptr<double[]> odata = out()->move_block(x2, x3);
          {
            // tensor label: I25
            std::unique_ptr<double[]> i0data = in(0)->get_block(x3, x2);
            sort_indices<1,0,1,1,1,1>(i0data, odata, x3.size(), x2.size());
          }
          out()->put_block(odata, x2, x3);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task31(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : DensityTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock());
      for (auto& x3 : *range[1])
        for (auto& x2 : *range[1])
          subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,2>{{x2, x3}}, in, t[0], range)));
    };
    ~Task31() {};
};



class Task32 : public DensityTask {
  protected:
    class Task_local : public SubTask<2,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,2>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<2,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index x3 = b(0);
          const Index x2 = b(1);

          // tensor label: I25
          std::unique_ptr<double[]> odata = out()->move_block(x3, x2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x3, x2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x3, x2), 0.0);

          for (auto& x0 : *range_[1]) {
            for (auto& a1 : *range_[2]) {
              for (auto& x1 : *range_[1]) {
                for (auto& a2 : *range_[2]) {
                  // tensor label: t2
                  std::unique_ptr<double[]> i0data = in(0)->get_block(x0, a1, x1, a2);
                  std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x0, a1, x1, a2)]);
                  sort_indices<0,1,2,3,0,1,1,1>(i0data, i0data_sorted, x0.size(), a1.size(), x1.size(), a2.size());

                  // tensor label: I26
                  std::unique_ptr<double[]> i1data = in(1)->get_block(x0, x1, x3, x2, a1, a2);
                  std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x0, x1, x3, x2, a1, a2)]);
                  sort_indices<0,4,1,5,2,3,0,1,1,1>(i1data, i1data_sorted, x0.size(), x1.size(), x3.size(), x2.size(), a1.size(), a2.size());

                  dgemm_("T", "N", 1, x3.size()*x2.size(), x0.size()*x1.size()*a1.size()*a2.size(),
                         1.0, i0data_sorted, x0.size()*x1.size()*a1.size()*a2.size(), i1data_sorted, x0.size()*x1.size()*a1.size()*a2.size(),
                         1.0, odata_sorted, 1);
                }
              }
            }
          }

          sort_indices<0,1,1,1,1,1>(odata_sorted, odata, x3.size(), x2.size());
          out()->put_block(odata, x3, x2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task32(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : DensityTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock());
      for (auto& x2 : *range[1])
        for (auto& x3 : *range[1])
          subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,2>{{x3, x2}}, in, t[0], range)));
    };
    ~Task32() {};
};


class Task33 : public DensityTask {
  protected:
    class Task_local : public SubTask<6,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,6>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<6,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index x3 = b(2);
          const Index x2 = b(3);
          const Index a1 = b(4);
          const Index a2 = b(5);

          // tensor label: I26
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, x3, x2, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, x3, x2, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, x3, x2, a1, a2), 0.0);

          for (auto& x4 : *range_[1]) {
            for (auto& x5 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x5, a1, x4, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x5, a1, x4, a2)]);
              sort_indices<2,0,1,3,0,1,1,1>(i0data, i0data_sorted, x5.size(), a1.size(), x4.size(), a2.size());

              // tensor label: I27
              std::unique_ptr<double[]> i1data = in(1)->get_block(x5, x0, x4, x1, x3, x2);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x5, x0, x4, x1, x3, x2)]);
              sort_indices<2,0,1,3,4,5,0,1,1,1>(i1data, i1data_sorted, x5.size(), x0.size(), x4.size(), x1.size(), x3.size(), x2.size());

              dgemm_("T", "N", a1.size()*a2.size(), x0.size()*x1.size()*x3.size()*x2.size(), x5.size()*x4.size(),
                     1.0, i0data_sorted, x5.size()*x4.size(), i1data_sorted, x5.size()*x4.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,4,5,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), x0.size(), x1.size(), x3.size(), x2.size());
          out()->put_block(odata, x0, x1, x3, x2, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task33(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : DensityTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x2 : *range[1])
            for (auto& x3 : *range[1])
              for (auto& x1 : *range[1])
                for (auto& x0 : *range[1])
                  subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,6>{{x0, x1, x3, x2, a1, a2}}, in, t[0], range)));
    };
    ~Task33() {};
};


class Task34 : public DensityTask {
  protected:
    class Task_local : public SubTask<6,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,6>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<6,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index x5 = b(0);
          const Index x0 = b(1);
          const Index x4 = b(2);
          const Index x1 = b(3);
          const Index x3 = b(4);
          const Index x2 = b(5);

          // tensor label: I27
          std::unique_ptr<double[]> odata = out()->move_block(x5, x0, x4, x1, x3, x2);
          {
            // tensor label: Gamma9
            std::unique_ptr<double[]> i0data = in(0)->get_block(x5, x0, x4, x1, x3, x2);
            sort_indices<0,1,2,3,4,5,1,1,1,2>(i0data, odata, x5.size(), x0.size(), x4.size(), x1.size(), x3.size(), x2.size());
          }
          out()->put_block(odata, x5, x0, x4, x1, x3, x2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task34(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : DensityTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& x2 : *range[1])
        for (auto& x3 : *range[1])
          for (auto& x1 : *range[1])
            for (auto& x4 : *range[1])
              for (auto& x0 : *range[1])
                for (auto& x5 : *range[1])
                  subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,6>{{x5, x0, x4, x1, x3, x2}}, in, t[0], range)));
    };
    ~Task34() {};
};


class Task35 : public DensityTask {
  protected:
    class Task_local : public SubTask<2,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,2>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<2,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index a2 = b(0);
          const Index a3 = b(1);

          // tensor label: den2
          std::unique_ptr<double[]> odata = out()->move_block(a2, a3);
          {
            // tensor label: I28
            std::unique_ptr<double[]> i0data = in(0)->get_block(a3, a2);
            sort_indices<1,0,1,1,1,1>(i0data, odata, a3.size(), a2.size());
          }
          out()->put_block(odata, a2, a3);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task35(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : DensityTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock());
      for (auto& a3 : *range[2])
        for (auto& a2 : *range[2])
          subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,2>{{a2, a3}}, in, t[0], range)));
    };
    ~Task35() {};
};


class Task36 : public DensityTask {
  protected:
    class Task_local : public SubTask<2,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,2>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<2,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index a3 = b(0);
          const Index a2 = b(1);

          // tensor label: I28
          std::unique_ptr<double[]> odata = out()->move_block(a3, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(a3, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(a3, a2), 0.0);

          for (auto& x0 : *range_[1]) {
            for (auto& a1 : *range_[2]) {
              for (auto& x1 : *range_[1]) {
                // tensor label: t2
                std::unique_ptr<double[]> i0data = in(0)->get_block(x0, a1, x1, a2);
                std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x0, a1, x1, a2)]);
                sort_indices<0,1,2,3,0,1,1,1>(i0data, i0data_sorted, x0.size(), a1.size(), x1.size(), a2.size());

                // tensor label: I29
                std::unique_ptr<double[]> i1data = in(1)->get_block(x0, x1, a1, a3);
                std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x0, x1, a1, a3)]);
                sort_indices<0,2,1,3,0,1,1,1>(i1data, i1data_sorted, x0.size(), x1.size(), a1.size(), a3.size());

                dgemm_("T", "N", a2.size(), a3.size(), x0.size()*x1.size()*a1.size(),
                       1.0, i0data_sorted, x0.size()*x1.size()*a1.size(), i1data_sorted, x0.size()*x1.size()*a1.size(),
                       1.0, odata_sorted, a2.size());
              }
            }
          }

          sort_indices<1,0,1,1,1,1>(odata_sorted, odata, a2.size(), a3.size());
          out()->put_block(odata, a3, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task36(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : DensityTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a3 : *range[2])
          subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,2>{{a3, a2}}, in, t[0], range)));
    };
    ~Task36() {};
};


class Task37 : public DensityTask {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index a1 = b(2);
          const Index a3 = b(3);

          // tensor label: I29
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, a1, a3);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, a1, a3)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, a1, a3), 0.0);

          for (auto& x2 : *range_[1]) {
            for (auto& x3 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x3, a1, x2, a3);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x3, a1, x2, a3)]);
              sort_indices<2,0,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), x2.size(), a3.size());

              // tensor label: I30
              std::unique_ptr<double[]> i1data = in(1)->get_block(x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x3, x0, x2, x1)]);
              sort_indices<2,0,1,3,0,1,1,1>(i1data, i1data_sorted, x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a3.size(), x0.size()*x1.size(), x3.size()*x2.size(),
                     1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
                     1.0, odata_sorted, a1.size()*a3.size());
            }
          }

          sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a3.size(), x0.size(), x1.size());
          out()->put_block(odata, x0, x1, a1, a3);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task37(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : DensityTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a3 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, x1, a1, a3}}, in, t[0], range)));
    };
    ~Task37() {};
};


class Task38 : public DensityTask {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index x3 = b(0);
          const Index x0 = b(1);
          const Index x2 = b(2);
          const Index x1 = b(3);

          // tensor label: I30
          std::unique_ptr<double[]> odata = out()->move_block(x3, x0, x2, x1);
          {
            // tensor label: Gamma2
            std::unique_ptr<double[]> i0data = in(0)->get_block(x3, x0, x2, x1);
            sort_indices<0,1,2,3,1,1,1,1>(i0data, odata, x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task38(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : DensityTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x3, x0, x2, x1}}, in, t[0], range)));
    };
    ~Task38() {};
};


class Task39 : public Density2Task {
  protected:
    std::shared_ptr<Tensor> d2_;
    IndexRange closed_;
    IndexRange active_;
    IndexRange virt_;

    void compute_() {
      d2_->zero();
    };

  public:
    Task39(std::vector<std::shared_ptr<Tensor>> t) : Density2Task() {
      d2_ =  t[0];
    };
    ~Task39() {};
};


class Task40 : public Density2Task {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index x1 = b(0);
          const Index a2 = b(1);
          const Index x0 = b(2);
          const Index a1 = b(3);

          // tensor label: Den1
          std::unique_ptr<double[]> odata = out()->move_block(x1, a2, x0, a1);
          {
            // tensor label: I31
            std::unique_ptr<double[]> i0data = in(0)->get_block(x0, x1, a1, a2);
            sort_indices<1,3,0,2,1,1,1,1>(i0data, odata, x0.size(), x1.size(), a1.size(), a2.size());
          }
          out()->put_block(odata, x1, a2, x0, a1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task40(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : Density2Task() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[2]->nblock()*range[1]->nblock()*range[2]->nblock()*range[1]->nblock());
      for (auto& a1 : *range[2])
        for (auto& x0 : *range[1])
          for (auto& a2 : *range[2])
            for (auto& x1 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x1, a2, x0, a1}}, in, t[0], range)));
    };
    ~Task40() {};
};


class Task41 : public Density2Task {
  protected:
    class Task_local : public SubTask<4,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index x0 = b(0);
          const Index x1 = b(1);
          const Index a1 = b(2);
          const Index a2 = b(3);

          // tensor label: I31
          std::unique_ptr<double[]> odata = out()->move_block(x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(x0, x1, a1, a2), 0.0);

          for (auto& x2 : *range_[1]) {
            for (auto& x3 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x3, a1, x2, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x3, a1, x2, a2)]);
              sort_indices<2,0,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), x2.size(), a2.size());

              // tensor label: I32
              std::unique_ptr<double[]> i1data = in(1)->get_block(x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(x3, x0, x2, x1)]);
              sort_indices<2,0,1,3,0,1,1,1>(i1data, i1data_sorted, x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), x0.size()*x1.size(), x3.size()*x2.size(),
                     1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), x0.size(), x1.size());
          out()->put_block(odata, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task41(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : Density2Task() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x0, x1, a1, a2}}, in, t[0], range)));
    };
    ~Task41() {};
};


class Task42 : public Density2Task {
  protected:
    class Task_local : public SubTask<4,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,3> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,4>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,3>& ran)
          : SubTask<4,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index x3 = b(0);
          const Index x0 = b(1);
          const Index x2 = b(2);
          const Index x1 = b(3);

          // tensor label: I32
          std::unique_ptr<double[]> odata = out()->move_block(x3, x0, x2, x1);
          {
            // tensor label: Gamma2
            std::unique_ptr<double[]> i0data = in(0)->get_block(x3, x0, x2, x1);
            sort_indices<0,1,2,3,1,1,1,1>(i0data, odata, x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task42(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,3> range) : Density2Task() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,4>{{x3, x0, x2, x1}}, in, t[0], range)));
    };
    ~Task42() {};
};


class Task43 : public DedciTask {
  protected:
    std::shared_ptr<Tensor> dec_;
    IndexRange closed_;
    IndexRange active_;
    IndexRange virt_;
    IndexRange ci_;

    void compute_() {
      dec_->zero();
    };

  public:
    Task43(std::vector<std::shared_ptr<Tensor>> t) : DedciTask() {
      dec_ =  t[0];
    };
    ~Task43() {};
};


class Task44 : public DedciTask {
  protected:
    class Task_local : public SubTask<1,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,1>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<1,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);

          // tensor label: deci
          std::unique_ptr<double[]> odata = out()->move_block(ci0);
          {
            // tensor label: I33
            std::unique_ptr<double[]> i0data = in(0)->get_block(ci0);
            sort_indices<0,1,1,1,1>(i0data, odata, ci0.size());
          }
          out()->put_block(odata, ci0);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task44(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[3]->nblock());
      for (auto& ci0 : *range[3])
        subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,1>{{ci0}}, in, t[0], range)));
    };
    ~Task44() {};
};


class Task45 : public DedciTask {
  protected:
    class Task_local : public SubTask<1,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,1>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<1,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);

          // tensor label: I33
          std::unique_ptr<double[]> odata = out()->move_block(ci0);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(ci0)]);
          std::fill_n(odata_sorted.get(), out()->get_size(ci0), 0.0);

          for (auto& x0 : *range_[1]) {
            for (auto& a1 : *range_[2]) {
              for (auto& x1 : *range_[1]) {
                for (auto& a2 : *range_[2]) {
                  // tensor label: t2
                  std::unique_ptr<double[]> i0data = in(0)->get_block(x0, a1, x1, a2);
                  std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x0, a1, x1, a2)]);
                  sort_indices<0,1,2,3,0,1,1,1>(i0data, i0data_sorted, x0.size(), a1.size(), x1.size(), a2.size());

                  // tensor label: I34
                  std::unique_ptr<double[]> i1data = in(1)->get_block(ci0, x0, x1, a1, a2);
                  std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(ci0, x0, x1, a1, a2)]);
                  sort_indices<1,3,2,4,0,0,1,1,1>(i1data, i1data_sorted, ci0.size(), x0.size(), x1.size(), a1.size(), a2.size());

                  dgemm_("T", "N", 1, ci0.size(), x0.size()*x1.size()*a1.size()*a2.size(),
                         1.0, i0data_sorted, x0.size()*x1.size()*a1.size()*a2.size(), i1data_sorted, x0.size()*x1.size()*a1.size()*a2.size(),
                         1.0, odata_sorted, 1);
                }
              }
            }
          }

          sort_indices<0,1,1,1,1>(odata_sorted, odata, ci0.size());
          out()->put_block(odata, ci0);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task45(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[3]->nblock());
      for (auto& ci0 : *range[3])
        subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,1>{{ci0}}, in, t[0], range)));
    };
    ~Task45() {};
};


class Task46 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x0 = b(1);
          const Index x1 = b(2);
          const Index a1 = b(3);
          const Index a2 = b(4);

          // tensor label: I34
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(ci0, x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(ci0, x0, x1, a1, a2), 0.0);

          for (auto& x4 : *range_[1]) {
            for (auto& x5 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x5, a1, x4, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x5, a1, x4, a2)]);
              sort_indices<2,0,1,3,0,1,1,1>(i0data, i0data_sorted, x5.size(), a1.size(), x4.size(), a2.size());

              // tensor label: I35
              std::unique_ptr<double[]> i1data = in(1)->get_block(ci0, x5, x0, x4, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(ci0, x5, x0, x4, x1)]);
              sort_indices<3,1,0,2,4,0,1,1,1>(i1data, i1data_sorted, ci0.size(), x5.size(), x0.size(), x4.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), ci0.size()*x0.size()*x1.size(), x5.size()*x4.size(),
                     1.0, i0data_sorted, x5.size()*x4.size(), i1data_sorted, x5.size()*x4.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,4,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), ci0.size(), x0.size(), x1.size());
          out()->put_block(odata, ci0, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task46(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x0, x1, a1, a2}}, in, t[0], range)));
    };
    ~Task46() {};
};


class Task47 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x5 = b(1);
          const Index x0 = b(2);
          const Index x4 = b(3);
          const Index x1 = b(4);

          // tensor label: I35
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x5, x0, x4, x1);
          {
            // tensor label: Gamma12
            std::unique_ptr<double[]> i0data = in(0)->get_block(ci0, x5, x0, x4, x1);
            sort_indices<0,1,2,3,4,1,1,1,2>(i0data, odata, ci0.size(), x5.size(), x0.size(), x4.size(), x1.size());
          }
          out()->put_block(odata, ci0, x5, x0, x4, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task47(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x4 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x5 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x5, x0, x4, x1}}, in, t[0], range)));
    };
    ~Task47() {};
};


class Task48 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x0 = b(1);
          const Index x1 = b(2);
          const Index a1 = b(3);
          const Index a2 = b(4);

          // tensor label: I34
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(ci0, x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(ci0, x0, x1, a1, a2), 0.0);

          for (auto& a3 : *range_[2]) {
            // tensor label: f1
            std::unique_ptr<double[]> i0data = in(0)->get_block(a3, a2);
            std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(a3, a2)]);
            sort_indices<0,1,0,1,1,1>(i0data, i0data_sorted, a3.size(), a2.size());

            // tensor label: I38
            std::unique_ptr<double[]> i1data = in(1)->get_block(ci0, x0, x1, a1, a3);
            std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(ci0, x0, x1, a1, a3)]);
            sort_indices<4,0,1,2,3,0,1,1,1>(i1data, i1data_sorted, ci0.size(), x0.size(), x1.size(), a1.size(), a3.size());

            dgemm_("T", "N", a2.size(), ci0.size()*x0.size()*x1.size()*a1.size(), a3.size(),
                   1.0, i0data_sorted, a3.size(), i1data_sorted, a3.size(),
                   1.0, odata_sorted, a2.size());
          }

          sort_indices<1,2,3,4,0,1,1,1,1>(odata_sorted, odata, a2.size(), ci0.size(), x0.size(), x1.size(), a1.size());
          out()->put_block(odata, ci0, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task48(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x0, x1, a1, a2}}, in, t[0], range)));
    };
    ~Task48() {};
};


class Task49 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x0 = b(1);
          const Index x1 = b(2);
          const Index a1 = b(3);
          const Index a3 = b(4);

          // tensor label: I38
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x0, x1, a1, a3);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(ci0, x0, x1, a1, a3)]);
          std::fill_n(odata_sorted.get(), out()->get_size(ci0, x0, x1, a1, a3), 0.0);

          for (auto& x2 : *range_[1]) {
            for (auto& x3 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x3, a1, x2, a3);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x3, a1, x2, a3)]);
              sort_indices<2,0,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), x2.size(), a3.size());

              // tensor label: I39
              std::unique_ptr<double[]> i1data = in(1)->get_block(ci0, x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(ci0, x3, x0, x2, x1)]);
              sort_indices<3,1,0,2,4,0,1,1,1>(i1data, i1data_sorted, ci0.size(), x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a3.size(), ci0.size()*x0.size()*x1.size(), x3.size()*x2.size(),
                     1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
                     1.0, odata_sorted, a1.size()*a3.size());
            }
          }

          sort_indices<2,3,4,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a3.size(), ci0.size(), x0.size(), x1.size());
          out()->put_block(odata, ci0, x0, x1, a1, a3);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task49(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& a3 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x0, x1, a1, a3}}, in, t[0], range)));
    };
    ~Task49() {};
};


class Task50 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x3 = b(1);
          const Index x0 = b(2);
          const Index x2 = b(3);
          const Index x1 = b(4);

          // tensor label: I39
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x3, x0, x2, x1);
          {
            // tensor label: Gamma13
            std::unique_ptr<double[]> i0data = in(0)->get_block(ci0, x3, x0, x2, x1);
            sort_indices<0,1,2,3,4,1,1,1,1>(i0data, odata, ci0.size(), x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, ci0, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task50(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x3, x0, x2, x1}}, in, t[0], range)));
    };
    ~Task50() {};
};


class Task51 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x0 = b(1);
          const Index x1 = b(2);
          const Index a1 = b(3);
          const Index a2 = b(4);

          // tensor label: I34
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(ci0, x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(ci0, x0, x1, a1, a2), 0.0);

          for (auto& x2 : *range_[1]) {
            for (auto& x3 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x3, a1, x2, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x3, a1, x2, a2)]);
              sort_indices<2,0,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), x2.size(), a2.size());

              // tensor label: I49
              std::unique_ptr<double[]> i1data = in(1)->get_block(ci0, x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(ci0, x3, x0, x2, x1)]);
              sort_indices<3,1,0,2,4,0,1,1,1>(i1data, i1data_sorted, ci0.size(), x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), ci0.size()*x0.size()*x1.size(), x3.size()*x2.size(),
                     1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,4,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), ci0.size(), x0.size(), x1.size());
          out()->put_block(odata, ci0, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task51(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x0, x1, a1, a2}}, in, t[0], range)));
    };
    ~Task51() {};
};


class Task52 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

        double e0_;

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran, const double e)
          : SubTask<5,1>(block, in, out), range_(ran), e0_(e) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x3 = b(1);
          const Index x0 = b(2);
          const Index x2 = b(3);
          const Index x1 = b(4);

          // tensor label: I49
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x3, x0, x2, x1);
          {
            // tensor label: Gamma13
            std::unique_ptr<double[]> i0data = in(0)->get_block(ci0, x3, x0, x2, x1);
            dscal_(ci0.size()*x3.size()*x0.size()*x2.size()*x1.size(), e0_, i0data.get(), 1);
            sort_indices<0,1,2,3,4,1,1,-1,2>(i0data, odata, ci0.size(), x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, ci0, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task52(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range, double e) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x3, x0, x2, x1}}, in, t[0], range, e)));
    };
    ~Task52() {};
};


class Task53 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x0 = b(1);
          const Index x1 = b(2);
          const Index a1 = b(3);
          const Index a2 = b(4);

          // tensor label: I34
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x0, x1, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(ci0, x0, x1, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(ci0, x0, x1, a1, a2), 0.0);

          for (auto& x2 : *range_[1]) {
            for (auto& x3 : *range_[1]) {
              // tensor label: v2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x3, a1, x2, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x3, a1, x2, a2)]);
              sort_indices<2,0,1,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), x2.size(), a2.size());

              // tensor label: I55
              std::unique_ptr<double[]> i1data = in(1)->get_block(ci0, x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(ci0, x3, x0, x2, x1)]);
              sort_indices<3,1,0,2,4,0,1,1,1>(i1data, i1data_sorted, ci0.size(), x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), ci0.size()*x0.size()*x1.size(), x3.size()*x2.size(),
                     1.0, i0data_sorted, x3.size()*x2.size(), i1data_sorted, x3.size()*x2.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,4,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), ci0.size(), x0.size(), x1.size());
          out()->put_block(odata, ci0, x0, x1, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task53(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x1 : *range[1])
            for (auto& x0 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x0, x1, a1, a2}}, in, t[0], range)));
    };
    ~Task53() {};
};


class Task54 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x3 = b(1);
          const Index x0 = b(2);
          const Index x2 = b(3);
          const Index x1 = b(4);

          // tensor label: I55
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x3, x0, x2, x1);
          {
            // tensor label: Gamma13
            std::unique_ptr<double[]> i0data = in(0)->get_block(ci0, x3, x0, x2, x1);
            sort_indices<0,1,2,3,4,1,1,1,1>(i0data, odata, ci0.size(), x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, ci0, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task54(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x3, x0, x2, x1}}, in, t[0], range)));
    };
    ~Task54() {};
};


class Task55 : public DedciTask {
  protected:
    class Task_local : public SubTask<1,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,1>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<1,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);

          // tensor label: I33
          std::unique_ptr<double[]> odata = out()->move_block(ci0);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(ci0)]);
          std::fill_n(odata_sorted.get(), out()->get_size(ci0), 0.0);

          for (auto& x5 : *range_[1]) {
            for (auto& a1 : *range_[2]) {
              for (auto& x4 : *range_[1]) {
                for (auto& a2 : *range_[2]) {
                  // tensor label: t2
                  std::unique_ptr<double[]> i0data = in(0)->get_block(x5, a1, x4, a2);
                  std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x5, a1, x4, a2)]);
                  sort_indices<0,1,2,3,0,1,1,1>(i0data, i0data_sorted, x5.size(), a1.size(), x4.size(), a2.size());

                  // tensor label: I41
                  std::unique_ptr<double[]> i1data = in(1)->get_block(ci0, x5, x4, a1, a2);
                  std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(ci0, x5, x4, a1, a2)]);
                  sort_indices<1,3,2,4,0,0,1,1,1>(i1data, i1data_sorted, ci0.size(), x5.size(), x4.size(), a1.size(), a2.size());

                  dgemm_("T", "N", 1, ci0.size(), x5.size()*x4.size()*a1.size()*a2.size(),
                         1.0, i0data_sorted, x5.size()*x4.size()*a1.size()*a2.size(), i1data_sorted, x5.size()*x4.size()*a1.size()*a2.size(),
                         1.0, odata_sorted, 1);
                }
              }
            }
          }

          sort_indices<0,1,1,1,1>(odata_sorted, odata, ci0.size());
          out()->put_block(odata, ci0);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task55(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[3]->nblock());
      for (auto& ci0 : *range[3])
        subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,1>{{ci0}}, in, t[0], range)));
    };
    ~Task55() {};
};


class Task56 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x5 = b(1);
          const Index x4 = b(2);
          const Index a1 = b(3);
          const Index a2 = b(4);

          // tensor label: I41
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x5, x4, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(ci0, x5, x4, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(ci0, x5, x4, a1, a2), 0.0);

          for (auto& x1 : *range_[1]) {
            for (auto& x0 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x0, a1, x1, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x0, a1, x1, a2)]);
              sort_indices<2,0,1,3,0,1,1,1>(i0data, i0data_sorted, x0.size(), a1.size(), x1.size(), a2.size());

              // tensor label: I42
              std::unique_ptr<double[]> i1data = in(1)->get_block(ci0, x5, x0, x4, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(ci0, x5, x0, x4, x1)]);
              sort_indices<4,2,0,1,3,0,1,1,1>(i1data, i1data_sorted, ci0.size(), x5.size(), x0.size(), x4.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), ci0.size()*x5.size()*x4.size(), x0.size()*x1.size(),
                     1.0, i0data_sorted, x0.size()*x1.size(), i1data_sorted, x0.size()*x1.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,4,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), ci0.size(), x5.size(), x4.size());
          out()->put_block(odata, ci0, x5, x4, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task56(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x4 : *range[1])
            for (auto& x5 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x5, x4, a1, a2}}, in, t[0], range)));
    };
    ~Task56() {};
};


class Task57 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x5 = b(1);
          const Index x0 = b(2);
          const Index x4 = b(3);
          const Index x1 = b(4);

          // tensor label: I42
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x5, x0, x4, x1);
          {
            // tensor label: Gamma12
            std::unique_ptr<double[]> i0data = in(0)->get_block(ci0, x5, x0, x4, x1);
            sort_indices<0,1,2,3,4,1,1,1,2>(i0data, odata, ci0.size(), x5.size(), x0.size(), x4.size(), x1.size());
          }
          out()->put_block(odata, ci0, x5, x0, x4, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task57(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x4 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x5 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x5, x0, x4, x1}}, in, t[0], range)));
    };
    ~Task57() {};
};


class Task58 : public DedciTask {
  protected:
    class Task_local : public SubTask<1,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,1>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<1,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);

          // tensor label: I33
          std::unique_ptr<double[]> odata = out()->move_block(ci0);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(ci0)]);
          std::fill_n(odata_sorted.get(), out()->get_size(ci0), 0.0);

          for (auto& x3 : *range_[1]) {
            for (auto& a1 : *range_[2]) {
              for (auto& x2 : *range_[1]) {
                for (auto& a2 : *range_[2]) {
                  // tensor label: t2
                  std::unique_ptr<double[]> i0data = in(0)->get_block(x3, a1, x2, a2);
                  std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x3, a1, x2, a2)]);
                  sort_indices<0,1,2,3,0,1,1,1>(i0data, i0data_sorted, x3.size(), a1.size(), x2.size(), a2.size());

                  // tensor label: I44
                  std::unique_ptr<double[]> i1data = in(1)->get_block(ci0, x3, x2, a1, a2);
                  std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(ci0, x3, x2, a1, a2)]);
                  sort_indices<1,3,2,4,0,0,1,1,1>(i1data, i1data_sorted, ci0.size(), x3.size(), x2.size(), a1.size(), a2.size());

                  dgemm_("T", "N", 1, ci0.size(), x3.size()*x2.size()*a1.size()*a2.size(),
                         1.0, i0data_sorted, x3.size()*x2.size()*a1.size()*a2.size(), i1data_sorted, x3.size()*x2.size()*a1.size()*a2.size(),
                         1.0, odata_sorted, 1);
                }
              }
            }
          }

          sort_indices<0,1,1,1,1>(odata_sorted, odata, ci0.size());
          out()->put_block(odata, ci0);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task58(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[3]->nblock());
      for (auto& ci0 : *range[3])
        subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,1>{{ci0}}, in, t[0], range)));
    };
    ~Task58() {};
};


class Task59 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x3 = b(1);
          const Index x2 = b(2);
          const Index a1 = b(3);
          const Index a2 = b(4);

          // tensor label: I44
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x3, x2, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(ci0, x3, x2, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(ci0, x3, x2, a1, a2), 0.0);

          for (auto& a3 : *range_[2]) {
            // tensor label: f1
            std::unique_ptr<double[]> i0data = in(0)->get_block(a3, a2);
            std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(a3, a2)]);
            sort_indices<0,1,0,1,1,1>(i0data, i0data_sorted, a3.size(), a2.size());

            // tensor label: I45
            std::unique_ptr<double[]> i1data = in(1)->get_block(ci0, x3, x2, a1, a3);
            std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(ci0, x3, x2, a1, a3)]);
            sort_indices<4,0,1,2,3,0,1,1,1>(i1data, i1data_sorted, ci0.size(), x3.size(), x2.size(), a1.size(), a3.size());

            dgemm_("T", "N", a2.size(), ci0.size()*x3.size()*x2.size()*a1.size(), a3.size(),
                   1.0, i0data_sorted, a3.size(), i1data_sorted, a3.size(),
                   1.0, odata_sorted, a2.size());
          }

          sort_indices<1,2,3,4,0,1,1,1,1>(odata_sorted, odata, a2.size(), ci0.size(), x3.size(), x2.size(), a1.size());
          out()->put_block(odata, ci0, x3, x2, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task59(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x2 : *range[1])
            for (auto& x3 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x3, x2, a1, a2}}, in, t[0], range)));
    };
    ~Task59() {};
};


class Task60 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x3 = b(1);
          const Index x2 = b(2);
          const Index a1 = b(3);
          const Index a3 = b(4);

          // tensor label: I45
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x3, x2, a1, a3);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(ci0, x3, x2, a1, a3)]);
          std::fill_n(odata_sorted.get(), out()->get_size(ci0, x3, x2, a1, a3), 0.0);

          for (auto& x1 : *range_[1]) {
            for (auto& x0 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x0, a1, x1, a3);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x0, a1, x1, a3)]);
              sort_indices<2,0,1,3,0,1,1,1>(i0data, i0data_sorted, x0.size(), a1.size(), x1.size(), a3.size());

              // tensor label: I46
              std::unique_ptr<double[]> i1data = in(1)->get_block(ci0, x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(ci0, x3, x0, x2, x1)]);
              sort_indices<4,2,0,1,3,0,1,1,1>(i1data, i1data_sorted, ci0.size(), x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a3.size(), ci0.size()*x3.size()*x2.size(), x0.size()*x1.size(),
                     1.0, i0data_sorted, x0.size()*x1.size(), i1data_sorted, x0.size()*x1.size(),
                     1.0, odata_sorted, a1.size()*a3.size());
            }
          }

          sort_indices<2,3,4,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a3.size(), ci0.size(), x3.size(), x2.size());
          out()->put_block(odata, ci0, x3, x2, a1, a3);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task60(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& a3 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x2 : *range[1])
            for (auto& x3 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x3, x2, a1, a3}}, in, t[0], range)));
    };
    ~Task60() {};
};


class Task61 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x3 = b(1);
          const Index x0 = b(2);
          const Index x2 = b(3);
          const Index x1 = b(4);

          // tensor label: I46
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x3, x0, x2, x1);
          {
            // tensor label: Gamma13
            std::unique_ptr<double[]> i0data = in(0)->get_block(ci0, x3, x0, x2, x1);
            sort_indices<0,1,2,3,4,1,1,1,1>(i0data, odata, ci0.size(), x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, ci0, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task61(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x3, x0, x2, x1}}, in, t[0], range)));
    };
    ~Task61() {};
};


class Task62 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x3 = b(1);
          const Index x2 = b(2);
          const Index a1 = b(3);
          const Index a2 = b(4);

          // tensor label: I44
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x3, x2, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(ci0, x3, x2, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(ci0, x3, x2, a1, a2), 0.0);

          for (auto& x1 : *range_[1]) {
            for (auto& x0 : *range_[1]) {
              // tensor label: t2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x0, a1, x1, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x0, a1, x1, a2)]);
              sort_indices<2,0,1,3,0,1,1,1>(i0data, i0data_sorted, x0.size(), a1.size(), x1.size(), a2.size());

              // tensor label: I52
              std::unique_ptr<double[]> i1data = in(1)->get_block(ci0, x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(ci0, x3, x0, x2, x1)]);
              sort_indices<4,2,0,1,3,0,1,1,1>(i1data, i1data_sorted, ci0.size(), x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), ci0.size()*x3.size()*x2.size(), x0.size()*x1.size(),
                     1.0, i0data_sorted, x0.size()*x1.size(), i1data_sorted, x0.size()*x1.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,4,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), ci0.size(), x3.size(), x2.size());
          out()->put_block(odata, ci0, x3, x2, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task62(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x2 : *range[1])
            for (auto& x3 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x3, x2, a1, a2}}, in, t[0], range)));
    };
    ~Task62() {};
};


class Task63 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

        double e0_;

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran, const double e)
          : SubTask<5,1>(block, in, out), range_(ran), e0_(e) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x3 = b(1);
          const Index x0 = b(2);
          const Index x2 = b(3);
          const Index x1 = b(4);

          // tensor label: I52
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x3, x0, x2, x1);
          {
            // tensor label: Gamma13
            std::unique_ptr<double[]> i0data = in(0)->get_block(ci0, x3, x0, x2, x1);
            dscal_(ci0.size()*x3.size()*x0.size()*x2.size()*x1.size(), e0_, i0data.get(), 1);
            sort_indices<0,1,2,3,4,1,1,-1,2>(i0data, odata, ci0.size(), x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, ci0, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task63(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range, double e) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x3, x0, x2, x1}}, in, t[0], range, e)));
    };
    ~Task63() {};
};


class Task64 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,2> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,2>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,2>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x3 = b(1);
          const Index x2 = b(2);
          const Index a1 = b(3);
          const Index a2 = b(4);

          // tensor label: I44
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x3, x2, a1, a2);
          std::unique_ptr<double[]> odata_sorted(new double[out()->get_size(ci0, x3, x2, a1, a2)]);
          std::fill_n(odata_sorted.get(), out()->get_size(ci0, x3, x2, a1, a2), 0.0);

          for (auto& x1 : *range_[1]) {
            for (auto& x0 : *range_[1]) {
              // tensor label: v2
              std::unique_ptr<double[]> i0data = in(0)->get_block(x0, a1, x1, a2);
              std::unique_ptr<double[]> i0data_sorted(new double[in(0)->get_size(x0, a1, x1, a2)]);
              sort_indices<2,0,1,3,0,1,1,1>(i0data, i0data_sorted, x0.size(), a1.size(), x1.size(), a2.size());

              // tensor label: I58
              std::unique_ptr<double[]> i1data = in(1)->get_block(ci0, x3, x0, x2, x1);
              std::unique_ptr<double[]> i1data_sorted(new double[in(1)->get_size(ci0, x3, x0, x2, x1)]);
              sort_indices<4,2,0,1,3,0,1,1,1>(i1data, i1data_sorted, ci0.size(), x3.size(), x0.size(), x2.size(), x1.size());

              dgemm_("T", "N", a1.size()*a2.size(), ci0.size()*x3.size()*x2.size(), x0.size()*x1.size(),
                     1.0, i0data_sorted, x0.size()*x1.size(), i1data_sorted, x0.size()*x1.size(),
                     1.0, odata_sorted, a1.size()*a2.size());
            }
          }

          sort_indices<2,3,4,0,1,1,1,1,1>(odata_sorted, odata, a1.size(), a2.size(), ci0.size(), x3.size(), x2.size());
          out()->put_block(odata, ci0, x3, x2, a1, a2);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task64(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,2> in = {{t[1], t[2]}};

      subtasks_.reserve(range[2]->nblock()*range[2]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& a2 : *range[2])
        for (auto& a1 : *range[2])
          for (auto& x2 : *range[1])
            for (auto& x3 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x3, x2, a1, a2}}, in, t[0], range)));
    };
    ~Task64() {};
};


class Task65 : public DedciTask {
  protected:
    class Task_local : public SubTask<5,1> {
      protected:
        const std::array<std::shared_ptr<const IndexRange>,4> range_;

        const Index& b(const size_t& i) const { return this->block(i); }
        const std::shared_ptr<const Tensor>& in(const size_t& i) const { return this->in_tensor(i); }
        const std::shared_ptr<Tensor>& out() const { return this->out_tensor(); }

      public:
        Task_local(const std::array<const Index,5>& block, const std::array<std::shared_ptr<const Tensor>,1>& in, std::shared_ptr<Tensor>& out,
                   std::array<std::shared_ptr<const IndexRange>,4>& ran)
          : SubTask<5,1>(block, in, out), range_(ran) { }


        void compute() override {
          const Index ci0 = b(0);
          const Index x3 = b(1);
          const Index x0 = b(2);
          const Index x2 = b(3);
          const Index x1 = b(4);

          // tensor label: I58
          std::unique_ptr<double[]> odata = out()->move_block(ci0, x3, x0, x2, x1);
          {
            // tensor label: Gamma13
            std::unique_ptr<double[]> i0data = in(0)->get_block(ci0, x3, x0, x2, x1);
            sort_indices<0,1,2,3,4,1,1,1,1>(i0data, odata, ci0.size(), x3.size(), x0.size(), x2.size(), x1.size());
          }
          out()->put_block(odata, ci0, x3, x0, x2, x1);
        }
    };

    std::vector<std::shared_ptr<Task_local>> subtasks_;

    void compute_() override {
      for (auto& i : subtasks_) {
        i->compute();
      }
    }

  public:
    Task65(std::vector<std::shared_ptr<Tensor>> t, std::array<std::shared_ptr<const IndexRange>,4> range) : DedciTask() {
      std::array<std::shared_ptr<const Tensor>,1> in = {{t[1]}};

      subtasks_.reserve(range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[1]->nblock()*range[3]->nblock());
      for (auto& x1 : *range[1])
        for (auto& x2 : *range[1])
          for (auto& x0 : *range[1])
            for (auto& x3 : *range[1])
              for (auto& ci0 : *range[3])
                subtasks_.push_back(std::shared_ptr<Task_local>(new Task_local(std::array<const Index,5>{{ci0, x3, x0, x2, x1}}, in, t[0], range)));
    };
    ~Task65() {};
};


}
}
}
#endif

