/* utils.h
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

#include <stdio.h>
#include <stdbool.h>

#if defined(__MINGW32__) || defined(_MSC_VER)
extern
wchar_t *sl_multibyte_to_wchar_str (const char *in_mb_str)

extern
wchar_t *longabspath               (wchar_t*    unicodePath);
#endif

extern
FILE    *sl_fopen                  (const char *filename,
                                    const char *mode);

extern
size_t   sl_fread                  (void  *buffer,
                                    size_t size,
                                    size_t count,
                                    FILE  *stream,
                                    bool   allow_early_eof);

extern
void     matrix_transpose          (double *m,
                                    int     w,
                                    int     h);

