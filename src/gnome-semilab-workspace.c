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
#include "gnome-semilab-application.h"

struct _GnomeSemilabworkspace
{
  AdwApplicationWindow  parent_instance;

  gchar                *ws_type;
}

G_DEFINE_FINAL_TYPE (GnomeSemilabWorkspace, gnome_semilab_workspace, ADW_TYPE_APPLICATION_WINDOW)

gchar *
gnome_semilab_workspace_get_ws_type (GnomeSemilabWorkspace *self)
{
  g_return_val_if_fail (GNOME_SEMILAB_IS_WORKSPACE (self), NULL);

  return self->ws_type;
}

void
gnome_semilab_workspace_activate (GnomeSemilabWorkspace *workspace)
{
  g_return_if_fail (GNOME_SEMILAB_IS_WORKSPACE (workspace));

  gtk_window_present_with_time (GTK_WINDOW (workspace), g_get_monotonic_time () / 1000L);
}

GnomeSemilabWorkspace *
gnome_semilab_workspace_new (GnomeSemilabApplication *app)
{
  return g_object_new (GNOME_SEMILAB_TYPE_WORKSPACE,
                       "application", app);
}

static void
gnome_semilab_workspace_class_init (GnomeSemilabWorkspace *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/github/yihuajack/GnomeSemilab/gnome-semilab-workspace.ui");
}

static void
gnome_semilab_workspace_init (GnomeSemilabWorkspace *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

