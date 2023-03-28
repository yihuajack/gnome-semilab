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

#include <glib/gi18n.h>

#include "gnome-semilab-workspace.h"
#include "gnome-semilab-workspace-private.h"
#include "sqlimit.h"

G_DEFINE_FINAL_TYPE (GnomeSemilabWorkspace, gnome_semilab_workspace, ADW_TYPE_APPLICATION_WINDOW)

enum {
  PROP_0,
  PROP_WS_TYPE,
  N_PROPS
};

static GParamSpec *properties[N_PROPS];

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

  gtk_native_dialog_hide (GTK_NATIVE_DIALOG (chooser));
}

#endif

static void
gnome_semilab_workspace_sim_action (GtkWidget   *widget,
                                    const gchar *action_name,
                                    GVariant   *param)
{
  GnomeSemilabWorkspace *self = (GnomeSemilabWorkspace *)widget;
  const char *path = g_file_get_path (self->table);
  if (path)
    {
      FILE *fp = fopen (path, "r");
      if (fp)
        {
          sqlimit_main(fp);
          fclose (fp);
        }
      else
        {
          g_warning ("Failed to open file %s", path);
        }
      g_free((gpointer) path);
    }
  else
    {
      g_warning ("Failed to get file path");
    }
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
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWorkspace, menu_button);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWorkspace, win_menu);

  gtk_widget_class_install_action (widget_class, "ws.import", NULL, gnome_semilab_workspace_open_action);
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

