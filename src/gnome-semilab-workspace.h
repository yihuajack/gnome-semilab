/* gnome-semilab-workspace.c
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

#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define GNOME_SEMILAB_TYPE_WORKSPACE (gnome_semilab_workspace_get_type())

G_DECLARE_FINAL_TYPE (GnomeSemilabWorkspace, gnome_semilab_workspace, GNOME_SEMILAB, WORKSPACE, AdwApplicationWindow)

extern
gchar                 *gnome_semilab_workspace_get_ws_type (GnomeSemilabWorkspace   *self);

extern
void                   gnome_semilab_workspace_activate    (GnomeSemilabWorkspace   *workspace);

extern
GnomeSemilabWorkspace *gnome_semilab_workspace_new         (GnomeSemilabApplication *app);

G_END_DECLS

