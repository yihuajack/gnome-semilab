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
#include <gsl/gsl_sf_exp.h>
#include <gsl/gsl_sf_log.h>
#include <gsl/gsl_multimin.h>
// #include <progressbar/progressbar.h>
#include <time.h>

#include "sqlimit.h"

struct spline_params
{
  gsl_spline       *spline;
  gsl_interp_accel *acc;
};

struct min_params
{
  double        Egap;
  double        Emax;
  gsl_function *F_s;
  gsl_function *F_RR0;
};

/* Solar Photons per unit Time, per unit photon Energy-range, and per unit Area of the solar cell
 * (assuming the cell is facing normal to the sun) */
static double
s_photons_per_tea (double  Ephoton,  /* J */
                   void   *params)
{
  // convert energy to wavelength
  double lambda = hPlanck * c0 / Ephoton;  /* m */
  gsl_spline *spline = ((struct spline_params *)params)->spline;
  gsl_interp_accel *acc = ((struct spline_params *)params)->acc;
  /* https://lists.libreplanet.org/archive/html/help-gsl/2013-06/msg00013.html
   * https://stackoverflow.com/questions/40931337/interpolation-error-with-gsl-interp-linear
   * x >= interp->xmin && x <= interp->xmax; otherwise
   * ERROR: interpolation error\nDefault GSL error handler invoked.
   * The documentation does not mention, see source code of `gsl_spline.h`:
   * gsl_spline has four members: gsl_interp *interp, double *x, double *y, and size_t size */
  if (lambda * 1E9 < spline->interp->xmin || lambda * 1E9 > spline->interp->xmax)
    {
      fprintf (stderr, "Error: current wavelength %.17g nm is not in the range of [%.17g nm, %.17g nm].\n", lambda * 1E9, spline->interp->xmin, spline->interp->xmax);
    }
  /* printf ("Interpolated intensity at wavelength %lf nm is %lf W / m^3.\n", lambda * 1E9, gsl_spline_eval (spline, lambda * 1E9, acc) * 1E9); */
  return gsl_spline_eval (spline, lambda * 1E9 /* m -> nm */, acc) /* W / (m^2 * nm) */ * 1E9 /* W / (m^2 * nm) -> W / m^3 */ / gsl_pow_3 (Ephoton) * hPlanck * c0;  /* (eV * m^2 * s)^(-1) */
}

static double
interp_eval (double  lambda,  /* m */
             void   *params)
{
  gsl_spline *spline = ((struct spline_params *)params)->spline;
  gsl_interp_accel *acc = ((struct spline_params *)params)->acc;
  return gsl_spline_eval (spline, lambda * 1E9, acc) * 1E9;
}

static double
power_per_tea (double  Ephoton,  /* J */
               void   *params)
{
  return Ephoton * s_photons_per_tea (Ephoton, params);
}

static double
solar_photons_above_gap (double        Egap,  /* J */
                         double        Emax,  /* J */
                         gsl_function *F_s)
{
  double result, error;
  size_t iter_lim = 50;
  gsl_integration_workspace *s_int_ws = gsl_integration_workspace_alloc (iter_lim);
  gsl_integration_qags (F_s, Egap, Emax, 1.49E-08, 1.49E-08, iter_lim, s_int_ws, &result, &error);
  gsl_integration_workspace_free (s_int_ws);
  /* (m^2 s)^(-1) */
  return result;
}

static double
RR0_integrand (double  E,  /* J */
               void   *params)
{
  return E * E  / (gsl_sf_exp (E / (kB * Tcell)) - 1);
}

/* Recombination rate when electron QFL and hole QFL are split
 * QFL: Quasi-Fermi Level  */
static double
RR0 (double        Egap,  /* J */
     double        Emax,  /* J */
     gsl_function *F_RR0)
{
  double integral, error;
  size_t iter_lim = 50;
  gsl_integration_workspace *s_int_ws = gsl_integration_workspace_alloc (iter_lim);
  gsl_integration_qags (F_RR0, Egap, Emax, 1.49E-08, 1.49E-08, iter_lim, s_int_ws, &integral, &error);
  gsl_integration_workspace_free (s_int_ws);
  /* (m^2 s)^(-1) */
  return 2 * M_PI / (c0 * c0 * gsl_pow_3 (hPlanck)) * integral;
}

static double
current_density (double             voltage,  /* V */
                 struct min_params *params)
{
  /* A/m^2 */
  return eV * (solar_photons_above_gap (params->Egap, params->Emax, params->F_s) - RR0 (params->Egap, params->Emax, params->F_RR0) * gsl_sf_exp (eV * voltage / (kB * Tcell)));
}

/* Short-circuit current density */
static double
JSC (struct min_params *params)
{
  /* A/m^2 */
  return current_density (0, params);
}

/* Open-circuit voltage */
static double
VOC (struct min_params *params)
{
  /* V */
  return (kB * Tcell / eV) * gsl_sf_log (solar_photons_above_gap (params->Egap, params->Emax, params->F_s) / RR0 (params->Egap, params->Emax, params->F_RR0));
}

static double
func_to_minimize (const gsl_vector *v,
                  void             *params)
{
  double voltage = gsl_vector_get (v, 0);
  return -voltage * current_density (voltage, (struct min_params *)params);
}

static double
V_mpp (gsl_multimin_function *min_func)
{
  const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;
  gsl_multimin_fminimizer *s = NULL;
  gsl_vector *ss /* step size */, *x;
  double retval;

  size_t iter = 0;
  int status;
  double size;

  x = gsl_vector_alloc (1);
  // Compare to initial guess x0 = 0
  gsl_vector_set (x, 0, 0);

  ss = gsl_vector_alloc (1);
  // Initial step size step_size = 1.0
  gsl_vector_set (ss, 0, 1.0);

  s = gsl_multimin_fminimizer_alloc (T, 1);
  gsl_multimin_fminimizer_set (s, min_func, x, ss);

  do
    {
      iter++;
      status = gsl_multimin_fminimizer_iterate (s);

      if (status)
        break;

      size = gsl_multimin_fminimizer_size (s);
      // Compare to absolute error in xopt between iterations that is acceptable for convergence xtol = 0.0001
      // Compare to absolute error in func(xopt) between iterations that is acceptable for convergence ftol = 0.0001
      status = gsl_multimin_test_size (size, 1E-3);

      if (status == GSL_SUCCESS)
        {
          retval = gsl_vector_get (s->x, 0);
          /* printf ("converged to minimum at iter=%lu V=%lf P=%lf size=%lf\n", iter, retval, -s->fval, size); */
        }
    }
  // Compare to maximum number of iterations to perform maxiter = None
  while (status == GSL_CONTINUE /* && iter < 100 */);

  gsl_vector_free (x);
  gsl_multimin_fminimizer_free (s);

  return retval;
}

static double
J_mpp (gsl_multimin_function *min_func)
{
  return current_density (V_mpp (min_func), min_func->params);
}

static double
max_power (gsl_multimin_function *min_func)
{
  const double voltage = V_mpp (min_func);
  return voltage * current_density (voltage, min_func->params);
}

static double
max_efficiency (double                 radiation,
                gsl_multimin_function *min_func)
{
  return max_power (min_func) / radiation;
}

static double
fill_factor (gsl_multimin_function *min_func)
{
  return max_power (min_func) / (JSC (min_func->params) * VOC (min_func->params));
}

static double
absorbed_power (double                     absorption_edge,  /* m */
                double                     lambda_min,       /* m */
                double                     lambda_max,       /* m */
                double                     radiation,        /* W/m^2 */
                struct spline_params      *params,
                gsl_integration_workspace *int_ws)
{
  gsl_function F_interp;
  F_interp.function = &interp_eval;
  F_interp.params = params;

  if (absorption_edge > lambda_max)
    {
      return radiation;
    }
  else
    {
      double result, error;
      const double iter_lim = 50;
      gsl_integration_qags (&F_interp, lambda_min, absorption_edge, 1.49E-08, 1.49E-08, iter_lim, int_ws, &result, &error);
      return result;
    }
}

static double
rad_integrand (double  lambda,  /* m */
               void   *params)
{
  double *temperature = (double *)params;
  double E_over_kT = hPlanck * c0 / (lambda * kB * *temperature);
  return (E_over_kT < 20) ? 1 / (gsl_pow_5 (lambda) * (gsl_sf_exp (E_over_kT) - 1)) : 0;
}

static double
emitted_radiation (double                     temperature,  /* T */
                   double                     absorption_edge,  /* m */
                   gsl_integration_workspace *int_ws)
{
  gsl_function F_rad;
  F_rad.function = &rad_integrand;
  F_rad.params = &temperature;
  double result, error;
  const double iter_lim = 50;

  gsl_integration_qags (&F_rad, 5E-8 /* m */, absorption_edge, 1.49E-08, 1.49E-08, iter_lim, int_ws, &result, &error);

  return 2 * M_PI * hPlanck * c0 * c0 * result;
}

static double
power_generation (double                     T_hot,            /* K */
                  double                     absorption_edge,  /* m */
                  unsigned int               concentration,
                  double                     lambda_min,       /* m */
                  double                     lambda_max,       /* m */
                  double                     radiation,   /* W/m^2 */
                  struct spline_params      *params,
                  gsl_integration_workspace *int_ws)
{
  const double T_ambient = 300 /* K */;
  if (T_hot < T_ambient)
    return 0;

  const double hot_side_absorption = absorbed_power (absorption_edge, lambda_min, lambda_max, radiation, params, int_ws);
  const double hot_side_emission = emitted_radiation (T_hot, absorption_edge, int_ws);
  const double hot_side_net_absorption = hot_side_absorption - hot_side_emission;
  const double carnot_efficiency = 1 - T_ambient / T_hot;
  return (hot_side_net_absorption > 0) ? hot_side_net_absorption * carnot_efficiency : 0;
}

double *linspace (double start,
                  double stop,
                  size_t num)
{
  double *arr = (double *)calloc (num, sizeof (double));
  double step = (stop - start) / (num - 1);
  for (size_t i = 0; i < num; i++)
    {
      arr[i] = start + i * step;
    }
  return arr;
}

struct eff_bg
sqlimit_main (struct csv_data *spectrum,
              bool             axis)
{
  struct eff_bg eff_bg_data = {0};

  gsl_interp_accel *acc = gsl_interp_accel_alloc ();
  DEBUG_PRINT ("Interpolation lookup accelerator object allocated.\n");
  // scipy.interpolate.interp1d use `linear` by default
  const gsl_interp_type *t = gsl_interp_linear;
  /* gsl_spline workspace provides a higher level interface for the gsl_interp object
   * The allocated size must match the initialized size; otherwise, in spline.c gsl_spline_init ()
   * if (size != spline->size)
   * GSL_ERROR ("data must match size of spline object", GSL_EINVAL);
   * If the size is less than the total size of data, the data will be truncated */
  int spline_status;
  gsl_spline *spline = gsl_spline_alloc (t, spectrum->num_datarows);
  DEBUG_PRINT ("Spline allocated of size %u.\n", spectrum->num_datarows);
  if (axis == VERTICAL)  // vertical
    {
      spline_status = gsl_spline_init (spline, spectrum->wavelengths, spectrum->intensities, spectrum->num_datarows);
    }
  else
    {
      fprintf (stderr, "Axis = 0, Dim = 1 is not implemented.\n");
      return eff_bg_data;
    }
  if (spline_status)
    {
      fprintf (stderr, "ERROR: %s\n",  gsl_strerror (spline_status));
    }
  DEBUG_PRINT ("Spline initialized with error number %d.\n", spline_status);

  /* Need to allocate enough size; otherwise
   * ERROR: a maximum of one iteration was insufficient
   * The size allocated for the workspace must be greater than or equal to the iteration limit of QAG; otherwise
   * if (limit > workspace->limit)
   * GSL_ERROR ("iteration limit exceeds available workspace", GSL_EINVAL) ;  */
  const size_t iter_lim = 50;
  gsl_integration_workspace *p_int_ws = gsl_integration_workspace_alloc (iter_lim);

  // For the solar spectrum, the radiation is the solar constant approximately equal to 1000 W/m^2
  double radiation;  // the final approximation from the extrapolation `result`
  double error;  // an estimate of the absolute error `abserr`
  struct spline_params sql_spline_params;
  sql_spline_params.spline = spline;
  sql_spline_params.acc = acc;

  gsl_function F_p, F_s, F_RR0;
  F_p.function = &power_per_tea;
  F_s.function = &s_photons_per_tea;
  F_RR0.function = &RR0_integrand;
  F_p.params = &sql_spline_params;
  F_s.params = &sql_spline_params;
  F_RR0.params = NULL;

  // No need to manually calculate xmin and xmax by gsl_statistics' gsl_stats_minmax()
  double lambda_min = spline->interp->xmin * 1E-9, lambda_max = spline->interp->xmax * 1E-9;  /* m */
  double E_min = hPlanck * c0 / lambda_max, E_max = hPlanck * c0 / lambda_min;  /* J */
  double E_mean = (E_min + E_max) / 2;  /* J */
  double E_min_eV = E_min / eV, E_max_eV = E_max / eV, E_mean_eV = E_mean / eV;  /* eV */
  DEBUG_PRINT ("λ_min = %lf nm, λ_max = %lf nm, E_min = %lf eV, E_max = %lf eV.\n", spline->interp->xmin , spline->interp->xmax, E_min_eV, E_max_eV);
  DEBUG_PRINT ("EXAMPLE: s_photons_per_tea(E_mean = %lf eV) = %.17g\n", E_mean_eV, s_photons_per_tea (E_mean, F_s.params) * 1E-3 * eV);

  struct min_params sql_min_params;
  sql_min_params.Emax = E_max;
  sql_min_params.F_s = &F_s;
  sql_min_params.F_RR0 = &F_RR0;

  /* epsabs, epsrel, and limit=50 keep same as scipy.integrate.quad (full_output=0 to show full output)
   * points=None, weight=None; no infinite bounds => QUADPACK routine is qagse
   * (globally adaptive interval subdivision in connection with extrapolation)
   * By testing, qags gives smaller error than qag with GSL_INTEG_GAUSS61
   * IntegrationWarning: The maximum number of subdivisions (50) has been achieved.
   * If increasing the limit yields no improvement it is advised to analyze
   * the integrand in order to determine the difficulties.  If the position of a
   * local difficulty can be determined (singularity, discontinuity) one will
   * probably gain from splitting up the interval and calling the integrator
   * on the subranges.  Perhaps a special-purpose integrator should be used.
   * If setting limit to 1000, there would be error
   * ERROR: roundoff error prevents tolerance from being achieved
   * because the absolute and relative tolerances are too stringent.
   * If setting limit to 50, there would be error
   * else if (iteration == limit)
   * GSL_ERROR ("maximum number of subdivisions reached", GSL_EMAXITER);
   * error code is GSL_EMAXITER = 11
   * exceeded max number of iterations
   * Thus, we have to "pass" the error  */
  gsl_error_handler_t *default_handler = gsl_set_error_handler_off ();
  int err_code = gsl_integration_qags (&F_p, E_min, E_max, 1.49E-08, 1.49E-08, iter_lim, p_int_ws, &radiation, &error);
  DEBUG_PRINT ("(Error code %d) Calculated radiation is %lf W/m^2 with error %lf.\n", err_code, radiation, error);
  DEBUG_PRINT ("EXAMPLE: solar_photons_above_gap(E_g = %lf eV) = %lf / (m^2 s)\n", 1.5, solar_photons_above_gap (1.5 * eV, E_max, &F_s));

  /* Use Nelder-Mead (downhill) Simplex algorithm (minimizing without derivatives)
   * gsl_multimin_fminizer_nmsimplex and gsl_multimin_fminimizer_nmsimplex2 are both of O(N^2) memory usage
   * but gsl_multimin_fminimizer_nmsimplex2 is O(N) operations while gsl_multimin_fminimizer_nmsimplex is O(N^2) operations */
  gsl_multimin_function min_func;

  min_func.n = 1;  // Number of function components
  min_func.f = &func_to_minimize;
  sql_min_params.Egap = 1.5 * eV;
  min_func.params = &sql_min_params;

  DEBUG_PRINT ("EXAMPLE: RR0(E_g = %lf eV) = %lf /(m^2 s)\n", 1.5, RR0 (sql_min_params.Egap, sql_min_params.Emax, sql_min_params.F_RR0));
  DEBUG_PRINT ("EXAMPLE: JSC(E_g = %lf eV) = %lf A/m^2\n", 1.5, JSC (&sql_min_params));
  DEBUG_PRINT ("EXAMPLE: VOC(E_g = %lf eV) = %lf V\n", 1.5, VOC (&sql_min_params));
  DEBUG_PRINT ("EXAMPLE: V_mpp(E_g = %lf eV) = %lf V\n", 1.5, V_mpp (&min_func));
  DEBUG_PRINT ("EXAMPLE: max_efficiency(E_g = %lf eV) = %lf%%\n", 1.5, max_efficiency (radiation, &min_func) * 100);
  DEBUG_PRINT ("EXAMPLE: fill_factor(E_g = %lf eV) = %lf\n", 1.5, fill_factor (&min_func));

  eff_bg_data.length = 100;
  // Start and end of the linear space should be a little narrower, otherwise it will stuck at the last loop below.
  // Also do not exceed the E_min and E_max limit.
  eff_bg_data.bandgap = linspace (E_min + 0.01 * eV, E_max - 0.01 * eV, eff_bg_data.length);
  eff_bg_data.efficiency = (double *)calloc (eff_bg_data.length, sizeof (double));

  clock_t timer;
  timer = clock ();
  for (size_t i = 0; i < eff_bg_data.length; i++)
    {
      sql_min_params.Egap = eff_bg_data.bandgap[i];
      min_func.params = &sql_min_params;
      eff_bg_data.efficiency[i] = max_efficiency (radiation, &min_func);
    }
  timer = clock () - timer;
  DEBUG_PRINT ("Time cost: %lf s\n", ((double) timer) / CLOCKS_PER_SEC);
  gsl_vector_view eff_list = gsl_vector_view_array (eff_bg_data.efficiency, eff_bg_data.length);
  printf ("Max efficiency %lf%% at %lf eV\n", gsl_vector_max (&eff_list.vector) * 100, E_min_eV + gsl_vector_max_index (&eff_list.vector) * (E_max_eV - E_min_eV) / (eff_bg_data.length - 1));

#ifdef DEBUG
  eff_bg_data.fill_factor = (double *)calloc (eff_bg_data.length, sizeof (double));
  for (size_t i = 0; i < eff_bg_data.length; i++)
    {
      sql_min_params.Egap = eff_bg_data.bandgap[i];
      min_func.params = &sql_min_params;
      eff_bg_data.fill_factor[i] = fill_factor (&min_func);
    }
#endif

  DEBUG_PRINT ("EXAMPLE: absorbed_power(1000 nm) = %lf\n", absorbed_power (1E-6, lambda_min, lambda_max, radiation, &sql_spline_params, p_int_ws));
  DEBUG_PRINT ("check Stefan–Boltzmann law (should equal 1): %lf\n", sigma_SB * gsl_pow_4 (345 /* K */) / emitted_radiation (345 /*K*/, 8E-5 /* m */, p_int_ws));

  // Restore the default error handler
  gsl_set_error_handler (default_handler);

  gsl_spline_free (spline);
  gsl_interp_accel_free (acc);
  gsl_integration_workspace_free (p_int_ws);

  return eff_bg_data;
}

struct eff_bg_2d
sqlimit_main_2d (struct csv_data_2d *spectrum,
                 bool                axis)
{
  unsigned int i = 0;
  struct eff_bg_2d eff_bg_data = {0};

  gsl_interp_accel *acc = gsl_interp_accel_alloc ();
  const gsl_interp_type *t = gsl_interp_linear;
  gsl_spline **splines = (gsl_spline **)calloc (spectrum->num_datarows, sizeof (gsl_spline *));
  if (axis == HORIZONTAL)  // horizontal
    {
      for (i = 0; i < spectrum->num_datarows; i++)
        {
          splines[i] = gsl_spline_alloc (t, spectrum->num_fields);
          gsl_spline_init (splines[i], spectrum->wavelengths, spectrum->intensities[i], spectrum->num_fields);
        }
    }
  else
    {
      fprintf (stderr, "Axis = 1, Dim = 2 is not implemented.\n");
      return eff_bg_data;
    }

  const size_t iter_lim = 50;
  gsl_integration_workspace *p_int_ws = gsl_integration_workspace_alloc (iter_lim);

  double radiation, error, lambda_min, lambda_max, E_min, E_max;
  struct spline_params sql_spline_params;
  sql_spline_params.acc = acc;

  gsl_function F_p, F_s, F_RR0;
  F_p.function = &power_per_tea;
  F_s.function = &s_photons_per_tea;
  F_RR0.function = &RR0_integrand;
  F_RR0.params = NULL;

  gsl_multimin_function min_func;
  min_func.n = 1;
  min_func.f = &func_to_minimize;

  lambda_min = splines[0]->interp->xmin * 1E-9;
  lambda_max = splines[0]->interp->xmax * 1E-9;
  E_min = hPlanck * c0 / lambda_max;
  E_max = hPlanck * c0 / lambda_min;

  struct min_params sql_min_params;
  sql_min_params.Emax = E_max;
  sql_min_params.F_RR0 = &F_RR0;

  eff_bg_data.length = 100;
  /* The stop value should be less than E_max; otherwise, wavelengths will be out of range;
   * however, the start value could be less than or equal to E_min.
   * The stop value should also not be exactly same as E_max;
   * otherwise, the program will iterate infinitely in V_mpp(). */
  eff_bg_data.bandgap = linspace (E_min, 0.999 * E_max, eff_bg_data.length);
  eff_bg_data.efficiency = (double **)calloc (spectrum->num_datarows, sizeof (double *));

  gsl_vector_view eff_list;
  gsl_error_handler_t *default_handler = gsl_set_error_handler_off ();
//  progressbar *minimizing_eff = progressbar_new ("", spectrum->num_datarows);
  FILE *fout = fopen ("/home/ayka-tsuzuki/gnome-semilab/test/poly_spectrum_results.txt", "w");

  for (i = 0; i < spectrum->num_datarows; i++)
    {
      eff_bg_data.efficiency[i] = (double *)calloc (eff_bg_data.length, sizeof (double));

      sql_spline_params.spline = splines[i];
      F_p.params = &sql_spline_params;
      F_s.params = &sql_spline_params;
      sql_min_params.F_s = &F_s;

      gsl_integration_qags (&F_p, E_min, E_max, 1.49E-08, 1.49E-08, iter_lim, p_int_ws, &radiation, &error);

      for (size_t j = 0; j < eff_bg_data.length; j++)
        {
          sql_min_params.Egap = eff_bg_data.bandgap[j];
          min_func.params = &sql_min_params;
          eff_bg_data.efficiency[i][j] = max_efficiency (radiation, &min_func);
        }

      eff_list = gsl_vector_view_array (eff_bg_data.efficiency[i], eff_bg_data.length);

      fprintf (fout, "Max efficiency %lf%% at %lf eV\n", gsl_vector_max(&eff_list.vector) * 100, (E_min + gsl_vector_max_index (&eff_list.vector) * (0.999 * E_max - E_min) / (eff_bg_data.length - 1)) / eV);

      gsl_spline_free (splines[i]);
//      progressbar_inc (minimizing_eff);
    }

  fclose (fout);
//  progressbar_finish (minimizing_eff);

  gsl_set_error_handler (default_handler);

  free (splines);
  gsl_interp_accel_free (acc);
  gsl_integration_workspace_free (p_int_ws);

  return eff_bg_data;
}

