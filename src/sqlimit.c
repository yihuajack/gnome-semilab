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

#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include <unistd.h>

#include "sqlimit.h"
#include "csv_reader.h"

static void
interpolate_interp1d (struct csv_data *spectrum)
{
  gsl_interp_accel *acc = gsl_interp_accel_alloc ();
  const gsl_interp_type *t = gsl_interp_linear;
  gsl_spline *spline = gsl_spline_alloc (t, 10);
  gsl_spline_init (spline, spectrum->wavelengths, spectrum->intensities, spectrum->num_datarows);
}

void
sqlimit_main (FILE *fp)
{
  struct csv_data *spectrum = read_csv (fp);
  interpolate_interp1d (spectrum);
}

