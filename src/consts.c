/* consts.c
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

// numericalunits.py
// Speed of light in vacuum
// warning: c0 initialized and declared 'extern'
const double c0 = 299792458;  // m / s

// Electronvolt
const double eV = 1.602176634E-19;  // J

// Planck constant
const double hPlanck = 6.62607015E-34;  // J * s

const double Tcell = 300;  // K

// Boltzmann constant
const double kB = 1.380649E-23;  // J / K

