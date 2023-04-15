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

int
main (int   argc,
      char *argv[])
{
  /* FILE *fp = fopen ("/home/ayka-tsuzuki/gnome-semilab/test/Tungsten-Halogen 3300K.csv", "r"); */
  FILE *fp = fopen ("/home/ayka-tsuzuki/gnome-semilab/test/astmg173.csv", "r");
  if (!fp)
    {
      perror ("File opening failed\n");
      exit (EXIT_FAILURE);
    }
  rewind (fp);
  struct csv_data *spectrum = read_csv (fp);
  sqlimit_main (spectrum);
  fclose (fp);
  exit (EXIT_SUCCESS);
}

