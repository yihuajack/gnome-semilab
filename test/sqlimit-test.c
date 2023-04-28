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

#define TEST_POLY

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
#ifdef TEST_ASTMG173
  fp = fopen ("/home/ayka-tsuzuki/gnome-semilab/test/astmg173.csv", "r");
  if (!fp)
    {
      perror ("File opening failed\n");
      exit (EXIT_FAILURE);
    }
  rewind (fp);
  struct csv_data *spectrum = read_csv (fp, true, true, 1);
  sqlimit_main (spectrum, VERTICAL);
  fclose (fp);
// #elifdef and #elifndef are not supported by MSVC yet
#elif defined TEST_POLY
  fp = fopen ("/home/ayka-tsuzuki/gnome-semilab/test/poly_spectrum.csv", "r");
  struct csv_data_2d *spectrum_2d = read_csv (fp, false, false, 2);
  sqlimit_main_2d (spectrum_2d, HORIZONTAL);
  fclose (fp);
#endif
  exit (EXIT_SUCCESS);
}

