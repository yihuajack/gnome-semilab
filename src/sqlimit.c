/* sqlimit.c
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

#include "sqlimit.h"

struct counts
{
  gulong fields;
  gulong rows;
};

void cb1 (void *s, gsize len, void *data)
{
  ((struct counts *)data)->fields++;
}

void cb2 (gint c, void *data)
{
  ((struct counts *)data)->rows++;
}

void read_csv (FILE *fp)
{
  struct csv_parser p;
  char buf[1024];
  size_t bytes_read;
  struct counts c = {0, 0};

  if (csv_init(&p, 0) != 0)
    return;

  while ((bytes_read = fread (buf, 1, 1024, fp)) > 0)
    {
      if (csv_parse (&p, buf, bytes_read, cb1, cb2, &c) != bytes_read)
        {
          fprintf (stderr, "libcsv: Error while parsing file: %s\n", csv_strerror (csv_error (&p)));
          return;
        }
    }

  csv_fini (&p, cb1, cb2, &c);

  printf ("libcsv: %lu fields, %lu rows\n", c.fields, c.rows);

  csv_free (&p);
}

void sqlimit_main (FILE *fp)
{
  read_csv (fp);
}

