//
// BAGEL - Brilliantly Advanced General Electronic Structure Library
// Filename: bagel_interface.cc
// Copyright (C) 2014 Toru Shiozaki
//
// Author: Ryan D. Reynolds <RyanDReynolds@u.northwestern.edu>
// Maintainer: Shiozaki group
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

// This file defines a function that can be called from Bagel, using its inputs to compute the ERI using the functional architecture from my test code.
// It loops over the various contracted function and angular momentum combinations to compute an entire cartesian batch in series, for easy comparison to Bagel's output.

#include <cassert>
#include <complex>
#include <array>
#include <memory>
#include <vector>
#include <utility>
#include "src/molecule/shell.h"
#include "src/molecule/molecule.h"
#include "ericompute.h"
#include "bagel_interface.h"

using namespace std;

namespace test {

// Returns a vector identifying all the possible Cartesian functions with total angular momentum L
vector<vector<int>> assign_angular (const int L) {
  vector<vector<int>> out = {};
  int x, y, z;
  for (int i=0; i<=L; i++) {
    z = i;
    for (int j=0; j<=(L-i); j++) {
      y = j;
      x = L - i - j;
      vector<int> next = {x, y, z};
      out.push_back (next);
    }
  }
#if 0
  cout << endl << "Assigning angular momentum: Total = " << L << endl;
  for (int i=0; i!=out.size(); i++) {
    cout << out[i][0] << " " << out[i][1] << " " << out[i][2] << endl;
  }
#endif
  return out;
}


// Returns a vector identifying all possible Spherical functions with total angular momentum L
// The elements of this vector identify linear combinations of the Cartesian functions produced by assign_angular (const int L)
vector<vector<double>> spherical_combinations (const int L, const bool convert) {
  vector<vector<double>> out = {};
  if (convert) {
    out.resize (2*L+1);
    if (L == 0) {
      const double one = 1.0;
      //        000      <-- Cartesian orbital indices
      out[0] = {one}; // s
    }
    if (L == 1) {
      const double one = 1.0;
      //        100  010  001      <-- Cartesian orbital indices
      out[0] = {one, 0.0, 0.0}; // p_x
      out[1] = {0.0, one, 0.0}; // p_y
      out[2] = {0.0, 0.0, one}; // p_z
    }
    if (L == 2) {
      const double one = 1.0;
      const double c0 = (sqrt(3.0)/2.0);
      const double c2 = 0.5;
      const double c1 = sqrt(3.0);
      //        200  110  020  101  011  002      <-- Cartesian orbital indices
      out[0] = { c0, 0.0, -c0, 0.0, 0.0, 0.0}; // d_x2-y2
      out[1] = {0.0,  c1, 0.0, 0.0, 0.0, 0.0}; // d_xy
      out[2] = {0.0, 0.0, 0.0,  c1, 0.0, 0.0}; // d_xz
      out[3] = {0.0, 0.0, 0.0, 0.0,  c1, 0.0}; // d_yz
      out[4] = {-c2, 0.0, -c2, 0.0, 0.0, one}; // d_z2
    }
    if (L == 3) {
      const double one = 1.0;
      const double c3 = 1.0 * sqrt(15);
      const double c6 = (1.5/sqrt(5.0)) * sqrt(5.0);
      const double c4 = sqrt(1.2) * sqrt(5.0);
      const double c5 = (sqrt(6.0)/4.0);
      const double c7 = (sqrt(1.2)/4.0) * sqrt(5.0);
      const double c2 = (sqrt(3.0)/2.0) * sqrt(5.0);
      const double c0 = (sqrt(10.0)/4.0);
      const double c1 = (1.5/sqrt(2.0)) * sqrt(5.0);
      //        300  210  120  030  201  111  021  102  012  003      <-- Cartesian orbital indices
      out[0] = { c0, 0.0, -c1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // f_x3-3xy2
      out[1] = {0.0,  c1, 0.0, -c0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // f_3x2y-y3
      out[2] = {0.0, 0.0, 0.0, 0.0,  c2, 0.0, -c2, 0.0, 0.0, 0.0}; // f_zx2-zy2
      out[3] = {0.0, 0.0, 0.0, 0.0, 0.0,  c3, 0.0, 0.0, 0.0, 0.0}; // f_xyz
      out[4] = {-c5, 0.0, -c7, 0.0, 0.0, 0.0, 0.0,  c4, 0.0, 0.0}; // f_xz2
      out[5] = {0.0, -c7, 0.0, -c5, 0.0, 0.0, 0.0, 0.0,  c4, 0.0}; // f_yz2
      out[6] = {0.0, 0.0, 0.0, 0.0, -c6, 0.0, -c6, 0.0, 0.0, one}; // f_z3
    }
    if (L > 3) throw runtime_error ("The test code can only do Cartesian-to-spherical conversion for f-type orbitals and below.");
  } else {
    // If not doing the spherical conversion, just assign all coefficients as one or zero
    const int size = (((L+1)*(L+2))/2);
    out.resize (size);
    for (int i=0; i!=size; i++) {
      out[i].resize(size);
      for (int j=0; j!=size; j++) {
        if (i==j) out[i][j] = 1.0;
        else out[i][j] = 0.0;
      }
    }
  }
  return out;
}


vector<pair<vector<int>,complex<double>>> get_comparison_ERI (const array<shared_ptr<const bagel::Shell>,4>& basisinfo, const array<double,3> magnetic_field) {

  const bool normalize_basis = 0;
  const bool scale_input = 0;
  const bool orthogonalize = 0;

  const vector<double> field = { magnetic_field[0], magnetic_field[1], magnetic_field[2] };
#if 0
  cout << "Magnetic field:  { ";
  for (int i=0; i!=3; i++) {
    cout << field[i];
    if (i!=2) cout << ", ";
    else cout << " }" << endl;
  }
#endif

  // Declare vectors to be used below
  vector<double> positions = {};
  vector<int> full_angular = {};
  vector<int> angular = {};
  vector<int> ncontracted = {};
  vector<bool> spherical = {};
  vector<vector<int>> nprimitive = {};
  vector<vector<double>> exponents = {};
  vector<vector<double>> contraction_coefficients = {};
  nprimitive.resize(4);
  exponents.resize(4);
  contraction_coefficients.resize(4);

  // Pull input data from the four bagel::Shells
  for (int i=0; i!=4; i++) {
    spherical.push_back (basisinfo[i]->spherical());

    full_angular.push_back (basisinfo[i]->angular_number());
    for (int k=0; k!=3; k++) {
      positions.push_back (basisinfo[i]->position(k));
    }
    const int nexponents = basisinfo[i]->num_primitive();
    int counter = 0;
    ncontracted.push_back (basisinfo[i]->num_contracted());
    for (int j=0; j!=nexponents; j++) {
      exponents[i].push_back (basisinfo[i]->exponents(j));
    }
    for (int j=0; j!=ncontracted[i]; j++) {
      const int start = basisinfo[i]->contraction_ranges(j).first;
      const int finish = basisinfo[i]->contraction_ranges(j).second;
      const int nfunctions = finish - start;
      nprimitive[i].push_back (nfunctions);
      counter = counter + nfunctions;
      for (int k=start; k!=finish; k++) {
        contraction_coefficients[i].push_back (basisinfo[i]->contractions(j)[k]);
      }
    }
    assert (counter == nexponents);
  }
  assert (spherical[0]==spherical[1]);
  assert (spherical[2]==spherical[3]);

  vector<int> basis_size = {};
  vector<vector<vector<int>>> cartesian_all = {};
  vector<vector<vector<double>>> spherical_all = {};
  for (int i=0; i!=4; i++) {
    basis_size.push_back (((full_angular[i]+1)*(full_angular[i]+2))/2);
    cartesian_all.push_back (assign_angular (full_angular[i]));
    spherical_all.push_back (spherical_combinations(full_angular[i],spherical[i]));
  }

#if 0
  for (int i=0; i!=4; i++) {
    cout << "MO coefficients for center " << i+1 << ":" << endl;
    for (int j=0; j!=spherical_all[i].size(); j++) {
      for (int k=0; k!=spherical_all[i][j].size(); k++) {
        cout << spherical_all[i][j][k] << "  ";
        cout << endl;
      }
    }
    cout << "Positions for center " << i+1 << ":" << endl;
    for (int j=0; j!=3; j++) {
      cout << "    " << positions[3*i+j] << endl;
    }
  }
#endif

  const int ang0 = spherical_all[0].size();
  const int ang1 = spherical_all[1].size();
  const int ang2 = spherical_all[2].size();
  const int ang3 = spherical_all[3].size();

  // Check that the right values have been assigned for the number of angular momentum possibilities
#if 1
  assert (spherical[0] || ang0 == basis_size[0]);
  assert (spherical[1] || ang1 == basis_size[1]);
  assert (spherical[2] || ang2 == basis_size[2]);
  assert (spherical[3] || ang3 == basis_size[3]);
  assert (!spherical[0] || ang0 == (2*full_angular[0] + 1) );
  assert (!spherical[1] || ang1 == (2*full_angular[1] + 1) );
  assert (!spherical[2] || ang2 == (2*full_angular[2] + 1) );
  assert (!spherical[3] || ang3 == (2*full_angular[3] + 1) );
#endif

  const int fnc0 = ncontracted[0];
  const int fnc1 = ncontracted[1];
  const int fnc2 = ncontracted[2];
  const int fnc3 = ncontracted[3];

  const int total = ang0*ang1*ang2*ang3*fnc0*fnc1*fnc2*fnc3;
  vector<pair<vector<int>,complex<double>>> out = {};

  const int nbasis_contracted = basis_size[0] + basis_size[1] + basis_size[2] + basis_size[3];

  // Iterate over all possible combinations of contracted basis functions and angular momentum for a given shell
  for (int p=0; p!=fnc3; p++) {
    for (int l=0; l!=ang3; l++) {
      for (int o=0; o!=fnc2; o++) {
        for (int k=0; k!=ang2; k++) {
          for (int n=0; n!=fnc1; n++) {
            for (int j=0; j!=ang1; j++) {
              for (int m=0; m!=fnc0; m++) {
                for (int i=0; i!=ang0; i++) {

                  vector<int> indices = {i,j,k,l,m,n,o,p};
                  vector<int> ordered_indices = {p,l,o,k,n,j,m,i};
                  vector<int> ordered_indices2 = {l,k,j,i,p,o,n,m};
                  //const int nbasis = nprimitive[0][m] + nprimitive[1][n] + nprimitive[2][o] + nprimitive[3][p];
                  vector<double> positions_now = {};
                  vector<int> angular_now = {};
                  vector<double> exponents_now = {};
                  vector<double> contractions_now = {};
                  vector<int> nprimitive_now = {};
                  vector<vector<complex<double>>> orbitals = {};
                  orbitals.resize(4);

                  for (int q=0; q!=4; q++) {
                    for (int x=0; x!=basis_size[q]; x++) {
                      const int s = indices[q];
                      const int t = indices[4+q];
                      nprimitive_now.push_back(nprimitive[q][t]);
                      for (int r=0; r!=nprimitive[q][t]; r++) {
                        for (int u=0; u!=3; u++) {
                          positions_now.push_back (positions[3*q+u]);
                          angular_now.push_back (cartesian_all[q][x][u]);
                        }
                      }
                      int position = 0;
                      for (int v=0; v!=t; v++) position += nprimitive[q][v];
                      for (int w=0; w!=nprimitive[q][t]; w++) {
                        exponents_now.push_back (exponents[q][position+w]);
                        contractions_now.push_back (contraction_coefficients[q][position+w]);
                      }
                      for (int y=0; y!=4; y++) {
                        if (y==q) orbitals[y].push_back(spherical_all[q][s][x]);
                        else orbitals[y].push_back(0.0);
                      }
                    }
                  }
                  for (int q=0; q!=4; q++) {
                    assert (orbitals[q].size()==nbasis_contracted);
                    //cout << "Mol. Orbital " << q+1 << ":  ";
                    //for (int z=0; z!=orbitals[q].size(); z++) {
                    //  cout << orbitals[q][z] << "  ";
                    //}
                    //cout << endl;
                  }

                  // Compute the ERI for this particular term!
                  pair<vector<atomic_orbital>,vector<molecular_orbital>> input = prepare_orbitals (nbasis_contracted, normalize_basis, scale_input, orthogonalize, field,
                             positions_now, angular_now, exponents_now, contractions_now, nprimitive_now, orbitals[0], orbitals[1], orbitals[2], orbitals[3]);
                  complex<double> eri = compute_eri (input.first, input.second, field);
                  pair<vector<int>,complex<double>> result (ordered_indices, eri);
                  out.push_back(result);
                }
              }
            }
          }
        }
      }
    }
  }

  assert (total == out.size());

  return out;

}

vector<pair<vector<int>,complex<double>>> get_comparison_NAI (const array<shared_ptr<const bagel::Shell>,2>& basisinfo, const std::shared_ptr<const bagel::Molecule> mol, const array<double,3> magnetic_field) {

  const bool normalize_basis = 0;
  const bool scale_input = 0;
  const bool orthogonalize = 0;

  const vector<double> field = { magnetic_field[0], magnetic_field[1], magnetic_field[2] };

  // Declare vectors to be used below
  vector<double> positions = {};
  vector<int> full_angular = {};
  vector<int> angular = {};
  vector<int> ncontracted = {};
  vector<bool> spherical = {};
  vector<vector<int>> nprimitive = {};
  vector<vector<double>> exponents = {};
  vector<vector<double>> contraction_coefficients = {};
  nprimitive.resize(4);
  exponents.resize(4);
  contraction_coefficients.resize(4);

  // Pull input data from the four bagel::Shells
  for (int i=0; i!=2; i++) {
    spherical.push_back (basisinfo[i]->spherical());

    full_angular.push_back (basisinfo[i]->angular_number());
    for (int k=0; k!=3; k++) {
      positions.push_back (basisinfo[i]->position(k));
    }
    const int nexponents = basisinfo[i]->num_primitive();
    int counter = 0;
    ncontracted.push_back (basisinfo[i]->num_contracted());
    for (int j=0; j!=nexponents; j++) {
      exponents[i].push_back (basisinfo[i]->exponents(j));
    }
    for (int j=0; j!=ncontracted[i]; j++) {
      const int start = basisinfo[i]->contraction_ranges(j).first;
      const int finish = basisinfo[i]->contraction_ranges(j).second;
      const int nfunctions = finish - start;
      nprimitive[i].push_back (nfunctions);
      counter = counter + nfunctions;
      for (int k=start; k!=finish; k++) {
        contraction_coefficients[i].push_back (basisinfo[i]->contractions(j)[k]);
      }
    }
    assert (counter == nexponents);
  }
  assert (spherical[0]==spherical[1]);

  vector<int> basis_size = {};
  vector<vector<vector<int>>> cartesian_all = {};
  vector<vector<vector<double>>> spherical_all = {};
  for (int i=0; i!=2; i++) {
    basis_size.push_back (((full_angular[i]+1)*(full_angular[i]+2))/2);
    cartesian_all.push_back (assign_angular (full_angular[i]));
    spherical_all.push_back (spherical_combinations(full_angular[i],spherical[i]));
  }

#if 0
  for (int i=0; i!=2; i++) {
    cout << "MO coefficients for center " << i+1 << ":" << endl;
    for (int j=0; j!=spherical_all[i].size(); j++) {
      for (int k=0; k!=spherical_all[i][j].size(); k++) {
        cout << spherical_all[i][j][k] << "  ";
        cout << endl;
      }
    }
  }
#endif

  const int ang0 = spherical_all[0].size();
  const int ang1 = spherical_all[1].size();

  // Check that the right values have been assigned for the number of angular momentum possibilities
#if 0
  assert (spherical[0] || ang0 == basis_size[0]);
  assert (spherical[1] || ang1 == basis_size[1]);
  assert (!spherical[0] || ang0 == (2*full_angular[0] + 1) );
  assert (!spherical[1] || ang1 == (2*full_angular[1] + 1) );
#endif

  const int fnc0 = ncontracted[0];
  const int fnc1 = ncontracted[1];

  const int natom = mol->natom();
  const int total = ang0*ang1*fnc0*fnc1*natom;
  vector<pair<vector<int>,complex<double>>> out = {};

  const int nbasis_contracted = basis_size[0] + basis_size[1];

  // Save the nucleus data in a vector
  vector<nucleus> atoms = {};
  for (int i=0; i!=natom; i++) {
    const double Z = mol->atoms(i)->atom_charge();
    const double exp = mol->atoms(i)->atom_exponent();
    const int Zi = Z;
    const double Zd = Zi;
    assert ( Zd == Z );
    vector<double> coords = { 0.0, 0.0, 0.0 };
    for (int j=0; j!=3; j++) {
      coords[j] = mol->atoms(i)->position(j);
    }
    nucleus current (Zi, coords, exp);
    atoms.push_back(current);
  }

  // Iterate over all possible combinations of contracted basis functions and angular momentum for a given shell
  for (int n=0; n!=fnc1; n++) {
    for (int j=0; j!=ang1; j++) {
      for (int m=0; m!=fnc0; m++) {
        for (int i=0; i!=ang0; i++) {
          vector<int> indices = {i,j,m,n};
          vector<int> ordered_indices = {n,j,m,i};
          vector<int> ordered_indices2 = {j,i,n,m};
          complex<double> full_nai = 0.0;
          for (int k=0; k!=natom; k++) {
            //const int nbasis = nprimitive[0][m] + nprimitive[1][n];
            vector<double> positions_now = {};
            vector<int> angular_now = {};
            vector<double> exponents_now = {};
            vector<double> contractions_now = {};
            vector<int> nprimitive_now = {};
            vector<vector<complex<double>>> orbitals = {};
            vector<nucleus> nuclei = {atoms[k]};
            orbitals.resize(4);

            for (int q=0; q!=2; q++) {
              for (int x=0; x!=basis_size[q]; x++) {
                const int s = indices[q];
                const int t = indices[2+q];
                nprimitive_now.push_back(nprimitive[q][t]);
                for (int r=0; r!=nprimitive[q][t]; r++) {
                  for (int u=0; u!=3; u++) {
                    positions_now.push_back (positions[3*q+u]);
                    angular_now.push_back (cartesian_all[q][x][u]);
                  }
                }
                int position = 0;
                for (int v=0; v!=t; v++) position += nprimitive[q][v];
                for (int w=0; w!=nprimitive[q][t]; w++) {
                  exponents_now.push_back (exponents[q][position+w]);
                  contractions_now.push_back (contraction_coefficients[q][position+w]);
                }
                for (int y=0; y!=2; y++) {
                  if (y==q) orbitals[y].push_back(spherical_all[q][s][x]);
                  else orbitals[y].push_back(0.0);
                  orbitals[y+2].push_back(0.0);  // very crude fix.  orbitals[] should only contain two vectors for the NAI.
                }
              }
            }
            for (int q=0; q!=2; q++) {
              assert (orbitals[q].size()==nbasis_contracted);
              //cout << "q = " << q << endl;
              //cout << "orbitals[" << q << "].size() = " << orbitals[q].size() << endl;
              //cout << "nbasis_contracted = " << nbasis_contracted << endl;
              //cout << "Mol. Orbital " << q+1 << ":  ";
              //for (int z=0; z!=orbitals[q].size(); z++) {
              //  cout << orbitals[q][z] << "  ";
              //}
              //cout << endl;
            }

            // Compute the NAI for this particular term!
            pair<vector<atomic_orbital>,vector<molecular_orbital>> input = prepare_orbitals (nbasis_contracted, normalize_basis, scale_input, orthogonalize, field,
                             positions_now, angular_now, exponents_now, contractions_now, nprimitive_now, orbitals[0], orbitals[1], orbitals[2], orbitals[3]);
            complex<double> nai = compute_nai (input.first, input.second, field, nuclei);
            full_nai += nai;
          }

          pair<vector<int>,complex<double>> result (ordered_indices, full_nai);
          out.push_back(result);
        }
      }
    }
  }

  assert (total/natom == out.size());

  return out;
}

vector<pair<vector<int>,complex<double>>> get_comparison_overlap (const array<shared_ptr<const bagel::Shell>,2>& basisinfo, const array<double,3> magnetic_field) {

  const bool normalize_basis = 0;
  const bool scale_input = 0;
  const bool orthogonalize = 0;

  const vector<double> field = { magnetic_field[0], magnetic_field[1], magnetic_field[2] };

  // Declare vectors to be used below
  vector<double> positions = {};
  vector<int> full_angular = {};
  vector<int> angular = {};
  vector<int> ncontracted = {};
  vector<bool> spherical = {};
  vector<vector<int>> nprimitive = {};
  vector<vector<double>> exponents = {};
  vector<vector<double>> contraction_coefficients = {};
  nprimitive.resize(4);
  exponents.resize(4);
  contraction_coefficients.resize(4);

  // Pull input data from the two bagel::Shells
  for (int i=0; i!=2; i++) {
    spherical.push_back (basisinfo[i]->spherical());

    full_angular.push_back (basisinfo[i]->angular_number());
    for (int k=0; k!=3; k++) {
      positions.push_back (basisinfo[i]->position(k));
    }
    const int nexponents = basisinfo[i]->num_primitive();
    int counter = 0;
    ncontracted.push_back (basisinfo[i]->num_contracted());
    for (int j=0; j!=nexponents; j++) {
      exponents[i].push_back (basisinfo[i]->exponents(j));
    }
    for (int j=0; j!=ncontracted[i]; j++) {
      const int start = basisinfo[i]->contraction_ranges(j).first;
      const int finish = basisinfo[i]->contraction_ranges(j).second;
      const int nfunctions = finish - start;
      nprimitive[i].push_back (nfunctions);
      counter = counter + nfunctions;
      for (int k=start; k!=finish; k++) {
        contraction_coefficients[i].push_back (basisinfo[i]->contractions(j)[k]);
      }
    }
    assert (counter == nexponents);
  }
  assert (spherical[0]==spherical[1]);

  vector<int> basis_size = {};
  vector<vector<vector<int>>> cartesian_all = {};
  vector<vector<vector<double>>> spherical_all = {};
  for (int i=0; i!=2; i++) {
    basis_size.push_back (((full_angular[i]+1)*(full_angular[i]+2))/2);
    cartesian_all.push_back (assign_angular (full_angular[i]));
    spherical_all.push_back (spherical_combinations(full_angular[i],spherical[i]));
  }

#if 0
  for (int i=0; i!=2; i++) {
    cout << "MO coefficients for center " << i+1 << ":" << endl;
    for (int j=0; j!=spherical_all[i].size(); j++) {
      cout << "  Orbital " << j+1 << ":  ";
      for (int k=0; k!=spherical_all[i][j].size(); k++) {
        cout << spherical_all[i][j][k] << "  ";
      }
      cout << endl;
    }
  }
#endif

  const int ang0 = spherical_all[0].size();
  const int ang1 = spherical_all[1].size();

  // Check that the right values have been assigned for the number of angular momentum possibilities
#if 1
  assert (spherical[0] || ang0 == basis_size[0]);
  assert (spherical[1] || ang1 == basis_size[1]);
  assert (!spherical[0] || ang0 == (2*full_angular[0] + 1) );
  assert (!spherical[1] || ang1 == (2*full_angular[1] + 1) );
#endif

  const int fnc0 = ncontracted[0];
  const int fnc1 = ncontracted[1];

  const int total = ang0*ang1*fnc0*fnc1;
  vector<pair<vector<int>,complex<double>>> out = {};

  const int nbasis_contracted = basis_size[0] + basis_size[1];

  // Iterate over all possible combinations of contracted basis functions and angular momentum for a given shell
  for (int n=0; n!=fnc1; n++) {
    for (int j=0; j!=ang1; j++) {
      for (int m=0; m!=fnc0; m++) {
        for (int i=0; i!=ang0; i++) {
          vector<int> indices = {i,j,m,n};
          vector<int> ordered_indices = {n,j,m,i};
          vector<int> ordered_indices2 = {j,i,n,m};
          //const int nbasis = nprimitive[0][m] + nprimitive[1][n];
          vector<double> positions_now = {};
          vector<int> angular_now = {};
          vector<double> exponents_now = {};
          vector<double> contractions_now = {};
          vector<int> nprimitive_now = {};
          vector<vector<complex<double>>> orbitals = {};
          orbitals.resize(4);

          for (int q=0; q!=2; q++) {
            for (int x=0; x!=basis_size[q]; x++) {
              const int s = indices[q];
              const int t = indices[2+q];
              nprimitive_now.push_back(nprimitive[q][t]);
              for (int r=0; r!=nprimitive[q][t]; r++) {
                for (int u=0; u!=3; u++) {
                  positions_now.push_back (positions[3*q+u]);
                  angular_now.push_back (cartesian_all[q][x][u]);
                }
              }
              int position = 0;
              for (int v=0; v!=t; v++) position += nprimitive[q][v];
              for (int w=0; w!=nprimitive[q][t]; w++) {
                exponents_now.push_back (exponents[q][position+w]);
                contractions_now.push_back (contraction_coefficients[q][position+w]);
              }
              for (int y=0; y!=2; y++) {
                if (y==q) orbitals[y].push_back(spherical_all[q][s][x]);
                else orbitals[y].push_back(0.0);
                orbitals[y+2].push_back(0.0);  // very crude fix.  orbitals[] should only contain two vectors for the NAI.
              }
            }
          }
          for (int q=0; q!=2; q++) {
            assert (orbitals[q].size()==nbasis_contracted);
#if 0
            cout << "q = " << q << endl;
            cout << "orbitals[" << q << "].size() = " << orbitals[q].size() << endl;
            cout << "nbasis_contracted = " << nbasis_contracted << endl;
            cout << "Mol. Orbital " << q+1 << ":  ";
            for (int z=0; z!=orbitals[q].size(); z++) {
              cout << orbitals[q][z] << "  ";
            }
            cout << endl;
#endif
          }

          // Compute the NAI for this particular term!
          pair<vector<atomic_orbital>,vector<molecular_orbital>> input = prepare_orbitals (nbasis_contracted, normalize_basis, scale_input, orthogonalize, field,
                           positions_now, angular_now, exponents_now, contractions_now, nprimitive_now, orbitals[0], orbitals[1], orbitals[2], orbitals[3]);
          complex<double> overlap = overlap_MO (field, input.second[0], input.second[1], input.first);

          pair<vector<int>,complex<double>> result (ordered_indices, overlap);
          out.push_back(result);
        }
      }
    }
  }

  assert (total == out.size());

  return out;
}

vector<pair<vector<int>,complex<double>>> get_comparison_kinetic (const array<shared_ptr<const bagel::Shell>,2>& basisinfo, const array<double,3> magnetic_field) {

  const bool normalize_basis = 0;
  const bool scale_input = 0;
  const bool orthogonalize = 0;

  const vector<double> field = { magnetic_field[0], magnetic_field[1], magnetic_field[2] };

  // Declare vectors to be used below
  vector<double> positions = {};
  vector<int> full_angular = {};
  vector<int> angular = {};
  vector<int> ncontracted = {};
  vector<bool> spherical = {};
  vector<vector<int>> nprimitive = {};
  vector<vector<double>> exponents = {};
  vector<vector<double>> contraction_coefficients = {};
  nprimitive.resize(4);
  exponents.resize(4);
  contraction_coefficients.resize(4);

  // Pull input data from the two bagel::Shells
  for (int i=0; i!=2; i++) {
    spherical.push_back (basisinfo[i]->spherical());

    full_angular.push_back (basisinfo[i]->angular_number());
    for (int k=0; k!=3; k++) {
      positions.push_back (basisinfo[i]->position(k));
    }
    const int nexponents = basisinfo[i]->num_primitive();
    int counter = 0;
    ncontracted.push_back (basisinfo[i]->num_contracted());
    for (int j=0; j!=nexponents; j++) {
      exponents[i].push_back (basisinfo[i]->exponents(j));
    }
    for (int j=0; j!=ncontracted[i]; j++) {
      const int start = basisinfo[i]->contraction_ranges(j).first;
      const int finish = basisinfo[i]->contraction_ranges(j).second;
      const int nfunctions = finish - start;
      nprimitive[i].push_back (nfunctions);
      counter = counter + nfunctions;
      for (int k=start; k!=finish; k++) {
        contraction_coefficients[i].push_back (basisinfo[i]->contractions(j)[k]);
      }
    }
    assert (counter == nexponents);
  }
  assert (spherical[0]==spherical[1]);

  vector<int> basis_size = {};
  vector<vector<vector<int>>> cartesian_all = {};
  vector<vector<vector<double>>> spherical_all = {};
  for (int i=0; i!=2; i++) {
    basis_size.push_back (((full_angular[i]+1)*(full_angular[i]+2))/2);
    cartesian_all.push_back (assign_angular (full_angular[i]));
    spherical_all.push_back (spherical_combinations(full_angular[i],spherical[i]));
  }

#if 0
  for (int i=0; i!=2; i++) {
    cout << "MO coefficients for center " << i+1 << ":" << endl;
    for (int j=0; j!=spherical_all[i].size(); j++) {
      cout << "  Orbital " << j+1 << ":  ";
      for (int k=0; k!=spherical_all[i][j].size(); k++) {
        cout << spherical_all[i][j][k] << "  ";
      }
      cout << endl;
    }
  }
#endif

  const int ang0 = spherical_all[0].size();
  const int ang1 = spherical_all[1].size();

  // Check that the right values have been assigned for the number of angular momentum possibilities
#if 1
  assert (spherical[0] || ang0 == basis_size[0]);
  assert (spherical[1] || ang1 == basis_size[1]);
  assert (!spherical[0] || ang0 == (2*full_angular[0] + 1) );
  assert (!spherical[1] || ang1 == (2*full_angular[1] + 1) );
#endif

  const int fnc0 = ncontracted[0];
  const int fnc1 = ncontracted[1];

  const int total = ang0*ang1*fnc0*fnc1;
  vector<pair<vector<int>,complex<double>>> out = {};

  const int nbasis_contracted = basis_size[0] + basis_size[1];

  // Iterate over all possible combinations of contracted basis functions and angular momentum for a given shell
  for (int n=0; n!=fnc1; n++) {
    for (int j=0; j!=ang1; j++) {
      for (int m=0; m!=fnc0; m++) {
        for (int i=0; i!=ang0; i++) {
          vector<int> indices = {i,j,m,n};
          vector<int> ordered_indices = {n,j,m,i};
          vector<int> ordered_indices2 = {j,i,n,m};
          //const int nbasis = nprimitive[0][m] + nprimitive[1][n];
          vector<double> positions_now = {};
          vector<int> angular_now = {};
          vector<double> exponents_now = {};
          vector<double> contractions_now = {};
          vector<int> nprimitive_now = {};
          vector<vector<complex<double>>> orbitals = {};
          orbitals.resize(4);

          for (int q=0; q!=2; q++) {
            for (int x=0; x!=basis_size[q]; x++) {
              const int s = indices[q];
              const int t = indices[2+q];
              nprimitive_now.push_back(nprimitive[q][t]);
              for (int r=0; r!=nprimitive[q][t]; r++) {
                for (int u=0; u!=3; u++) {
                  positions_now.push_back (positions[3*q+u]);
                  angular_now.push_back (cartesian_all[q][x][u]);
                }
              }
              int position = 0;
              for (int v=0; v!=t; v++) position += nprimitive[q][v];
              for (int w=0; w!=nprimitive[q][t]; w++) {
                exponents_now.push_back (exponents[q][position+w]);
                contractions_now.push_back (contraction_coefficients[q][position+w]);
              }
              for (int y=0; y!=2; y++) {
                if (y==q) orbitals[y].push_back(spherical_all[q][s][x]);
                else orbitals[y].push_back(0.0);
                orbitals[y+2].push_back(0.0);  // very crude fix.  orbitals[] should only contain two vectors for the NAI.
              }
            }
          }
          for (int q=0; q!=2; q++) {
            assert (orbitals[q].size()==nbasis_contracted);
#if 0
            cout << "q = " << q << endl;
            cout << "orbitals[" << q << "].size() = " << orbitals[q].size() << endl;
            cout << "nbasis_contracted = " << nbasis_contracted << endl;
            cout << "Mol. Orbital " << q+1 << ":  ";
            for (int z=0; z!=orbitals[q].size(); z++) {
              cout << orbitals[q][z] << "  ";
            }
            cout << endl;
#endif
          }

          // Compute the NAI for this particular term!
          pair<vector<atomic_orbital>,vector<molecular_orbital>> input = prepare_orbitals (nbasis_contracted, normalize_basis, scale_input, orthogonalize, field,
                           positions_now, angular_now, exponents_now, contractions_now, nprimitive_now, orbitals[0], orbitals[1], orbitals[2], orbitals[3]);
          complex<double> kinetic = kinetic_MO (field, input.second[0], input.second[1], input.first);

          pair<vector<int>,complex<double>> result (ordered_indices, kinetic);
          out.push_back(result);
        }
      }
    }
  }

  assert (total == out.size());

  return out;
}

vector<pair<vector<int>,complex<double>>> get_comparison_momentum (const array<shared_ptr<const bagel::Shell>,2>& basisinfo, const array<double,3> magnetic_field) {

  const bool normalize_basis = 0;
  const bool scale_input = 0;
  const bool orthogonalize = 0;

  const vector<double> field = { magnetic_field[0], magnetic_field[1], magnetic_field[2] };

  // Declare vectors to be used below
  vector<double> positions = {};
  vector<int> full_angular = {};
  vector<int> angular = {};
  vector<int> ncontracted = {};
  vector<bool> spherical = {};
  vector<vector<int>> nprimitive = {};
  vector<vector<double>> exponents = {};
  vector<vector<double>> contraction_coefficients = {};
  nprimitive.resize(4);
  exponents.resize(4);
  contraction_coefficients.resize(4);

  // Pull input data from the two bagel::Shells
  for (int i=0; i!=2; i++) {
    spherical.push_back (basisinfo[i]->spherical());

    full_angular.push_back (basisinfo[i]->angular_number());
    for (int k=0; k!=3; k++) {
      positions.push_back (basisinfo[i]->position(k));
    }
    const int nexponents = basisinfo[i]->num_primitive();
    int counter = 0;
    ncontracted.push_back (basisinfo[i]->num_contracted());
    for (int j=0; j!=nexponents; j++) {
      exponents[i].push_back (basisinfo[i]->exponents(j));
    }
    for (int j=0; j!=ncontracted[i]; j++) {
      const int start = basisinfo[i]->contraction_ranges(j).first;
      const int finish = basisinfo[i]->contraction_ranges(j).second;
      const int nfunctions = finish - start;
      nprimitive[i].push_back (nfunctions);
      counter = counter + nfunctions;
      for (int k=start; k!=finish; k++) {
        contraction_coefficients[i].push_back (basisinfo[i]->contractions(j)[k]);
      }
    }
    assert (counter == nexponents);
  }
  assert (spherical[0]==spherical[1]);

  vector<int> basis_size = {};
  vector<vector<vector<int>>> cartesian_all = {};
  vector<vector<vector<double>>> spherical_all = {};
  for (int i=0; i!=2; i++) {
    basis_size.push_back (((full_angular[i]+1)*(full_angular[i]+2))/2);
    cartesian_all.push_back (assign_angular (full_angular[i]));
    spherical_all.push_back (spherical_combinations(full_angular[i],spherical[i]));
  }

#if 0
  for (int i=0; i!=2; i++) {
    cout << "MO coefficients for center " << i+1 << ":" << endl;
    for (int j=0; j!=spherical_all[i].size(); j++) {
      cout << "  Orbital " << j+1 << ":  ";
      for (int k=0; k!=spherical_all[i][j].size(); k++) {
        cout << spherical_all[i][j][k] << "  ";
      }
      cout << endl;
    }
  }
#endif

  const int ang0 = spherical_all[0].size();
  const int ang1 = spherical_all[1].size();

  // Check that the right values have been assigned for the number of angular momentum possibilities
#if 1
  assert (spherical[0] || ang0 == basis_size[0]);
  assert (spherical[1] || ang1 == basis_size[1]);
  assert (!spherical[0] || ang0 == (2*full_angular[0] + 1) );
  assert (!spherical[1] || ang1 == (2*full_angular[1] + 1) );
#endif

  const int fnc0 = ncontracted[0];
  const int fnc1 = ncontracted[1];

  const int total = ang0*ang1*fnc0*fnc1;
  vector<pair<vector<int>,vector<complex<double>>>> intermediate = {};
  vector<pair<vector<int>,complex<double>>> out = {};

  const int nbasis_contracted = basis_size[0] + basis_size[1];

  // Iterate over all possible combinations of contracted basis functions and angular momentum for a given shell
  for (int n=0; n!=fnc1; n++) {
    for (int j=0; j!=ang1; j++) {
      for (int m=0; m!=fnc0; m++) {
        for (int i=0; i!=ang0; i++) {
          vector<int> indices = {i,j,m,n};
          vector<int> ordered_indices = {n,j,m,i};
          vector<int> ordered_indices2 = {j,i,n,m};
          //const int nbasis = nprimitive[0][m] + nprimitive[1][n];
          vector<double> positions_now = {};
          vector<int> angular_now = {};
          vector<double> exponents_now = {};
          vector<double> contractions_now = {};
          vector<int> nprimitive_now = {};
          vector<vector<complex<double>>> orbitals = {};
          orbitals.resize(4);

          for (int q=0; q!=2; q++) {
            for (int x=0; x!=basis_size[q]; x++) {
              const int s = indices[q];
              const int t = indices[2+q];
              nprimitive_now.push_back(nprimitive[q][t]);
              for (int r=0; r!=nprimitive[q][t]; r++) {
                for (int u=0; u!=3; u++) {
                  positions_now.push_back (positions[3*q+u]);
                  angular_now.push_back (cartesian_all[q][x][u]);
                }
              }
              int position = 0;
              for (int v=0; v!=t; v++) position += nprimitive[q][v];
              for (int w=0; w!=nprimitive[q][t]; w++) {
                exponents_now.push_back (exponents[q][position+w]);
                contractions_now.push_back (contraction_coefficients[q][position+w]);
              }
              for (int y=0; y!=2; y++) {
                if (y==q) orbitals[y].push_back(spherical_all[q][s][x]);
                else orbitals[y].push_back(0.0);
                orbitals[y+2].push_back(0.0);  // very crude fix.  orbitals[] should only contain two vectors for 2-index integrals.
              }
            }
          }
          for (int q=0; q!=2; q++) {
            assert (orbitals[q].size()==nbasis_contracted);
          }

          // Compute the result for this particular term!
          pair<vector<atomic_orbital>,vector<molecular_orbital>> input = prepare_orbitals (nbasis_contracted, normalize_basis, scale_input, orthogonalize, field,
                           positions_now, angular_now, exponents_now, contractions_now, nprimitive_now, orbitals[0], orbitals[1], orbitals[2], orbitals[3]);
          vector<complex<double>> momentum = momentum_MO (field, input.second[0], input.second[1], input.first);

          pair<vector<int>,vector<complex<double>>> result (ordered_indices, momentum);
          intermediate.push_back(result);
        }
      }
    }
  }

  assert (total == intermediate.size());

  // Now sort the x, y, and z dimensions in the order expected by Bagel
  for (int i=0; i!=3; i++) {
    for (int j=0; j!=total; j++) {
      complex<double> current = intermediate[j].second[i];
      vector<int> current_indices = intermediate[j].first;
      current_indices.push_back(i);
      pair <vector<int>,complex<double>> output (current_indices, current);
      out.push_back(output);
    }
  }

  return out;
}


vector<pair<vector<int>,complex<double>>> get_comparison_orb_angular (const array<shared_ptr<const bagel::Shell>,2>& basisinfo, const array<double,3> magnetic_field, const array<double,3> Mcoord) {

  const bool normalize_basis = 0;
  const bool scale_input = 0;
  const bool orthogonalize = 0;

  const vector<double> field = { magnetic_field[0], magnetic_field[1], magnetic_field[2] };

  // Declare vectors to be used below
  vector<double> positions = {};
  vector<int> full_angular = {};
  vector<int> angular = {};
  vector<int> ncontracted = {};
  vector<bool> spherical = {};
  vector<vector<int>> nprimitive = {};
  vector<vector<double>> exponents = {};
  vector<vector<double>> contraction_coefficients = {};
  nprimitive.resize(4);
  exponents.resize(4);
  contraction_coefficients.resize(4);

  // Pull input data from the two bagel::Shells
  for (int i=0; i!=2; i++) {
    spherical.push_back (basisinfo[i]->spherical());

    full_angular.push_back (basisinfo[i]->angular_number());
    for (int k=0; k!=3; k++) {
      positions.push_back (basisinfo[i]->position(k));
    }
    const int nexponents = basisinfo[i]->num_primitive();
    int counter = 0;
    ncontracted.push_back (basisinfo[i]->num_contracted());
    for (int j=0; j!=nexponents; j++) {
      exponents[i].push_back (basisinfo[i]->exponents(j));
    }
    for (int j=0; j!=ncontracted[i]; j++) {
      const int start = basisinfo[i]->contraction_ranges(j).first;
      const int finish = basisinfo[i]->contraction_ranges(j).second;
      const int nfunctions = finish - start;
      nprimitive[i].push_back (nfunctions);
      counter = counter + nfunctions;
      for (int k=start; k!=finish; k++) {
        contraction_coefficients[i].push_back (basisinfo[i]->contractions(j)[k]);
      }
    }
    assert (counter == nexponents);
  }
  assert (spherical[0]==spherical[1]);

  vector<int> basis_size = {};
  vector<vector<vector<int>>> cartesian_all = {};
  vector<vector<vector<double>>> spherical_all = {};
  for (int i=0; i!=2; i++) {
    basis_size.push_back (((full_angular[i]+1)*(full_angular[i]+2))/2);
    cartesian_all.push_back (assign_angular (full_angular[i]));
    spherical_all.push_back (spherical_combinations(full_angular[i],spherical[i]));
  }

#if 0
  for (int i=0; i!=2; i++) {
    cout << "MO coefficients for center " << i+1 << ":" << endl;
    for (int j=0; j!=spherical_all[i].size(); j++) {
      cout << "  Orbital " << j+1 << ":  ";
      for (int k=0; k!=spherical_all[i][j].size(); k++) {
        cout << spherical_all[i][j][k] << "  ";
      }
      cout << endl;
    }
  }
#endif

  const int ang0 = spherical_all[0].size();
  const int ang1 = spherical_all[1].size();

  // Check that the right values have been assigned for the number of angular momentum possibilities
#if 1
  assert (spherical[0] || ang0 == basis_size[0]);
  assert (spherical[1] || ang1 == basis_size[1]);
  assert (!spherical[0] || ang0 == (2*full_angular[0] + 1) );
  assert (!spherical[1] || ang1 == (2*full_angular[1] + 1) );
#endif

  const int fnc0 = ncontracted[0];
  const int fnc1 = ncontracted[1];

  const int total = ang0*ang1*fnc0*fnc1;
  vector<pair<vector<int>,vector<complex<double>>>> intermediate = {};
  vector<pair<vector<int>,complex<double>>> out = {};

  const int nbasis_contracted = basis_size[0] + basis_size[1];

  // Iterate over all possible combinations of contracted basis functions and angular momentum for a given shell
  for (int n=0; n!=fnc1; n++) {
    for (int j=0; j!=ang1; j++) {
      for (int m=0; m!=fnc0; m++) {
        for (int i=0; i!=ang0; i++) {
          vector<int> indices = {i,j,m,n};
          vector<int> ordered_indices = {n,j,m,i};
          vector<int> ordered_indices2 = {j,i,n,m};
          //const int nbasis = nprimitive[0][m] + nprimitive[1][n];
          vector<double> positions_now = {};
          vector<int> angular_now = {};
          vector<double> exponents_now = {};
          vector<double> contractions_now = {};
          vector<int> nprimitive_now = {};
          vector<vector<complex<double>>> orbitals = {};
          orbitals.resize(4);

          for (int q=0; q!=2; q++) {
            for (int x=0; x!=basis_size[q]; x++) {
              const int s = indices[q];
              const int t = indices[2+q];
              nprimitive_now.push_back(nprimitive[q][t]);
              for (int r=0; r!=nprimitive[q][t]; r++) {
                for (int u=0; u!=3; u++) {
                  positions_now.push_back (positions[3*q+u]);
                  angular_now.push_back (cartesian_all[q][x][u]);
                }
              }
              int position = 0;
              for (int v=0; v!=t; v++) position += nprimitive[q][v];
              for (int w=0; w!=nprimitive[q][t]; w++) {
                exponents_now.push_back (exponents[q][position+w]);
                contractions_now.push_back (contraction_coefficients[q][position+w]);
              }
              for (int y=0; y!=2; y++) {
                if (y==q) orbitals[y].push_back(spherical_all[q][s][x]);
                else orbitals[y].push_back(0.0);
                orbitals[y+2].push_back(0.0);  // very crude fix.  orbitals[] should only contain two vectors for 2-index integrals.
              }
            }
          }
          for (int q=0; q!=2; q++) {
            assert (orbitals[q].size()==nbasis_contracted);
          }

          // Compute the result for this particular term!
          pair<vector<atomic_orbital>,vector<molecular_orbital>> input = prepare_orbitals (nbasis_contracted, normalize_basis, scale_input, orthogonalize, field,
                           positions_now, angular_now, exponents_now, contractions_now, nprimitive_now, orbitals[0], orbitals[1], orbitals[2], orbitals[3]);
          vector<complex<double>> orb_angular = orb_angular_MO(field, Mcoord, input.second[0], input.second[1], input.first);

          pair<vector<int>,vector<complex<double>>> result (ordered_indices, orb_angular);
          intermediate.push_back(result);
        }
      }
    }
  }

  assert (total == intermediate.size());

  // Now sort the x, y, and z dimensions in the order expected by Bagel
  for (int i=0; i!=3; i++) {
    for (int j=0; j!=total; j++) {
      complex<double> current = intermediate[j].second[i];
      vector<int> current_indices = intermediate[j].first;
      current_indices.push_back(i);
      pair <vector<int>,complex<double>> output (current_indices, current);
      out.push_back(output);
    }
  }

  return out;
}


vector<pair<vector<int>,complex<double>>> get_comparison_small_orb_angular (const array<shared_ptr<const bagel::Shell>,2>& basisinfo, const array<double,3> magnetic_field, const array<double,3> Mcoord, const int ia, const int ib) {

  const bool normalize_basis = 0;
  const bool scale_input = 0;
  const bool orthogonalize = 0;

  const vector<double> field = { magnetic_field[0], magnetic_field[1], magnetic_field[2] };

  // Declare vectors to be used below
  vector<double> positions = {};
  vector<int> full_angular = {};
  vector<int> angular = {};
  vector<int> ncontracted = {};
  vector<bool> spherical = {};
  vector<vector<int>> nprimitive = {};
  vector<vector<double>> exponents = {};
  vector<vector<double>> contraction_coefficients = {};
  nprimitive.resize(4);
  exponents.resize(4);
  contraction_coefficients.resize(4);

  // Pull input data from the two bagel::Shells
  for (int i=0; i!=2; i++) {
    spherical.push_back (basisinfo[i]->spherical());

    full_angular.push_back (basisinfo[i]->angular_number());
    for (int k=0; k!=3; k++) {
      positions.push_back (basisinfo[i]->position(k));
    }
    const int nexponents = basisinfo[i]->num_primitive();
    int counter = 0;
    ncontracted.push_back (basisinfo[i]->num_contracted());
    for (int j=0; j!=nexponents; j++) {
      exponents[i].push_back (basisinfo[i]->exponents(j));
    }
    for (int j=0; j!=ncontracted[i]; j++) {
      const int start = basisinfo[i]->contraction_ranges(j).first;
      const int finish = basisinfo[i]->contraction_ranges(j).second;
      const int nfunctions = finish - start;
      nprimitive[i].push_back (nfunctions);
      counter = counter + nfunctions;
      for (int k=start; k!=finish; k++) {
        contraction_coefficients[i].push_back (basisinfo[i]->contractions(j)[k]);
      }
    }
    assert (counter == nexponents);
  }
  assert (spherical[0]==spherical[1]);

  vector<int> basis_size = {};
  vector<vector<vector<int>>> cartesian_all = {};
  vector<vector<vector<double>>> spherical_all = {};
  for (int i=0; i!=2; i++) {
    basis_size.push_back (((full_angular[i]+1)*(full_angular[i]+2))/2);
    cartesian_all.push_back (assign_angular (full_angular[i]));
    spherical_all.push_back (spherical_combinations(full_angular[i],spherical[i]));
  }

#if 0
  for (int i=0; i!=2; i++) {
    cout << "MO coefficients for center " << i+1 << ":" << endl;
    for (int j=0; j!=spherical_all[i].size(); j++) {
      cout << "  Orbital " << j+1 << ":  ";
      for (int k=0; k!=spherical_all[i][j].size(); k++) {
        cout << spherical_all[i][j][k] << "  ";
      }
      cout << endl;
    }
  }
#endif

  const int ang0 = spherical_all[0].size();
  const int ang1 = spherical_all[1].size();

  // Check that the right values have been assigned for the number of angular momentum possibilities
#if 1
  assert (spherical[0] || ang0 == basis_size[0]);
  assert (spherical[1] || ang1 == basis_size[1]);
  assert (!spherical[0] || ang0 == (2*full_angular[0] + 1) );
  assert (!spherical[1] || ang1 == (2*full_angular[1] + 1) );
#endif

  const int fnc0 = ncontracted[0];
  const int fnc1 = ncontracted[1];

  const int total = ang0*ang1*fnc0*fnc1;
  vector<pair<vector<int>,vector<complex<double>>>> intermediate = {};
  vector<pair<vector<int>,complex<double>>> out = {};

  const int nbasis_contracted = basis_size[0] + basis_size[1];

  // Iterate over all possible combinations of contracted basis functions and angular momentum for a given shell
  for (int n=0; n!=fnc1; n++) {
    for (int j=0; j!=ang1; j++) {
      for (int m=0; m!=fnc0; m++) {
        for (int i=0; i!=ang0; i++) {
          vector<int> indices = {i,j,m,n};
          vector<int> ordered_indices = {n,j,m,i};
          vector<int> ordered_indices2 = {j,i,n,m};
          //const int nbasis = nprimitive[0][m] + nprimitive[1][n];
          vector<double> positions_now = {};
          vector<int> angular_now = {};
          vector<double> exponents_now = {};
          vector<double> contractions_now = {};
          vector<int> nprimitive_now = {};
          vector<vector<complex<double>>> orbitals = {};
          orbitals.resize(4);

          for (int q=0; q!=2; q++) {
            for (int x=0; x!=basis_size[q]; x++) {
              const int s = indices[q];
              const int t = indices[2+q];
              nprimitive_now.push_back(nprimitive[q][t]);
              for (int r=0; r!=nprimitive[q][t]; r++) {
                for (int u=0; u!=3; u++) {
                  positions_now.push_back (positions[3*q+u]);
                  angular_now.push_back (cartesian_all[q][x][u]);
                }
              }
              int position = 0;
              for (int v=0; v!=t; v++) position += nprimitive[q][v];
              for (int w=0; w!=nprimitive[q][t]; w++) {
                exponents_now.push_back (exponents[q][position+w]);
                contractions_now.push_back (contraction_coefficients[q][position+w]);
              }
              for (int y=0; y!=2; y++) {
                if (y==q) orbitals[y].push_back(spherical_all[q][s][x]);
                else orbitals[y].push_back(0.0);
                orbitals[y+2].push_back(0.0);  // very crude fix.  orbitals[] should only contain two vectors for 2-index integrals.
              }
            }
          }
          for (int q=0; q!=2; q++) {
            assert (orbitals[q].size()==nbasis_contracted);
          }

          // Compute the result for this particular term!
          pair<vector<atomic_orbital>,vector<molecular_orbital>> input = prepare_orbitals (nbasis_contracted, normalize_basis, scale_input, orthogonalize, field,
                           positions_now, angular_now, exponents_now, contractions_now, nprimitive_now, orbitals[0], orbitals[1], orbitals[2], orbitals[3]);
          vector<complex<double>> xyzout = {};
          for (int dim = 0; dim != 3; dim++) {
            complex<double> orbout = compute_small_orb_angular(field, Mcoord, dim, input.second[0], input.second[1], input.first, ia, ib);
            xyzout.push_back(orbout);
          }

          pair<vector<int>,vector<complex<double>>> result (ordered_indices, xyzout);
          intermediate.push_back(result);
        }
      }
    }
  }

  assert (total == intermediate.size());

  // Now sort the x, y, and z dimensions in the order expected by Bagel
  for (int i=0; i!=3; i++) {
    for (int j=0; j!=total; j++) {
      complex<double> current = intermediate[j].second[i];
      vector<int> current_indices = intermediate[j].first;
      current_indices.push_back(i);
      pair <vector<int>,complex<double>> output (current_indices, current);
      out.push_back(output);
    }
  }

  return out;
}


vector<pair<vector<int>,complex<double>>> get_comparison_smallNAI (const array<shared_ptr<const bagel::Shell>,2>& basisinfo, const shared_ptr<const bagel::Molecule> mol, const array<double,3> magnetic_field, const int ia, const int ib) {

  const bool normalize_basis = 0;
  const bool scale_input = 0;
  const bool orthogonalize = 0;

  const vector<double> field = { magnetic_field[0], magnetic_field[1], magnetic_field[2] };

  // Declare vectors to be used below
  vector<double> positions = {};
  vector<int> full_angular = {};
  vector<int> angular = {};
  vector<int> ncontracted = {};
  vector<bool> spherical = {};
  vector<vector<int>> nprimitive = {};
  vector<vector<double>> exponents = {};
  vector<vector<double>> contraction_coefficients = {};
  nprimitive.resize(4);
  exponents.resize(4);
  contraction_coefficients.resize(4);

  // Pull input data from the four bagel::Shells
  for (int i=0; i!=2; i++) {
    spherical.push_back (basisinfo[i]->spherical());

    full_angular.push_back (basisinfo[i]->angular_number());
    for (int k=0; k!=3; k++) {
      positions.push_back (basisinfo[i]->position(k));
    }
    const int nexponents = basisinfo[i]->num_primitive();
    int counter = 0;
    ncontracted.push_back (basisinfo[i]->num_contracted());
    for (int j=0; j!=nexponents; j++) {
      exponents[i].push_back (basisinfo[i]->exponents(j));
    }
    for (int j=0; j!=ncontracted[i]; j++) {
      const int start = basisinfo[i]->contraction_ranges(j).first;
      const int finish = basisinfo[i]->contraction_ranges(j).second;
      const int nfunctions = finish - start;
      nprimitive[i].push_back (nfunctions);
      counter = counter + nfunctions;
      for (int k=start; k!=finish; k++) {
        contraction_coefficients[i].push_back (basisinfo[i]->contractions(j)[k]);
      }
    }
    assert (counter == nexponents);
  }
  assert (spherical[0]==spherical[1]);

  vector<int> basis_size = {};
  vector<vector<vector<int>>> cartesian_all = {};
  vector<vector<vector<double>>> spherical_all = {};
  for (int i=0; i!=2; i++) {
    basis_size.push_back (((full_angular[i]+1)*(full_angular[i]+2))/2);
    cartesian_all.push_back (assign_angular (full_angular[i]));
    spherical_all.push_back (spherical_combinations(full_angular[i],spherical[i]));
  }

#if 0
  for (int i=0; i!=2; i++) {
    cout << "MO coefficients for center " << i+1 << ":" << endl;
    for (int j=0; j!=spherical_all[i].size(); j++) {
      for (int k=0; k!=spherical_all[i][j].size(); k++) {
        cout << spherical_all[i][j][k] << "  ";
        cout << endl;
      }
    }
  }
#endif

  const int ang0 = spherical_all[0].size();
  const int ang1 = spherical_all[1].size();

  // Check that the right values have been assigned for the number of angular momentum possibilities
#if 0
  assert (spherical[0] || ang0 == basis_size[0]);
  assert (spherical[1] || ang1 == basis_size[1]);
  assert (!spherical[0] || ang0 == (2*full_angular[0] + 1) );
  assert (!spherical[1] || ang1 == (2*full_angular[1] + 1) );
#endif

  const int fnc0 = ncontracted[0];
  const int fnc1 = ncontracted[1];

  const int natom = mol->natom();
  const int total = ang0*ang1*fnc0*fnc1*natom;
  vector<pair<vector<int>,complex<double>>> out = {};

  const int nbasis_contracted = basis_size[0] + basis_size[1];

  // Save the nucleus data in a vector
  vector<nucleus> atoms = {};
  for (int i=0; i!=natom; i++) {
    const double Z = mol->atoms(i)->atom_charge();
    const double exp = mol->atoms(i)->atom_exponent();
    const int Zi = Z;
    const double Zd = Zi;
    assert ( Zd == Z );
    vector<double> coords = { 0.0, 0.0, 0.0 };
    for (int j=0; j!=3; j++) {
      coords[j] = mol->atoms(i)->position(j);
    }
    nucleus current (Zi, coords, exp);
    atoms.push_back(current);
  }

  // Iterate over all possible combinations of contracted basis functions and angular momentum for a given shell
  for (int n=0; n!=fnc1; n++) {
    for (int j=0; j!=ang1; j++) {
      for (int m=0; m!=fnc0; m++) {
        for (int i=0; i!=ang0; i++) {
          vector<int> indices = {i,j,m,n};
          vector<int> ordered_indices = {n,j,m,i};
          vector<int> ordered_indices2 = {j,i,n,m};
          complex<double> full_nai = 0.0;
          for (int k=0; k!=natom; k++) {
            //const int nbasis = nprimitive[0][m] + nprimitive[1][n];
            vector<double> positions_now = {};
            vector<int> angular_now = {};
            vector<double> exponents_now = {};
            vector<double> contractions_now = {};
            vector<int> nprimitive_now = {};
            vector<vector<complex<double>>> orbitals = {};
            vector<nucleus> nuclei = {atoms[k]};
            orbitals.resize(4);

            for (int q=0; q!=2; q++) {
              for (int x=0; x!=basis_size[q]; x++) {
                const int s = indices[q];
                const int t = indices[2+q];
                nprimitive_now.push_back(nprimitive[q][t]);
                for (int r=0; r!=nprimitive[q][t]; r++) {
                  for (int u=0; u!=3; u++) {
                    positions_now.push_back (positions[3*q+u]);
                    angular_now.push_back (cartesian_all[q][x][u]);
                  }
                }
                int position = 0;
                for (int v=0; v!=t; v++) position += nprimitive[q][v];
                for (int w=0; w!=nprimitive[q][t]; w++) {
                  exponents_now.push_back (exponents[q][position+w]);
                  contractions_now.push_back (contraction_coefficients[q][position+w]);
                }
                for (int y=0; y!=2; y++) {
                  if (y==q) orbitals[y].push_back(spherical_all[q][s][x]);
                  else orbitals[y].push_back(0.0);
                  orbitals[y+2].push_back(0.0);  // very crude fix.  orbitals[] should only contain two vectors for the NAI.
                }
              }
            }
            for (int q=0; q!=2; q++) {
              assert (orbitals[q].size()==nbasis_contracted);
              //cout << "q = " << q << endl;
              //cout << "orbitals[" << q << "].size() = " << orbitals[q].size() << endl;
              //cout << "nbasis_contracted = " << nbasis_contracted << endl;
              //cout << "Mol. Orbital " << q+1 << ":  ";
              //for (int z=0; z!=orbitals[q].size(); z++) {
              //  cout << orbitals[q][z] << "  ";
              //}
              //cout << endl;
            }

            // Compute the NAI for this particular term!
            pair<vector<atomic_orbital>,vector<molecular_orbital>> input = prepare_orbitals (nbasis_contracted, normalize_basis, scale_input, orthogonalize, field,
                             positions_now, angular_now, exponents_now, contractions_now, nprimitive_now, orbitals[0], orbitals[1], orbitals[2], orbitals[3]);
            complex<double> nai = compute_smallnai (input.first, input.second, field, nuclei, ia, ib);
            full_nai += nai;
          }

          pair<vector<int>,complex<double>> result (ordered_indices, full_nai);
          out.push_back(result);
        }
      }
    }
  }

  assert (total/natom == out.size());

  return out;
}


vector<pair<vector<int>,complex<double>>> get_comparison_smalloverlap (const array<shared_ptr<const bagel::Shell>,2>& basisinfo, const array<double,3> magnetic_field, const int ia, const int ib) {

  const bool normalize_basis = 0;
  const bool scale_input = 0;
  const bool orthogonalize = 0;

  const vector<double> field = { magnetic_field[0], magnetic_field[1], magnetic_field[2] };

  // Declare vectors to be used below
  vector<double> positions = {};
  vector<int> full_angular = {};
  vector<int> angular = {};
  vector<int> ncontracted = {};
  vector<bool> spherical = {};
  vector<vector<int>> nprimitive = {};
  vector<vector<double>> exponents = {};
  vector<vector<double>> contraction_coefficients = {};
  nprimitive.resize(4);
  exponents.resize(4);
  contraction_coefficients.resize(4);

  // Pull input data from the two bagel::Shells
  for (int i=0; i!=2; i++) {
    spherical.push_back (basisinfo[i]->spherical());

    full_angular.push_back (basisinfo[i]->angular_number());
    for (int k=0; k!=3; k++) {
      positions.push_back (basisinfo[i]->position(k));
    }
    const int nexponents = basisinfo[i]->num_primitive();
    int counter = 0;
    ncontracted.push_back (basisinfo[i]->num_contracted());
    for (int j=0; j!=nexponents; j++) {
      exponents[i].push_back (basisinfo[i]->exponents(j));
    }
    for (int j=0; j!=ncontracted[i]; j++) {
      const int start = basisinfo[i]->contraction_ranges(j).first;
      const int finish = basisinfo[i]->contraction_ranges(j).second;
      const int nfunctions = finish - start;
      nprimitive[i].push_back (nfunctions);
      counter = counter + nfunctions;
      for (int k=start; k!=finish; k++) {
        contraction_coefficients[i].push_back (basisinfo[i]->contractions(j)[k]);
      }
    }
    assert (counter == nexponents);
  }
  assert (spherical[0]==spherical[1]);

  vector<int> basis_size = {};
  vector<vector<vector<int>>> cartesian_all = {};
  vector<vector<vector<double>>> spherical_all = {};
  for (int i=0; i!=2; i++) {
    basis_size.push_back (((full_angular[i]+1)*(full_angular[i]+2))/2);
    cartesian_all.push_back (assign_angular (full_angular[i]));
    spherical_all.push_back (spherical_combinations(full_angular[i],spherical[i]));
  }

#if 0
  for (int i=0; i!=2; i++) {
    cout << "MO coefficients for center " << i+1 << ":" << endl;
    for (int j=0; j!=spherical_all[i].size(); j++) {
      cout << "  Orbital " << j+1 << ":  ";
      for (int k=0; k!=spherical_all[i][j].size(); k++) {
        cout << spherical_all[i][j][k] << "  ";
      }
      cout << endl;
    }
  }
#endif

  const int ang0 = spherical_all[0].size();
  const int ang1 = spherical_all[1].size();

  // Check that the right values have been assigned for the number of angular momentum possibilities
#if 1
  assert (spherical[0] || ang0 == basis_size[0]);
  assert (spherical[1] || ang1 == basis_size[1]);
  assert (!spherical[0] || ang0 == (2*full_angular[0] + 1) );
  assert (!spherical[1] || ang1 == (2*full_angular[1] + 1) );
#endif

  const int fnc0 = ncontracted[0];
  const int fnc1 = ncontracted[1];

  const int total = ang0*ang1*fnc0*fnc1;
  vector<pair<vector<int>,complex<double>>> out = {};

  const int nbasis_contracted = basis_size[0] + basis_size[1];

  // Iterate over all possible combinations of contracted basis functions and angular momentum for a given shell
  for (int n=0; n!=fnc1; n++) {
    for (int j=0; j!=ang1; j++) {
      for (int m=0; m!=fnc0; m++) {
        for (int i=0; i!=ang0; i++) {
          vector<int> indices = {i,j,m,n};
          vector<int> ordered_indices = {n,j,m,i};
          vector<int> ordered_indices2 = {j,i,n,m};
          //const int nbasis = nprimitive[0][m] + nprimitive[1][n];
          vector<double> positions_now = {};
          vector<int> angular_now = {};
          vector<double> exponents_now = {};
          vector<double> contractions_now = {};
          vector<int> nprimitive_now = {};
          vector<vector<complex<double>>> orbitals = {};
          orbitals.resize(4);

          for (int q=0; q!=2; q++) {
            for (int x=0; x!=basis_size[q]; x++) {
              const int s = indices[q];
              const int t = indices[2+q];
              nprimitive_now.push_back(nprimitive[q][t]);
              for (int r=0; r!=nprimitive[q][t]; r++) {
                for (int u=0; u!=3; u++) {
                  positions_now.push_back (positions[3*q+u]);
                  angular_now.push_back (cartesian_all[q][x][u]);
                }
              }
              int position = 0;
              for (int v=0; v!=t; v++) position += nprimitive[q][v];
              for (int w=0; w!=nprimitive[q][t]; w++) {
                exponents_now.push_back (exponents[q][position+w]);
                contractions_now.push_back (contraction_coefficients[q][position+w]);
              }
              for (int y=0; y!=2; y++) {
                if (y==q) orbitals[y].push_back(spherical_all[q][s][x]);
                else orbitals[y].push_back(0.0);
                orbitals[y+2].push_back(0.0);  // very crude fix.  orbitals[] should only contain two vectors for the NAI.
              }
            }
          }
          for (int q=0; q!=2; q++) {
            assert (orbitals[q].size()==nbasis_contracted);
#if 0
            cout << "q = " << q << endl;
            cout << "orbitals[" << q << "].size() = " << orbitals[q].size() << endl;
            cout << "nbasis_contracted = " << nbasis_contracted << endl;
            cout << "Mol. Orbital " << q+1 << ":  ";
            for (int z=0; z!=orbitals[q].size(); z++) {
              cout << orbitals[q][z] << "  ";
            }
            cout << endl;
#endif
          }

          // Compute the overlap for this particular term!
          pair<vector<atomic_orbital>,vector<molecular_orbital>> input = prepare_orbitals (nbasis_contracted, normalize_basis, scale_input, orthogonalize, field,
                           positions_now, angular_now, exponents_now, contractions_now, nprimitive_now, orbitals[0], orbitals[1], orbitals[2], orbitals[3]);
          complex<double> overlap = compute_smalloverlap (field, input.second[0], input.second[1], input.first, ia, ib);

          pair<vector<int>,complex<double>> result (ordered_indices, overlap);
          out.push_back(result);
        }
      }
    }
  }

  assert (total == out.size());

  return out;

}


vector<pair<vector<int>,complex<double>>> get_comparison_small_finiteNAI (const array<shared_ptr<const bagel::Shell>,2>& basisinfo, const std::shared_ptr<const bagel::Molecule> mol, const array<double,3> magnetic_field, const int ia, const int ib) {

  const bool normalize_basis = 0;
  const bool scale_input = 0;
  const bool orthogonalize = 0;

  const vector<double> field = { magnetic_field[0], magnetic_field[1], magnetic_field[2] };

  // Declare vectors to be used below
  vector<double> positions = {};
  vector<int> full_angular = {};
  vector<int> angular = {};
  vector<int> ncontracted = {};
  vector<bool> spherical = {};
  vector<vector<int>> nprimitive = {};
  vector<vector<double>> exponents = {};
  vector<vector<double>> contraction_coefficients = {};
  nprimitive.resize(4);
  exponents.resize(4);
  contraction_coefficients.resize(4);

  // Pull input data from the four bagel::Shells
  for (int i=0; i!=2; i++) {
    spherical.push_back (basisinfo[i]->spherical());

    full_angular.push_back (basisinfo[i]->angular_number());
    for (int k=0; k!=3; k++) {
      positions.push_back (basisinfo[i]->position(k));
    }
    const int nexponents = basisinfo[i]->num_primitive();
    int counter = 0;
    ncontracted.push_back (basisinfo[i]->num_contracted());
    for (int j=0; j!=nexponents; j++) {
      exponents[i].push_back (basisinfo[i]->exponents(j));
    }
    for (int j=0; j!=ncontracted[i]; j++) {
      const int start = basisinfo[i]->contraction_ranges(j).first;
      const int finish = basisinfo[i]->contraction_ranges(j).second;
      const int nfunctions = finish - start;
      nprimitive[i].push_back (nfunctions);
      counter = counter + nfunctions;
      for (int k=start; k!=finish; k++) {
        contraction_coefficients[i].push_back (basisinfo[i]->contractions(j)[k]);
      }
    }
    assert (counter == nexponents);
  }
  assert (spherical[0]==spherical[1]);

  vector<int> basis_size = {};
  vector<vector<vector<int>>> cartesian_all = {};
  vector<vector<vector<double>>> spherical_all = {};
  for (int i=0; i!=2; i++) {
    basis_size.push_back (((full_angular[i]+1)*(full_angular[i]+2))/2);
    cartesian_all.push_back (assign_angular (full_angular[i]));
    spherical_all.push_back (spherical_combinations(full_angular[i],spherical[i]));
  }

#if 0
  for (int i=0; i!=2; i++) {
    cout << "MO coefficients for center " << i+1 << ":" << endl;
    for (int j=0; j!=spherical_all[i].size(); j++) {
      for (int k=0; k!=spherical_all[i][j].size(); k++) {
        cout << spherical_all[i][j][k] << "  ";
        cout << endl;
      }
    }
  }
#endif

  const int ang0 = spherical_all[0].size();
  const int ang1 = spherical_all[1].size();

  // Check that the right values have been assigned for the number of angular momentum possibilities
#if 0
  assert (spherical[0] || ang0 == basis_size[0]);
  assert (spherical[1] || ang1 == basis_size[1]);
  assert (!spherical[0] || ang0 == (2*full_angular[0] + 1) );
  assert (!spherical[1] || ang1 == (2*full_angular[1] + 1) );
#endif

  const int fnc0 = ncontracted[0];
  const int fnc1 = ncontracted[1];

  const int natom = mol->natom();
  const int total = ang0*ang1*fnc0*fnc1*natom;
  vector<pair<vector<int>,complex<double>>> out = {};

  const int nbasis_contracted = basis_size[0] + basis_size[1];

  // Save the nucleus data in a vector
  vector<nucleus> atoms = {};
  for (int i=0; i!=natom; i++) {
    const double Z = mol->atoms(i)->atom_charge();
    const double exp = mol->atoms(i)->atom_exponent();
    const int Zi = Z;
    const double Zd = Zi;
    assert ( Zd == Z );
    vector<double> coords = { 0.0, 0.0, 0.0 };
    for (int j=0; j!=3; j++) {
      coords[j] = mol->atoms(i)->position(j);
    }
    nucleus current (Zi, coords, exp);
    atoms.push_back(current);
  }

  // Iterate over all possible combinations of contracted basis functions and angular momentum for a given shell
  for (int n=0; n!=fnc1; n++) {
    for (int j=0; j!=ang1; j++) {
      for (int m=0; m!=fnc0; m++) {
        for (int i=0; i!=ang0; i++) {
          vector<int> indices = {i,j,m,n};
          vector<int> ordered_indices = {n,j,m,i};
          vector<int> ordered_indices2 = {j,i,n,m};
          complex<double> full_nai = 0.0;
          for (int k=0; k!=natom; k++) {
            //const int nbasis = nprimitive[0][m] + nprimitive[1][n];
            vector<double> positions_now = {};
            vector<int> angular_now = {};
            vector<double> exponents_now = {};
            vector<double> contractions_now = {};
            vector<int> nprimitive_now = {};
            vector<vector<complex<double>>> orbitals = {};
            vector<nucleus> nuclei = {atoms[k]};
            orbitals.resize(4);

            for (int q=0; q!=2; q++) {
              for (int x=0; x!=basis_size[q]; x++) {
                const int s = indices[q];
                const int t = indices[2+q];
                nprimitive_now.push_back(nprimitive[q][t]);
                for (int r=0; r!=nprimitive[q][t]; r++) {
                  for (int u=0; u!=3; u++) {
                    positions_now.push_back (positions[3*q+u]);
                    angular_now.push_back (cartesian_all[q][x][u]);
                  }
                }
                int position = 0;
                for (int v=0; v!=t; v++) position += nprimitive[q][v];
                for (int w=0; w!=nprimitive[q][t]; w++) {
                  exponents_now.push_back (exponents[q][position+w]);
                  contractions_now.push_back (contraction_coefficients[q][position+w]);
                }
                for (int y=0; y!=2; y++) {
                  if (y==q) orbitals[y].push_back(spherical_all[q][s][x]);
                  else orbitals[y].push_back(0.0);
                  orbitals[y+2].push_back(0.0);  // very crude fix.  orbitals[] should only contain two vectors for the NAI.
                }
              }
            }
            for (int q=0; q!=2; q++) {
              assert (orbitals[q].size()==nbasis_contracted);
              //cout << "q = " << q << endl;
              //cout << "orbitals[" << q << "].size() = " << orbitals[q].size() << endl;
              //cout << "nbasis_contracted = " << nbasis_contracted << endl;
              //cout << "Mol. Orbital " << q+1 << ":  ";
              //for (int z=0; z!=orbitals[q].size(); z++) {
              //  cout << orbitals[q][z] << "  ";
              //}
              //cout << endl;
            }

            // Compute the NAI for this particular term!
            pair<vector<atomic_orbital>,vector<molecular_orbital>> input = prepare_orbitals (nbasis_contracted, normalize_basis, scale_input, orthogonalize, field,
                             positions_now, angular_now, exponents_now, contractions_now, nprimitive_now, orbitals[0], orbitals[1], orbitals[2], orbitals[3]);
            complex<double> nai = compute_small_finitenai (input.first, input.second, field, nuclei, ia, ib);
            full_nai += nai;
          }

          pair<vector<int>,complex<double>> result (ordered_indices, full_nai);
          out.push_back(result);
        }
      }
    }
  }

  assert (total/natom == out.size());

  return out;
}


vector<pair<vector<int>,complex<double>>> get_comparison_finiteNAI (const array<shared_ptr<const bagel::Shell>,2>& basisinfo, const std::shared_ptr<const bagel::Atom> atom, const array<double,3> magnetic_field) {

  const bool normalize_basis = 0;
  const bool scale_input = 0;
  const bool orthogonalize = 0;

  const vector<double> field = { magnetic_field[0], magnetic_field[1], magnetic_field[2] };

  // Declare vectors to be used below
  vector<double> positions = {};
  vector<int> full_angular = {};
  vector<int> angular = {};
  vector<int> ncontracted = {};
  vector<bool> spherical = {};
  vector<vector<int>> nprimitive = {};
  vector<vector<double>> exponents = {};
  vector<vector<double>> contraction_coefficients = {};
  nprimitive.resize(4);
  exponents.resize(4);
  contraction_coefficients.resize(4);

  // Pull input data from the four bagel::Shells
  for (int i=0; i!=2; i++) {
    spherical.push_back (basisinfo[i]->spherical());

    full_angular.push_back (basisinfo[i]->angular_number());
    for (int k=0; k!=3; k++) {
      positions.push_back (basisinfo[i]->position(k));
    }
    const int nexponents = basisinfo[i]->num_primitive();
    int counter = 0;
    ncontracted.push_back (basisinfo[i]->num_contracted());
    for (int j=0; j!=nexponents; j++) {
      exponents[i].push_back (basisinfo[i]->exponents(j));
    }
    for (int j=0; j!=ncontracted[i]; j++) {
      const int start = basisinfo[i]->contraction_ranges(j).first;
      const int finish = basisinfo[i]->contraction_ranges(j).second;
      const int nfunctions = finish - start;
      nprimitive[i].push_back (nfunctions);
      counter = counter + nfunctions;
      for (int k=start; k!=finish; k++) {
        contraction_coefficients[i].push_back (basisinfo[i]->contractions(j)[k]);
      }
    }
    assert (counter == nexponents);
  }
  assert (spherical[0]==spherical[1]);

  vector<int> basis_size = {};
  vector<vector<vector<int>>> cartesian_all = {};
  vector<vector<vector<double>>> spherical_all = {};
  for (int i=0; i!=2; i++) {
    basis_size.push_back (((full_angular[i]+1)*(full_angular[i]+2))/2);
    cartesian_all.push_back (assign_angular (full_angular[i]));
    spherical_all.push_back (spherical_combinations(full_angular[i],spherical[i]));
  }

#if 0
  for (int i=0; i!=2; i++) {
    cout << "MO coefficients for center " << i+1 << ":" << endl;
    for (int j=0; j!=spherical_all[i].size(); j++) {
      for (int k=0; k!=spherical_all[i][j].size(); k++) {
        cout << spherical_all[i][j][k] << "  ";
        cout << endl;
      }
    }
  }
#endif

  const int ang0 = spherical_all[0].size();
  const int ang1 = spherical_all[1].size();

  // Check that the right values have been assigned for the number of angular momentum possibilities
#if 0
  assert (spherical[0] || ang0 == basis_size[0]);
  assert (spherical[1] || ang1 == basis_size[1]);
  assert (!spherical[0] || ang0 == (2*full_angular[0] + 1) );
  assert (!spherical[1] || ang1 == (2*full_angular[1] + 1) );
#endif

  const int fnc0 = ncontracted[0];
  const int fnc1 = ncontracted[1];

//  const int natom = mol->natom();
  const int natom = 1;
  const int total = ang0*ang1*fnc0*fnc1*natom;
  vector<pair<vector<int>,complex<double>>> out = {};

  const int nbasis_contracted = basis_size[0] + basis_size[1];

  // Save the nucleus data in a vector
  vector<nucleus> atoms = {};
  for (int i=0; i!=natom; i++) {
    const double Z = atom->atom_charge();
    const double exp = atom->atom_exponent();
    //const double Z = mol->atoms(i)->atom_charge();
    //const double exp = mol->atoms(i)->atom_exponent();
    const int Zi = Z;
    const double Zd = Zi;
    assert ( Zd == Z );
    vector<double> coords = { 0.0, 0.0, 0.0 };
    for (int j=0; j!=3; j++) {
      coords[j] = atom->position(j);
    }
    nucleus current (Zi, coords, exp);
    atoms.push_back(current);
  }

  // Iterate over all possible combinations of contracted basis functions and angular momentum for a given shell
  for (int n=0; n!=fnc1; n++) {
    for (int j=0; j!=ang1; j++) {
      for (int m=0; m!=fnc0; m++) {
        for (int i=0; i!=ang0; i++) {
          vector<int> indices = {i,j,m,n};
          vector<int> ordered_indices = {n,j,m,i};
          vector<int> ordered_indices2 = {j,i,n,m};
          complex<double> full_nai = 0.0;
          for (int k=0; k!=natom; k++) {
            //const int nbasis = nprimitive[0][m] + nprimitive[1][n];
            vector<double> positions_now = {};
            vector<int> angular_now = {};
            vector<double> exponents_now = {};
            vector<double> contractions_now = {};
            vector<int> nprimitive_now = {};
            vector<vector<complex<double>>> orbitals = {};
            vector<nucleus> nuclei = {atoms[k]};
            orbitals.resize(4);

            for (int q=0; q!=2; q++) {
              for (int x=0; x!=basis_size[q]; x++) {
                const int s = indices[q];
                const int t = indices[2+q];
                nprimitive_now.push_back(nprimitive[q][t]);
                for (int r=0; r!=nprimitive[q][t]; r++) {
                  for (int u=0; u!=3; u++) {
                    positions_now.push_back (positions[3*q+u]);
                    angular_now.push_back (cartesian_all[q][x][u]);
                  }
                }
                int position = 0;
                for (int v=0; v!=t; v++) position += nprimitive[q][v];
                for (int w=0; w!=nprimitive[q][t]; w++) {
                  exponents_now.push_back (exponents[q][position+w]);
                  contractions_now.push_back (contraction_coefficients[q][position+w]);
                }
                for (int y=0; y!=2; y++) {
                  if (y==q) orbitals[y].push_back(spherical_all[q][s][x]);
                  else orbitals[y].push_back(0.0);
                  orbitals[y+2].push_back(0.0);  // very crude fix.  orbitals[] should only contain two vectors for the NAI.
                }
              }
            }
            for (int q=0; q!=2; q++) {
              assert (orbitals[q].size()==nbasis_contracted);
              //cout << "q = " << q << endl;
              //cout << "orbitals[" << q << "].size() = " << orbitals[q].size() << endl;
              //cout << "nbasis_contracted = " << nbasis_contracted << endl;
              //cout << "Mol. Orbital " << q+1 << ":  ";
              //for (int z=0; z!=orbitals[q].size(); z++) {
              //  cout << orbitals[q][z] << "  ";
              //}
              //cout << endl;
            }

            // Compute the NAI for this particular term!
            pair<vector<atomic_orbital>,vector<molecular_orbital>> input = prepare_orbitals (nbasis_contracted, normalize_basis, scale_input, orthogonalize, field,
                             positions_now, angular_now, exponents_now, contractions_now, nprimitive_now, orbitals[0], orbitals[1], orbitals[2], orbitals[3]);
            complex<double> nai = compute_finite_nai (input.first, input.second, field, nuclei);
            full_nai += nai;
          }

          pair<vector<int>,complex<double>> result (ordered_indices, full_nai);
          out.push_back(result);
        }
      }
    }
  }

  assert (total/natom == out.size());

  return out;
}


vector<pair<vector<int>,complex<double>>> get_comparison_smallERI (const array<shared_ptr<const bagel::Shell>,4>& basisinfo, const array<double,3> magnetic_field, const int ia, const int ib) {

  const bool normalize_basis = 0;
  const bool scale_input = 0;
  const bool orthogonalize = 0;

  const vector<double> field = { magnetic_field[0], magnetic_field[1], magnetic_field[2] };
#if 0
  cout << "Magnetic field:  { ";
  for (int i=0; i!=3; i++) {
    cout << field[i];
    if (i!=2) cout << ", ";
    else cout << " }" << endl;
  }
#endif

  // Declare vectors to be used below
  vector<double> positions = {};
  vector<int> full_angular = {};
  vector<int> angular = {};
  vector<int> ncontracted = {};
  vector<bool> spherical = {};
  vector<vector<int>> nprimitive = {};
  vector<vector<double>> exponents = {};
  vector<vector<double>> contraction_coefficients = {};
  nprimitive.resize(4);
  exponents.resize(4);
  contraction_coefficients.resize(4);

#if 1 // Check to ensure that A_ = dummy orbital, B_ = auxiliary function
  for (int i=0; i!=3; i++) assert(basisinfo[0]->vector_potential(i) == 0.0);
  for (int i=0; i!=3; i++) assert(basisinfo[1]->vector_potential(i) == 0.0);
  for (int i=0; i!=3; i++) assert(basisinfo[0]->position(i) == 0.0);
  assert(basisinfo[0]->angular_number() == 0);
#endif

  // Pull input data from the four bagel::Shells
  for (int i=0; i!=4; i++) {
    spherical.push_back (basisinfo[i]->spherical());

    full_angular.push_back (basisinfo[i]->angular_number());
    for (int k=0; k!=3; k++) {
      positions.push_back (basisinfo[i]->position(k));
    }
    const int nexponents = basisinfo[i]->num_primitive();
    int counter = 0;
    ncontracted.push_back (basisinfo[i]->num_contracted());
    for (int j=0; j!=nexponents; j++) {
      exponents[i].push_back (basisinfo[i]->exponents(j));
    }
    for (int j=0; j!=ncontracted[i]; j++) {
      const int start = basisinfo[i]->contraction_ranges(j).first;
      const int finish = basisinfo[i]->contraction_ranges(j).second;
      const int nfunctions = finish - start;
      nprimitive[i].push_back (nfunctions);
      counter = counter + nfunctions;
      for (int k=start; k!=finish; k++) {
        contraction_coefficients[i].push_back (basisinfo[i]->contractions(j)[k]);
      }
    }
    assert (counter == nexponents);
  }
  assert (spherical[0]==spherical[1]);
  assert (spherical[2]==spherical[3]);

  vector<int> basis_size = {};
  vector<vector<vector<int>>> cartesian_all = {};
  vector<vector<vector<double>>> spherical_all = {};
  for (int i=0; i!=4; i++) {
    basis_size.push_back (((full_angular[i]+1)*(full_angular[i]+2))/2);
    cartesian_all.push_back (assign_angular (full_angular[i]));
    spherical_all.push_back (spherical_combinations(full_angular[i],spherical[i]));
  }

#if 0
  for (int i=0; i!=4; i++) {
    cout << "MO coefficients for center " << i+1 << ":" << endl;
    for (int j=0; j!=spherical_all[i].size(); j++) {
      for (int k=0; k!=spherical_all[i][j].size(); k++) {
        cout << spherical_all[i][j][k] << "  ";
        cout << endl;
      }
    }
    cout << "Positions for center " << i+1 << ":" << endl;
    for (int j=0; j!=3; j++) {
      cout << "    " << positions[3*i+j] << endl;
    }
  }
#endif

  const int ang0 = spherical_all[0].size();
  const int ang1 = spherical_all[1].size();
  const int ang2 = spherical_all[2].size();
  const int ang3 = spherical_all[3].size();

  // Check that the right values have been assigned for the number of angular momentum possibilities
#if 1
  assert (spherical[0] || ang0 == basis_size[0]);
  assert (spherical[1] || ang1 == basis_size[1]);
  assert (spherical[2] || ang2 == basis_size[2]);
  assert (spherical[3] || ang3 == basis_size[3]);
  assert (!spherical[0] || ang0 == (2*full_angular[0] + 1) );
  assert (!spherical[1] || ang1 == (2*full_angular[1] + 1) );
  assert (!spherical[2] || ang2 == (2*full_angular[2] + 1) );
  assert (!spherical[3] || ang3 == (2*full_angular[3] + 1) );
#endif

  const int fnc0 = ncontracted[0];
  const int fnc1 = ncontracted[1];
  const int fnc2 = ncontracted[2];
  const int fnc3 = ncontracted[3];

  const int total = ang0*ang1*ang2*ang3*fnc0*fnc1*fnc2*fnc3;
  vector<pair<vector<int>,complex<double>>> out = {};

  const int nbasis_contracted = basis_size[0] + basis_size[1] + basis_size[2] + basis_size[3];

  // Iterate over all possible combinations of contracted basis functions and angular momentum for a given shell
  for (int p=0; p!=fnc3; p++) {
    for (int l=0; l!=ang3; l++) {
      for (int o=0; o!=fnc2; o++) {
        for (int k=0; k!=ang2; k++) {
          for (int n=0; n!=fnc1; n++) {
            for (int j=0; j!=ang1; j++) {
              for (int m=0; m!=fnc0; m++) {
                for (int i=0; i!=ang0; i++) {

                  vector<int> indices = {i,j,k,l,m,n,o,p};
                  vector<int> ordered_indices = {p,l,o,k,n,j,m,i};
                  vector<int> ordered_indices2 = {l,k,j,i,p,o,n,m};
                  //const int nbasis = nprimitive[0][m] + nprimitive[1][n] + nprimitive[2][o] + nprimitive[3][p];
                  vector<double> positions_now = {};
                  vector<int> angular_now = {};
                  vector<double> exponents_now = {};
                  vector<double> contractions_now = {};
                  vector<int> nprimitive_now = {};
                  vector<vector<complex<double>>> orbitals = {};
                  orbitals.resize(4);

                  for (int q=0; q!=4; q++) {
                    for (int x=0; x!=basis_size[q]; x++) {
                      const int s = indices[q];
                      const int t = indices[4+q];
                      nprimitive_now.push_back(nprimitive[q][t]);
                      for (int r=0; r!=nprimitive[q][t]; r++) {
                        for (int u=0; u!=3; u++) {
                          positions_now.push_back (positions[3*q+u]);
                          angular_now.push_back (cartesian_all[q][x][u]);
                        }
                      }
                      int position = 0;
                      for (int v=0; v!=t; v++) position += nprimitive[q][v];
                      for (int w=0; w!=nprimitive[q][t]; w++) {
                        exponents_now.push_back (exponents[q][position+w]);
                        contractions_now.push_back (contraction_coefficients[q][position+w]);
                      }
                      for (int y=0; y!=4; y++) {
                        if (y==q) orbitals[y].push_back(spherical_all[q][s][x]);
                        else orbitals[y].push_back(0.0);
                      }
                    }
                  }
                  for (int q=0; q!=4; q++) {
                    assert (orbitals[q].size()==nbasis_contracted);
                    //cout << "Mol. Orbital " << q+1 << ":  ";
                    //for (int z=0; z!=orbitals[q].size(); z++) {
                    //  cout << orbitals[q][z] << "  ";
                    //}
                    //cout << endl;
                  }

                  // Compute the ERI for this particular term!
                  pair<vector<atomic_orbital>,vector<molecular_orbital>> input = prepare_orbitals (nbasis_contracted, normalize_basis, scale_input, orthogonalize, field,
                             positions_now, angular_now, exponents_now, contractions_now, nprimitive_now, orbitals[0], orbitals[1], orbitals[2], orbitals[3]);
                  complex<double> eri = compute_smalleri (input.first, input.second, field, ia, ib);
                  pair<vector<int>,complex<double>> result (ordered_indices, eri);
                  out.push_back(result);
                }
              }
            }
          }
        }
      }
    }
  }

  assert (total == out.size());

  return out;

}


vector<pair<vector<int>,complex<double>>> get_comparison_mixedERI (const array<shared_ptr<const bagel::Shell>,4>& basisinfo, const array<double,3> magnetic_field, const int ia, const int ib) {

  const bool normalize_basis = 0;
  const bool scale_input = 0;
  const bool orthogonalize = 0;

  const vector<double> field = { magnetic_field[0], magnetic_field[1], magnetic_field[2] };
#if 0
  cout << "Magnetic field:  { ";
  for (int i=0; i!=3; i++) {
    cout << field[i];
    if (i!=2) cout << ", ";
    else cout << " }" << endl;
  }
#endif

  // Declare vectors to be used below
  vector<double> positions = {};
  vector<int> full_angular = {};
  vector<int> angular = {};
  vector<int> ncontracted = {};
  vector<bool> spherical = {};
  vector<vector<int>> nprimitive = {};
  vector<vector<double>> exponents = {};
  vector<vector<double>> contraction_coefficients = {};
  nprimitive.resize(4);
  exponents.resize(4);
  contraction_coefficients.resize(4);

#if 1 // Check to ensure that A_ = dummy orbital, B_ = auxiliary function
  for (int i=0; i!=3; i++) assert(basisinfo[0]->vector_potential(i) == 0.0);
  for (int i=0; i!=3; i++) assert(basisinfo[1]->vector_potential(i) == 0.0);
  for (int i=0; i!=3; i++) assert(basisinfo[0]->position(i) == 0.0);
  assert(basisinfo[0]->angular_number() == 0);
#endif

  // Pull input data from the four bagel::Shells
  for (int i=0; i!=4; i++) {
    spherical.push_back (basisinfo[i]->spherical());

    full_angular.push_back (basisinfo[i]->angular_number());
    for (int k=0; k!=3; k++) {
      positions.push_back (basisinfo[i]->position(k));
    }
    const int nexponents = basisinfo[i]->num_primitive();
    int counter = 0;
    ncontracted.push_back (basisinfo[i]->num_contracted());
    for (int j=0; j!=nexponents; j++) {
      exponents[i].push_back (basisinfo[i]->exponents(j));
    }
    for (int j=0; j!=ncontracted[i]; j++) {
      const int start = basisinfo[i]->contraction_ranges(j).first;
      const int finish = basisinfo[i]->contraction_ranges(j).second;
      const int nfunctions = finish - start;
      nprimitive[i].push_back (nfunctions);
      counter = counter + nfunctions;
      for (int k=start; k!=finish; k++) {
        contraction_coefficients[i].push_back (basisinfo[i]->contractions(j)[k]);
      }
    }
    assert (counter == nexponents);
  }
  assert (spherical[0]==spherical[1]);
  assert (spherical[2]==spherical[3]);

  vector<int> basis_size = {};
  vector<vector<vector<int>>> cartesian_all = {};
  vector<vector<vector<double>>> spherical_all = {};
  for (int i=0; i!=4; i++) {
    basis_size.push_back (((full_angular[i]+1)*(full_angular[i]+2))/2);
    cartesian_all.push_back (assign_angular (full_angular[i]));
    spherical_all.push_back (spherical_combinations(full_angular[i],spherical[i]));
  }

#if 0
  for (int i=0; i!=4; i++) {
    cout << "MO coefficients for center " << i+1 << ":" << endl;
    for (int j=0; j!=spherical_all[i].size(); j++) {
      for (int k=0; k!=spherical_all[i][j].size(); k++) {
        cout << spherical_all[i][j][k] << "  ";
        cout << endl;
      }
    }
    cout << "Positions for center " << i+1 << ":" << endl;
    for (int j=0; j!=3; j++) {
      cout << "    " << positions[3*i+j] << endl;
    }
  }
#endif

  const int ang0 = spherical_all[0].size();
  const int ang1 = spherical_all[1].size();
  const int ang2 = spherical_all[2].size();
  const int ang3 = spherical_all[3].size();

  // Check that the right values have been assigned for the number of angular momentum possibilities
#if 1
  assert (spherical[0] || ang0 == basis_size[0]);
  assert (spherical[1] || ang1 == basis_size[1]);
  assert (spherical[2] || ang2 == basis_size[2]);
  assert (spherical[3] || ang3 == basis_size[3]);
  assert (!spherical[0] || ang0 == (2*full_angular[0] + 1) );
  assert (!spherical[1] || ang1 == (2*full_angular[1] + 1) );
  assert (!spherical[2] || ang2 == (2*full_angular[2] + 1) );
  assert (!spherical[3] || ang3 == (2*full_angular[3] + 1) );
#endif

  const int fnc0 = ncontracted[0];
  const int fnc1 = ncontracted[1];
  const int fnc2 = ncontracted[2];
  const int fnc3 = ncontracted[3];

  const int total = ang0*ang1*ang2*ang3*fnc0*fnc1*fnc2*fnc3;
  vector<pair<vector<int>,complex<double>>> out = {};

  const int nbasis_contracted = basis_size[0] + basis_size[1] + basis_size[2] + basis_size[3];

  // Iterate over all possible combinations of contracted basis functions and angular momentum for a given shell
  for (int p=0; p!=fnc3; p++) {
    for (int l=0; l!=ang3; l++) {
      for (int o=0; o!=fnc2; o++) {
        for (int k=0; k!=ang2; k++) {
          for (int n=0; n!=fnc1; n++) {
            for (int j=0; j!=ang1; j++) {
              for (int m=0; m!=fnc0; m++) {
                for (int i=0; i!=ang0; i++) {

                  vector<int> indices = {i,j,k,l,m,n,o,p};
                  vector<int> ordered_indices = {p,l,o,k,n,j,m,i};
                  vector<int> ordered_indices2 = {l,k,j,i,p,o,n,m};
                  //const int nbasis = nprimitive[0][m] + nprimitive[1][n] + nprimitive[2][o] + nprimitive[3][p];
                  vector<double> positions_now = {};
                  vector<int> angular_now = {};
                  vector<double> exponents_now = {};
                  vector<double> contractions_now = {};
                  vector<int> nprimitive_now = {};
                  vector<vector<complex<double>>> orbitals = {};
                  orbitals.resize(4);

                  for (int q=0; q!=4; q++) {
                    for (int x=0; x!=basis_size[q]; x++) {
                      const int s = indices[q];
                      const int t = indices[4+q];
                      nprimitive_now.push_back(nprimitive[q][t]);
                      for (int r=0; r!=nprimitive[q][t]; r++) {
                        for (int u=0; u!=3; u++) {
                          positions_now.push_back (positions[3*q+u]);
                          angular_now.push_back (cartesian_all[q][x][u]);
                        }
                      }
                      int position = 0;
                      for (int v=0; v!=t; v++) position += nprimitive[q][v];
                      for (int w=0; w!=nprimitive[q][t]; w++) {
                        exponents_now.push_back (exponents[q][position+w]);
                        contractions_now.push_back (contraction_coefficients[q][position+w]);
                      }
                      for (int y=0; y!=4; y++) {
                        if (y==q) orbitals[y].push_back(spherical_all[q][s][x]);
                        else orbitals[y].push_back(0.0);
                      }
                    }
                  }
                  for (int q=0; q!=4; q++) {
                    assert (orbitals[q].size()==nbasis_contracted);
                    //cout << "Mol. Orbital " << q+1 << ":  ";
                    //for (int z=0; z!=orbitals[q].size(); z++) {
                    //  cout << orbitals[q][z] << "  ";
                    //}
                    //cout << endl;
                  }

                  // Compute the ERI for this particular term!
                  pair<vector<atomic_orbital>,vector<molecular_orbital>> input = prepare_orbitals (nbasis_contracted, normalize_basis, scale_input, orthogonalize, field,
                             positions_now, angular_now, exponents_now, contractions_now, nprimitive_now, orbitals[0], orbitals[1], orbitals[2], orbitals[3]);
                  complex<double> eri = compute_mixederi (input.first, input.second, field, ia, ib);
                  pair<vector<int>,complex<double>> result (ordered_indices, eri);
                  out.push_back(result);
                }
              }
            }
          }
        }
      }
    }
  }

  assert (total == out.size());

  return out;

}


}
