/* sqlimit-test.c
 *
 * Copyright 2023 Yihua Liu <yihuajack@live.cn>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdlib.h>
#include <unistd.h>

#include "../src/sqlimit.h"

// #elifdef and #elifndef are not supported by MSVC yet
#if __GNUC__ >= 12 || __clang_major__ >= 13
#define ELIFDEF_SUPPORTED
#endif

#if 0
#elifndef UNDEFINED_MACRO
#define ELIFDEF_SUPPORTED
#else
#endif

int
main (int   argc,
      char *argv[])
{
  FILE *fp;
  /* FILE *fp = fopen ("/home/ayka-tsuzuki/gnome-semilab/test/Tungsten-Halogen 3300K.csv", "r"); */
  // ASTM G173-03 Reference Spectra Derived from SMARTS v2.9.2
  // Note that the incident light intensity of the light coming from the sun and sky at at typical latitude on a clear day
  // is the THIRD column (Global tilt W*m-2*nm-1) of the original dataset file
  // TODO: directly read from https://www.nrel.gov/grid/solar-resource/assets/data/astmg173.xls
#ifdef TEST_CSV  // like astmg173.csv
  // Currently UTF-8 csv files are not supported yet.
  fp = fopen ("/home/ayka-tsuzuki/gnome-semilab/test/spectra/astmg173.csv", "r");
  if (!fp)
    {
      perror ("File opening failed\n");
      exit (EXIT_FAILURE);
    }
  rewind (fp);
  struct csv_data *spectrum = read_csv (fp, true, true, 1);
  sqlimit_main (spectrum, VERTICAL);
  fclose (fp);
#elif defined TEST_POLY
  fp = fopen ("/home/ayka-tsuzuki/gnome-semilab/test/spectra/poly_spectrum.csv", "r");
  struct csv_data_2d *spectrum = read_csv (fp, false, false, 2);
  struct eff_bg_2d eff_bg_data = sqlimit_main_2d (spectrum, HORIZONTAL);
  fclose (fp);
  for (size_t i = 0; i < spectrum->num_datarows; i++)
    free (spectrum->intensities[i]);

  free (eff_bg_data.bandgap);
  for (size_t i = 0; i < eff_bg_data.length; i++)
    free (eff_bg_data.efficiency[i]);

  free (eff_bg_data.efficiency);
#elif defined TEST_SPE
  fp = fopen ("/home/ayka-tsuzuki/gnome-semilab/test/spectra/AM1.5G ed2 1 sun.spe", "r");
  struct csv_data *spectrum = read_spe (fp);
  sqlimit_main (spectrum, VERTICAL);
#endif
  free (spectrum->wavelengths);
  free (spectrum->intensities);
  free (spectrum);
  exit (EXIT_SUCCESS);
}

