/* utils.c
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

#include "utils.h"

#if defined(__MINGW32__) || defined(_MSC_VER)
wchar_t *
sl_multibyte_to_wchar_str (const char* in_mb_str)
{
  int length = MultiByteToWideChar(CP_UTF8, 0, in_mb_str, -1, NULL, 0);

  wchar_t* out_wc_str = (wchar_t *)malloc (length * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, in_mb_str, -1, out_wc_str, length);

  return out_wc_str;
}

wchar_t *
longabspath (wchar_t* unicodePath)
{
  DWORD retval;
  wchar_t unicodeAbsPath[BUFSIZE];
  wchar_t unicodeLongAbsPath[BUFSIZE];
  wchar_t* path;

  retval = GetFullPathNameW(unicodePath, BUFSIZE, unicodeAbsPath, NULL);
  if (retval == 0)
    {
      printf("GetFullPathName failed for %ls with error code %d\n", unicodePath, GetLastError());
      return NULL;
    }
  if (wcslen(unicodeAbsPath) >= MAX_PATH)
    {
      printf("Warning: Maximum path length limitation reached while opening\n"
             "\t%ls\n"
             "Consider changing the working directory, "
             "using shorter names or to enable longer paths in Windows.\n"
             "See https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation for more information.\n", unicodeAbsPath);

      const wchar_t longPathPrefix[] = L"\\\\\?\\";
      size_t longPathPrefix_size =  wcslen(longPathPrefix);
      wcsncpy(unicodeLongAbsPath, longPathPrefix, longPathPrefix_size);
      wcsncpy(unicodeLongAbsPath+longPathPrefix_size, unicodeAbsPath, BUFSIZE - longPathPrefix_size -1);
      path = _wcsdup(unicodeLongAbsPath);
    }
  else
    {
      path = _wcsdup(unicodeAbsPath);
    }

  return path;
}
#endif

FILE *
sl_fopen (const char *filename,
          const char *mode)
{
#if defined(__MINGW32__) || defined(_MSC_VER)
  wchar_t* unicodeFilename = omc_multibyte_to_wchar_str(filename);
  wchar_t* unicodeMode = omc_multibyte_to_wchar_str(mode);

  wchar_t* unicodeLongFileName = longabspath(unicodeFilename);
  FILE *f = _wfopen(unicodeLongFileName, unicodeMode);

  free(unicodeLongFileName);
  free(unicodeFilename);
  free(unicodeMode);
#else /* unix */
  FILE *f = fopen(filename, mode);
#endif
  if (f == NULL || ferror(f)) {
    return NULL;
  }
  return f;
}

size_t
sl_fread (void  *buffer,
          size_t size,
          size_t count,
          FILE  *stream,
          bool   allow_early_eof)
{
  size_t read_len = fread (buffer, size, count, stream);
  if (read_len != count)
    {
      if (feof (stream) && !allow_early_eof)
        {
          fprintf (stderr, "Error reading stream: unexpected end of file.\n");
          fprintf (stderr, "Expected to read %ld. Read only %ld\n", count, read_len);
        }
      else if (ferror (stream))
        {
          fprintf (stderr, "Error: sl_fread() failed to read file.\n");
        }
    }
  return read_len;
}

void
matrix_transpose(double *m,
                 int     w,
                 int     h)
{
  int start;
  double tmp;

  for (start = 0; start <= w * h - 1; start++)
    {
      int next = start;
      int i = 0;
      do
        {
          i++;
          next = (next % h) * w + next / h;
        } while (next > start);
      if (next < start || i == 1)
        continue;

      tmp = m[next = start];
      do
        {
          i = (next % h) * w + next / h;
          m[next] = (i == start) ? tmp : m[i];
          next = i;
        } while (next > start);
    }
}

