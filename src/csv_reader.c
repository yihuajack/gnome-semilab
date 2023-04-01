/* csv_reader.c
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

#include <stdlib.h>
#include <string.h>
#include <csv.h>

#include "utils.h"
#include "csv_reader.h"

struct csv_count
{
  unsigned long num_cells;
  unsigned long num_rows;
};

struct csv_head
{
  char **fields;
  int size;  // number of data cells
  int buffer_size;
  unsigned long num_rows;
};

struct csv_body
{
  unsigned int *wavelengths;
  double *intensities;
  int size;
  int buffer_size;
  int num_rows;
  unsigned int curr_num_cols;
  unsigned int num_cols;
  bool error;
  bool buf_reallocated;
};

static void
count_rows_head (int   c,
                 void *t)
{
  struct csv_head *head = (struct csv_head *)t;
  head->num_rows++;
}

static void
count_rows (int   c,
            void *t)
{
  struct csv_count *count = (struct csv_count *)t;
  count->num_rows++;
}

static void
add_fields (void  *data,
            size_t len,
            void  *t)
{
  struct csv_head *head = (struct csv_head *)t;
  if (head->num_rows)
    {
      // Number of rows in the header has been non-zero
      // Should only be entered once column
      return;
    }
  if (head->size + 1 >= head->buffer_size)
    {
      head->buffer_size = head->buffer_size ? 2 * head->buffer_size : 512;
      head->fields = (char **) realloc (head->fields, sizeof (char *)*head->buffer_size);
    }
  head->fields[head->size++] = strdup (data ? (char *)data : "");
}

char **read_csv_fields (FILE *fp,
                        int  *length)
{
  const unsigned int buf_size = 4096;
  char buf[4096];
  struct csv_parser p;
  struct csv_head head = {0};
  size_t bytes_read;
  csv_init (&p, CSV_STRICT | CSV_REPALL_NL | CSV_STRICT_FINI | CSV_APPEND_NULL | CSV_EMPTY_IS_NULL);
  csv_set_realloc_func (&p, realloc);
  csv_set_free_func (&p, free);
  do
    {
      bytes_read = sl_fread (buf, 1, buf_size, fp, true);
      if (bytes_read != buf_size && !feof (fp))
        {
          fprintf (stderr, "fread length is not equal to buffer size and is not feof.\n");
          csv_free (&p);
          return NULL;
        }
      csv_parse (&p, buf, bytes_read, add_fields, count_rows_head, &head);
    } while (!head.num_rows && !feof (fp));
  csv_free (&p);
  if (!head.num_rows)
    {
      fprintf (stderr, "Number of rows in the header is 0.\n");
      return NULL;
    }
  *length = head.size - 1;
  // Head fields should not be double-quoted; otherwise, csv_parse will give wrong results!
  return head.fields;
}

/* add cells */
void
cb1 (void  *s,
     size_t len,
     void  *data)
{
  struct csv_body *body = (struct csv_body *)data;
  char *endptr = "";
  if (body->error)
    return;
  // Skip header rows
  if (!body->num_rows)
    {
      body->curr_num_cols++;
      body->num_cols++;
      return;
    }
  // To simplify, only consider two columns; here, judge which column by size!
  if (body->size % 2 == 0)
    {
      if (body->size + 1 >= body->buffer_size)
        {
          body->buffer_size = body->wavelengths ? 2 * body->buffer_size : body->num_cols * 1024;
          body->buffer_size = body->buffer_size > 0 ? body->buffer_size : 1024;
          body->wavelengths = body->wavelengths ? (unsigned int *)realloc (body->wavelengths, sizeof (unsigned int)*body->buffer_size)
                                                : (unsigned int *)malloc (sizeof (unsigned int)*body->buffer_size);
          body->buf_reallocated = true;
        }
      // Suffix/postfix increment and decrement have precedence 1
      // Multiplication, division, and remainder have precedence 3
      // Note that the actual "data" is the "s" argument; strtoul(data) would be ��UUUU
      body->wavelengths[body->size++ / 2] = s ? strtoul ((const char *)s, &endptr, 10) : 0;
      if (*endptr)
        {
          fprintf (stderr, "Found non-unsigned-int data in the first column of csv result-file: %s\n", (char *)s);
          body->error = true;
        }
    }
  else
    {
      if (body->buf_reallocated)
        {
          body->intensities = body->intensities ? (double *)malloc (sizeof (double)*body->buffer_size)
                                                : (double *)realloc (body->intensities, sizeof (double)*body->buffer_size);
          body->buf_reallocated = false;
        }
      body->intensities[body->size++ / 2] = s ? strtod ((const char *)s, &endptr) : 0.0;
      if (*endptr)
        {
          fprintf (stderr, "Found non-double data in the second column of csv result-file: %s\n", (char *)s);
          body->error = true;
        }
    }
}

/* add rows */
void
cb2 (int   c,
     void *data)
{
  struct csv_body *body = (struct csv_body *)data;
  body->num_rows++;
  if (body->curr_num_cols != body->num_cols)
    {
      fprintf(stderr, "Did not find data for all fields for row: %d\n", body->num_rows);
      body->error = true;
    }
}

struct csv_data *
read_csv (FILE *fp)
{
  const unsigned int buf_size = 4096;
  char buf[4096];
  char **fields;
  int dummy;
  struct csv_parser p;
  struct csv_body body = {0};
  struct csv_data *result;
  size_t bytes_read;

  fields = read_csv_fields (fp, &dummy);
  if (!fields)
    {
      fprintf (stderr, "Failed to read csv fields.\n");
      return NULL;
    }

  if (csv_init(&p, CSV_STRICT | CSV_REPALL_NL | CSV_STRICT_FINI | CSV_APPEND_NULL | CSV_EMPTY_IS_NULL) != 0)
    {
      fprintf (stderr, "csv_init() in read_csv() failed.\n");
      return NULL;
    }
  rewind (fp);  // equivalent to fseek (fp, 0, SEEK_SET);

  csv_set_realloc_func (&p, realloc);
  csv_set_free_func (&p, free);
  do
    {
      bytes_read = sl_fread (buf, 1, buf_size, fp, true);
      if (bytes_read != buf_size && !feof (fp))
        {
          csv_free (&p);
          return NULL;
        }
      if (csv_parse (&p, buf, bytes_read, cb1, cb2, &body) != bytes_read)
        {
          fprintf (stderr, "libcsv: Error while parsing file: %s\n", csv_strerror (csv_error (&p)));
          return NULL;
        }
    } while (!body.error && !feof (fp));

  csv_fini (&p, cb1, cb2, &body);
  csv_free (&p);
  if (body.error)
    {
      fprintf (stderr, "csv_body Error\n");
      return NULL;
    }
  result = (struct csv_data *)malloc (sizeof (struct csv_data));
  if (!result)
    {
      fprintf (stderr, "malloc csv_data failed.\n");
      return NULL;
    }
  result->fields = fields;
  result->wavelengths = body.wavelengths;
  result->intensities = body.intensities;
  result->num_fields = body.num_cols;
  result->num_datarows = body.size / body.num_cols;
  return result;
}

