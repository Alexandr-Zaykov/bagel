//
// BAGEL - Parallel electron correlation program.
// Filename: asd/orbopt/ras/rasscf.cc
// Copyright (C) 2015 Toru Shiozaki
//
// Author: Inkoo Kim: <inkoo.kim@northwestern.edu>
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

// This is a base class for various ASDRASSCF solvers.
// The assumpation is made that the CI and orbital optimizations are done independently.
// This should be a good strategy in large systems.
//

#ifndef __BAGEL_ASDRASSCF_ASDRASSCF_H
#define __BAGEL_ASDRASSCF_ASDRASSCF_H

#include <src/wfn/reference.h>
#include <src/ci/fci/knowles.h>
#include <src/asd/orbopt/ras/rasrotfile.h>

#include <src/df/dfblock.h>
#include <src/asd/dimer/dimer.h>


namespace bagel {

class ASDRASSCF : public Method, public std::enable_shared_from_this<ASDRASSCF> {

  protected:
//ADDED
    std::shared_ptr<Dimer> dimer_;
    std::shared_ptr<PTree> asdinput_;
//  std::shared_ptr<const Geometry> geom_;
//  std::shared_ptr<const Reference> ref_;
    std::tuple<std::shared_ptr<RDM<1>>,
               std::shared_ptr<RDM<2>>> rdms_; //untransformed RDM
    std::shared_ptr<RDM<1>> rdm1_; //store transformed rdm (natural orb)
    std::shared_ptr<RDM<2>> rdm2_; // same
//END ADDED
    // some internal information
    int nocc_; // sum of nact_ + nclosed_
    int nclosed_;
    int nact_;
    int nvirt_;
    //ASD
    std::array<int,3> rasA_;
    std::array<int,3> rasB_;
    int nactA_;
    int nactB_;
    // number of MO orbitals. TODO rename to norb. "nbasis" is confusing.
    int nbasis_;
    int nstate_;
    int istate_;
    int max_iter_;
    int max_micro_iter_;
    double thresh_;
    double thresh_micro_;
    bool natocc_;

    VectorB occup_;
    std::shared_ptr<const Coeff> coeff_;

    //TODO: remove fci_
  //std::shared_ptr<FCI> fci_;

    void print_header() const;
    void print_iteration(int iter, int miter, int tcount, const std::vector<double> energy, const double error, const double time) const;
    void common_init();

    void mute_stdcout();
    void resume_stdcout();

    std::shared_ptr<const Matrix> hcore_;

    std::shared_ptr<const Coeff> update_coeff(const std::shared_ptr<const Matrix> cold, std::shared_ptr<const Matrix> natorb) const;
    std::shared_ptr<const Coeff> semi_canonical_orb() const;
    std::shared_ptr<Matrix> form_natural_orbs();

    std::vector<double> schwarz_;

    // energy
    std::vector<double> energy_;
    double rms_grad_;

//ADDED(from fci.cc, fci_rdm.cc)
    std::shared_ptr<DFHalfDist> half_;
    //natural
    std::pair<std::shared_ptr<Matrix>, VectorB> natorb_convert();
    void update_rdms(const std::shared_ptr<Matrix>& coeff);
    std::shared_ptr<Matrix> Qvec(const int n, const int m, std::shared_ptr<const Matrix> c, const size_t nclosed) const;
    double check_symmetric(std::shared_ptr<Matrix>& mat) const;
//END ADDED


  public:
    ASDRASSCF(const std::shared_ptr<const PTree> idat, const std::shared_ptr<const Geometry> geom, const std::shared_ptr<const Reference> = nullptr);

  //ASDRASSCF(const std::shared_ptr<const PTree> idat, const std::shared_ptr<const Geometry> geom, const std::shared_ptr<const Reference> ref,
  //           std::tuple<std::shared_ptr<RDM<1>>,
  //                      std::shared_ptr<RDM<2>>> rdms );
    virtual ~ASDRASSCF();

    virtual void compute() override = 0;

    std::shared_ptr<const Reference> ref() const { return ref_; };
    virtual std::shared_ptr<const Reference> conv_to_ref() const override;

  //std::shared_ptr<FCI> fci() { return fci_; };
  //std::shared_ptr<const FCI> fci() const { return fci_; };

    // functions to retrieve protected members
    int nocc() const { return nocc_; }
    int nclosed() const { return nclosed_; }
    int nact() const { return nact_; }
    //ASD
    int nactA() const { return nactA_; }
    int nactB() const { return nactB_; }
    std::array<int,3> rasA() const { return rasA_; }
    std::array<int,3> rasB() const { return rasB_; }

    int nvirt() const { return nvirt_; }
    int nbasis() const { return nbasis_; }
    int nstate() const { return nstate_; }
    int istate() const { return istate_; }
    int max_iter() const { return max_iter_; }
    int max_micro_iter() const { return max_micro_iter_; }
    double thresh() const { return thresh_; }
    double thresh_micro() const { return thresh_micro_; }

    void set_occup(const VectorB& o) { occup_ = o; };
    double occup(const int i) const { return occup_(i); }

    double energy(const int i) const { return energy_[i]; };
    double energy() const { return energy_[istate_]; };
    double rms_grad() const { return rms_grad_; };

    // TODO I need this function in CP-ASDRASSCF, but only for denominator. Should be separated.
    void one_body_operators(std::shared_ptr<Matrix>&, std::shared_ptr<Matrix>&, std::shared_ptr<Matrix>&, std::shared_ptr<Matrix>&,
                            std::shared_ptr<ASDRASRotFile>&, const bool superci=true) const;
    std::shared_ptr<Matrix> ao_rdm1(std::shared_ptr<const RDM<1>> rdm1, const bool inactive_only = false) const;
    std::shared_ptr<const Matrix> hcore() const { return hcore_; };

    std::shared_ptr<const Coeff> coeff() const { return coeff_; };
    void print_natocc() const;
};

//TODO: commented as it has already defined in asdscf.h
//static const double occup_thresh_ = 1.0e-10;
const double occup_thresh3_ = 1.0e-10;

}

#endif
