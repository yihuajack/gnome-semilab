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
  GtkWindowGroup   parent_instance;
}

G_DEFINE_FINAL_TYPE (GnomeSemilabWorkspace, gnome_semilab_workspace, ADW_TYPE_APPLICATION_WINDOW)

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

