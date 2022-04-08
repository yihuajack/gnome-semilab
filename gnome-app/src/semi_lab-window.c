/* semi_lab-window.c
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

#include "semi_lab-config.h"
#include "semi_lab-window.h"

struct _SemiLabWindow
{
  GtkApplicationWindow  parent_instance;

  /* Template widgets */
  GtkHeaderBar        *header_bar;
  GtkLabel            *label_bg;
  GtkEntry            *entry_bg;
  GtkLabel            *label_wl;
  GtkEntry            *entry_wl;
  GtkButton           *plot_button;
};

G_DEFINE_TYPE (SemiLabWindow, semi_lab_window, GTK_TYPE_APPLICATION_WINDOW)

static void
semi_lab_window_class_init (SemiLabWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/github/yihuajack/SemiLab/semi_lab-window.ui");
  gtk_widget_class_bind_template_child (widget_class, SemiLabWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, SemiLabWindow, label_bg);
  gtk_widget_class_bind_template_child (widget_class, SemiLabWindow, entry_bg);
  gtk_widget_class_bind_template_child (widget_class, SemiLabWindow, label_wl);
  gtk_widget_class_bind_template_child (widget_class, SemiLabWindow, entry_wl);
  gtk_widget_class_bind_template_child (widget_class, SemiLabWindow, plot_button);
}

static void
semi_lab_window__plot_dialog (GAction *action G_GNUC_UNUSED, GVariant *parameter G_GNUC_UNUSED, SemiLabWindow *self)
{
  GtkWidget *image = gtk_image_new_from_file ("myfile.png");
}

static void
semi_lab_window_init (SemiLabWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  g_autoptr (GSimpleAction) plot_action = g_simple_action_new ("plot", NULL);
  g_signal_connect (plot_action, "activate", G_CALLBACK (semi_lab_window__plot_dialog), self);
  g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (plot_action));
}
