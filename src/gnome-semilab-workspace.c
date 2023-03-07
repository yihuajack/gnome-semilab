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

#include "gnome-semilab-workspace.h"
#include "gnome-semilab-workspace-private.h"

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
                       "application", app);
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

  properties[PROP_WS_TYPE] = g_param_spec_string ("ws-type", "Workspace Type", "Workspace Type", "sqlimit", (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/github/yihuajack/GnomeSemilab/gnome-semilab-workspace.ui");
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

