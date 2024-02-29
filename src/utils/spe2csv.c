/* spe2csv.c
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
#include <ctype.h>

#include "../data_io.h"

void
spe2csv (FILE *fp)
{
  size_t i = 1, MAX_NUM_ROWS = 1024;
  const size_t MAX_COMMENT_LEN = 256;
  char line[MAX_COMMENT_LEN];
  FILE *o_csvfp = fopen ("");

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

