/* gnome-semilab-application.h
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

#include <adwaita.h>
#include "gnome-semilab-workspace.h"

G_BEGIN_DECLS

#define GNOME_SEMILAB_TYPE_APPLICATION (gnome_semilab_application_get_type())
#define GNOME_SEMILAB_APPLICATION_DEFAULT GNOME_SEMILAB_APPLICATION(g_application_get_default())

G_DECLARE_FINAL_TYPE (GnomeSemilabApplication, gnome_semilab_application, GNOME_SEMILAB, APPLICATION, AdwApplication)

extern
void                     gnome_semilab_application_add_workspace     (GnomeSemilabApplication *self,
                                                                      GnomeSemilabWorkspace   *workspace);

extern
void                     gnome_semilab_application_remove_workspace  (GnomeSemilabApplication *self,
                                                                      GnomeSemilabWorkspace   *workspace);

extern
void                     gnome_semilab_application_foreach_workspace (GnomeSemilabApplication *self,
                                                                      GFunc                    callback,
                                                                      gpointer                 user_data);
extern
GnomeSemilabWorkspace   *gnome_semilab_application_find_project      (GnomeSemilabApplication *self,
                                                                      const gchar             *ws_type);

extern
void                     gnome_semilab_application_create_project    (GnomeSemilabApplication *self);

GnomeSemilabApplication *gnome_semilab_application_new               (const char              *application_id,
                                                                      GApplicationFlags        flags);

G_END_DECLS

