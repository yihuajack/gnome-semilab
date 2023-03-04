/* gnome-semilab-application.c
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

#include "gnome-semilab-application.h"
#include "gnome-semilab-window.h"
#include "gnome-semilab-workspace.h"

struct _GnomeSemilabApplication
{
  AdwApplication parent_instance;
};

G_DEFINE_TYPE (GnomeSemilabApplication, gnome_semilab_application, ADW_TYPE_APPLICATION)

GnomeSemilabApplication *
gnome_semilab_application_new (const char        *application_id,
                               GApplicationFlags  flags)
{
  g_return_val_if_fail (application_id != NULL, NULL);

  return g_object_new (GNOME_SEMILAB_TYPE_APPLICATION,
                       "application-id", application_id,
                       "flags", flags,
                       NULL);
}

static void
gnome_semilab_application_activate (GApplication *app)
{
  GtkWindow *window;

  g_assert (GNOME_SEMILAB_IS_APPLICATION (app));

  window = gtk_application_get_active_window (GTK_APPLICATION (app));
  if (window == NULL)
    window = g_object_new (GNOME_SEMILAB_TYPE_WINDOW,
                           "application", app,
                           NULL);

  gtk_window_present (window);
}

static void
gnome_semilab_application_class_init (GnomeSemilabApplicationClass *klass)
{
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  app_class->activate = gnome_semilab_application_activate;
}

static void
gnome_semilab_application_about_action (GSimpleAction *action,
                                        GVariant      *parameter,
                                        gpointer       user_data)
{
  static const char *developers[] = {"Yihua", NULL};
  GnomeSemilabApplication *self = user_data;
  GtkWindow *window = NULL;

  g_assert (GNOME_SEMILAB_IS_APPLICATION (self));

  window = gtk_application_get_active_window (GTK_APPLICATION (self));

  adw_show_about_window (window,
                         "application-name", "gnome-semilab",
                         "application-icon", "com.github.yihuajack.GnomeSemiLab",
                         "developer-name", "Yihua Liu",
                         "version", "0.1.0",
                         "developers", developers,
                         "copyright", "© 2022-2023 Yihua Liu",
                         NULL);
}

static void
gnome_semilab_application_quit_action (GSimpleAction *action,
                                       GVariant      *parameter,
                                       gpointer       user_data)
{
  GnomeSemilabApplication *self = user_data;

  g_assert (GNOME_SEMILAB_IS_APPLICATION (self));

  g_application_quit (G_APPLICATION (self));
}

void
gnome_semilab_application_load_project (GSimpleAction *action,
                                        GVariant      *parameter,
                                        gpointer       user_data)
{
  GnomeSemilabApplication *self = user_data;
  g_autoptr(GnomeSemilabWorkspace) workspace = NULL;

  g_assert (!action || G_IS_SIMPLE_ACTION (action));
  g_assert (!parameter || g_variant_is_of_type (parameter, G_VARIANT_TYPE_STRING));
  g_assert (GNOME_SEMILAB_IS_APPLICATION (self));

  workspace = gnome_semilab_workspace_new (self);
}

static const GActionEntry app_actions[] = {
  { "quit", gnome_semilab_application_quit_action },
  { "about", gnome_semilab_application_about_action },
  { "load-project", gnome_semilab_application_load_project, "s" },
};

static void
gnome_semilab_application_init (GnomeSemilabApplication *self)
{
  g_action_map_add_action_entries (G_ACTION_MAP (self),
                                   app_actions,
                                   G_N_ELEMENTS (app_actions),
                                   self);
  gtk_application_set_accels_for_action (GTK_APPLICATION (self),
                                         "app.quit",
                                         (const char *[]) { "<primary>q", NULL });
}

