/* gsp-create-project-widget.c
 *
 * Copyright 2022-2023 Yihua Liu <yihuajack@live.cn>
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

#define G_LOG_DOMAIN "gsp-create-project-widget"

#include "config.h"

#include "gnome-semilab-global.h"
#include "gnome-semilab-window.h"
#include "gsp-create-project-widget.h"

struct _GspCreateProjectWidget
{
  GtkWidget            parent_instance;

  /* Template widgets */
  GtkWidget           *main_page;
  GtkButton           *expand_button;
  AdwPreferencesRow   *simulation_row;
};

G_DEFINE_FINAL_TYPE (GspCreateProjectWidget, gsp_create_project_widget, GTK_TYPE_WIDGET)

static void
expand_action (GtkWidget   *widget,
               const gchar *action_name,
               GVariant    *param)
{
  GspCreateProjectWidget *self = (GspCreateProjectWidget *)widget;
  GnomeSemilabWindow *greeter;

  g_assert (GSP_IS_CREATE_PROJECT_WIDGET (self));

  greeter = GNOME_SEMILAB_WINDOW (gnome_semilab_widget_get_window (widget));
  gtk_widget_action_set_enabled (widget, "create-project.expand", FALSE);

  gnome_semilab_window_open_project (greeter, "sqlimit");
}

static void
gsp_create_project_widget_dispose (GObject *object)
{
  GspCreateProjectWidget *self = (GspCreateProjectWidget *)object;

  // AdwPreferencesPage *main passing argument 1 of ‘gtk_widget_unparent’ from incompatible pointer type
  g_clear_pointer (&self->main_page, gtk_widget_unparent);

  G_OBJECT_CLASS (gsp_create_project_widget_parent_class)->dispose (object);
}

static void
gsp_create_project_widget_class_init (GspCreateProjectWidgetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = gsp_create_project_widget_dispose;

  gtk_widget_class_set_template_from_resource (widget_class, "/com/github/yihuajack/GnomeSemiLab/gsp-create-project-widget.ui");
  gtk_widget_class_bind_template_child (widget_class, GspCreateProjectWidget, main_page);
  gtk_widget_class_bind_template_child (widget_class, GspCreateProjectWidget, expand_button);
  gtk_widget_class_bind_template_child (widget_class, GspCreateProjectWidget, simulation_row);

  gtk_widget_class_install_action (widget_class, "create-project.expand", NULL, expand_action);

  // Trying to snapshot AdwPreferencesPage without a current allocation
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
}

static void
gsp_create_project_widget_init (GspCreateProjectWidget *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

