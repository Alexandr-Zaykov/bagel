//
// BAGEL - Parallel electron correlation program.
// Filename: multi/rasscfh/rasscf.h
// Copyright (C) 2015 Toru Shiozaki
//
// Author: Inkoo Kim <inkoo.kim@northwestern.edu>
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

// This is a base class for various RASSCF solvers.
// The assumpation is made that the CI and orbital optimizations are done independently.
// This should be a good strategy in large systems.
//

#ifndef __BAGEL_RASSCF_RASSCF_H
#define __BAGEL_RASSCF_RASSCF_H

#include <src/wfn/reference.h>
#include <src/ci/ras/rasci.h>
#include <src/multi/rasscf/rotfile.h>

namespace bagel {

class RASSCF : public Method, public std::enable_shared_from_this<RASSCF> {

  protected:
    // some internal information
    int nocc_; // sum of nact_ + nclosed_
    int nclosed_;
    int nact_;
    int nvirt_;
    // number of MO orbitals. TODO rename to norb. "nbasis" is confusing.
    int nbasis_;
    int nstate_;
    int max_iter_;
    int max_micro_iter_;
    double thresh_;
    double thresh_micro_;
    bool natocc_;

    std::array<int,3> ras_;

    VectorB occup_;
    std::shared_ptr<const Coeff> coeff_;

    std::shared_ptr<RASCI> rasci_;
    void print_header() const;
    void print_iteration(int iter, int miter, int tcount, const std::vector<double> energy, const double error, const double time) const;
    void common_init();

    void mute_stdcout();
    void resume_stdcout();

    const std::shared_ptr<const Matrix> hcore_;

    std::shared_ptr<const Coeff> update_coeff(const std::shared_ptr<const Matrix> cold, std::shared_ptr<const Matrix> natorb) const;
    std::shared_ptr<const Coeff> semi_canonical_orb() const;
    std::shared_ptr<Matrix> form_natural_orbs();

    std::vector<double> schwarz_;

    // energy
    std::vector<double> energy_;
    double rms_grad_;

  public:
    RASSCF(const std::shared_ptr<const PTree> idat, const std::shared_ptr<const Geometry> geom, const std::shared_ptr<const Reference> = nullptr);
    virtual ~RASSCF();

    virtual void compute() override = 0;

    std::shared_ptr<const Reference> ref() const { return ref_; }
    virtual std::shared_ptr<const Reference> conv_to_ref() const override;

    std::shared_ptr<RASCI> rasci() { return rasci_; }
    std::shared_ptr<const RASCI> rasci() const { return rasci_; }

    // functions to retrieve protected members
    int nocc() const { return nocc_; }
    int nclosed() const { return nclosed_; }
    int nact() const { return nact_; }
    int nvirt() const { return nvirt_; }
    int nbasis() const { return nbasis_; }
    int nstate() const { return nstate_; }
    int max_iter() const { return max_iter_; }
    int max_micro_iter() const { return max_micro_iter_; }
    double thresh() const { return thresh_; }
    double thresh_micro() const { return thresh_micro_; }

    void set_occup(const VectorB& o) { occup_ = o; }
    double occup(const int i) const { return occup_(i); }

    double energy(const int i) const { return energy_[i]; }
    double energy_av() const { return blas::average(energy_); }
    const std::vector<double>& energy() const { return energy_; }
    double rms_grad() const { return rms_grad_; }

    // TODO I need this function in CP-RASSCF, but only for denominator. Should be separated.
    void one_body_operators(std::shared_ptr<Matrix>&, std::shared_ptr<Matrix>&, std::shared_ptr<Matrix>&, std::shared_ptr<Matrix>&,
                            std::shared_ptr<RASRotFile>&, const bool superci=true) const;
    std::shared_ptr<Matrix> ao_rdm1(std::shared_ptr<const RDM<1>> rdm1, const bool inactive_only = false) const;
    std::shared_ptr<const Matrix> hcore() const { return hcore_; }

    std::shared_ptr<const Coeff> coeff() const { return coeff_; }
    void print_natocc() const;
};

static const double roccup_thresh = 1.0e-10;

}

#endif
