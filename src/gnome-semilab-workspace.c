/* gnome-semilab-workspace.c
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

#define G_LOG_DOMAIN "gnome-semilab-workspace"

#include <stdlib.h>
#include <glib/gi18n.h>
#include <plplot.h>
#include <gsl/gsl_statistics.h>

#include "gnome-semilab-workspace.h"
#include "gnome-semilab-workspace-private.h"

G_DEFINE_FINAL_TYPE (GnomeSemilabWorkspace, gnome_semilab_workspace, ADW_TYPE_APPLICATION_WINDOW)

enum {
  PROP_0,
  PROP_WS_TYPE,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
open_file_as_spectrum (GnomeSemilabWorkspace *self)
{
  const char *path = g_file_get_path (self->table);
  if (path)
    {
      // /run/user/1000/doc/9b9ece0/Tungsten-Halogen 3300K.csv
      FILE *fp = fopen (path, "r");
      if (fp)
        {
          self->spectrum = read_csv (fp, true, true, 1);
          printf ("INFO: Read spectrum CSV data file %s\n", path);
          fclose (fp);
        }
      else
        {
          g_warning ("Failed to open file %s", path);
        }
      g_free ((gpointer) path);
    }
  else
    {
      g_warning ("Failed to get file path");
    }
}

static void
draw_spec_function (GtkDrawingArea *area,
                    cairo_t        *cr,
                    int             width,
                    int             height,
                    gpointer        data)
{
  struct csv_data *spectrum_data = (struct csv_data *)data;
  const char *xlabel = spectrum_data->fields[0];
  const char *ylabel = spectrum_data->fields[1];
  double xmin, xmax, ymin, ymax;
  gsl_stats_minmax (&xmin, &xmax, spectrum_data->wavelengths, 1, spectrum_data->num_datarows);
  gsl_stats_minmax (&ymin, &ymax, spectrum_data->intensities, 1, spectrum_data->num_datarows);
  printf ("INFO: Plot ranges calculated.\n");

  plsdev ("extcairo");
  printf ("INFO: Device name set to extcairo.\n");
  plinit ();
  printf ("INFO: PLplot initialized.\n");
  pl_cmd (PLESC_DEVINIT, cr);
  /* just = 0: the x and y axes are scaled independently to use as much of the screen as possible. */
  plenv (xmin, xmax, 0, ymax, 0, 0);
  printf ("INFO: Standard window and draw box set.\n");
  pllab (xlabel, ylabel, "Spectrum");
  printf ("INFO: Labels set.\n");
  plcol0 (3);  // Green
  plline (spectrum_data->num_datarows, spectrum_data->wavelengths, spectrum_data->intensities);
  plend ();
  printf ("INFO: Plotting session ended.\n");
}

static void
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

static void
draw_eff_bg_function (GtkDrawingArea *area,
                      cairo_t        *cr,
                      int             width,
                      int             height,
                      gpointer        data)
{
  struct eff_bg *eff_bg_data = (struct eff_bg *)data;
  double xmin, xmax, ymin, ymax;
  gsl_stats_minmax (&xmin, &xmax, eff_bg_data->bandgap, 1, eff_bg_data->length);
  gsl_stats_minmax (&ymin, &ymax, eff_bg_data->efficiency, 1, eff_bg_data->length);

  plsdev ("extcairo");
  plinit ();
  pl_cmd (PLESC_DEVINIT, cr);
  plenv (xmin, xmax, 0, ymax, 0, 0);
  pllab ("Bandgap energy (eV)", "Max efficiency (%)", "Efficiency vs. Bandgap");
  plcol0 (3);
  plline (eff_bg_data->length, eff_bg_data->bandgap, eff_bg_data->efficiency);
  plend ();
}

static void
draw_ff_function (GtkDrawingArea *area,
                  cairo_t        *cr,
                  int             width,
                  int             height,
                  gpointer        data)
{
  struct eff_bg *eff_bg_data = (struct eff_bg *)data;
  double xmin, xmax, ymin, ymax;
  gsl_stats_minmax (&xmin, &xmax, eff_bg_data->bandgap, 1, eff_bg_data->length);
  gsl_stats_minmax (&ymin, &ymax, eff_bg_data->fill_factor, 1, eff_bg_data->length);

  plsdev ("extcairo");
  plinit ();
  pl_cmd (PLESC_DEVINIT, cr);
  plenv (xmin, xmax, 0, ymax, 0, 0);
  pllab ("Bandgap energy (eV)", "Ideal fill factor", "Ideal fill factor vs. Bandgap");
  plcol0 (3);
  plline (eff_bg_data->length, eff_bg_data->bandgap, eff_bg_data->fill_factor);
  plend ();
}

void
gnome_semilab_workspace_activate (GnomeSemilabWorkspace *workspace)
{
  g_return_if_fail (GNOME_SEMILAB_IS_WORKSPACE (workspace));

  gtk_window_present_with_time (GTK_WINDOW (workspace), g_get_monotonic_time () / 1000L);
}

GnomeSemilabWorkspace *
gnome_semilab_workspace_new (AdwApplication *app)
{
  return g_object_new (GNOME_SEMILAB_TYPE_WORKSPACE,
                       "application", app,
                       NULL);
}

static void
gnome_semilab_workspace_open_response_cb (GnomeSemilabWorkspace *self,
                                          gint                   response,
                                          GtkFileChooserNative  *chooser)
{
  g_assert (GNOME_SEMILAB_IS_WORKSPACE (self));
  g_assert (GTK_IS_FILE_CHOOSER_NATIVE (chooser));

  if (response == GTK_RESPONSE_ACCEPT)
    {
      g_autoptr(GFile) file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (chooser));
      self->table = file;
    }

  gtk_native_dialog_destroy (GTK_NATIVE_DIALOG (chooser));
}

// GtkFileChooser is deprecated since 4.10
// GtkFileDialog since 4.10
#if GTK_CHECK_VERSION (4, 10, 0)
static void
gnome_semilab_workspace_open_action (GtkWidget   *instance,
                                     const gchar *action_name,
                                     GVariant    *param)
{
  g_autoptr(GtkFileDialog) file_dialog = NULL;
  GnomeSemilabWorkspace *self = instance;

  g_assert (param == NULL);
  file_dialog = gtk_file_dialog_new ();
  gtk_file_dialog_set_initial_file (file_dialog, self->file);
}
#else
static void
gnome_semilab_workspace_open_action (GtkWidget   *instance,
                                     const gchar *action_name,
                                     GVariant    *param)
{
  GtkFileChooserNative *chooser;
  GnomeSemilabWorkspace *workspace = (GnomeSemilabWorkspace *)instance;

  g_assert (param == NULL);
  chooser = gtk_file_chooser_native_new (_("Import Spectra…"),
                                         GTK_WINDOW (workspace),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         _("_Open"),
                                         _("_Cancel"));
  gtk_native_dialog_set_modal (GTK_NATIVE_DIALOG (chooser), false);
  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (chooser), false);

  g_signal_connect_object (chooser, "response", G_CALLBACK (gnome_semilab_workspace_open_response_cb), workspace, G_CONNECT_SWAPPED);

  gtk_native_dialog_show (GTK_NATIVE_DIALOG (chooser));
}

#endif

static void
gnome_semilab_workspace_sim_action (GtkWidget   *widget,
                                    const gchar *action_name,
                                    GVariant    *param)
{
  GnomeSemilabWorkspace *self = (GnomeSemilabWorkspace *)widget;
  open_file_as_spectrum (self);
  g_assert (self->spectrum != NULL);

  /* FILE *fp = fopen ("/home/ayka-tsuzuki/gnome-semilab/test/Tungsten-Halogen_PCE.tsv", "r");
   * self->eff_bg_data.length = 200;
   * csv_sketch_read (fp, &self->eff_bg_data, ' ');
   * fclose (fp);  */

  self->eff_bg_data = sqlimit_main (self->spectrum);

  gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (self->eff_bg_plot), draw_eff_bg_function, &self->eff_bg_data, NULL);
}

static void
show_image (GtkWidget *widget)
{
  GnomeSemilabWorkspace *self = (GnomeSemilabWorkspace *)widget;
  GtkWidget *image;
  GdkPixbuf *pixbuf;
  pixbuf = gdk_pixbuf_new_from_file ("/home/ayka-tsuzuki/gnome-semilab/test/output.png", NULL);
  g_assert (pixbuf != NULL);
  image = gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (pixbuf);
  gtk_image_set_pixel_size (GTK_IMAGE (image), 400);
  gtk_box_append (self->ws_main_box, image);
}

static void
gnome_semilab_workspace_plot_spec_action (GtkWidget   *widget,
                                          const gchar *action_name,
                                          GVariant    *param)
{
  GnomeSemilabWorkspace *self = (GnomeSemilabWorkspace *)widget;
  open_file_as_spectrum (self);
  gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (self->spectrum_plot), draw_spec_function, self->spectrum, NULL);
}

static void
gnome_semilab_workspace_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  GnomeSemilabWorkspace *self = GNOME_SEMILAB_WORKSPACE (object);

  switch (prop_id)
    {
    case PROP_WS_TYPE:
      g_value_set_string (value, gnome_semilab_workspace_get_ws_type (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gnome_semilab_workspace_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  GnomeSemilabWorkspace *self = GNOME_SEMILAB_WORKSPACE (object);

  switch (prop_id)
    {
    case PROP_WS_TYPE:
      gnome_semilab_workspace_set_ws_type (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gnome_semilab_workspace_dispose (GObject *object)
{
  GnomeSemilabWorkspace *self = (GnomeSemilabWorkspace *)object;

  g_clear_pointer (&self->ws_type, g_free);
  free (self->spectrum->wavelengths);
  free (self->spectrum->intensities);
  free (self->spectrum);
  free (self->eff_bg_data.bandgap);
  free (self->eff_bg_data.efficiency);
  if (self->eff_bg_data.fill_factor)
    free (self->eff_bg_data.fill_factor);

  G_OBJECT_CLASS (gnome_semilab_workspace_parent_class)->dispose (object);
}

static void
gnome_semilab_workspace_class_init (GnomeSemilabWorkspaceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = gnome_semilab_workspace_dispose;
  object_class->get_property = gnome_semilab_workspace_get_property;
  object_class->set_property = gnome_semilab_workspace_set_property;

  properties[PROP_WS_TYPE] = g_param_spec_string ("ws-type", "Workspace Type", "Workspace Type", "sqlimit", (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/github/yihuajack/GnomeSemiLab/gnome-semilab-workspace.ui");
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWorkspace, ws_main_box);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWorkspace, menu_button);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWorkspace, win_menu);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWorkspace, spectrum_plot);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWorkspace, eff_bg_plot);

  gtk_widget_class_install_action (widget_class, "ws.import", NULL, gnome_semilab_workspace_open_action);
  gtk_widget_class_install_action (widget_class, "ws.plot-spec", NULL, gnome_semilab_workspace_plot_spec_action);
  gtk_widget_class_install_action (widget_class, "ws.start-sim", NULL, gnome_semilab_workspace_sim_action);

  /* GtkBuilder *builder = gtk_builder_new_from_resource ("/com/github/yihuajack/GnomeSemiLab/gtk/workspace-menus.ui");
   * GMenuModel *menu = G_MENU_MODEL (gtk_builder_get_object (builder, "workspace-menu"));
   * gtk_popover_menu_set_menu_model (GTK_POPOVER_MENU (win_menu), menu);
   * gtk_menu_button_set_primary (GTK_MENU_BUTTON (menu_button), true);
   * g_object_unref (builder); */
}

static void
gnome_semilab_workspace_init (GnomeSemilabWorkspace *self)
{
  self->ws_type = g_strdup ("sqlimit");

  gtk_widget_init_template (GTK_WIDGET (self));
}

gchar *
gnome_semilab_workspace_get_ws_type (GnomeSemilabWorkspace *self)
{
  g_return_val_if_fail (GNOME_SEMILAB_IS_WORKSPACE (self), NULL);

  return self->ws_type;
}

void
gnome_semilab_workspace_set_ws_type (GnomeSemilabWorkspace *self,
                                     const gchar           *ws_type)
{
  g_return_if_fail (GNOME_SEMILAB_IS_WORKSPACE (self));

  if (ws_type != self->ws_type)
    {
      g_strlcpy (self->ws_type, ws_type, sizeof(self->ws_type));
    }
}

