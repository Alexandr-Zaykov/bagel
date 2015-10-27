//
// BAGEL - Parallel electron correlation program.
// Filename: ericompute_impl.cpp
// Copyright (C) 2014 Toru Shiozaki
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

// This file defines functions with declarations in ericompute.h, used to generate comparison data for the debugging of the London orbital ERI.

#include <iostream>
#include <iomanip>
#include <cassert>
#include <complex>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <utility>
#include "polynomial.h"
#include "ericompute.h"
#include "../comperirootlist.h"

using namespace std;
const static bagel::ComplexERIRootList mapT;

namespace test {

std::complex<double> overlap_Ix (const int dimension, const std::vector<double> field, atomic_orbital A_, atomic_orbital B_) {

  const int a = A_.angular_momentum[dimension];
  const int b = B_.angular_momentum[dimension];
  assert(a >= 0 && b >= 0);
  const int ab = a + b;
  const double alpha = A_.exponent;
  const double beta  = B_.exponent;
  const double p = alpha + beta;
  const double A = A_.position[dimension];
  const double B = B_.position[dimension];
  const double BA = B - A;
  const double P = (alpha*A + beta*B)/p;
  const double A_BA = B_.vector_potential[dimension] - A_.vector_potential[dimension];
  const std::complex<double> Pprime ( P , -0.5/p*A_BA );
  const double BAsq = BA*BA;
  const double A_BAsq = A_BA * A_BA;
  const double PdotA_BA = P*A_BA;
  const double E_AB = std::exp(-alpha*beta/p*BAsq);
  const std::complex<double> X_P = std::polar ( std::exp( -1.0/(4.0*p)*A_BAsq ) , (-1.0*PdotA_BA ) );
  const double pip = std::sqrt(pi/p);
  const std::complex<double> ss = E_AB * X_P * pip;

  std::vector<std::vector<std::complex<double>>> Iab;

  // set vector size and instantiate each number with the ss value.
  Iab.resize(ab+1);
  for (int i=0; i<=ab; i++) {
    Iab[i].resize(ab+1,ss);
  }

  const std::complex<double> PA = Pprime - A;
  const std::complex<double> PB = Pprime - B;

  // vertical recurrence relations (VRR)
  for (int n=0; n!=ab; n++) {
    for (int i=0; i<=n; i++) {
      std::complex<double> term1 = 0.0;
      std::complex<double> term2 = 0.0;
      std::complex<double> term3 = 0.0;
      if (n-i) term1 = 0.5*(n-i)/p * Iab[n-i-1][i];
      if (i) term2 = 0.5*(i)/p * Iab[n-i][i-1];
      term3 = PA * Iab[n-i][i];
      Iab[n-i+1][i] = term1 + term2 + term3;
    }
    std::complex<double> term1 = 0.0;
    std::complex<double> term2 = 0.0;
    if (n) term1 = 0.5*n/p*Iab[0][n-1];
    term2 = PB*Iab[0][n];
    Iab[0][n+1] = term1 + term2;
  }
#if 0
  char dim;
  if (dimension==0) dim = 'x';
  if (dimension==1) dim = 'y';
  if (dimension==2) dim = 'z';
  cout << endl;
  cout << "a = " << a << endl;
  cout << "b = " << b << endl;
  cout << "alpha = " << alpha << endl;
  cout << "beta = " << beta << endl;
  cout << "A = " << A << endl;
  cout << "B = " << B << endl;
  cout << "A_BA = " << A_BA << endl;
  cout << "Pbar = " << Pprime << endl;
  cout << "A_BAsq = " << A_BAsq << endl;
  cout << "PdotA_BA = " << PdotA_BA << endl;
  cout << "X_P = " << X_P << endl;
  cout << "X_Pmag = " << abs(X_P) << endl;
  cout << "X_Pmag = " << std::exp( -1.0/(4.0*p)*A_BAsq) << endl;
  cout << "ss_" << dim << " = " << ss << endl;
  cout << "I_" << dim << " = " << Iab[a][b]/ss << std::endl;
#endif
  return Iab[a][b];
}

complex<double> get_eri_matrix_element (const vector<double> field, atomic_orbital A_, atomic_orbital B_, atomic_orbital C_, atomic_orbital D_) {

  polynomial<complex<double>> III = get_ERI_III (field, A_, B_, C_, D_);
  const int nroot = ( III.rank/2 + 1 );

  pair<complex<double>,complex<double>> ssss =  compute_eri_ssss(field, A_, B_, C_, D_);

  // Here we assign the values of each T we will evaluate
  complex<double> Ts[1] = {ssss.first};

  // Evaluate each one first using the mapping in the generated files
  complex<double> mapt[nroot];
  complex<double> mapw[nroot];
  assert (nroot < 14);
  mapT.root(nroot, Ts, mapt, mapw, 1);

  // Evaluate the integral by Gaussian quadrature
  complex<double> quad_term;
  complex<double> quad_integral (0,0);
  for(int j = 0; j!=nroot; j++){                 // For each root and weight pair...
    quad_term = mapw[j] * III.evaluate(mapt[j]);
    quad_integral += quad_term;
  }
  complex<double> ERI = quad_integral * ssss.second;

#if 0
  cout << "This matrix element = " << ERI << endl;
  cout << "Magnitude = " << abs(ERI) << endl;
  cout << "Phase = "     << arg(ERI) << endl;
  cout << "   ssss coeff = " << ssss.second << endl;
  cout << "   T = " << ssss.first << endl;
  cout << "   # roots needed = " << nroot << endl;
  cout << "   Quadrature (sum of w_i P(t_i) terms) = " << quad_integral << endl;
  cout << "   III(t) = ";
  III.show();
  cout << "   The roots are:   ";
  for(int i=0; i!=nroot; i++) cout << mapt[i] << " ";
  cout << endl << "   The weights are: ";
  for(int i=0; i!=nroot; i++) cout << mapw[i] << " ";
  cout << endl;
#endif
  return ERI;
}

complex<double> overlap_MO (vector<double> field, molecular_orbital A_, molecular_orbital B_, vector<atomic_orbital> basis) {
  const int nbasis = basis.size();
  complex<double> coeff_prod;
  complex<double> current_term;
  complex<double> Full_overlap = 0.0;
  for (int i = 0; i!=nbasis; i++) {
    for (int j = 0; j!=nbasis; j++) {
      coeff_prod = conj(A_.coefficient[i]) * B_.coefficient[j];
      if (abs(coeff_prod)) {
        current_term = overlap(field, basis[i], basis[j]);
        Full_overlap += (coeff_prod * current_term);
      }
    }
  }
  return Full_overlap;
}

std::complex<double> overlap (const std::vector<double> field, atomic_orbital A_, atomic_orbital B_) {
  const std::complex<double> Ix = overlap_Ix (0, field, A_, B_);
  const std::complex<double> Iy = overlap_Ix (1, field, A_, B_);
  const std::complex<double> Iz = overlap_Ix (2, field, A_, B_);
  const std::complex<double> out = Ix * Iy * Iz * A_.prefactor * B_.prefactor;
  return out;
}

void atomic_orbital::set_data (const double* pos, const double exp, const int* ang_mom, const std::vector<double> field) {
  for (int i = 0; i!=3; i++) {
    position[i] = pos[i];
    angular_momentum[i] = ang_mom[i];
  }
#if 1  // For London orbitals
  vector_potential[0] = 0.5*(field[1]*pos[2] - field[2]*pos[1]);
  vector_potential[1] = 0.5*(field[2]*pos[0] - field[0]*pos[2]);
  vector_potential[2] = 0.5*(field[0]*pos[1] - field[1]*pos[0]);
#else  // For common origin
  vector_potential[0] = 0.0;
  vector_potential[1] = 0.0;
  vector_potential[2] = 0.0;
#endif
  exponent = exp;
  prefactor = 1.0;
}

void atomic_orbital::change_angular (const int ax, const int ay, const int az) {
  angular_momentum[0] = ax;
  angular_momentum[1] = ay;
  angular_momentum[2] = az;
}

// Run the VRR and HRR to account for angular momentum in one particular dimension
test::polynomial<std::complex<double>> get_ERI_Ix (const int dimension, const std::vector<double> field, atomic_orbital A_, atomic_orbital B_, atomic_orbital C_, atomic_orbital D_) {

  const int a = A_.angular_momentum[dimension];
  const int b = B_.angular_momentum[dimension];
  const int c = C_.angular_momentum[dimension];
  const int d = D_.angular_momentum[dimension];
  const int ab = a + b;
  const int cd = c + d;
  const int abcd = ab + cd;

  std::vector<std::complex<double>> C00v = {0.0,0.0};
  std::vector<std::complex<double>> D00v = {0.0,0.0};
  std::vector<std::complex<double>> B00v = {0.0,0.0};
  std::vector<std::complex<double>> B10v = {0.0,0.0};
  std::vector<std::complex<double>> B01v = {0.0,0.0};

  const std::complex<double> BA = B_.position[dimension] - A_.position[dimension];
  const std::complex<double> DC = D_.position[dimension] - C_.position[dimension];

  {
    const double alpha = A_.exponent;
    const double beta  = B_.exponent;
    const double gamma = C_.exponent;
    const double delta = D_.exponent;
    const double p = alpha + beta;
    const double q = gamma + delta;
    std::complex<double> Abar;
    std::complex<double> Bbar;
    std::complex<double> Cbar;
    std::complex<double> Dbar;
    {
      Abar.real(A_.position[dimension]);
      Bbar.real(B_.position[dimension]);
      Cbar.real(C_.position[dimension]);
      Dbar.real(D_.position[dimension]);
      Abar.imag((alpha==0)?0.0:(-0.5*A_.vector_potential[dimension]/alpha));
      Bbar.imag((beta==0)?0.0:(-0.5*B_.vector_potential[dimension]/beta));
      Cbar.imag((gamma==0)?0.0:(-0.5*C_.vector_potential[dimension]/gamma));
      Dbar.imag((delta==0)?0.0:(-0.5*D_.vector_potential[dimension]/delta));
    }
    const std::complex<double> Pbar = (alpha*conj(Abar) + beta*Bbar)/p;
    const std::complex<double> Qbar = (gamma*conj(Cbar) + delta*Dbar)/q;
    C00v[0] = ( Pbar - A_.position[dimension] );
    D00v[0] = ( Qbar - C_.position[dimension] );
    B00v[0] = ( 0.0 );
    B10v[0] = ( 0.5/p );
    B01v[0] = ( 0.5/q );
    C00v[1] = (-1.0*q/(p+q)*(Pbar-Qbar) );
    D00v[1] = ( 1.0*p/(p+q)*(Pbar-Qbar) );
    B00v[1] = ( 0.5/(p+q) );
    B10v[1] = (-0.5*q/(p*(p+q)) );
    B01v[1] = (-0.5*p/(q*(p+q)) );
  }

  const test::polynomial<std::complex<double>> C00 (C00v);
  const test::polynomial<std::complex<double>> D00 (D00v);
  const test::polynomial<std::complex<double>> B00 (B00v);
  const test::polynomial<std::complex<double>> B10 (B10v);
  const test::polynomial<std::complex<double>> B01 (B01v);

#if 0
  std::cout << "C00 = "; C00.show();
  std::cout << "D00 = "; D00.show();
  std::cout << "B00 = "; B00.show();
  std::cout << "B10 = "; B10.show();
  std::cout << "B01 = "; B01.show();
  std::cout << "angular momentum indices: " << a << ", " << b << ", " << c << ", " << d << std::endl;
#endif

  std::vector<std::vector<std::vector<std::vector<test::polynomial<std::complex<double>>>>>> Iabcd;
  std::vector<std::complex<double>> one = {1.0};
  std::vector<std::complex<double>> zero = {0.0};
  test::polynomial<std::complex<double>> ssss (one);

  // set vector size and instantiate each polynomial as 1
  Iabcd.resize(abcd+1);
  for (int i=0; i<=abcd; i++) {
    Iabcd[i].resize(b+1);
    for (int j=0; j<=b; j++) {
      Iabcd[i][j].resize(abcd+1);
      for (int k=0; k<=abcd; k++) {
        Iabcd[i][j][k].resize(d+1,ssss);
      }
    }
  }

  // vertical recurrence relations (VRR)
  const std::complex<double> uno = 1.0;
  for (int n=0; n!=abcd; n++) {
    for (int i=0; i<=n; i++) {
      const std::complex<double> aa = i;
      const std::complex<double> cc = n-i;
      polynomial<std::complex<double>> term1 (zero);
      polynomial<std::complex<double>> term2 (zero);
      polynomial<std::complex<double>> term3 (zero);
      term1 = scalar_polynomial ( multiply_polynomials ( C00, Iabcd[i][0][n-i][0]   ), uno);
      if (i)   term2 = scalar_polynomial ( multiply_polynomials ( B10, Iabcd[i-1][0][n-i][0] ), aa );
      if (n-i) term3 = scalar_polynomial ( multiply_polynomials ( B00, Iabcd[i][0][n-i-1][0] ), cc );
      Iabcd[i+1][0][n-i][0] = add_polynomials ( term1 , add_polynomials ( term2 , term3 ) );
    }
    const std::complex<double> cc = n;
    polynomial<std::complex<double>> term1 (zero);
    polynomial<std::complex<double>> term2 (zero);
    term1 = scalar_polynomial ( multiply_polynomials ( D00, Iabcd[0][0][n][0]   ), uno);
    if (n) term2 = scalar_polynomial ( multiply_polynomials ( B01, Iabcd[0][0][n-1][0] ), cc );
    Iabcd[0][0][n+1][0] = add_polynomials ( term1 , term2 );
  }

  // horizontal recurrence relations (HRR)
  for (int k=0; k<=cd; k++) {
    for (int n=0; n!=b; n++) {
      for (int i=0; i!=b-n; i++) {
        polynomial<std::complex<double>> term1 = Iabcd[a+i+1][n][k][0];
        polynomial<std::complex<double>> term2 = scalar_polynomial ( Iabcd[a+i][n][k][0] , BA );
        Iabcd[a+i][n+1][k][0] = subtract_polynomials ( term1, term2 );
      }
    }
  }

  for (int n=0; n!=d; n++) {
    for (int i=0; i!=d-n; i++) {
      polynomial<std::complex<double>> term1 = Iabcd[a][b][c+i+1][n];
      polynomial<std::complex<double>> term2 = scalar_polynomial ( Iabcd[a][b][c+i][n] , DC );
      Iabcd[a][b][c+i][n+1] = subtract_polynomials ( term1, term2 );
    }
  }

  return Iabcd[a][b][c][d];
}

std::pair<std::complex<double>,std::complex<double>> compute_eri_ssss (const std::vector<double> field, atomic_orbital A_, atomic_orbital B_, atomic_orbital C_, atomic_orbital D_) {

  const double* A = A_.position;
  const double* B = B_.position;
  const double* C = C_.position;
  const double* D = D_.position;

  const double alpha = A_.exponent;
  const double beta  = B_.exponent;
  const double gamma = C_.exponent;
  const double delta = D_.exponent;
  const double p = alpha + beta;
  const double q = gamma + delta;
  const double rho = p*q/(p+q);
#if 1 // For London orbitals
  double A_A[3] = { 0.5*(field[1]*A[2] - field[2]*A[1]), 0.5*(field[2]*A[0] - field[0]*A[2]), 0.5*(field[0]*A[1] - field[1]*A[0]) };
  double A_B[3] = { 0.5*(field[1]*B[2] - field[2]*B[1]), 0.5*(field[2]*B[0] - field[0]*B[2]), 0.5*(field[0]*B[1] - field[1]*B[0]) };
  double A_C[3] = { 0.5*(field[1]*C[2] - field[2]*C[1]), 0.5*(field[2]*C[0] - field[0]*C[2]), 0.5*(field[0]*C[1] - field[1]*C[0]) };
  double A_D[3] = { 0.5*(field[1]*D[2] - field[2]*D[1]), 0.5*(field[2]*D[0] - field[0]*D[2]), 0.5*(field[0]*D[1] - field[1]*D[0]) };
#else // For common origin
  double A_A[3] = { 0.0, 0.0, 0.0 };
  double A_B[3] = { 0.0, 0.0, 0.0 };
  double A_C[3] = { 0.0, 0.0, 0.0 };
  double A_D[3] = { 0.0, 0.0, 0.0 };
#endif
  if (alpha == 0.0 || beta == 0.0) {
    for (int i=0; i!=3; i++) A_A[i] = 0.0;
    for (int i=0; i!=3; i++) A_B[i] = 0.0;
  }
  if (gamma == 0.0 || delta == 0.0) {
    for (int i=0; i!=3; i++) A_C[i] = 0.0;
    for (int i=0; i!=3; i++) A_D[i] = 0.0;
  }

  complex<double> Abar[3];
  complex<double> Bbar[3];
  complex<double> Cbar[3];
  complex<double> Dbar[3];
  complex<double> Pbar[3];
  complex<double> Qbar[3];
  for (int i=0; i!=3; i++) {
    Abar[i].real(A[i]);
    Abar[i].imag((alpha==0)?0.0:(-0.5*A_A[i]/alpha));
    Bbar[i].real(B[i]);
    Bbar[i].imag((beta==0)?0.0:(-0.5*A_B[i]/beta));
    Cbar[i].real(C[i]);
    Cbar[i].imag((gamma==0)?0.0:(-0.5*A_C[i]/gamma));
    Dbar[i].real(D[i]);
    Dbar[i].imag((delta==0)?0.0:(-0.5*A_D[i]/delta));
    Pbar[i] = (alpha*conj(Abar[i]) + beta*Bbar[i])/p;
    Qbar[i] = (gamma*conj(Cbar[i]) + delta*Dbar[i])/q;
  }

  const complex<double> X_A = (alpha==0) ? 1.0 : std::polar ( std::exp( -1.0/(4.0*alpha)*(A_A[0]*A_A[0] + A_A[1]*A_A[1] + A_A[2]*A_A[2]) ) , (-1.0*( A[0]*A_A[0] + A[1]*A_A[1] + A[2]*A_A[2] ) ) );
  const complex<double> X_B = (beta ==0) ? 1.0 : std::polar ( std::exp( -1.0/(4.0*beta )*(A_B[0]*A_B[0] + A_B[1]*A_B[1] + A_B[2]*A_B[2]) ) , (-1.0*( B[0]*A_B[0] + B[1]*A_B[1] + B[2]*A_B[2] ) ) );
  const complex<double> X_C = (gamma==0) ? 1.0 : std::polar ( std::exp( -1.0/(4.0*gamma)*(A_C[0]*A_C[0] + A_C[1]*A_C[1] + A_C[2]*A_C[2]) ) , (-1.0*( C[0]*A_C[0] + C[1]*A_C[1] + C[2]*A_C[2] ) ) );
  const complex<double> X_D = (delta==0) ? 1.0 : std::polar ( std::exp( -1.0/(4.0*delta)*(A_D[0]*A_D[0] + A_D[1]*A_D[1] + A_D[2]*A_D[2]) ) , (-1.0*( D[0]*A_D[0] + D[1]*A_D[1] + D[2]*A_D[2] ) ) );

  assert (std::abs( X_A.imag()) < 3e-16 );
  assert (std::abs( X_B.imag()) < 3e-16 );
  assert (std::abs( X_C.imag()) < 3e-16 );
  assert (std::abs( X_D.imag()) < 3e-16 );
//  const double X_Areal = X_A.real();
//  const double X_Breal = X_B.real();
//  const double X_Creal = X_C.real();
//  const double X_Dreal = X_D.real();

  const std::complex<double> ABbar[3] = { std::conj(Abar[0]) - Bbar[0], std::conj(Abar[1]) - Bbar[1], std::conj(Abar[2]) - Bbar[2] };
  const std::complex<double> CDbar[3] = { std::conj(Cbar[0]) - Dbar[0], std::conj(Cbar[1]) - Dbar[1], std::conj(Cbar[2]) - Dbar[2] };
  const std::complex<double> ABbarsq = ABbar[0]*ABbar[0] + ABbar[1]*ABbar[1] + ABbar[2]*ABbar[2];
  const std::complex<double> CDbarsq = CDbar[0]*CDbar[0] + CDbar[1]*CDbar[1] + CDbar[2]*CDbar[2];
  const std::complex<double> PQbar[3] = { Pbar[0] - Qbar[0], Pbar[1] - Qbar[1], Pbar[2] - Qbar[2] };
  const std::complex<double> PQbarsq = PQbar[0]*PQbar[0] + PQbar[1]*PQbar[1] + PQbar[2]*PQbar[2];

  const std::complex<double> e_ABbar = std::exp( -1.0*alpha*beta /p*ABbarsq );
  const std::complex<double> e_CDbar = std::exp( -1.0*gamma*delta/q*CDbarsq );
  const std::complex<double> E_ABbar = std::conj(X_A)*X_B*e_ABbar;
  const std::complex<double> E_CDbar = std::conj(X_C)*X_D*e_CDbar;

  const std::complex<double> Tbar = rho * PQbarsq;
  const std::complex<double> coeff =  2.0*pi*pi*std::sqrt(pi)/(p*q*std::sqrt(p+q))*E_ABbar*E_CDbar*A_.prefactor*B_.prefactor*C_.prefactor*D_.prefactor;

#if 0
  //Alternate approach that uses the original derivation without complex Gaussians
  double A_BA[3];
  double A_DC[3];
  double P[3];
  double Q[3];
  double BA[3];
  double DC[3];
  for (int i=0; i!=3; i++) {
    A_BA[i] = A_B[i] - A_A[i];
    A_DC[i] = A_D[i] - A_C[i];
    BA[i] = B[i] - A[i];
    DC[i] = D[i] - C[i];
    P[i] = (alpha*A[i] + beta*B[i])/p;
    Q[i] = (gamma*C[i] + delta*D[i])/q;
  }
  double BAsq = BA[0]*BA[0] + BA[1]*BA[1] + BA[2]*BA[2];
  double DCsq = DC[0]*DC[0] + DC[1]*DC[1] + DC[2]*DC[2];

  double A_BAsq = A_BA[0]*A_BA[0] + A_BA[1]*A_BA[1] + A_BA[2]*A_BA[2];
  double A_DCsq = A_DC[0]*A_DC[0] + A_DC[1]*A_DC[1] + A_DC[2]*A_DC[2];
  double expr = -0.25*(1/p*A_BAsq + 1/q*A_DCsq);    //  possible error here?
  double PdotA_BA = Pbar[0].real()*A_BA[0] + Pbar[1].real()*A_BA[1] + Pbar[2].real()*A_BA[2];
  double QdotA_DC = Qbar[0].real()*A_DC[0] + Qbar[1].real()*A_DC[1] + Qbar[2].real()*A_DC[2];
  double expi =  -1.0*(PdotA_BA+QdotA_DC);    //  POSSIBLE ERROR HERE
  std::complex<double> factor2 ( expr , expi );
  double E_AB_classic = std::exp(-alpha*beta/p*BAsq);
  double E_CD_classic = std::exp(-gamma*delta/q*DCsq);
  double coeff_classic = E_AB_classic * E_CD_classic * 2.0*pi*pi*std::sqrt(pi)/(p*q*std::sqrt(p+q));
  std::complex<double> coeff_test = coeff_classic * std::exp(factor2);
#endif

#if 0
  std::cout << "From comprehensive ERI test file!" << std::endl;
  std::cout << "Field = " << field[0] << ", " << field[1] << ", " << field[2] << std::endl;
  std::cout << "A = " << A[0] << ", " << A[1] << ", " << A[2] << std::endl;
  std::cout << "B = " << B[0] << ", " << B[1] << ", " << B[2] << std::endl;
  std::cout << "C = " << C[0] << ", " << C[1] << ", " << C[2] << std::endl;
  std::cout << "D = " << D[0] << ", " << D[1] << ", " << D[2] << std::endl;
  std::cout << "alpha = " << alpha << std::endl;
  std::cout << "beta  = " << beta  << std::endl;
  std::cout << "gamma = " << gamma << std::endl;
  std::cout << "delta = " << delta << std::endl;
  std::cout << "(combined) norm. const = " << A_.prefactor*B_.prefactor*C_.prefactor*D_.prefactor << std::endl;
  std::cout << "A_A = " << A_A[0] << ", " << A_A[1] << ", " << A_A[2] << std::endl;
  std::cout << "A_B = " << A_B[0] << ", " << A_B[1] << ", " << A_B[2] << std::endl;
  std::cout << "A_C = " << A_C[0] << ", " << A_C[1] << ", " << A_C[2] << std::endl;
  std::cout << "A_D = " << A_D[0] << ", " << A_D[1] << ", " << A_D[2] << std::endl;
  std::cout << "Abar = " << Abar[0] << ", " << Abar[1] << ", " << Abar[2] << std::endl;
  std::cout << "Bbar = " << Bbar[0] << ", " << Bbar[1] << ", " << Bbar[2] << std::endl;
  std::cout << "Cbar = " << Cbar[0] << ", " << Cbar[1] << ", " << Cbar[2] << std::endl;
  std::cout << "Dbar = " << Dbar[0] << ", " << Dbar[1] << ", " << Dbar[2] << std::endl;
  std::cout << "p = " << p << std::endl;
  std::cout << "q = " << q << std::endl;
  std::cout << "rho = " << rho << std::endl;
  std::cout << "X_A = " << X_A << std::endl;
  std::cout << "X_B = " << X_B << std::endl;
  std::cout << "X_C = " << X_C << std::endl;
  std::cout << "X_D = " << X_D << std::endl;
  std::cout << "Pbar = " << Pbar[0] << ", " << Pbar[1] << ", " << Pbar[2] << std::endl;
  std::cout << "Qbar = " << Qbar[0] << ", " << Qbar[1] << ", " << Qbar[2] << std::endl;
  std::cout << "conj(X_A) = " << conj(X_A) << std::endl;
  std::cout << "conj(X_C) = " << conj(X_C) << std::endl;
  std::cout << "e_ABbar = " << e_ABbar << std::endl;
  std::cout << "e_CDbar = " << e_CDbar << std::endl;
  std::cout << "E_ABbar = " << E_ABbar << std::endl;
  std::cout << "E_CDbar = " << E_CDbar << std::endl;
  std::cout << "Tbar = " << Tbar << std::endl;
  std::cout << "coeff = " << coeff << std::endl;
  std::cout << "mag of coeff = " << std::abs(coeff) << std::endl;
#endif
#if 0
  std::cout << "NEW TESTS" << std::endl;
  std::cout << "A_BAsq = " << A_BAsq << std::endl;
  std::cout << "A_DCsq = " << A_DCsq << std::endl;
  std::cout << "PdotA_BA = " <<  PdotA_BA << std::endl;
  std::cout << "QdotA_DC = " << QdotA_DC << std::endl;
  std::cout << "expr = " << expr << std::endl;
  std::cout << "expi = " << expi << std::endl;
  std::cout << "E_AB_classic = " << E_AB_classic << std::endl;
  std::cout << "E_CD_classic = " << E_CD_classic << std::endl;
  std::cout << "coeff_classic = " << coeff_classic << std::endl;
  std::cout << "factor2 = " << factor2 << std::endl;
  std::cout << "coeff_test = " << coeff_test << std::endl;
#endif

  const std::pair<std::complex<double>,std::complex<double>> out = std::make_pair (Tbar,coeff);
  return out;
}

test::polynomial<std::complex<double>> get_ERI_III (const std::vector<double> field, atomic_orbital A_, atomic_orbital B_, atomic_orbital C_, atomic_orbital D_) {
  const test::polynomial<std::complex<double>> Ix = get_ERI_Ix (0, field, A_, B_, C_, D_);
  const test::polynomial<std::complex<double>> Iy = get_ERI_Ix (1, field, A_, B_, C_, D_);
  const test::polynomial<std::complex<double>> Iz = get_ERI_Ix (2, field, A_, B_, C_, D_);
  const test::polynomial<std::complex<double>> IxIy = test::multiply_polynomials (Ix, Iy);
  const test::polynomial<std::complex<double>> IxIyIz = test::multiply_polynomials (IxIy, Iz);
  return IxIyIz;
}

// Projection of B_ onto A_
molecular_orbital projection_MO (vector<double> field, molecular_orbital A_, molecular_orbital B_, vector<atomic_orbital> basis) {
  complex<double> num = overlap_MO (field, A_, B_, basis);
  complex<double> denom = overlap_MO (field, A_, A_, basis);
  complex<double> factor = num / denom;
  molecular_orbital out = scalar_MO (factor, A_);
  return out;
}

// Multiply all coefficients of some MO by a scalar
molecular_orbital scalar_MO (complex<double> scalar, molecular_orbital A_) {
  const int nbasis = A_.coefficient.size();
  std::vector<std::complex<double>> scaled_coefficient;
  for (int i=0; i!=nbasis; i++) scaled_coefficient.push_back( scalar * A_.coefficient[i] );
  molecular_orbital out (scaled_coefficient);
  return out;
}

// Add A_ and B_
molecular_orbital add_MOs (molecular_orbital A_, molecular_orbital B_) {
  const int nbasis = A_.coefficient.size();
  assert ( A_.coefficient.size() == B_.coefficient.size() );
  std::vector<std::complex<double>> sum_coefficient;
  for (int i=0; i!=nbasis; i++) sum_coefficient.push_back( A_.coefficient[i] + B_.coefficient[i] );
  molecular_orbital out (sum_coefficient);
  return out;
}

// Subtract B_ from A_
molecular_orbital subtract_MOs (molecular_orbital A_, molecular_orbital B_) {
  complex<double> minus = -1.0;
  molecular_orbital minusB = scalar_MO (minus, B_);
  return add_MOs (A_, minusB);
}


// Given any basis set of MOs, construct an an orthogonal basis of MOs using linear combinations thereof
// The Gram-Schmidt process defines this algorithm
vector<molecular_orbital> orthogonalize_basis (vector<double> field, vector<molecular_orbital> old_basis, vector<atomic_orbital> basis) {
  const int nbasis = basis.size();
  vector<molecular_orbital> new_basis;
  for (int i=0; i!=nbasis; i++) {
    molecular_orbital ortho_MO = old_basis[i];
    for (int j=0; j!=i; j++) {
      molecular_orbital proj = projection_MO (field, new_basis[j], old_basis[i], basis);
      ortho_MO = subtract_MOs (ortho_MO, proj);
    }
    new_basis.push_back(ortho_MO);
  }
  assert (new_basis.size() == old_basis.size());
  return new_basis;
}

pair<vector<atomic_orbital>,vector<molecular_orbital>> prepare_orbitals (int nbasis_contracted, bool normalize_basis, bool scale_input, bool orthogonalize, vector<double> field,
    vector<double> positions, vector<int> angular, vector<double> exponents, vector<double> contraction_coefficients, vector<int> nprimitive,
    vector<complex<double>> orbital1, vector<complex<double>> orbital2, vector<complex<double>> orbital3, vector<complex<double>> orbital4) {

  const int nbasis = exponents.size();

#if 0
  // to print out input parameters
  cout << endl << endl;
  cout << "Full ERI summation over two molecular orbitals specified below,";
  if (scale_input) cout << "with coefficients scaled to one, ";
  else cout << "with coefficients taken as-is, ";
  cout << endl << "using a basis of " << nbasis;
  if (orthogonalize) cout << " orthogonal combinations of ";
  else cout << " atom-centered (nonorthogonal) ";
  if (normalize_basis) cout << "normalized";
  else cout << "unnormalized";
  cout << " London orbitals." << endl;

  if (nbasis < 4) throw runtime_error ("Need at least four basis orbitals, but some can be unused if you set their coefficients to zero in the MO.");
  cout << "  nbasis = " << nbasis << endl;
  cout << "  normalize_basis = " << normalize_basis << endl;
  cout << "  scale_input = " << scale_input << endl;
  cout << "  orthogonalize = " << orthogonalize << endl;
  cout << "  field = " << field[0] << ", " << field[1] << ", " << field[2] << endl;

  cout << "  molecular orbital 1 coefficients = ";
  for (int j=0; j!=nbasis_contracted; j++) {
    cout << orbital1[j];
    if (j!=nbasis_contracted-1) cout << ", ";
    else cout << endl;
  }
  cout << "  molecular orbital 2 coefficients = ";
  for (int j=0; j!=nbasis_contracted; j++) {
    cout << orbital2[j];
    if (j!=nbasis_contracted-1) cout << ", ";
    else cout << endl;
  }
  cout << "  molecular orbital 3 coefficients = ";
  for (int j=0; j!=nbasis_contracted; j++) {
    cout << orbital3[j];
    if (j!=nbasis_contracted-1) cout << ", ";
    else cout << endl;
  }
  cout << "  molecular orbital 4 coefficients = ";
  for (int j=0; j!=nbasis_contracted; j++) {
    cout << orbital4[j];
    if (j!=nbasis_contracted-1) cout << ", ";
    else cout << endl;
  }
  for (int i = 0; i!=nbasis; i++) {
    cout << "  Atomic orbital " << i << endl;
    cout << "    exponent = " << exponents[i] << endl;
    cout << "    positions = " << positions[3*i+0] << ", " << positions[3*i+1] << ", " << positions[3*i+2] << endl;
    cout << "    angular = " << angular[3*i+0] << ", " << angular[3*i+1] << ", " << angular[3*i+2] << endl;
  }
#endif

  // Define atomic orbitals
  vector<atomic_orbital> basis;
  basis.resize(nbasis);
  for (int i = 0; i!=nbasis; i++) {
    basis[i].set_data(&positions[3*i],exponents[i],&angular[3*i],field);
  }

  // Normalize, if desired
  if (normalize_basis) {
    int offset = 0;
    for (int i=0; i!=nbasis_contracted; i++) {
      vector<complex<double>> contractedAO = {};
      int start = offset;
      int end = offset + nprimitive[i];
      for (int j=0; j!= nbasis; j++) {
        if (j < start) contractedAO.push_back(0.0);
        else if (j < end) contractedAO.push_back(contraction_coefficients[j]);
        else contractedAO.push_back(0.0);
      }
      const molecular_orbital contracted_AO (contractedAO);
      const complex<double> mag = overlap_MO (field, contracted_AO, contracted_AO, basis);
      assert (std::abs(mag.real()) > (std::abs(mag.imag())*1e14));
      const double factor = 1.0 / sqrt(mag.real());
      for (int k=0; k!=nprimitive[i]; k++) {
        basis[offset+k].prefactor *= factor;
      }
      offset += nprimitive[i];
    }
  }

  // Define a basis set of molecular orbitals, taking into account contraction coefficients
  // Technically, each primitive orbital in the basis gets its own MO - this is probably not the best way to do it
  vector<molecular_orbital> basis_MO;
  for(int i=0; i!=nbasis; i++) {
    vector<complex<double>> identity;
    for (int j=0; j!=nbasis; j++) {
      if (i == j) identity.push_back(contraction_coefficients[i]);
      else identity.push_back(0.0);
    }
    molecular_orbital oneAO (identity);
    basis_MO.push_back(oneAO);
  }
  assert(basis_MO.size() == nbasis);

  // Form a new, orthogonal basis set
  if (orthogonalize) {
    if (nbasis != nbasis_contracted) throw runtime_error ("Orthogonalization code does not work with contracted orbitals.  Use one exponenet for each basis orbital.");
    vector<molecular_orbital> new_basis = orthogonalize_basis (field, basis_MO, basis);
    if (normalize_basis) {
      for (int i=0; i!=nbasis; i++) {
        complex<double> factor = 1.0/sqrt(overlap_MO(field,new_basis[i],new_basis[i],basis));
        new_basis[i] = scalar_MO ( factor, new_basis[i] );
      }
    }
    for (int i=0; i!=nbasis; i++) basis_MO[i] = new_basis[i];
  }

  // Scale MO coefficients so their squares add up to one
  if (scale_input) {
    complex<double> sum1 = 0.0;
    complex<double> sum2 = 0.0;
    complex<double> sum3 = 0.0;
    complex<double> sum4 = 0.0;
    for (int i=0; i!=nbasis_contracted; i++) {
      sum1 += (orbital1[i]*orbital1[i]);
      sum2 += (orbital2[i]*orbital2[i]);
      sum3 += (orbital3[i]*orbital3[i]);
      sum4 += (orbital4[i]*orbital4[i]);
    }
    const complex<double> sqrtsum1 = sqrt(sum1);
    const complex<double> sqrtsum2 = sqrt(sum2);
    const complex<double> sqrtsum3 = sqrt(sum3);
    const complex<double> sqrtsum4 = sqrt(sum4);
    for (int i=0; i!=nbasis_contracted; i++) {
      orbital1[i] = orbital1[i]/sqrtsum1;
      orbital2[i] = orbital2[i]/sqrtsum2;
      orbital3[i] = orbital3[i]/sqrtsum3;
      orbital4[i] = orbital4[i]/sqrtsum4;
    }
  }

  // Now compute the final coefficients using input file data and the MO basis constructed above
  vector<complex<double>> MO1;
  vector<complex<double>> MO2;
  vector<complex<double>> MO3;
  vector<complex<double>> MO4;
  for (int j=0; j!=nbasis; j++) {
    int ncounter = 0;
    complex<double> sum1 = 0.0;
    complex<double> sum2 = 0.0;
    complex<double> sum3 = 0.0;
    complex<double> sum4 = 0.0;
    for (int i=0; i!=nbasis_contracted; i++) {
      for (int k=0; k!=nprimitive[i]; k++) {
        sum1 += (orbital1[i]*basis_MO[ncounter].coefficient[j]);
        sum2 += (orbital2[i]*basis_MO[ncounter].coefficient[j]);
        sum3 += (orbital3[i]*basis_MO[ncounter].coefficient[j]);
        sum4 += (orbital4[i]*basis_MO[ncounter].coefficient[j]);
        ncounter++;
      }
    }
    MO1.push_back(sum1);
    MO2.push_back(sum2);
    MO3.push_back(sum3);
    MO4.push_back(sum4);
  }

  // These are the orbitals we actually use in compute the ERI
  molecular_orbital MO_A (MO1);
  molecular_orbital MO_B (MO2);
  molecular_orbital MO_C (MO3);
  molecular_orbital MO_D (MO4);

  vector<molecular_orbital> MOs = {MO_A, MO_B, MO_C, MO_D};
  pair<vector<atomic_orbital>,vector<molecular_orbital>> out (basis, MOs);
  return out;
}

complex<double> compute_eri (vector<atomic_orbital> basis, vector<molecular_orbital> input, vector<double> field) {

  // Use a four-fold summation to compute the ERI over MO_A, MO_B, MO_C, and MO_D
  const int nbasis = basis.size();
  complex<double> Full_ERI = 0.0;
  complex<double> current_term;
  complex<double> coeff_prod;

  for (int i = 0; i!=nbasis; i++) {
    for (int j = 0; j!=nbasis; j++) {
      for (int k = 0; k!=nbasis; k++) {
        for (int l = 0; l!=nbasis; l++) {
          coeff_prod = conj(input[0].coefficient[i]) * input[1].coefficient[j] * conj(input[2].coefficient[k]) * input[3].coefficient[l];
          if (abs(coeff_prod)) {
            current_term = get_eri_matrix_element (field, basis[i], basis[j], basis[k], basis[l]);
            Full_ERI += (coeff_prod * current_term);
#if 0
            cout << "   orbital coeffs = " << input[0].coefficient[i] << ", " << input[1].coefficient[j] << ", " << input[2].coefficient[k] << ", " << input[3].coefficient[l] << endl;
            cout << "   coefficient for this term = " << coeff_prod << endl;
            cout << "indices = " << i << j << k << l << endl;
            cout << "Scaled matrix element = " << coeff_prod * current_term << endl;
            cout << "Magnitude = " << abs(coeff_prod * current_term) << endl;
            cout << "Phase = " << arg(coeff_prod * current_term) << endl << endl;
#endif
          }
        }
      }
    }
  }
#if 0
  cout << "Full ERI = " << Full_ERI << endl;
  cout << "Magnitude = " << abs(Full_ERI) << endl;
  cout << "Phase = " << arg(Full_ERI) << endl;

  cout << "Molecular Orbital A coeffs: ";
  for (int i=0; i!=nbasis; i++) {
    cout << input[0].coefficient[i];
    if (i!=nbasis-1) cout << ", ";
    else cout << endl;
  }
  cout << "Molecular Orbital B coeffs: ";
  for (int i=0; i!=nbasis; i++) {
    cout << input[1].coefficient[i];
    if (i!=nbasis-1) cout << ", ";
    else cout << endl;
  }
  cout << "Molecular Orbital C coeffs: ";
  for (int i=0; i!=nbasis; i++) {
    cout << input[2].coefficient[i];
    if (i!=nbasis-1) cout << ", ";
    else cout << endl;
  }
  cout << "Molecular Orbital D coeffs: ";
  for (int i=0; i!=nbasis; i++) {
    cout << input[3].coefficient[i];
    if (i!=nbasis-1) cout << ", ";
    else cout << endl;
  }
#endif
#if 0
// to check overlap integrals
  cout << endl << "Overlap integrals:" << endl;
  cout << "<a|a> = " << overlap_MO(field,input[0],input[0],basis) << endl;
  cout << "<b|b> = " << overlap_MO(field,input[1],input[1],basis) << endl;
  cout << "<c|c> = " << overlap_MO(field,input[2],input[2],basis) << endl;
  cout << "<d|d> = " << overlap_MO(field,input[3],input[3],basis) << endl;
#endif
#if 0
  cout << "<a|b> = " << overlap_MO(field,input[0],input[1],basis) << endl;
  cout << "<b|a> = " << overlap_MO(field,input[1],input[0],basis) << endl;
  cout << "<a|c> = " << overlap_MO(field,input[0],input[2],basis) << endl;
  cout << "<c|a> = " << overlap_MO(field,input[2],input[0],basis) << endl;
  cout << "<a|d> = " << overlap_MO(field,input[0],input[3],basis) << endl;
  cout << "<d|a> = " << overlap_MO(field,input[3],input[0],basis) << endl;
  cout << "<b|c> = " << overlap_MO(field,input[1],input[2],basis) << endl;
  cout << "<c|b> = " << overlap_MO(field,input[2],input[1],basis) << endl;
  cout << "<b|d> = " << overlap_MO(field,input[1],input[3],basis) << endl;
  cout << "<d|b> = " << overlap_MO(field,input[3],input[1],basis) << endl;
  cout << "<c|d> = " << overlap_MO(field,input[2],input[3],basis) << endl;
  cout << "<d|c> = " << overlap_MO(field,input[3],input[2],basis) << endl;
#endif

  return Full_ERI;

}

pair<complex<double>,complex<double>> compute_ss (const vector<double> field, atomic_orbital A_, atomic_orbital B_, nucleus C_) {
  const double* A = A_.position;
  const double* B = B_.position;
  const double* C = C_.position;
  const int Z = C_.charge;

  const double alpha = A_.exponent;
  const double beta  = B_.exponent;
  const double p = alpha + beta;
#if 1 // For London orbitals
  const double A_A[3] = { 0.5*(field[1]*A[2] - field[2]*A[1]), 0.5*(field[2]*A[0] - field[0]*A[2]), 0.5*(field[0]*A[1] - field[1]*A[0]) };
  const double A_B[3] = { 0.5*(field[1]*B[2] - field[2]*B[1]), 0.5*(field[2]*B[0] - field[0]*B[2]), 0.5*(field[0]*B[1] - field[1]*B[0]) };
#else // For common origin
  const double A_A[3] = { 0.0, 0.0, 0.0 };
  const double A_B[3] = { 0.0, 0.0, 0.0 };
#endif

  complex<double> Abar[3];
  complex<double> Bbar[3];
  complex<double> Pbar[3];
  for (int i=0; i!=3; i++) {
    Abar[i].real(A[i]);
    Abar[i].imag(-0.5*A_A[i]/alpha);
    Bbar[i].real(B[i]);
    Bbar[i].imag(-0.5*A_B[i]/beta);
    Pbar[i] = (alpha*conj(Abar[i]) + beta*Bbar[i])/p;
  }

  const complex<double> X_A = std::polar ( std::exp( -1.0/(4.0*alpha)*(A_A[0]*A_A[0] + A_A[1]*A_A[1] + A_A[2]*A_A[2]) ) , (-1.0*( A[0]*A_A[0] + A[1]*A_A[1] + A[2]*A_A[2] ) ) );
  const complex<double> X_B = std::polar ( std::exp( -1.0/(4.0*beta )*(A_B[0]*A_B[0] + A_B[1]*A_B[1] + A_B[2]*A_B[2]) ) , (-1.0*( B[0]*A_B[0] + B[1]*A_B[1] + B[2]*A_B[2] ) ) );

  assert (std::abs( X_A.imag()) < 3e-16 );
  assert (std::abs( X_B.imag()) < 3e-16 );
//  const double X_Areal = X_A.real();
//  const double X_Breal = X_B.real();

  const std::complex<double> ABbar[3] = { std::conj(Abar[0]) - Bbar[0], std::conj(Abar[1]) - Bbar[1], std::conj(Abar[2]) - Bbar[2] };
  const std::complex<double> ABbarsq = ABbar[0]*ABbar[0] + ABbar[1]*ABbar[1] + ABbar[2]*ABbar[2];
  const std::complex<double> PC[3] = { Pbar[0] - C[0], Pbar[1] - C[1], Pbar[2] - C[2] };
  const std::complex<double> PCsq = PC[0]*PC[0] + PC[1]*PC[1] + PC[2]*PC[2];

  const std::complex<double> e_ABbar = std::exp( -1.0*alpha*beta /p*ABbarsq );
  const std::complex<double> E_ABbar = std::conj(X_A)*X_B*e_ABbar;

  const std::complex<double> Tbar = p * PCsq;
  const std::complex<double> coeff = -2.0*Z*(pi/p)*E_ABbar*A_.prefactor*B_.prefactor;

  const std::pair<std::complex<double>,std::complex<double>> out = std::make_pair (Tbar,coeff);
  return out;
}

// Run the VRR and HRR to account for angular momentum in one particular dimension
test::polynomial<std::complex<double>> get_NAI_Ix (const int dimension, const std::vector<double> field, atomic_orbital A_, atomic_orbital B_, nucleus C_) {

  const int a = A_.angular_momentum[dimension];
  const int b = B_.angular_momentum[dimension];
  const int ab = a + b;

  std::vector<std::complex<double>> R1pv = {0.0,0.0};
  std::vector<std::complex<double>> R1v  = {0.0,0.0};
  std::vector<std::complex<double>> R2v  = {0.0,0.0};

  const std::complex<double> AB = A_.position[dimension] - B_.position[dimension];

  {
    const double alpha = A_.exponent;
    const double beta  = B_.exponent;
    const double p = alpha + beta;
    std::complex<double> Abar;
    std::complex<double> Bbar;
    {
      Abar.real(A_.position[dimension]);
      Bbar.real(B_.position[dimension]);
      Abar.imag(-0.5*A_.vector_potential[dimension]/alpha);
      Bbar.imag(-0.5*B_.vector_potential[dimension]/beta);
    }
    const std::complex<double> Pbar = (alpha*conj(Abar) + beta*Bbar)/p;
    R1v[0] = ( Pbar - A_.position[dimension] );
    R1pv[0] = ( Pbar - B_.position[dimension] );
    R2v[0] = ( 0.5/p );
    R1v[1] = ( -1.0*( Pbar - C_.position[dimension] ) );
    R1pv[1] = (-1.0*( Pbar - C_.position[dimension] ) );
    R2v[1] = ( -0.5/p );
  }

  const test::polynomial<std::complex<double>> R1p (R1pv);
  const test::polynomial<std::complex<double>> R1 (R1v);
  const test::polynomial<std::complex<double>> R2 (R2v);

#if 0
  char dim;
  if (dimension==0) dim = 'x';
  if (dimension==1) dim = 'y';
  if (dimension==2) dim = 'z';
  cout << "Running recurrence relations!";
  cout << "computing I_" << dim << ":" << endl;
  std::cout << "A_" << dim << " = " << A_.position[dimension] << std::endl;
  std::cout << "B_" << dim << " = " << B_.position[dimension] << std::endl;
  std::cout << "C_" << dim << " = " << C_.position[dimension] << std::endl;
  std::cout << "a = " << a << std::endl;
  std::cout << "b = " << b << std::endl;
  //std::cout << "R1p = "; R1p.show();
  std::cout << "R1_" << dim << " = "; R1.show();
  std::cout << "R2_" << dim << " = "; R2.show();
#endif

  std::vector<std::vector<test::polynomial<std::complex<double>>>> Iab;
  std::vector<std::complex<double>> one = {1.0};
  std::vector<std::complex<double>> zero = {0.0};
  test::polynomial<std::complex<double>> ss (one);

  // set vector size and instantiate each polynomial as 1
  Iab.resize(ab+1);
  for (int i=0; i<=ab; i++) {
    Iab[i].resize(ab+1,ss);
  }

  // vertical recurrence relations (VRR)
  const std::complex<double> uno = 1.0;
  for (int n=0; n!=ab; n++) {
    const std::complex<double> aa = n;
    polynomial<std::complex<double>> term1 (zero);
    polynomial<std::complex<double>> term2 (zero);
    term1 = scalar_polynomial ( multiply_polynomials ( R1, Iab[n][0] ), uno);
    if (n)   term2 = scalar_polynomial ( multiply_polynomials ( R2, Iab[n-1][0] ), aa );
    Iab[n+1][0] = add_polynomials ( term1 , term2 );
    if(R1.coeff[1]!=0.0 && R2.coeff[1]!=0.0) {
      if (n>1) assert (Iab[n-1][0].rank > 0);
      if (n>0) assert (Iab[n][0].rank > 0);
      assert (Iab[n+1][0].rank > 0);
    }
  }

  // horizontal recurrence relations (HRR)
  for (int n=0; n!=b; n++) {
    for (int i=0; i!=b-n; i++) {
      polynomial<std::complex<double>> term1 = Iab[a+i+1][n];
      polynomial<std::complex<double>> term2 = scalar_polynomial ( Iab[a+i][n], AB );
      Iab[a+i][n+1] = add_polynomials ( term1, term2 );
      if(R1.coeff[1]!=0.0 && R2.coeff[1]!=0.0) {
        assert (Iab[a+i+1][n].rank > 0);
        if(a>0||i>0||n>0)assert (Iab[a+i][n].rank > 0);
        assert (Iab[a+i][n+1].rank > 0);
      }
    }
  }

  return Iab[a][b];
}

test::polynomial<std::complex<double>> get_NAI_III (const std::vector<double> field, atomic_orbital A_, atomic_orbital B_, nucleus C_) {
  const test::polynomial<std::complex<double>> Ix = get_NAI_Ix (0, field, A_, B_, C_);
  const test::polynomial<std::complex<double>> Iy = get_NAI_Ix (1, field, A_, B_, C_);
  const test::polynomial<std::complex<double>> Iz = get_NAI_Ix (2, field, A_, B_, C_);
/*
  const std::vector<std::complex<double>> one = {1.0};
  test::polynomial<std::complex<double>> Ix (one);
  test::polynomial<std::complex<double>> Iy (one);
  test::polynomial<std::complex<double>> Iz (one);

  int total_angular[3];
  for (int i=0; i!=3; i++) total_angular[i] = A_.angular_momentum[i] + B_.angular_momentum[i];
  // cout << "total_angular = " << total_angular[0] << "  " << total_angular[1] << "  " << total_angular[2] << endl;
  if (total_angular[0]) Ix = get_NAI_Ix (0, field, A_, B_, C_);
  if (total_angular[1]) Iy = get_NAI_Ix (1, field, A_, B_, C_);
  if (total_angular[2]) Iz = get_NAI_Ix (2, field, A_, B_, C_);
*/
  const test::polynomial<std::complex<double>> IxIy = test::multiply_polynomials (Ix, Iy);
  const test::polynomial<std::complex<double>> IxIyIz = test::multiply_polynomials (IxIy, Iz);
  return IxIyIz;
}

complex<double> get_nai_matrix_element (const vector<double> field, atomic_orbital A_, atomic_orbital B_, nucleus C_) {

  if (C_.exponent != 0.0) return 0.0; // Finite nucleus - this function should not run

  polynomial<complex<double>> III = get_NAI_III (field, A_, B_, C_);
  const int nroot = ( III.rank/2 + 1 );

  pair<complex<double>,complex<double>> ss =  compute_ss(field, A_, B_, C_);

  // Here we assign the values of each T we will evaluate
  complex<double> Ts[1] = {ss.first};

  // Evaluate each one first using the mapping in the generated files
  complex<double> mapt[nroot];
  complex<double> mapw[nroot];
  assert (nroot < 14);
  mapT.root(nroot, Ts, mapt, mapw, 1);

  // Evaluate the integral by Gaussian quadrature
  complex<double> quad_term;
  complex<double> quad_integral (0,0);
  for(int j = 0; j!=nroot; j++){                 // For each root and weight pair...
    quad_term = mapw[j] * III.evaluate(mapt[j]);
    quad_integral += quad_term;
  }
  complex<double> NAI = quad_integral * ss.second;

#if 0
  cout << "This matrix element = " << NAI << endl;
  cout << "Magnitude = " << abs(NAI) << endl;
  cout << "Phase = "     << arg(NAI) << endl;
  cout << "   ss coeff = " << ss.second << endl;
  cout << "   T = " << ss.first << endl;
  cout << "   # roots needed = " << nroot << endl;
  cout << "   Quadrature (sum of w_i P(t_i) terms) = " << quad_integral << endl;
  cout << "   III(t) = ";
  III.show();
  cout << "   The roots are:   ";
  for(int i=0; i!=nroot; i++) cout << mapt[i] << " ";
  cout << endl << "   The weights are: ";
  for(int i=0; i!=nroot; i++) cout << mapw[i] << " ";
  cout << endl;
#endif
  return NAI;
}

complex<double> compute_nai (vector<atomic_orbital> basis, vector<molecular_orbital> input, vector<double> field, vector<nucleus> nuclei) {
  // Use a four-fold summation to compute the ERI over MO_A, MO_B, MO_C, and MO_D
  const int nbasis = basis.size();
  const int natom = nuclei.size();
  complex<double> Full_NAI = 0.0;
  complex<double> current_term;
  complex<double> coeff_prod;

#if 0
  cout << "MO 1 coefficients: " << endl;
  for (int i=0; i!=nbasis; i++) cout << input[0].coefficient[i] << endl;
  cout << "MO 2 coefficients: " << endl;
  for (int i=0; i!=nbasis; i++) cout << input[1].coefficient[i] << endl;
#endif

  for (int i = 0; i!=nbasis; i++) {
    for (int j = 0; j!=nbasis; j++) {
      for (int k = 0; k!=natom; k++) {

        coeff_prod = conj(input[0].coefficient[i]) * input[1].coefficient[j];
        if (abs(coeff_prod)) {
          current_term = get_nai_matrix_element (field, basis[i], basis[j], nuclei[k]);
          Full_NAI += (coeff_prod * current_term);
#if 0
          cout << "   orbital coeffs = " << input[0].coefficient[i] << ", " << input[1].coefficient[j] << endl;
          cout << "   coefficient for this term = " << coeff_prod << endl;
          cout << "indices = " << i << j << k << endl;
          cout << "current_term = " << current_term << endl;
          cout << "Scaled matrix element = " << coeff_prod * current_term << endl;
          cout << "Magnitude = " << abs(coeff_prod * current_term) << endl;
          cout << "Phase = " << arg(coeff_prod * current_term) << endl << endl;
#endif
        }
      }
    }
  }
#if 0
  cout << "Full NAI = " << Full_NAI << endl;
  cout << "Magnitude = " << abs(Full_NAI) << endl;
  cout << "Phase = " << arg(Full_NAI) << endl;
#endif

  return Full_NAI;
}

complex<double> kinetic_MO (vector<double> field, molecular_orbital A_, molecular_orbital B_, vector<atomic_orbital> basis) {
  const int nbasis = basis.size();
  complex<double> coeff_prod;
  complex<double> current_term;
  complex<double> Full_kinetic = 0.0;
  for (int i = 0; i!=nbasis; i++) {
    for (int j = 0; j!=nbasis; j++) {
      coeff_prod = conj(A_.coefficient[i]) * B_.coefficient[j];
      if (abs(coeff_prod)) {
        current_term = kinetic(field, basis[i], basis[j]);
        Full_kinetic += (coeff_prod * current_term);
      }
    }
  }
  return Full_kinetic;
}

// Using Equation 227 from notebook page RDR-007-30, with pi^2 applied to the bra
// Defining "imag" as -i is a cheap trick to get the hermitian conjugate of the pi^2 operator
std::complex<double> kinetic (const std::vector<double> field, atomic_orbital A_, atomic_orbital B_) {
  const complex<double> imag (0.0, -1.0);
  atomic_orbital A = A_;
  atomic_orbital B = B_;
  const double alpha = A.exponent;
  const int ax = A_.angular_momentum[0];
  const int ay = A_.angular_momentum[1];
  const int az = A_.angular_momentum[2];
  const double axd = ax;
  const double ayd = ay;
  const double azd = az;
  const double Bx = field[0];
  const double By = field[1];
  const double Bz = field[2];

  complex<double> out = 0.0;

  if (ax > 1) {
    A.change_angular(ax-2,ay  ,az  );
    out -= axd * (axd-1) * overlap(field,A,B);
  }
  if (ay > 1) {
    A.change_angular(ax  ,ay-2,az  );
    out -= ayd * (ayd-1) * overlap(field,A,B);
  }
  if (az > 1) {
    A.change_angular(ax  ,ay  ,az-2);
    out -= azd * (azd-1) * overlap(field,A,B);
  }
  if (ax > 0) {
    A.change_angular(ax-1,ay+1,az  );
    out += imag * 1.0 * axd * Bz * overlap(field,A,B);
  }
  if (ay > 0) {
    A.change_angular(ax  ,ay-1,az+1);
    out += imag * 1.0 * ayd * Bx * overlap(field,A,B);
  }
  if (az > 0) {
    A.change_angular(ax+1,ay  ,az-1);
    out += imag * 1.0 * azd * By * overlap(field,A,B);
  }

  if (ax > 0) {
    A.change_angular(ax-1,ay  ,az+1);
    out -= imag * 1.0 * axd * By * overlap(field,A,B);
  }
  if (ay > 0) {
    A.change_angular(ax+1,ay-1,az  );
    out -= imag * 1.0 * ayd * Bz * overlap(field,A,B);
  }
  if (az > 0) {
    A.change_angular(ax  ,ay+1,az-1);
    out -= imag * 1.0 * azd * Bx * overlap(field,A,B);
  }

  A.change_angular(ax+2,ay  ,az  );
  out += ( 0.25 * (By*By + Bz*Bz) - 4.0 * alpha * alpha ) * overlap(field,A,B);
  A.change_angular(ax  ,ay+2,az  );
  out += ( 0.25 * (Bz*Bz + Bx*Bx) - 4.0 * alpha * alpha ) * overlap(field,A,B);
  A.change_angular(ax  ,ay  ,az+2);
  out += ( 0.25 * (Bx*Bx + By*By) - 4.0 * alpha * alpha ) * overlap(field,A,B);

  A.change_angular(ax+1,ay  ,az+1);
  out -= 0.5 * Bz * Bx * overlap(field,A,B);
  A.change_angular(ax+1,ay+1,az  );
  out -= 0.5 * Bx * By * overlap(field,A,B);
  A.change_angular(ax  ,ay+1,az+1);
  out -= 0.5 * By * Bz * overlap(field,A,B);

  A.change_angular(ax  ,ay  ,az  );
  out += ( 4.0 * alpha * ( axd + ayd + azd + 1.5 ) ) * overlap(field,A,B);

  /*
  A.change_angular(ax  ,ay  ,az  );
  complex<double> SSS = overlap(field,A,B);
  SSS += 0.0;

  A.change_angular(ax+1,ay  ,az  );
  complex<double> oSS = overlap(field,A,B);
  A.change_angular(ax  ,ay  ,az  );
  oSS += A.position[0] * overlap(field,A,B);

  complex<double> tSS = 0.0;
  A.change_angular(ax+2,ay  ,az  );
  tSS += overlap(field,A,B);
  A.change_angular(ax+1,ay  ,az  );
  tSS += 2.0 * A.position[0] * overlap(field,A,B);
  A.change_angular(ax  ,ay  ,az  );
  tSS += A.position[0] * A.position[0] * overlap(field,A,B);

  complex<double> dSS = 0.0;
  A.change_angular(ax-1,ay  ,az  );
  if(ax > 0) dSS += axd * overlap(field,A,B);
  A.change_angular(ax+1,ay  ,az  );
  dSS -= 2.0 * alpha * overlap(field,A,B);
  A.change_angular(ax  ,ay  ,az  );
  dSS -= imag * Ax * overlap(field,A,B);

  complex<double> TSS = 0.0;
  A.change_angular(ax-2,ay  ,az  );
  if(ax>1) TSS += axd * (axd - 1.0) * overlap(field,A,B);
  A.change_angular(ax-1,ay  ,az  );
  if(ax>0) TSS -= axd * 2.0 * imag * Ax * overlap(field,A,B);
  A.change_angular(ax  ,ay  ,az  );
  TSS -= 2.0 * alpha * (2.0 * axd + 1.0) * overlap(field,A,B);
  TSS -= Ax * Ax * overlap(field,A,B);
  A.change_angular(ax+1,ay  ,az  );
  TSS += imag * 4.0 * alpha * Ax * overlap(field,A,B);
  A.change_angular(ax+2,ay  ,az  );
  TSS += 4.0 * alpha * alpha * overlap(field,A,B);

  */

  return 0.5 * out;
}

vector<complex<double>> momentum_MO (vector<double> field, molecular_orbital A_, molecular_orbital B_, vector<atomic_orbital> basis) {
  const int nbasis = basis.size();
  complex<double> coeff_prod;
  vector<complex<double>> current_term;
  complex<double> fullx, fully, fullz;
  for (int i = 0; i!=nbasis; i++) {
    for (int j = 0; j!=nbasis; j++) {
      coeff_prod = conj(A_.coefficient[i]) * B_.coefficient[j];
      if (abs(coeff_prod)) {
        current_term = momentum(field, basis[i], basis[j]);
        fullx += (coeff_prod * current_term[0]);
        fully += (coeff_prod * current_term[1]);
        fullz += (coeff_prod * current_term[2]);
      }
    }
  }
  vector<complex<double>> Full_momentum = {fullx, fully, fullz};
  return Full_momentum;
}

vector<std::complex<double>> momentum (const std::vector<double> field, atomic_orbital A_, atomic_orbital B_) {
  const complex<double> imag (0.0, -1.0);
  atomic_orbital A = A_;
  atomic_orbital B = B_;
  const double alpha = A.exponent;
  const int ax = A_.angular_momentum[0];
  const int ay = A_.angular_momentum[1];
  const int az = A_.angular_momentum[2];
  //const int bx = B_.angular_momentum[0];
  //const int by = B_.angular_momentum[1];
  //const int bz = B_.angular_momentum[2];
  const double axd = ax;
  const double ayd = ay;
  const double azd = az;
  const double Bx = field[0];
  const double By = field[1];
  const double Bz = field[2];

  complex<double> outx = 0.0;
  complex<double> outy = 0.0;
  complex<double> outz = 0.0;

  if (ax > 0) {
    A.change_angular(ax-1,ay  ,az  );
    outx -= imag * 1.0 * axd * overlap(field,A,B);
  }
  if (ay > 0) {
    A.change_angular(ax  ,ay-1,az  );
    outy -= imag * 1.0 * ayd * overlap(field,A,B);
  }
  if (az > 0) {
    A.change_angular(ax  ,ay  ,az-1);
    outz -= imag * 1.0 * azd * overlap(field,A,B);
  }

  A.change_angular(ax+1,ay  ,az  );
  outx += imag * 2.0 * alpha * overlap(field,A,B);
  outy += 0.5 * Bz * overlap(field,A,B);
  outz -= 0.5 * By * overlap(field,A,B);

  A.change_angular(ax  ,ay+1,az  );
  outx -= 0.5 * Bz * overlap(field,A,B);
  outy += imag * 2.0 * alpha * overlap(field,A,B);
  outz += 0.5 * Bx * overlap(field,A,B);

  A.change_angular(ax  ,ay  ,az+1);
  outx += 0.5 * By * overlap(field,A,B);
  outy -= 0.5 * Bx * overlap(field,A,B);
  outz += imag * 2.0 * alpha * overlap(field,A,B);

  vector<complex<double>> out = {outx, outy, outz};
  return out;
}

vector<complex<double>> orb_angular_MO (vector<double> field, const array<double,3> Mcoord, molecular_orbital A_, molecular_orbital B_, vector<atomic_orbital> basis) {
  const int nbasis = basis.size();
  complex<double> coeff_prod;
  vector<complex<double>> current_term;
  complex<double> fullx, fully, fullz;
  for (int i = 0; i!=nbasis; i++) {
    for (int j = 0; j!=nbasis; j++) {
      coeff_prod = conj(A_.coefficient[i]) * B_.coefficient[j];
      if (abs(coeff_prod)) {
        current_term = orb_angular(field, Mcoord, basis[i], basis[j]);
        fullx += (coeff_prod * current_term[0]);
        fully += (coeff_prod * current_term[1]);
        fullz += (coeff_prod * current_term[2]);
      }
    }
  }
  vector<complex<double>> Full_orb_angular = {fullx, fully, fullz};
  return Full_orb_angular;
}

vector<std::complex<double>> orb_angular (const std::vector<double> field, const array<double,3> Mcoord, atomic_orbital A_, atomic_orbital B_) {
  const complex<double> imag (0.0, 1.0);
  atomic_orbital A = A_;
  atomic_orbital B = B_;
  const double beta = B.exponent;
  const int bx = B_.angular_momentum[0];
  const int by = B_.angular_momentum[1];
  const int bz = B_.angular_momentum[2];
  const double bxd = bx;
  const double byd = by;
  const double bzd = bz;
  const double Mx = Mcoord[0];
  const double My = Mcoord[1];
  const double Mz = Mcoord[2];
  const double Bx = B_.position[0];
  const double By = B_.position[1];
  const double Bz = B_.position[2];
  const double Fx = field[0];
  const double Fy = field[1];
  const double Fz = field[2];

  complex<double> outx = 0.0;
  complex<double> outy = 0.0;
  complex<double> outz = 0.0;

  if (bx > 0) {
    B.change_angular(bx-1,by  ,bz  );
    outy -= imag * bxd * (Bz - Mz) * overlap(field,A,B);
    outz += imag * bxd * (By - My) * overlap(field,A,B);

    B.change_angular(bx-1,by+1,bz  );
    outz += imag * bxd * overlap(field,A,B);

    B.change_angular(bx-1,by  ,bz+1);
    outy -= imag * bxd * overlap(field,A,B);
  }

  if (by > 0) {
    B.change_angular(bx  ,by-1,bz  );
    outz -= imag * byd * (Bx - Mx) * overlap(field,A,B);
    outx += imag * byd * (Bz - Mz) * overlap(field,A,B);

    B.change_angular(bx  ,by-1,bz+1);
    outx += imag * byd * overlap(field,A,B);

    B.change_angular(bx+1,by-1,bz  );
    outz -= imag * byd * overlap(field,A,B);
  }

  if (bz > 0) {
    B.change_angular(bx  ,by  ,bz-1);
    outx -= imag * bzd * (By - My) * overlap(field,A,B);
    outy += imag * bzd * (Bx - Mx) * overlap(field,A,B);

    B.change_angular(bx+1,by  ,bz-1);
    outy += imag * bzd * overlap(field,A,B);

    B.change_angular(bx  ,by+1,bz-1);
    outx -= imag * bzd * overlap(field,A,B);
  }


  B.change_angular(bx+1,by+1,bz  );
  outx -= 0.5 * Fy * overlap(field,A,B);
  outy -= 0.5 * Fx * overlap(field,A,B);

  B.change_angular(bx  ,by+1,bz+1);
  outy -= 0.5 * Fz * overlap(field,A,B);
  outz -= 0.5 * Fy * overlap(field,A,B);

  B.change_angular(bx+1,by  ,bz+1);
  outz -= 0.5 * Fx * overlap(field,A,B);
  outx -= 0.5 * Fz * overlap(field,A,B);


  B.change_angular(bx+2,by  ,bz  );
  outy += 0.5 * Fy * overlap(field,A,B);
  outz += 0.5 * Fz * overlap(field,A,B);

  B.change_angular(bx  ,by+2,bz  );
  outx += 0.5 * Fx * overlap(field,A,B);
  outz += 0.5 * Fz * overlap(field,A,B);

  B.change_angular(bx  ,by  ,bz+2);
  outx += 0.5 * Fx * overlap(field,A,B);
  outy += 0.5 * Fy * overlap(field,A,B);


  B.change_angular(bx+1,by  ,bz  );
  outx += 0.5 * (Fy * My - Fy * By - Fz * Bz + Fz * Mz) * overlap(field,A,B);
  outy += (0.5 * (Fy * Bx - Fy * Mx) + imag * (2.0 * beta * (Bz - Mz))) * overlap(field,A,B);
  outz += (0.5 * (Fz * Bx - Fz * Mx) - imag * (2.0 * beta * (By - My))) * overlap(field,A,B);

  B.change_angular(bx  ,by+1,bz  );
  outy += 0.5 * (Fz * Mz - Fz * Bz - Fx * Bx + Fx * Mx) * overlap(field,A,B);
  outz += (0.5 * (Fz * By - Fz * My) + imag * (2.0 * beta * (Bx - Mx))) * overlap(field,A,B);
  outx += (0.5 * (Fx * By - Fx * My) - imag * (2.0 * beta * (Bz - Mz))) * overlap(field,A,B);

  B.change_angular(bx  ,by  ,bz+1);
  outz += 0.5 * (Fx * Mx - Fx * Bx - Fy * By + Fy * My) * overlap(field,A,B);
  outx += (0.5 * (Fx * Bz - Fx * Mz) + imag * (2.0 * beta * (By - My))) * overlap(field,A,B);
  outy += (0.5 * (Fy * Bz - Fy * Mz) - imag * (2.0 * beta * (Bx - Mx))) * overlap(field,A,B);

  vector<complex<double>> out = {outx, outy, outz};
  return out;
}

complex<double> compute_smallnai (vector<atomic_orbital> basis, vector<molecular_orbital> input, vector<double> field, vector<nucleus> nuclei, const int ia, const int ib) {
  // Use a four-fold summation to compute the ERI over MO_A, MO_B, MO_C, and MO_D
  const int nbasis = basis.size();
  const int natom = nuclei.size();
  complex<double> Full_NAI = 0.0;
  complex<double> current_term;
  complex<double> coeff_prod;

#if 0
  cout << "MO 1 coefficients: " << endl;
  for (int i=0; i!=nbasis; i++) cout << input[0].coefficient[i] << endl;
  cout << "MO 2 coefficients: " << endl;
  for (int i=0; i!=nbasis; i++) cout << input[1].coefficient[i] << endl;
#endif

  for (int i = 0; i!=nbasis; i++) {
    for (int j = 0; j!=nbasis; j++) {
      for (int k = 0; k!=natom; k++) {

        coeff_prod = conj(input[0].coefficient[i]) * input[1].coefficient[j];
        if (abs(coeff_prod)) {
          current_term = get_smallnai_matrix_element (field, basis[i], basis[j], nuclei[k], ia, ib);
          Full_NAI += (coeff_prod * current_term);
#if 0
          cout << "   orbital coeffs = " << input[0].coefficient[i] << ", " << input[1].coefficient[j] << endl;
          cout << "   coefficient for this term = " << coeff_prod << endl;
          cout << "indices = " << i << j << k << endl;
          cout << "current_term = " << current_term << endl;
          cout << "Scaled matrix element = " << coeff_prod * current_term << endl;
          cout << "Magnitude = " << abs(coeff_prod * current_term) << endl;
          cout << "Phase = " << arg(coeff_prod * current_term) << endl << endl;
#endif
        }
      }
    }
  }
#if 0
  cout << "Full NAI = " << Full_NAI << endl;
  cout << "Magnitude = " << abs(Full_NAI) << endl;
  cout << "Phase = " << arg(Full_NAI) << endl;
#endif

  return Full_NAI;
}


complex<double> get_smallnai_matrix_element (const vector<double> field, atomic_orbital A_, atomic_orbital B_, nucleus C_, const int ia, const int ib) {

  atomic_orbital A = A_;
  atomic_orbital B = B_;
  const complex<double> imag (0.0, 1.0);
  const array<int,3> fwd = {{ 1, 2, 0 }};
  const array<int,3> bck = {{ 2, 0, 1 }};

  const array<int,5> xinc = {{ -1, 1, 0, 0, 0 }};
  const array<int,5> yinc = {{  0, 0, 0, 1, 0 }};
  const array<int,5> zinc = {{  0, 0, 1, 0, 0 }};
  const array<array<int,5>,3> ang = {{ xinc, yinc, zinc }};

  const double alpha = A.exponent;
  const double beta = B.exponent;
  const array<double,3> Ax = {{ A.position[0], A.position[1], A.position[2] }};
  const array<double,3> Bx = {{ B.position[0], B.position[1], B.position[2] }};
  const array<int,3> ax = {{ A.angular_momentum[0], A.angular_momentum[1], A.angular_momentum[2] }};
  const array<int,3> bx = {{ B.angular_momentum[0], B.angular_momentum[1], B.angular_momentum[2] }};
  array<double,3> axd;
  array<double,3> bxd;
  for (int i=0; i!=3; i++) axd[i] = ax[i];
  for (int i=0; i!=3; i++) bxd[i] = bx[i];

#if 0
  const int nterms = 5;  // For Gaussian orbitals (common origin)
#else
  const int nterms = 4;  // For London orbitals
#endif

  const array<complex<double>,5> Acoeff = {{  axd[ia]*imag, -2*alpha*imag, 0.5*field[fwd[ia]], -0.5*field[bck[ia]], 0.5*(field[fwd[ia]]*Ax[bck[ia]] - field[bck[ia]]*Ax[fwd[ia]]) }};
  const array<complex<double>,5> Bcoeff = {{ -bxd[ib]*imag,  2*beta*imag,  0.5*field[fwd[ib]], -0.5*field[bck[ib]], 0.5*(field[fwd[ib]]*Bx[bck[ib]] - field[bck[ib]]*Bx[fwd[ib]]) }};

  complex<double> out = 0.0;
  for (int a=0; a!=nterms; a++) {
    for (int b=0; b!=nterms; b++) {
      array<int,3> newa;
      array<int,3> newb;
      newa[ia] = ang[0][a];
      newa[fwd[ia]] = ang[1][a];
      newa[bck[ia]] = ang[2][a];
      newb[ib] = ang[0][b];
      newb[fwd[ib]] = ang[1][b];
      newb[bck[ib]] = ang[2][b];
      A.change_angular( ax[0]+newa[0], ax[1]+newa[1], ax[2]+newa[2] );
      B.change_angular( bx[0]+newb[0], bx[1]+newb[1], bx[2]+newb[2] );
      const complex<double> coef = Acoeff[a] * Bcoeff[b];
      if (coef != 0.0) out += coef * get_nai_matrix_element(field, A, B, C_);
    }
  }
  return out;
}


complex<double> compute_smalloverlap (vector<double> field, molecular_orbital A_, molecular_orbital B_, vector<atomic_orbital> basis, const int ia, const int ib) {
  const int nbasis = basis.size();
  complex<double> coeff_prod;
  complex<double> current_term;
  complex<double> Full_overlap = 0.0;
  for (int i = 0; i!=nbasis; i++) {
    for (int j = 0; j!=nbasis; j++) {
      coeff_prod = conj(A_.coefficient[i]) * B_.coefficient[j];
      if (abs(coeff_prod)) {
        current_term = get_smalloverlap_matrix_element(field, basis[i], basis[j], ia, ib);
        Full_overlap += (coeff_prod * current_term);
      }
    }
  }
  return Full_overlap;
}


complex<double> get_smalloverlap_matrix_element (const vector<double> field, atomic_orbital A_, atomic_orbital B_, const int ia, const int ib) {

  atomic_orbital A = A_;
  atomic_orbital B = B_;
  const complex<double> imag (0.0, 1.0);
  const array<int,3> fwd = {{ 1, 2, 0 }};
  const array<int,3> bck = {{ 2, 0, 1 }};

  const array<int,5> xinc = {{ -1, 1, 0, 0, 0 }};
  const array<int,5> yinc = {{  0, 0, 0, 1, 0 }};
  const array<int,5> zinc = {{  0, 0, 1, 0, 0 }};
  const array<array<int,5>,3> ang = {{ xinc, yinc, zinc }};

  const double alpha = A.exponent;
  const double beta = B.exponent;
  const array<double,3> Ax = {{ A.position[0], A.position[1], A.position[2] }};
  const array<double,3> Bx = {{ B.position[0], B.position[1], B.position[2] }};
  const array<int,3> ax = {{ A.angular_momentum[0], A.angular_momentum[1], A.angular_momentum[2] }};
  const array<int,3> bx = {{ B.angular_momentum[0], B.angular_momentum[1], B.angular_momentum[2] }};
  array<double,3> axd;
  array<double,3> bxd;
  for (int i=0; i!=3; i++) axd[i] = ax[i];
  for (int i=0; i!=3; i++) bxd[i] = bx[i];

#if 0
  const int nterms = 5;  // For Gaussian orbitals (common origin)
#else
  const int nterms = 4;  // For London orbitals
#endif

  const array<complex<double>,5> Acoeff = {{  axd[ia]*imag, -2*alpha*imag, 0.5*field[fwd[ia]], -0.5*field[bck[ia]], 0.5*(field[fwd[ia]]*Ax[bck[ia]] - field[bck[ia]]*Ax[fwd[ia]]) }};
  const array<complex<double>,5> Bcoeff = {{ -bxd[ib]*imag,  2*beta*imag,  0.5*field[fwd[ib]], -0.5*field[bck[ib]], 0.5*(field[fwd[ib]]*Bx[bck[ib]] - field[bck[ib]]*Bx[fwd[ib]]) }};

  complex<double> out = 0.0;
  for (int a=0; a!=nterms; a++) {
    for (int b=0; b!=nterms; b++) {
      array<int,3> newa;
      array<int,3> newb;
      newa[ia] = ang[0][a];
      newa[fwd[ia]] = ang[1][a];
      newa[bck[ia]] = ang[2][a];
      newb[ib] = ang[0][b];
      newb[fwd[ib]] = ang[1][b];
      newb[bck[ib]] = ang[2][b];
      A.change_angular( ax[0]+newa[0], ax[1]+newa[1], ax[2]+newa[2] );
      B.change_angular( bx[0]+newb[0], bx[1]+newb[1], bx[2]+newb[2] );
      const complex<double> coef = Acoeff[a] * Bcoeff[b];
      if (coef != 0.0) out += coef * overlap(field, A, B);
    }
  }
  return out;
}


complex<double> compute_small_finitenai (vector<atomic_orbital> basis, vector<molecular_orbital> input, vector<double> field, vector<nucleus> nuclei, const int ia, const int ib) {
  // Use a four-fold summation to compute the ERI over MO_A, MO_B, MO_C, and MO_D
  const int nbasis = basis.size();
  const int natom = nuclei.size();
  complex<double> Full_NAI = 0.0;
  complex<double> current_term;
  complex<double> coeff_prod;

#if 0
  cout << "MO 1 coefficients: " << endl;
  for (int i=0; i!=nbasis; i++) cout << input[0].coefficient[i] << endl;
  cout << "MO 2 coefficients: " << endl;
  for (int i=0; i!=nbasis; i++) cout << input[1].coefficient[i] << endl;
#endif

  for (int k = 0; k!=natom; k++) {
    complex<double> thisnai = 0.0;
    for (int i = 0; i!=nbasis; i++) {
      for (int j = 0; j!=nbasis; j++) {

        coeff_prod = conj(input[0].coefficient[i]) * input[1].coefficient[j];
        if (abs(coeff_prod)!=0.0 && nuclei[k].exponent!=0.0) {
          current_term = get_small_finitenai_matrix_element (field, basis[i], basis[j], nuclei[k], ia, ib);
          thisnai += (coeff_prod * current_term);
#if 0
          cout << "   orbital coeffs = " << input[0].coefficient[i] << ", " << input[1].coefficient[j] << endl;
          cout << "   coefficient for this term = " << coeff_prod << endl;
          cout << "indices = " << i << j << k << endl;
          cout << "current_term = " << current_term << endl;
          cout << "Scaled matrix element = " << coeff_prod * current_term << endl;
          cout << "Magnitude = " << abs(coeff_prod * current_term) << endl;
          cout << "Phase = " << arg(coeff_prod * current_term) << endl << endl;
#endif
        }
      }
    }
    const double charge = nuclei[k].charge;
    const double Zfac = -charge*pow((nuclei[k].exponent/(std::atan(1.0)*4.0)),1.5);
    thisnai *= Zfac;
    Full_NAI += thisnai;
  }
#if 0
  cout << "Full NAI = " << Full_NAI << endl;
  cout << "Magnitude = " << abs(Full_NAI) << endl;
  cout << "Phase = " << arg(Full_NAI) << endl;
#endif

  return Full_NAI;
}


complex<double> get_small_finitenai_matrix_element (const vector<double> field, atomic_orbital A_, atomic_orbital B_, nucleus C_, const int ia, const int ib) {

  atomic_orbital A = A_;
  atomic_orbital B = B_;
  const complex<double> imag (0.0, 1.0);
  const array<int,3> fwd = {{ 1, 2, 0 }};
  const array<int,3> bck = {{ 2, 0, 1 }};

  const array<int,5> xinc = {{ -1, 1, 0, 0, 0 }};
  const array<int,5> yinc = {{  0, 0, 0, 1, 0 }};
  const array<int,5> zinc = {{  0, 0, 1, 0, 0 }};
  const array<array<int,5>,3> ang = {{ xinc, yinc, zinc }};

  const double alpha = A.exponent;
  const double beta = B.exponent;
  const array<double,3> Ax = {{ A.position[0], A.position[1], A.position[2] }};
  const array<double,3> Bx = {{ B.position[0], B.position[1], B.position[2] }};
  const array<int,3> ax = {{ A.angular_momentum[0], A.angular_momentum[1], A.angular_momentum[2] }};
  const array<int,3> bx = {{ B.angular_momentum[0], B.angular_momentum[1], B.angular_momentum[2] }};
  array<double,3> axd;
  array<double,3> bxd;
  for (int i=0; i!=3; i++) axd[i] = ax[i];
  for (int i=0; i!=3; i++) bxd[i] = bx[i];

#if 0
  const int nterms = 5;  // For Gaussian orbitals (common origin)
#else
  const int nterms = 4;  // For London orbitals
#endif

  const int idummy[3] = { 0, 0, 0 };
  const double ddummy[3] = { 0.0, 0.0, 0.0 };
  const vector<double> nofield = {{ 0.0, 0.0, 0.0 }};

  atomic_orbital C; // Finite nuclear charge
  atomic_orbital D; // Dummy orbital
  C.set_data(C_.position, C_.exponent, idummy, nofield);
  D.set_data(ddummy, ddummy[0], idummy, nofield);
  //const double charge = C_.charge;

  const array<complex<double>,5> Acoeff = {{  axd[ia]*imag, -2*alpha*imag, 0.5*field[fwd[ia]], -0.5*field[bck[ia]], 0.5*(field[fwd[ia]]*Ax[bck[ia]] - field[bck[ia]]*Ax[fwd[ia]]) }};
  const array<complex<double>,5> Bcoeff = {{ -bxd[ib]*imag,  2*beta*imag,  0.5*field[fwd[ib]], -0.5*field[bck[ib]], 0.5*(field[fwd[ib]]*Bx[bck[ib]] - field[bck[ib]]*Bx[fwd[ib]]) }};

  complex<double> out = 0.0;
  for (int a=0; a!=nterms; a++) {
    for (int b=0; b!=nterms; b++) {
      array<int,3> newa;
      array<int,3> newb;
      newa[ia] = ang[0][a];
      newa[fwd[ia]] = ang[1][a];
      newa[bck[ia]] = ang[2][a];
      newb[ib] = ang[0][b];
      newb[fwd[ib]] = ang[1][b];
      newb[bck[ib]] = ang[2][b];
      A.change_angular( ax[0]+newa[0], ax[1]+newa[1], ax[2]+newa[2] );
      B.change_angular( bx[0]+newb[0], bx[1]+newb[1], bx[2]+newb[2] );
      const complex<double> coef = Acoeff[a] * Bcoeff[b];
      if (coef != 0.0) out += coef * get_eri_matrix_element(field, A, B, C, D);
      //cout << "                    out = " << scientific << out << endl;
    }
  }
  return out;
}


complex<double> compute_finite_nai (vector<atomic_orbital> basis, vector<molecular_orbital> input, vector<double> field, vector<nucleus> nuclei) {
  // Use a four-fold summation to compute the ERI over MO_A, MO_B, MO_C, and MO_D
  const int nbasis = basis.size();
  const int natom = nuclei.size();
  complex<double> Full_NAI = 0.0;
  complex<double> current_term;
  complex<double> coeff_prod;

#if 0
  cout << "MO 1 coefficients: " << endl;
  for (int i=0; i!=nbasis; i++) cout << input[0].coefficient[i] << endl;
  cout << "MO 2 coefficients: " << endl;
  for (int i=0; i!=nbasis; i++) cout << input[1].coefficient[i] << endl;
#endif

  for (int k = 0; k!=natom; k++) {
    complex<double> thisnai = 0.0;
    for (int i = 0; i!=nbasis; i++) {
      for (int j = 0; j!=nbasis; j++) {

        coeff_prod = conj(input[0].coefficient[i]) * input[1].coefficient[j];
        if (abs(coeff_prod)) {
          current_term = get_finite_nai_matrix_element (field, basis[i], basis[j], nuclei[k]);
          thisnai += (coeff_prod * current_term);
#if 0
          cout << "   orbital coeffs = " << input[0].coefficient[i] << ", " << input[1].coefficient[j] << endl;
          cout << "   coefficient for this term = " << coeff_prod << endl;
          cout << "indices = " << i << j << k << endl;
          cout << "current_term = " << current_term << endl;
          cout << "Scaled matrix element = " << coeff_prod * current_term << endl;
          cout << "Magnitude = " << abs(coeff_prod * current_term) << endl;
          cout << "Phase = " << arg(coeff_prod * current_term) << endl << endl;
#endif
        }
      }
    }
    const double charge = nuclei[k].charge;
    const double Zfac = -charge*pow((nuclei[k].exponent/(std::atan(1.0)*4.0)),1.5);
    thisnai *= Zfac;
    Full_NAI += thisnai;
    //cout << "Full NAI= " << scientific << Full_NAI << endl;
  }
#if 0
  cout << "Full NAI = " << Full_NAI << endl;
  cout << "Magnitude = " << abs(Full_NAI) << endl;
  cout << "Phase = " << arg(Full_NAI) << endl;
#endif

  return Full_NAI;
}


complex<double> get_finite_nai_matrix_element (const vector<double> field, atomic_orbital A_, atomic_orbital B_, nucleus C_) {

  const int idummy[3] = { 0, 0, 0 };
  const double ddummy[3] = { 0.0, 0.0, 0.0 };
  const vector<double> nofield = {{ 0.0, 0.0, 0.0 }};

  atomic_orbital C; // Finite nuclear charge
  atomic_orbital D; // Dummy orbital
  C.set_data(C_.position, C_.exponent, idummy, nofield);
  D.set_data(ddummy, ddummy[0], idummy, nofield);
  //const double charge = C_.charge;

  const complex<double> out = get_eri_matrix_element(field, A_, B_, C, D);

  return out;
}


complex<double> compute_smalleri (vector<atomic_orbital> basis, vector<molecular_orbital> input, vector<double> field, const int ia, const int ib) {

  // Use a four-fold summation to compute the ERI over MO_A, MO_B, MO_C, and MO_D
  const int nbasis = basis.size();
  complex<double> Full_ERI = 0.0;
  complex<double> current_term;
  complex<double> coeff_prod;

  for (int i = 0; i!=nbasis; i++) {
    for (int j = 0; j!=nbasis; j++) {
      for (int k = 0; k!=nbasis; k++) {
        for (int l = 0; l!=nbasis; l++) {
          coeff_prod = conj(input[0].coefficient[i]) * input[1].coefficient[j] * conj(input[2].coefficient[k]) * input[3].coefficient[l];
          if (abs(coeff_prod)) {
            current_term = get_smalleri_matrix_element (field, basis[i], basis[j], basis[k], basis[l], ia, ib);
            Full_ERI += (coeff_prod * current_term);
          }
        }
      }
    }
  }

  return Full_ERI;

}


complex<double> get_smalleri_matrix_element (const vector<double> field, atomic_orbital A_, atomic_orbital B_, atomic_orbital C_, atomic_orbital D_, const int ic, const int id) {

#if 1 // Check to ensure that A_ = dummy orbital, B_ = auxiliary function, C_ and D_ = main basis functions
  assert(A_.exponent == 0.0);
  assert(A_.prefactor == 1.0);
  for (int i=0; i!=3; i++) assert(A_.position[i] == 0.0);
  for (int i=0; i!=3; i++) assert(A_.angular_momentum[i] == 0);
  assert(B_.exponent != 0.0);
  assert(C_.exponent != 0.0);
  assert(D_.exponent != 0.0);
#endif

  atomic_orbital A;
  atomic_orbital B;
  const vector<double> nofield = {{ 0.0, 0.0, 0.0 }};
  A.set_data(A_.position, A_.exponent, A_.angular_momentum, nofield);
  B.set_data(B_.position, B_.exponent, B_.angular_momentum, nofield);

  atomic_orbital C = C_;
  atomic_orbital D = D_;
  const complex<double> imag (0.0, 1.0);
  const array<int,3> fwd = {{ 1, 2, 0 }};
  const array<int,3> bck = {{ 2, 0, 1 }};

  const array<int,5> xinc = {{ -1, 1, 0, 0, 0 }};
  const array<int,5> yinc = {{  0, 0, 0, 1, 0 }};
  const array<int,5> zinc = {{  0, 0, 1, 0, 0 }};
  const array<array<int,5>,3> ang = {{ xinc, yinc, zinc }};

  const double gamma = C.exponent;
  const double delta = D.exponent;
  const array<double,3> Cx = {{ C.position[0], C.position[1], C.position[2] }};
  const array<double,3> Dx = {{ D.position[0], D.position[1], D.position[2] }};
  const array<int,3> cx = {{ C.angular_momentum[0], C.angular_momentum[1], C.angular_momentum[2] }};
  const array<int,3> dx = {{ D.angular_momentum[0], D.angular_momentum[1], D.angular_momentum[2] }};
  array<double,3> cxd;
  array<double,3> dxd;
  for (int i=0; i!=3; i++) cxd[i] = cx[i];
  for (int i=0; i!=3; i++) dxd[i] = dx[i];

#if 0
  const int nterms = 5;  // For Gaussian orbitals (common origin)
#else
  const int nterms = 4;  // For London orbitals
#endif

  const array<complex<double>,5> Ccoeff = {{  cxd[ic]*imag, -2*gamma*imag, 0.5*field[fwd[ic]], -0.5*field[bck[ic]], 0.5*(field[fwd[ic]]*Cx[bck[ic]] - field[bck[ic]]*Cx[fwd[ic]]) }};
  const array<complex<double>,5> Dcoeff = {{ -dxd[id]*imag,  2*delta*imag,  0.5*field[fwd[id]], -0.5*field[bck[id]], 0.5*(field[fwd[id]]*Dx[bck[id]] - field[bck[id]]*Dx[fwd[id]]) }};

#if 0
  cout << endl;
  array<char,3> source = {{ 'x', 'y', 'z'}};
  array<char,3> cdim;
  array<char,3> ddim;
  cdim[0] = source[ic];
  cdim[1] = source[fwd[ic]];
  cdim[2] = source[bck[ic]];
  ddim[0] = source[id];
  ddim[1] = source[fwd[id]];
  ddim[2] = source[bck[id]];
  cout << "ic = " << ic << " ... " << cdim[0] << " " << cdim[1] << " " << cdim[2] << endl;
  cout << "id = " << id << " ... " << ddim[0] << " " << ddim[1] << " " << ddim[2] << endl;
#endif

  complex<double> out = 0.0;
  for (int c=0; c!=nterms; c++) {
    for (int d=0; d!=nterms; d++) {
      array<int,3> newc;
      array<int,3> newd;
      newc[ic] = ang[0][c];
      newc[fwd[ic]] = ang[1][c];
      newc[bck[ic]] = ang[2][c];
      newd[id] = ang[0][d];
      newd[fwd[id]] = ang[1][d];
      newd[bck[id]] = ang[2][d];
      C.change_angular( cx[0]+newc[0], cx[1]+newc[1], cx[2]+newc[2] );
      D.change_angular( dx[0]+newd[0], dx[1]+newd[1], dx[2]+newd[2] );
      const complex<double> coef = Ccoeff[c] * Dcoeff[d];
      if (coef != 0.0) out += coef * get_eri_matrix_element(field, A, B, C, D);
    }
  }
  return out;
}


complex<double> compute_mixederi (vector<atomic_orbital> basis, vector<molecular_orbital> input, vector<double> field, const int ia, const int ib) {

  // Use a four-fold summation to compute the ERI over MO_A, MO_B, MO_C, and MO_D
  const int nbasis = basis.size();
  complex<double> Full_ERI = 0.0;
  complex<double> current_term;
  complex<double> coeff_prod;

  for (int i = 0; i!=nbasis; i++) {
    for (int j = 0; j!=nbasis; j++) {
      for (int k = 0; k!=nbasis; k++) {
        for (int l = 0; l!=nbasis; l++) {
          coeff_prod = conj(input[0].coefficient[i]) * input[1].coefficient[j] * conj(input[2].coefficient[k]) * input[3].coefficient[l];
          if (abs(coeff_prod)) {
            current_term = get_mixederi_matrix_element (field, basis[i], basis[j], basis[k], basis[l], ia, ib);
            Full_ERI += (coeff_prod * current_term);
          }
        }
      }
    }
  }

  const complex<double> imag(0.0, 1.0);
  Full_ERI *= imag;
  return Full_ERI;

}


complex<double> get_mixederi_matrix_element (const vector<double> field, atomic_orbital A_, atomic_orbital B_, atomic_orbital C_, atomic_orbital D_, const int ic, const int id) {

#if 1 // Check to ensure that A_ = dummy orbital, B_ = auxiliary function, C_ and D_ = main basis functions
  assert(A_.exponent == 0.0);
  assert(A_.prefactor == 1.0);
  for (int i=0; i!=3; i++) assert(A_.position[i] == 0.0);
  for (int i=0; i!=3; i++) assert(A_.angular_momentum[i] == 0);
  assert(B_.exponent != 0.0);
  assert(C_.exponent != 0.0);
  assert(D_.exponent != 0.0);
#endif

  atomic_orbital A;
  atomic_orbital B;
  const vector<double> nofield = {{ 0.0, 0.0, 0.0 }};
  A.set_data(A_.position, A_.exponent, A_.angular_momentum, nofield);
  B.set_data(B_.position, B_.exponent, B_.angular_momentum, nofield);

  atomic_orbital C = C_;
  atomic_orbital D = D_;
  const complex<double> imag (0.0, 1.0);
  const array<int,3> fwd = {{ 1, 2, 0 }};
  const array<int,3> bck = {{ 2, 0, 1 }};

  const array<int,5> xinc = {{ -1, 1, 0, 0, 0 }};
  const array<int,5> yinc = {{  0, 0, 0, 1, 0 }};
  const array<int,5> zinc = {{  0, 0, 1, 0, 0 }};
  const array<array<int,5>,3> ang = {{ xinc, yinc, zinc }};

  const double gamma = C.exponent;
  const array<double,3> Cx = {{ C.position[0], C.position[1], C.position[2] }};
  const array<int,3> cx = {{ C.angular_momentum[0], C.angular_momentum[1], C.angular_momentum[2] }};
  array<double,3> cxd;
  for (int i=0; i!=3; i++) cxd[i] = cx[i];

#if 0
  const int nterms = 5;  // For Gaussian orbitals (common origin)
#else
  const int nterms = 4;  // For London orbitals
#endif

  const array<complex<double>,5> Ccoeff = {{  cxd[ic]*imag, -2*gamma*imag, 0.5*field[fwd[ic]], -0.5*field[bck[ic]], 0.5*(field[fwd[ic]]*Cx[bck[ic]] - field[bck[ic]]*Cx[fwd[ic]]) }};

#if 0
  cout << endl;
  array<char,3> source = {{ 'x', 'y', 'z'}};
  array<char,3> cdim;
  cdim[0] = source[ic];
  cdim[1] = source[fwd[ic]];
  cdim[2] = source[bck[ic]];
  cout << "ic = " << ic << " ... " << cdim[0] << " " << cdim[1] << " " << cdim[2];
  cout << ", D is a large-component function" << endl;
#endif

  complex<double> out = 0.0;
  for (int c=0; c!=nterms; c++) {
    array<int,3> newc;
    newc[ic] = ang[0][c];
    newc[fwd[ic]] = ang[1][c];
    newc[bck[ic]] = ang[2][c];
    C.change_angular( cx[0]+newc[0], cx[1]+newc[1], cx[2]+newc[2] );
    const complex<double> coef = Ccoeff[c];
    if (coef != 0.0) out += coef * get_eri_matrix_element(field, A, B, C, D);
  }
  return out;
}


complex<double> compute_small_orb_angular(vector<double> field, const array<double,3> Mcoord, const int dim, molecular_orbital A_, molecular_orbital B_, vector<atomic_orbital> basis, const int ia, const int ib) {
  const int nbasis = basis.size();
  complex<double> coeff_prod;
  complex<double> current_term;
  complex<double> Full_angular = 0.0;
  for (int i = 0; i!=nbasis; i++) {
    for (int j = 0; j!=nbasis; j++) {
      coeff_prod = conj(A_.coefficient[i]) * B_.coefficient[j];
      if (abs(coeff_prod)) {
        current_term = get_small_orb_angular_matrix_element(field, Mcoord, dim, basis[i], basis[j], ia, ib);
        Full_angular += (coeff_prod * current_term);
      }
    }
  }
  return Full_angular;
}


complex<double> get_small_orb_angular_matrix_element (const vector<double> field, const array<double,3> Mcoord, const int dim, atomic_orbital A_, atomic_orbital B_, const int ia, const int ib) {

  atomic_orbital A = A_;
  atomic_orbital B = B_;
  const complex<double> imag (0.0, 1.0);
  const array<int,3> fwd = {{ 1, 2, 0 }};
  const array<int,3> bck = {{ 2, 0, 1 }};

  const array<int,5> xinc = {{ -1, 1, 0, 0, 0 }};
  const array<int,5> yinc = {{  0, 0, 0, 1, 0 }};
  const array<int,5> zinc = {{  0, 0, 1, 0, 0 }};
  const array<array<int,5>,3> ang = {{ xinc, yinc, zinc }};

  const double alpha = A.exponent;
  const double beta = B.exponent;
  const array<double,3> Ax = {{ A.position[0], A.position[1], A.position[2] }};
  const array<double,3> Bx = {{ B.position[0], B.position[1], B.position[2] }};
  const array<int,3> ax = {{ A.angular_momentum[0], A.angular_momentum[1], A.angular_momentum[2] }};
  const array<int,3> bx = {{ B.angular_momentum[0], B.angular_momentum[1], B.angular_momentum[2] }};
  array<double,3> axd;
  array<double,3> bxd;
  for (int i=0; i!=3; i++) axd[i] = ax[i];
  for (int i=0; i!=3; i++) bxd[i] = bx[i];

#if 0
  const int nterms = 5;  // For Gaussian orbitals (common origin)
#else
  const int nterms = 4;  // For London orbitals
#endif

  const array<complex<double>,5> Acoeff = {{  axd[ia]*imag, -2*alpha*imag, 0.5*field[fwd[ia]], -0.5*field[bck[ia]], 0.5*(field[fwd[ia]]*Ax[bck[ia]] - field[bck[ia]]*Ax[fwd[ia]]) }};
  const array<complex<double>,5> Bcoeff = {{ -bxd[ib]*imag,  2*beta*imag,  0.5*field[fwd[ib]], -0.5*field[bck[ib]], 0.5*(field[fwd[ib]]*Bx[bck[ib]] - field[bck[ib]]*Bx[fwd[ib]]) }};

  complex<double> out = 0.0;
  for (int a=0; a!=nterms; a++) {
    for (int b=0; b!=nterms; b++) {
      array<int,3> newa;
      array<int,3> newb;
      newa[ia] = ang[0][a];
      newa[fwd[ia]] = ang[1][a];
      newa[bck[ia]] = ang[2][a];
      newb[ib] = ang[0][b];
      newb[fwd[ib]] = ang[1][b];
      newb[bck[ib]] = ang[2][b];
      A.change_angular( ax[0]+newa[0], ax[1]+newa[1], ax[2]+newa[2] );
      B.change_angular( bx[0]+newb[0], bx[1]+newb[1], bx[2]+newb[2] );
      const complex<double> coef = Acoeff[a] * Bcoeff[b];
      if (coef != 0.0) {
        const vector<complex<double>> rawdata = orb_angular(field, Mcoord, A, B);
        out += coef * rawdata[dim];
        assert(rawdata.size() == 3);
      }
    }
  }
  return out;
}


}
