/*
 *@BEGIN LICENSE
 *
 * PSI4: an ab initio quantum chemistry software package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *@END LICENSE
 */

/*! \file
    \ingroup CCSORT
    \brief Enter brief description of file here 
*/
#include <cstdio>
#include <cstdlib>
#include <libpsio/psio.h>
#include <libciomr/libciomr.h>
#include <libdpd/dpd.h>
#include "Params.h"
#include "MOInfo.h"
#define EXTERN
#include "globals.h"

#include <libmints/wavefunction.h>
#include <libtrans/mospace.h>
#include <libmints/matrix.h>

#define DEBUG 1

namespace psi { namespace ccsort {

/* 
** fock(): Build the alpha and beta Fock matrices from the
** one-electron integrals/frozen-core operator and active two-electron
** integrals on disk.
**
** TDC, 1996
** Modified to include UHF references, TDC, June 2001.
**
** Notes:
**
** (1) This routine isn't absolutely necessary for RHF and UHF
** references, as one could simply read the Fock eigenvalues from
** PSIF_CHKPT and be happy with that. However, this code is useful as a
** partial check of the integral transformation and sorting routines.
**
** (2) An alternative but currently unused algorithm may be found in 
** fock_build.c.
*/

void fock_rhf(void);
void fock_uhf(void);

void fock(void)
{
  if(params.ref == 2) fock_uhf();
  else fock_rhf();
}

void fock_uhf(void)
{
  int h, nirreps;
  int i, j;
  int a, b;
  int *aoccpi, *boccpi;
  int *avirtpi, *bvirtpi;
  int *frdocc;
  dpdfile2 fIJ, fij, fAB, fab, fIA, fia;

  nirreps = moinfo.nirreps;
  aoccpi = moinfo.aoccpi;
  boccpi = moinfo.boccpi;
  avirtpi = moinfo.avirtpi;
  bvirtpi = moinfo.bvirtpi;
  frdocc = moinfo.frdocc;

  SharedMatrix Fa = SharedMatrix(Process::environment.wavefunction()->Fa());
  SharedMatrix Fb = SharedMatrix(Process::environment.wavefunction()->Fb());
  SharedMatrix Ca = SharedMatrix(Process::environment.wavefunction()->Ca());
  SharedMatrix Cb = SharedMatrix(Process::environment.wavefunction()->Cb());
  Fa->transform(Ca);
  Fb->transform(Cb);

#if DEBUG
  Fa->print();
  Fb->print();
#endif

  dpd_file2_init(&fIJ, PSIF_CC_OEI, 0, 0, 0, "fIJ");
  dpd_file2_init(&fij, PSIF_CC_OEI, 0, 2, 2, "fij");
  dpd_file2_mat_init(&fIJ);
  dpd_file2_mat_init(&fij);

  for(h=0; h < nirreps; h++) {

    for(i=0; i < aoccpi[h]; i++)
      for(j=0; j < aoccpi[h]; j++)
          fIJ.matrix[h][i][j] = Fa->get(h,i + frdocc[h],j + frdocc[h]);

    for(i=0; i < boccpi[h]; i++)
      for(j=0; j < boccpi[h]; j++)
        fij.matrix[h][i][j] = Fb->get(h,i + frdocc[h],j + frdocc[h]);
  }

  dpd_file2_mat_wrt(&fIJ);
  dpd_file2_mat_wrt(&fij);
#if DEBUG
  dpd_file2_print(&fIJ, outfile);
  dpd_file2_print(&fij, outfile);
#endif
  dpd_file2_close(&fIJ);
  dpd_file2_close(&fij);

  dpd_file2_init(&fAB, PSIF_CC_OEI, 0, 1, 1, "fAB");
  dpd_file2_init(&fab, PSIF_CC_OEI, 0, 3, 3, "fab");

  dpd_file2_mat_init(&fAB);
  dpd_file2_mat_init(&fab);

  for(h=0; h < nirreps; h++) {

    for(a=0; a < avirtpi[h]; a++)
      for(b=0; b < avirtpi[h]; b++)
        fAB.matrix[h][a][b] = Fa->get(h, a + frdocc[h] + aoccpi[h], b + frdocc[h] + aoccpi[h]);

    for(a=0; a < bvirtpi[h]; a++)
      for(b=0; b < bvirtpi[h]; b++)
        fab.matrix[h][a][b] = Fb->get(h, a + frdocc[h] + boccpi[h], b + frdocc[h] + boccpi[h]);
  }

  dpd_file2_mat_wrt(&fAB);
  dpd_file2_mat_wrt(&fab);
#if DEBUG
  dpd_file2_print(&fAB, outfile);
  dpd_file2_print(&fab, outfile);
#endif
  dpd_file2_close(&fAB);
  dpd_file2_close(&fab);

  /* Prepare the alpha and beta occ-vir Fock matrix files */
  dpd_file2_init(&fIA, PSIF_CC_OEI, 0, 0, 1, "fIA");
  dpd_file2_init(&fia, PSIF_CC_OEI, 0, 2, 3, "fia");
  dpd_file2_mat_init(&fIA);
  dpd_file2_mat_init(&fia);

  /* One-electron (frozen-core) contributions */
  for(h=0; h < nirreps; h++) {

    for(i=0; i < aoccpi[h]; i++)
      for(a=0; a < avirtpi[h]; a++)
        fIA.matrix[h][i][a] = Fa->get(h, i + frdocc[h], a + frdocc[h] + aoccpi[h]);

    for(i=0; i < boccpi[h]; i++)
      for(a=0; a < bvirtpi[h]; a++)
        fia.matrix[h][i][a] = Fb->get(h, i + frdocc[h], a + frdocc[h] + boccpi[h]);
  }

  /* Close the alpha and beta occ-vir Fock matrix files */
  dpd_file2_mat_wrt(&fIA);
  dpd_file2_mat_wrt(&fia);
  dpd_file2_mat_close(&fIA);
  dpd_file2_mat_close(&fia);
#if DEBUG
  dpd_file2_print(&fIA, outfile);
  dpd_file2_print(&fia, outfile);
#endif
  dpd_file2_close(&fIA);
  dpd_file2_close(&fia);

}

void fock_rhf(void)
{
  int h;
  int i,j,a,b;
  int nirreps;
  int *occpi, *virtpi;
  int *openpi;
  int *frdocc;
  int *fruocc;
  dpdfile2 fIJ, fij, fAB, fab, fIA, fia;

  // Take Fock matrix from the wavefunction and transform it to the MO basis

  nirreps = moinfo.nirreps;
  occpi = moinfo.occpi; virtpi = moinfo.virtpi;
  openpi = moinfo.openpi;
  frdocc = moinfo.frdocc;
  fruocc = moinfo.fruocc;

  SharedMatrix Fa = SharedMatrix(Process::environment.wavefunction()->Fa());
  SharedMatrix Fb = SharedMatrix(Process::environment.wavefunction()->Fb());
  SharedMatrix Ca = SharedMatrix(Process::environment.wavefunction()->Ca());
  Fa->transform(Ca);
  Fb->transform(Ca);
#if DEBUG
  Fa->print();
  Fb->print();
#endif

  /* Prepare the alpha and beta occ-occ Fock matrix files */
  dpd_file2_init(&fIJ, PSIF_CC_OEI, 0, 0, 0, "fIJ");
  dpd_file2_init(&fij, PSIF_CC_OEI, 0, 0, 0, "fij");
  dpd_file2_mat_init(&fIJ);
  dpd_file2_mat_init(&fij);

  /* One-electron (frozen-core) contributions */
  for(h=0; h < nirreps; h++) {

      for(i=0; i < occpi[h]; i++)
          for(j=0; j < occpi[h]; j++)
              fIJ.matrix[h][i][j] = Fa->get(h,i + frdocc[h],j + frdocc[h]);

      for(i=0; i < (occpi[h]-openpi[h]); i++)
          for(j=0; j < (occpi[h]-openpi[h]); j++)
              fij.matrix[h][i][j] = Fb->get(h,i + frdocc[h],j + frdocc[h]);
  }

  /* Close the alpha and beta occ-occ Fock matrix files */
  dpd_file2_mat_wrt(&fIJ);
  dpd_file2_mat_wrt(&fij);
  dpd_file2_mat_close(&fIJ);
  dpd_file2_mat_close(&fij);
#if DEBUG
  dpd_file2_print(&fIJ, outfile);
  dpd_file2_print(&fij, outfile);
#endif
  dpd_file2_close(&fIJ);
  dpd_file2_close(&fij);

  /* Prepare the alpha and beta vir-vir Fock matrix files */
  dpd_file2_init(&fAB, PSIF_CC_OEI, 0, 1, 1, "fAB");
  dpd_file2_init(&fab, PSIF_CC_OEI, 0, 1, 1, "fab");
  dpd_file2_mat_init(&fAB);
  dpd_file2_mat_init(&fab);

  /* One-electron (frozen-core) contributions */
  for(h=0; h < nirreps; h++) {

      for(a=0; a < (virtpi[h] - openpi[h]); a++)
          for(b=0; b < (virtpi[h] - openpi[h]); b++)
              fAB.matrix[h][a][b] = Fa->get(h, a + frdocc[h] + occpi[h], b + frdocc[h] + occpi[h]);

      for(a=0; a < (virtpi[h] - openpi[h]); a++)
          for(b=0; b <(virtpi[h] - openpi[h]); b++)
              fab.matrix[h][a][b] = Fb->get(h, a + frdocc[h] + occpi[h], b + frdocc[h] + occpi[h]);

      for(a=0; a < openpi[h]; a++)
          for(b=0; b < openpi[h]; b++)
              fab.matrix[h][virtpi[h] - openpi[h] + a][virtpi[h] - openpi[h] + b] = Fb->get(h, a + frdocc[h] + occpi[h] - openpi[h], b + frdocc[h] + occpi[h] - openpi[h]);

      for(a=0; a < (virtpi[h] - openpi[h]); a++)
          for(b=0; b < openpi[h]; b++)
              fab.matrix[h][a][virtpi[h] - openpi[h] + b] = Fb->get(h, a + frdocc[h] + occpi[h], b + frdocc[h] + occpi[h] - openpi[h]);

      for(a=0; a < openpi[h]; a++)
          for(b=0; b < (virtpi[h] - openpi[h]); b++)
              fab.matrix[h][virtpi[h] - openpi[h] + a][b] = Fb->get(h, a + frdocc[h] + occpi[h] - openpi[h], b + frdocc[h] + occpi[h]);
  }

  /* Close the alpha and beta vir-vir Fock matrix files */
  dpd_file2_mat_wrt(&fAB);
  dpd_file2_mat_wrt(&fab);
  dpd_file2_mat_close(&fAB);
  dpd_file2_mat_close(&fab);
#if DEBUG
  dpd_file2_print(&fAB, outfile);
  dpd_file2_print(&fab, outfile);
#endif
  dpd_file2_close(&fAB);
  dpd_file2_close(&fab);

  /* Prepare the alpha and beta occ-vir Fock matrix files */
  dpd_file2_init(&fIA, PSIF_CC_OEI, 0, 0, 1, "fIA");
  dpd_file2_init(&fia, PSIF_CC_OEI, 0, 0, 1, "fia");
  dpd_file2_mat_init(&fIA);
  dpd_file2_mat_init(&fia);

  /* One-electron (frozen-core) contributions */
  for(h=0; h < nirreps; h++) {

      for(i=0; i < occpi[h]; i++)
          for(a=0; a < (virtpi[h] - openpi[h]); a++)
              fIA.matrix[h][i][a] = Fa->get(h, i + frdocc[h], a + frdocc[h] + occpi[h]);

      for(i=0; i < (occpi[h] - openpi[h]); i++)
          for(a=0; a < (virtpi[h] - openpi[h]); a++)
              fia.matrix[h][i][a] = Fb->get(h, i + frdocc[h], a + frdocc[h] + occpi[h]);

      for(i=0; i < (occpi[h] - openpi[h]); i++)
          for(a=0; a < openpi[h]; a++)
              fia.matrix[h][i][virtpi[h] - openpi[h] + a] = Fb->get(h, i + frdocc[h], a + frdocc[h] + occpi[h] - openpi[h]);

  }

  /* Close the alpha and beta occ-vir Fock matrix files */
  dpd_file2_mat_wrt(&fIA);
  dpd_file2_mat_wrt(&fia);
  dpd_file2_mat_close(&fIA);
  dpd_file2_mat_close(&fia);
#if DEBUG
  dpd_file2_print(&fIA, outfile);
  dpd_file2_print(&fia, outfile);
#endif
  dpd_file2_close(&fIA);
  dpd_file2_close(&fia);

}

}} // namespace psi::ccsort
