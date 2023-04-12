/* gsl-test.c
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

#include <gsl/gsl_statistics.h>

#include "../src/csv_reader.h"

int
main (int   argc,
      char *argv[])
{
  FILE *fp = fopen ("/home/ayka-tsuzuki/gnome-semilab/test/Tungsten-Halogen 3300K.csv", "r");
  if (!fp)
    {
      perror("File opening failed");
      exit (EXIT_FAILURE);
    }
  struct csv_data *spectrum_data = read_csv (fp);

  double xmin, xmax, ymin, ymax;
  printf ("Wavelengths: %f %f ...\n", spectrum_data->wavelengths[0], spectrum_data->wavelengths[1]);
  printf ("Wavelengths: %f %f ...\n", spectrum_data->intensities[0], spectrum_data->intensities[1]);
  printf ("Number of data rows: %d\n", spectrum_data->num_datarows);
  gsl_stats_minmax (&xmin, &xmax, spectrum_data->wavelengths, 1, spectrum_data->num_datarows);
  gsl_stats_minmax (&ymin, &ymax, spectrum_data->intensities, 1, spectrum_data->num_datarows);
  printf ("%f %f %f %f\n", xmin, xmax, ymin, ymax);

  fclose (fp);
}

