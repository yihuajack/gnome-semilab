/* gnome-semilab-window.c
 *
 * Copyright 2022 Yihua Liu
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

#include "config.h"

#include "gnome-semilab-window.h"

struct _GnomeSemilabWindow
{
  AdwApplicationWindow  parent_instance;

  /* Template widgets */
  GtkButton           *back_button;
  GtkStackSidebar     *stack_sidebar;
  GtkStack            *pages;
};

G_DEFINE_FINAL_TYPE (GnomeSemilabWindow, gnome_semilab_window, ADW_TYPE_APPLICATION_WINDOW)

static void
gnome_semilab_window_set_page_name (GnomeSemilabWindow *self,
                                    const gchar        *name)
{
  g_return_if_fail (GNOME_IS_SEMILAB_WINDOW (self));

  if (name == NULL)
    name = "overview";

  gtk_stack_set_visible_child_name (self->pages, name);
  gtk_widget_set_visible (GTK_WIDGET (self->back_button),
                          g_strcmp0 (name, "overview"));
}

static void
greeter_page_action (GtkWidget   *widget,
                     const gchar *action_name,
                     GVariant    *param)
{
  GnomeSemilabWindow *self = (GnomeSemilabWindow *)widget;

  g_assert (GNOME_IS_SEMILAB_WINDOW (self));
  g_assert (g_variant_is_of_type (param, G_VARIANT_TYPE_STRING));

  gnome_semilab_window_set_page_name (self,
                                      g_variant_get_string (param, NULL));
}

static void
gnome_semilab_window_class_init (GnomeSemilabWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/github/yihuajack/GnomeSemiLab/gnome-semilab-window.ui");
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWindow, back_button);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWindow, stack_sidebar);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWindow, pages);
  gtk_widget_class_install_action (widget_class, "greeter.page", "s", greeter_page_action);
  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_W, GDK_CONTROL_MASK, "window.close", NULL);
}

static void
gnome_semilab_window_init (GnomeSemilabWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  gtk_stack_sidebar_set_stack (self->stack_sidebar, self->pages);
}

