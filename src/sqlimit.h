/* sqlimit.h
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

#include "csv_reader.h"

#ifndef SQLIMIT_H
#define SQLIMIT_H

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) printf("INFO: " fmt, __VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) do {} while (0)
#endif

enum eff_bg_types
{
  EFF_BG_1D,
  EFF_BG_2D
};

struct eff_bg
{
  double *bandgap;
  double *efficiency;
  double *fill_factor;
  size_t  length;
};

struct eff_bg_2d
{
  double  *bandgap;
  double **efficiency;
  size_t   length;
};

struct var_eff_bg
{
  enum eff_bg_types type;
  union
    {
      struct eff_bg    data_1d;
      struct eff_bg_2d data_2d;
    } value;
};

extern
const double c0;

extern
const double eV;

extern
const double hPlanck;

extern
const double hbar;

extern
const double Tcell;

extern
const double kB;

extern
const double sigma_SB;

extern
double           *linspace        (double start,
                                   double stop,
                                   size_t num);

extern
struct eff_bg     sqlimit_main    (void *spectrum,
                                   bool  axis);

extern
struct eff_bg_2d  sqlimit_main_2d (void *spectrum,
                                   bool  axis);

#endif  /* SQLIMIT_H */

