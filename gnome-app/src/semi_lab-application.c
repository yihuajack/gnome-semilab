/* semi_lab-application.c
 *
 * Copyright 2022 yihua_liu
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
 */

#include "semi_lab-application.h"
#include "semi_lab-window.h"

struct _SemiLabApplication
{
  GtkApplication parent_instance;
};

G_DEFINE_TYPE (SemiLabApplication, semi_lab_application, GTK_TYPE_APPLICATION)

SemiLabApplication *
semi_lab_application_new (gchar *application_id,
                          GApplicationFlags  flags)
{
  return g_object_new (SEMI_LAB_TYPE_APPLICATION,
                       "application-id", application_id,
                       "flags", flags,
                       NULL);
}

static void
semi_lab_application_finalize (GObject *object)
{
  SemiLabApplication *self = (SemiLabApplication *)object;

  G_OBJECT_CLASS (semi_lab_application_parent_class)->finalize (object);
}

static void
semi_lab_application_activate (GApplication *app)
{
  GtkWindow *window;

  /* It's good practice to check your parameters at the beginning of the
   * function. It helps catch errors early and in development instead of
   * by your users.
   */
  g_assert (GTK_IS_APPLICATION (app));

  /* Get the current window or create one if necessary. */
  window = gtk_application_get_active_window (GTK_APPLICATION (app));
  if (window == NULL)
    window = g_object_new (SEMI_LAB_TYPE_WINDOW,
                           "application", app,
                           "default-width", 600,
                           "default-height", 300,
                           NULL);

  /* Ask the window manager/compositor to present the window. */
  gtk_window_present (window);
}


static void
semi_lab_application_class_init (SemiLabApplicationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  object_class->finalize = semi_lab_application_finalize;

  /*
   * We connect to the activate callback to create a window when the application
   * has been launched. Additionally, this callback notifies us when the user
   * tries to launch a "second instance" of the application. When they try
   * to do that, we'll just present any existing window.
   */
  app_class->activate = semi_lab_application_activate;
}

static void
semi_lab_application_show_about (GSimpleAction *action,
                                 GVariant      *parameter,
                                 gpointer       user_data)
{
  SemiLabApplication *self = SEMI_LAB_APPLICATION (user_data);
  GtkWindow *window = NULL;
  const gchar *authors[] = {"yihua_liu", NULL};

  g_return_if_fail (SEMI_LAB_IS_APPLICATION (self));

  window = gtk_application_get_active_window (GTK_APPLICATION (self));

  gtk_show_about_dialog (window,
                         "program-name", "semi-lab",
                         "authors", authors,
                         "version", "0.1.0",
                         NULL);
}


static void
semi_lab_application_init (SemiLabApplication *self)
{
  GSimpleAction *quit_action = g_simple_action_new ("quit", NULL);
  g_signal_connect_swapped (quit_action, "activate", G_CALLBACK (g_application_quit), self);
  g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (quit_action));

  GSimpleAction *about_action = g_simple_action_new ("about", NULL);
  g_signal_connect (about_action, "activate", G_CALLBACK (semi_lab_application_show_about), self);
  g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (about_action));

  const char *accels[] = {"<primary>q", NULL};
  gtk_application_set_accels_for_action (GTK_APPLICATION (self), "app.quit", accels);

  gtk_application_set_accels_for_action (GTK_APPLICATION (self), "op.plot", (const char *[]) {"<Ctrl>p", NULL});
}
