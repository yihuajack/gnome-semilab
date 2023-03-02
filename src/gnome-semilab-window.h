/* gnome-semilab-window.h
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

#pragma once

#include <adwaita.h>"

G_BEGIN_DECLS

#define GNOME_SEMILAB_TYPE_WINDOW (gnome_semilab_window_get_type())

G_DECLARE_FINAL_TYPE (GnomeSemilabWindow, gnome_semilab_window, GNOME_SEMILAB, WINDOW, AdwApplicationWindow)

extern
GtkWidget   *gnome_semilab_window_get_page      (GnomeSemilabWindow *self);

extern
void         gnome_semilab_window_set_page      (GnomeSemilabWindow *self,
                                                 GtkWidget          *page);

extern
const gchar *gnome_semilab_window_get_page_name (GnomeSemilabWindow *self);

extern
void         gnome_semilab_window_set_page_name (GnomeSemilabWindow *self,
                                                 const gchar        *name);

extern
void         gnome_semilab_window_add_page      (GnomeSemilabWindow *self,
                                                 GtkWidget          *widget,
                                                 const gchar        *name,
                                                 const gchar        *title);

extern
void         gnome_semilab_window_remove_page   (GnomeSemilabWindow *self,
                                                 GtkWidget          *widget);

G_END_DECLS


