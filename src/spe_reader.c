/* spe_reader.c
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

/* The SPE reader is used to read SCAPS spectrum files (*.spe)
 * SPE file consists of the comment part and the data part
 * The comment part starts with '>' and introduces the spectrum
 * The data part consists of two columns separated by a space or a tab like TSV files
 * The first column is wavelength in nm
 * The second column is intensity in W/m^2
 * Note that SCAPS is a closed-source software without high-quality maintenance
 * Some spectra are commented with wrong units (W/cm^2) but data with correct units (W/m^2)
 * */

#include <stdlib.h>
#include <ctype.h>

#include "csv_reader.h"

struct csv_data *
read_spe (FILE *fp)
{
  size_t i = 1, MAX_NUM_ROWS = 1024;
  const size_t MAX_COMMENT_LEN = 256;
  char line[MAX_COMMENT_LEN];
  struct csv_data *spectrum = (struct csv_data *)malloc (sizeof (struct csv_data));
  spectrum->num_fields = 2;
  spectrum->wavelengths = (double *)calloc (MAX_NUM_ROWS, sizeof (double));
  spectrum->intensities = (double *)calloc (MAX_NUM_ROWS, sizeof (double));

  while (fgets (line, MAX_COMMENT_LEN, fp))
    {
      // Some SPE comments are tampered so that a comment line may not start with '>'
      if (isdigit (line[0]))
        break;
    }
  sscanf (line, "%lf%lf", spectrum->wavelengths, spectrum->intensities);
  /* starting from 1, first row is separately read; be careful using ++ operator
   * To be less strict, check the return value of fscanf() != -1 or != EOF
   * fscanf() returns the number of matches; for this formatted string, the expected number of matches is 2. */
  while (fscanf (fp, "%lf%lf", &spectrum->wavelengths[i], &spectrum->intensities[i]) == 2)
    {
      if (++i > MAX_NUM_ROWS)
        {
          MAX_NUM_ROWS *= 2;
          spectrum->wavelengths = (double *)realloc (spectrum->wavelengths, MAX_NUM_ROWS * sizeof (double));
          spectrum->intensities = (double *)realloc (spectrum->intensities, MAX_NUM_ROWS * sizeof (double));
        }
    }
  spectrum->num_datarows = i;
  return spectrum;
}

