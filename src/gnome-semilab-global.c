/* gnome-semilab-global.h
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

#define G_LOG_DOMAIN "gnome-semilab-global"

#include "gnome-semilab-global.h"

GnomeSemilabWorkspace *
gnome_semilab_widget_get_workspace (GtkWidget *widget)
{
  GtkWindow *transient_for;
  GtkRoot *root;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

  if (GTK_IS_ROOT (widget))
    root = GTK_ROOT (widget);
  else
    root = gtk_widget_get_root (widget);

  transient_for = gtk_window_get_transient_for (GTK_WINDOW (root));
  if (root && !GNOME_SEMILAB_IS_WORKSPACE (root) && GTK_IS_WINDOW (root) && transient_for)
    return gnome_semilab_widget_get_workspace (GTK_WIDGET (transient_for));

  return GNOME_SEMILAB_IS_WORKSPACE (root) ? GNOME_SEMILAB_WORKSPACE (root) : NULL;
}


