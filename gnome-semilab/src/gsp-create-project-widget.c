/* gsp-create-project-widget.c
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

#define G_LOG_DOMAIN "gsp-create-project-widget"

#include "config.h"

#include "gsp-create-project-widget.h"

struct _GspCreateProjectWidget
{
  GtkWidget     *parent_instance;

  GtkWidget     *main;
}

G_DEFINE_FINAL_TYPE (GspCreateProjectWidget, gsp_create_project_widget, GTK_TYPE_WIDGET)

static void
gsp_create_project_widget_init (GspCreateProjectWidget *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

