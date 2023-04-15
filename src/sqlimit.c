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
#include <gsl/gsl_integration.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_math.h>

#include "sqlimit.h"

struct spline_params
{
  gsl_spline       *spline;
  gsl_interp_accel *acc;
};

/* Solar Photons per unit Time, per unit photon Energy-range, and per unit Area of the solar cell
 * (assuming the cell is facing normal to the sun) */
static double
s_photons_per_tea (double  Ephoton, /* J */
                   void   *params)
{
  // convert energy to wavelength
  double lambda = hPlanck * c0 / Ephoton;  /* m */
  gsl_spline *spline = ((struct spline_params *)params)->spline;
  gsl_interp_accel *acc = ((struct spline_params *)params)->acc;
  // https://lists.libreplanet.org/archive/html/help-gsl/2013-06/msg00013.html
  // https://stackoverflow.com/questions/40931337/interpolation-error-with-gsl-interp-linear
  // x >= interp->xmin && x <= interp->xmax; otherwise
  // ERROR: interpolation error\nDefault GSL error handler invoked.
  // The documentation does not mention, see source code of `gsl_spline.h`:
  // gsl_spline has four members: gsl_interp *interp, double *x, double *y, and size_t size
  if (lambda * 1E9 < spline->interp->xmin || lambda * 1E9 > spline->interp->xmax)
    {
      fprintf (stderr, "Error: current wavelength %.17g nm is not in the range of [%.17g nm, %.17g nm].\n", lambda * 1E9, spline->interp->xmin, spline->interp->xmax);
    }
  /* printf ("Interpolated intensity at wavelength %lf nm is %lf W / m^3.\n", lambda * 1E9, gsl_spline_eval (spline, lambda * 1E9, acc) * 1E9); */
  return gsl_spline_eval (spline, lambda * 1E9 /* m -> nm */, acc) /* W / (m^2 * nm) */ * 1E9 /* W / (m^2 * nm) -> W / m^3 */ / gsl_pow_3 (Ephoton) * hPlanck * c0;  /* (eV * m^2 * s)^(-1) */
}

static double
power_per_tea (double  Ephoton, /* J */
               void   *params)
{
  return Ephoton * s_photons_per_tea (Ephoton, params);
}

void
sqlimit_main (struct csv_data *spectrum)
{
  gsl_interp_accel *acc = gsl_interp_accel_alloc ();
  printf ("INFO: Interpolation lookup accelerator object allocated.\n");
  // scipy.interpolate.interp1d use `linear` by default
  const gsl_interp_type *t = gsl_interp_linear;
  // gsl_spline workspace provides a higher level interface for the gsl_interp object
  // The allocated size must match the initialized size; otherwise, in spline.c gsl_spline_init ()
  // if (size != spline->size)
  // GSL_ERROR ("data must match size of spline object", GSL_EINVAL);
  // If the size is less than the total size of data, the data will be truncated
  gsl_spline *spline = gsl_spline_alloc (t, spectrum->num_datarows);
  printf ("INFO: Spline allocated of size %u.\n", spectrum->num_datarows);
  int status = gsl_spline_init (spline, spectrum->wavelengths, spectrum->intensities, spectrum->num_datarows);
  if (status)
    {
      fprintf (stderr, "ERROR: %s\n",  gsl_strerror (status));
    }
  printf ("INFO: Spline initialized with error number %d.\n", status);

  // Need to allocate enough size; otherwise
  // ERROR: a maximum of one iteration was insufficient
  gsl_integration_workspace *p_int_ws = gsl_integration_workspace_alloc (50);

  double result, error;
  struct spline_params sql_spline_params;
  sql_spline_params.spline = spline;
  sql_spline_params.acc = acc;

  gsl_function F_p, F_s;
  F_p.function = &power_per_tea;
  F_s.function = &s_photons_per_tea;
  F_p.params = &sql_spline_params;
  F_s.params = &sql_spline_params;

  // No need to manually calculate xmin and xmax by gsl_statistics' gsk_stats_minmax()
  double lambda_min = spline->interp->xmin * 1E-9, lambda_max = spline->interp->xmax * 1E-9;  /* m */
  double E_min = hPlanck * c0 / lambda_max, E_max = hPlanck * c0 / lambda_min;  /* J */
  printf ("INFO: λ_min = %lf nm, λ_max = %lf nm, E_min = %lf eV, E_max = %lf eV.\n", spline->interp->xmin , spline->interp->xmax, E_min / eV, E_max / eV);
  printf ("EXAMPLE: %.17g\n", s_photons_per_tea (2 * eV, F_s.params) * 1E-3 * eV);

  // epsabs, epsrel, and limit keep same as scipy.integrate.quad
  const double solar_constant = gsl_integration_qag (&F_p, E_min, E_max, 1.49E-08, 1.49E-08, 50, GSL_INTEG_GAUSS61, p_int_ws, &result, &error);
  printf ("Calculated solar constant is %.17g\n.", solar_constant);

  gsl_spline_free (spline);
  gsl_interp_accel_free (acc);
  gsl_integration_workspace_free (p_int_ws);
}

