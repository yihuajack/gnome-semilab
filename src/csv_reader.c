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

struct csv_body_uint_double
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

struct csv_body_double_double
{
  double *data;
  int size;
  int buffer_size;
  int num_rows;
  unsigned int curr_num_cols;
  unsigned int num_cols;
  bool error;
};

typedef void (*CB1_UINT_DOUBLE)(void *, size_t, struct csv_body_uint_double *);
typedef void (*CB1_DOUBLE_DOUBLE)(void *, size_t, struct csv_body_double_double *);
typedef void (*CB2_UINT_DOUBLE)(int, struct csv_body_uint_double *);
typedef void (*CB2_DOUBLE_DOUBLE)(int, struct csv_body_double_double *);

/* enum
{
  UINT_DOUBLE_DATA,
  DOUBLE_DOUBLE_DATA
}; */

// https://stackoverflow.com/questions/40096584/c11-generic-usage
#define cb1_generic(s, len, data) _Generic((data),     \
    struct csv_body_uint_double *: cb1_uint_double,    \
  struct csv_body_double_double *: cb1_double_double   \
)(s, len, data)

#define cb2_generic(c, data) _Generic((data),          \
    struct csv_body_uint_double *: cb2_uint_double,    \
  struct csv_body_double_double *: cb2_double_double   \
)(c, data)

// https://stackoverflow.com/questions/47858491/c-macro-for-type-safe-callbacks
// You can also optionally add any types of cb1 and cb2 to csv_parse_generic
// and csv_fini generic function macros to specify that cb1 is cb1_uint_double
// or cb1_double_double and cb2 is cb2_uint_double or cb2_double_double.
#define csv_parse_generic(p, s, len, data)             \
    csv_parse(                                         \
        p,                                             \
        s,                                             \
        len,                                           \
        _Generic(data,                                 \
    struct csv_body_uint_double *: cb1_uint_double,    \
  struct csv_body_double_double *: cb1_double_double), \
        _Generic(data,                                 \
    struct csv_body_uint_double *: cb2_uint_double,    \
  struct csv_body_double_double *: cb2_double_double), \
        data)

#define csv_fini_generic(p, data)                      \
    csv_fini(                                          \
        p,                                             \
        _Generic(data,                                 \
    struct csv_body_uint_double *: cb1_uint_double,    \
  struct csv_body_double_double *: cb1_double_double), \
        _Generic(data,                                 \
    struct csv_body_uint_double *: cb2_uint_double,    \
  struct csv_body_double_double *: cb2_double_double), \
        data)

#define typename(x) _Generic((x),                      \
      struct csv_body_uint_double: UINT_DOUBLE_DATA,   \
    struct csv_body_double_double: DOUBLE_DOUBLE_DATA)

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
add_fields (void   *data,
            size_t  len,
            void   *t)
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

char **
read_csv_fields (FILE *fp,
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
          fprintf (stderr, "ERROR: fread length is not equal to buffer size and is not feof.\n");
          csv_free (&p);
          return NULL;
        }
      // libcsv: Error while parsing file: error parsing data while strict checking enabled
      // Found non-double data in the first column of csv result-file: 300gsten-Halogen 3300K""
      // The fields must not be quoted!
      // size=1\nfield=`R�/�\nsize=2\nfield=
      csv_parse (&p, buf, bytes_read, add_fields, count_rows_head, &head);
    } while (!head.num_rows && !feof (fp));
  csv_free (&p);
  if (!head.num_rows)
    {
      fprintf (stdout, "INFO: Number of rows in the header is 0.\n");
      return NULL;
    }
  *length = head.size - 1;
  // Head fields should not be double-quoted; otherwise, csv_parse will give wrong results!
  // head.num_rows = 326, head.size = 2, head.fields[0] = 73, head.fields[1] = 0.0913
  return head.fields;
}

/* add cells */
void
cb1_uint_double (void   *s,
                 size_t  len,
                 void   *data)
{
  struct csv_body_uint_double *body = (struct csv_body_uint_double *)data;
  // If using OpenModelica's code `char *ptr = "";`:
  // warning: initialization discards ‘const’ qualifier from pointer target type [-Wdiscarded-qualifiers]
  char *endptr = NULL;
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
          body->wavelengths = body->wavelengths ? (unsigned int *)realloc (body->wavelengths, sizeof (unsigned int) * body->buffer_size)
                                                : (unsigned int *)malloc (sizeof (unsigned int) * body->buffer_size);
          body->buf_reallocated = true;
        }
      // Suffix/postfix increment and decrement have precedence 1
      // Multiplication, division, and remainder have precedence 3
      // Note that the actual "data" is the "s" argument; strtoul(data) would be ��UUUU
      body->wavelengths[body->size++ / 2] = s ? strtoul ((const char *)s, &endptr, 10) : 0;
      if (*endptr)
        {
          fprintf (stderr, "ERROR: Found non-unsigned-int data in the first column of csv result-file: %s\n", (char *)s);
          body->error = true;
        }
    }
  else
    {
      if (body->buf_reallocated)
        {
          body->intensities = body->intensities ? (double *)malloc (sizeof (double) * body->buffer_size)
                                                : (double *)realloc (body->intensities, sizeof (double) * body->buffer_size);
          body->buf_reallocated = false;
        }
      body->intensities[body->size++ / 2] = s ? strtod ((const char *)s, &endptr) : 0.0;
      if (*endptr)
        {
          fprintf (stderr, "ERROR: Found non-double data in the second column of csv result-file: %s\n", (char *)s);
          body->error = true;
        }
    }
}

void
cb1_double_double (void  *s,
                   size_t len,
                   void   *data)
{
  struct csv_body_double_double *body = (struct csv_body_double_double *)data;
  char *endptr = NULL;
  if (body->error)
    return;
  if (!body->num_rows)
    {
      body->curr_num_cols++;
      body->num_cols++;
      return;
    }
  if (body->size + 1 >= body->buffer_size)
    {
      body->buffer_size = body->data ? 2 * body->buffer_size : body->num_cols * 1024;
      body->buffer_size = body->buffer_size > 0 ? body->buffer_size : 1024;
      body->data = body->data ? (double *)realloc (body->data, sizeof (double) * body->buffer_size)
                              : (double *)calloc (body->buffer_size, sizeof (double));
    }
  body->data[body->size++] = s ? strtod ((const char *)s, &endptr) : 0;
  if (*endptr)
    {
      fprintf (stderr, "ERROR: Found non-double data in the csv result-file: %s\n", (char *)s);
      body->error = true;
    }
}

/* add rows */
void
cb2_uint_double (int   c,
                 void *data)
{
  struct csv_body_uint_double *body = (struct csv_body_uint_double *)data;
  body->num_rows++;
  if (body->curr_num_cols != body->num_cols)
    {
      fprintf(stderr, "ERROR: Did not find data for all fields for row: %d\n", body->num_rows);
      body->error = true;
    }
}

void
cb2_double_double (int   c,
                   void *data)
{
  struct csv_body_double_double *body = (struct csv_body_double_double *)data;
  body->num_rows++;
  if (body->curr_num_cols != body->num_cols)
    {
      fprintf(stderr, "ERROR: Did not find data for all fields for row: %d\n", body->num_rows);
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
  struct csv_body_double_double body = {0};
  struct csv_data *result;
  size_t bytes_read;

  fields = read_csv_fields (fp, &dummy);
  if (!fields)
    {
      fprintf (stderr, "ERROR: Failed to read csv fields.\n");
      return NULL;
    }

  if (csv_init(&p, CSV_STRICT | CSV_REPALL_NL | CSV_STRICT_FINI | CSV_APPEND_NULL | CSV_EMPTY_IS_NULL) != 0)
    {
      fprintf (stderr, "ERROR: csv_init() in read_csv() failed.\n");
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
      if (csv_parse_generic (&p, buf, bytes_read, &body) != bytes_read)
        {
          fprintf (stderr, "ERROR: failed to parse file: %s\n", csv_strerror (csv_error (&p)));
          return NULL;
        }
    } while (!body.error && !feof (fp));

  csv_fini_generic (&p, &body);
  csv_free (&p);
  if (body.error)
    {
      fprintf (stderr, "ERROR: csv_body error\n");
      return NULL;
    }
  result = (struct csv_data *)malloc (sizeof (struct csv_data));
  if (!result)
    {
      fprintf (stderr, "ERROR: malloc csv_data failed.\n");
      return NULL;
    }
  result->fields = fields;
  result->num_fields = body.num_cols;
  result->num_datarows = body.size / body.num_cols;
  double *data = body.data;
#ifdef UINT_DOUBLE_DATA
  result->wavelengths = body.wavelengths;
  result->intensities = body.intensities;
#else
  matrix_transpose (data, result->num_fields, result->num_datarows);
  // https://stackoverflow.com/questions/5850000/how-to-split-array-into-two-arrays-in-c
  // Use calloc() to initialize all bytes in the allocated storage to zero
  result->wavelengths = (double *)calloc (body.buffer_size, sizeof (double));
  result->intensities = (double *)calloc (body.buffer_size, sizeof (double));
  memcpy (result->wavelengths, data, result->num_datarows * sizeof (double));
  memcpy (result->intensities, data + result->num_datarows, result->num_datarows * sizeof (double));
#endif
  return result;
}

