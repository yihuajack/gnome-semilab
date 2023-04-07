/* gnome-semilab-workspace-private.h
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

#include "gnome-semilab-workspace.h"
#include "csv_reader.h"

G_BEGIN_DECLS

struct _GnomeSemilabWorkspace
{
  AdwApplicationWindow  parent_instance;

  gchar                *ws_type;
  GFile                *table;
  struct csv_data      *spectrum;

  GtkBox               *ws_main_box;
  GtkMenuButton        *menu_button;
  GtkPopoverMenu       *win_menu;
  GtkDrawingArea       *spectrum_plot;
};

G_END_DECLS

