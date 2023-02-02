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
#include <glib/gi18n.h>

struct _GnomeSemilabWindow
{
  AdwApplicationWindow  parent_instance;

  /* Template widgets */
  GtkButton           *back_button;
  GtkButton           *new_proj_button;
  GtkStackSidebar     *stack_sidebar;
  GtkStack            *pages;
  AdwWindowTitle      *title;
};

G_DEFINE_FINAL_TYPE (GnomeSemilabWindow, gnome_semilab_window, ADW_TYPE_APPLICATION_WINDOW)

static void
stack_notify_visible_child_cb (GnomeSemilabWindow *self,
                               GParamSpec         *pspec,
                               GtkStack           *stack)
{
  g_autofree gchar *title = NULL;
  g_autofree gchar *full_title = NULL;
  GtkStackPage *page;
  GtkWidget *visible_child;
  gboolean overview;

  g_assert (GNOME_SEMILAB_IS_WINDOW (self));
  g_assert (GTK_IS_STACK (stack));

  visible_child = gtk_stack_get_visible_child (stack);
  page = gtk_stack_get_page (stack, visible_child);

  if (page != NULL)
    {
      if ((title = g_strdup (gtk_stack_page_get_title (page))))
        full_title = g_strdup_printf (_("SemiLab — %s"), title);
    }

  adw_window_title_set_title (self->title, title);
  gtk_window_set_title (GTK_WINDOW (self), title);

  overview = !g_strcmp0 ("overview", gtk_stack_get_visible_child_name (stack));

  gtk_widget_set_visible (GTK_WIDGET (self->back_button), !overview);
}

GtkWidget *
gnome_semilab_window_get_page (GnomeSemilabWindow *self)
{
  g_return_val_if_fail (GNOME_SEMILAB_IS_WINDOW (self), NULL);

  return gtk_stack_get_visible_child (self->pages);
}

void
gnome_semilab_window_set_page (GnomeSemilabWindow *self,
                               GtkWidget          *page)
{
  g_return_if_fail (GNOME_SEMILAB_IS_WINDOW (self));
  g_return_if_fail (!page || GTK_IS_WIDGET (page));

  if (page != NULL)
    gtk_stack_set_visible_child (self->pages, page);
  else
    gtk_stack_set_visible_child_name (self->pages, "overview");
}

const gchar *
gnome_semilab_window_get_page_name (GnomeSemilabWindow *self)
{
  g_return_val_if_fail (GNOME_SEMILAB_IS_WINDOW (self), NULL);

  return gtk_stack_get_visible_child_name (self->pages);
}

void
gnome_semilab_window_set_page_name (GnomeSemilabWindow *self,
                                    const gchar        *name)
{
  g_return_if_fail (GNOME_SEMILAB_IS_WINDOW (self));

  if (name == NULL)
    name = "overview";

  gtk_stack_set_visible_child_name (self->pages, name);
  gtk_widget_set_visible (GTK_WIDGET (self->back_button), g_strcmp0 (name, "overview"));
}

static void
gnome_semilab_window_page_action (GtkWidget   *widget,
                                  const gchar *action_name,
                                  GVariant    *param)
{
  GnomeSemilabWindow *self = (GnomeSemilabWindow *)widget;

  g_assert (GNOME_SEMILAB_IS_WINDOW (self));
  g_assert (g_variant_is_of_type (param, G_VARIANT_TYPE_STRING));

  gnome_semilab_window_set_page_name (self, g_variant_get_string (param, NULL));
}

void
gnome_semilab_window_add_page (GnomeSemilabWindow *self,
                               GtkWidget          *page,
                               const char         *name,
                               const char         *title)
{
  GtkStackPage *child;

  g_return_if_fail (GNOME_SEMILAB_IS_WINDOW (self));
  g_return_if_fail (GTK_IS_WIDGET (page));

  child = gtk_stack_add_named (self->pages, page, name);
  gtk_stack_page_set_title (child, title);
}

static void
gnome_semilab_window_class_init (GnomeSemilabWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/github/yihuajack/GnomeSemiLab/gnome-semilab-window.ui");
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWindow, back_button);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWindow, stack_sidebar);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWindow, pages);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWindow, new_proj_button);
  gtk_widget_class_bind_template_callback (widget_class, stack_notify_visible_child_cb);

  gtk_widget_class_install_action (widget_class, "greeter.page", "s", gnome_semilab_window_page_action);

  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_W, GDK_CONTROL_MASK, "window.close", NULL);
}

static void
gnome_semilab_window_init (GnomeSemilabWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  gtk_stack_sidebar_set_stack (self->stack_sidebar, self->pages);
  stack_notify_visible_child_cb (self, NULL, self->pages);
  g_signal_connect (self->new_proj_button, "clicked", G_CALLBACK (gnome_semilab_window_add_page), NULL);
}

