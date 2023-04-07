/* csv_reader.h
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

#include <stdio.h>

struct csv_data_uint_double
{
  char        **fields;
  unsigned int *wavelengths;
  double       *intensities;
  unsigned int  num_fields;
  unsigned int  num_datarows; // excluding header rows
};

struct csv_data
{
  char        **fields;
  double       *wavelengths;
  double       *intensities;
  unsigned int  num_fields;
  unsigned int  num_datarows;
};

extern
char           **read_csv_fields (FILE *fp,
                                  int  *length);

extern
void             cb1_uint_double (void   *s,
                                  size_t  len,
                                  void   *data);

extern
void             cb1_double_double (void   *s,
                                    size_t  len,
                                    void    *data);

extern
void             cb2_uint_double (int   c,
                                  void *data);

extern
void             cb2_double_double (int   c,
                                    void *data);

extern
struct csv_data *read_csv        (FILE *fp);

