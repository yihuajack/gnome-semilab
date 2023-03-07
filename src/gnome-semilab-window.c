/* gnome-semilab-window.c
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

#include "config.h"

#include "gnome-semilab-global.h"
#include "gnome-semilab-window.h"
#include "gsp-create-project-widget.h"
#include "gnome-semilab-resources.h"
#include <glib/gi18n.h>

struct _GnomeSemilabWindow
{
  AdwApplicationWindow    parent_instance;

  /* Template widgets */
  GtkButton              *back_button;
  GtkButton              *new_proj_button;
  GtkStackSidebar        *stack_sidebar;
  GtkStack               *pages;
  AdwWindowTitle         *title;
  GspCreateProjectWidget *create_project_page;
  AdwFlap                *content_flap;
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
                               const gchar        *name,
                               const gchar        *title)
{
  GtkStackPage *child;

  g_return_if_fail (GNOME_SEMILAB_IS_WINDOW (self));
  g_return_if_fail (GTK_IS_WIDGET (page));

  child = gtk_stack_add_named (self->pages, page, name);
  gtk_stack_page_set_title (child, title);
}

void
gnome_semilab_window_remove_page (GnomeSemilabWindow *self,
                                  GtkWidget          *page)
{
  g_return_if_fail (GNOME_SEMILAB_IS_WINDOW (self));
  g_return_if_fail (GTK_IS_WIDGET (page));

  gtk_stack_remove (self->pages, page);
}

void
gnome_semilab_window_open_project (GnomeSemilabWindow *self,
                                   const gchar        *ws_type)
{
  GnomeSemilabWorkspace *workspace;

  g_return_if_fail (GNOME_SEMILAB_IS_WORKSPACE (self));

  if ((workspace = gnome_semilab_application_find_project (GNOME_SEMILAB_APPLICATION_DEFAULT, ws_type)))
    {
      gnome_semilab_workspace_activate (workspace);
      gtk_window_destroy (GTK_WINDOW (self));
    }
}

static void
gnome_semilab_window_constructed (GObject *object)
{
  GnomeSemilabWindow *self = (GnomeSemilabWindow *)object;

  G_OBJECT_CLASS (gnome_semilab_window_parent_class)->constructed (object);

  /* To manually set the action target of new_proj_button:
   * gtk_actionable_set_action_target (GTK_ACTIONABLE (self->new_proj_button), "s", "new-simulation");
   * To manually add page, assertion 'GTK_IS_WIDGET (page)' failed */
  self->create_project_page = g_object_new (GSP_TYPE_CREATE_PROJECT_WIDGET, NULL);
  gnome_semilab_window_add_page (GNOME_SEMILAB_WINDOW (self), GTK_WIDGET (self->create_project_page), "new-simulation", _("New Simulation"));
}

static void
gnome_semilab_window_dispose (GObject *object)
{
  /* instance of invalid non-instantiatable type '(null)'
   * g_signal_handlers_disconnect_matched: assertion 'G_TYPE_CHECK_INSTANCE (instance)' failed
   * GnomeSemilabWindow *self = (GnomeSemilabWindow *)object;
   * gnome_semilab_window_remove_page (GNOME_SEMILAB_WINDOW (self), GTK_WIDGET (self->create_project_page));
   * self->create_project_page = NULL;
   * https://mail.gnome.org/archives/gtk-app-devel-list/2007-May/msg00163.html
   * */

  // Template widgets do not need g_clear_object()

  // Finalizing GnomeSemilabWindow, but it still has children left: AdwGizmo
  G_OBJECT_CLASS (gnome_semilab_window_parent_class)->dispose (object);
}

static void
gnome_semilab_window_class_init (GnomeSemilabWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = gnome_semilab_window_constructed;
  object_class->dispose = gnome_semilab_window_dispose;

  gtk_widget_class_set_template_from_resource (widget_class, "/com/github/yihuajack/GnomeSemiLab/gnome-semilab-window.ui");
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWindow, back_button);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWindow, new_proj_button);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWindow, stack_sidebar);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWindow, pages);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWindow, title);
  gtk_widget_class_bind_template_child (widget_class, GnomeSemilabWindow, content_flap);
  gtk_widget_class_bind_template_callback (widget_class, stack_notify_visible_child_cb);

  gtk_widget_class_install_action (widget_class, "greeter.page", "s", gnome_semilab_window_page_action);

  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_W, GDK_CONTROL_MASK, "window.close", NULL);

  g_type_ensure (GSP_TYPE_CREATE_PROJECT_WIDGET);

  g_resources_register (gnome_semilab_get_resource ());
}

static void
gnome_semilab_window_init (GnomeSemilabWindow *self)
{
  gsp_create_project_widget_get_type ();
  gtk_widget_init_template (GTK_WIDGET (self));
  gtk_stack_sidebar_set_stack (self->stack_sidebar, self->pages);
  stack_notify_visible_child_cb (self, NULL, self->pages);
}


