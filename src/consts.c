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
#include "math.h"

// numericalunits.py
// Speed of light in vacuum
// warning: c0 initialized and declared 'extern'
const double c0 = 299792458;  // m / s

// Electronvolt
const double eV = 1.602176634E-19;  // J

// Planck constant
const double hPlanck = 6.62607015E-34;  // J * s

// Reduced Planck constant
// hbar = h / (2 * pi)
const double hbar = hPlanck / (2 * M_PI);

const double Tcell = 300;  // K

// Boltzmann constant
const double kB = 1.380649E-23;  // J / K

// Stefan–Boltzmann constant
// sigma_SB = (2 * pi^5 * k^4) / (15 * c^2 * h^3) = (pi^2 * kB^4) / (60 * hbar^3 * c0^2);
// Initializer element must be a compile-time constant
const double sigma_SB = (M_PI * M_PI * kB * kB * kB * kB) / (60 * hbar * hbar * hbar * c0 * c0);  // W / (m^2 * K^4)

