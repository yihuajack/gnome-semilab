/* csv-sketch-reader.c
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

#include "../src/sqlimit.h"

void
csv_sketch_read (FILE          *fp,
                 struct eff_bg *eff_bg_data,
                 char           delim)
{
  eff_bg_data->bandgap = (double *)calloc (eff_bg_data->length, sizeof (double));
  eff_bg_data->efficiency = (double *)calloc (eff_bg_data->length, sizeof (double));
  if (delim == ' ')
    {
      for (size_t i = 0; i < eff_bg_data->length; i++)
        {
          fscanf (fp, "%lf%lf", &eff_bg_data->bandgap[i], &eff_bg_data->efficiency[i]);
        }
    }
}

int
main (int   argc,
      char *argv[])
{
  FILE *fp = fopen ("/home/ayka-tsuzuki/gnome-semilab/test/Tungsten-Halogen_PCE.tsv", "r");
  struct eff_bg eff_bg_data;
  eff_bg_data.length = 200;
  csv_sketch_read (fp, &eff_bg_data, ' ');
  fclose (fp);
  for (size_t i = 0; i < eff_bg_data.length; i++)
    {
      printf ("%lf %lf\n", eff_bg_data.bandgap[i], eff_bg_data.efficiency[i]);
    }
  free (eff_bg_data.bandgap);
  free (eff_bg_data.efficiency);
  return 0;
}

