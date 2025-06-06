/*
  $NiH: zip_source_buffer.c,v 1.5 2005/06/09 19:57:10 dillo Exp $

  zip_source_buffer.c -- create zip data source from buffer
  Copyright (C) 1999, 2003, 2004, 2005 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <nih@giga.or.at>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



#include <stdlib.h>
#include <string.h>

#include "zip.h"
#include "zipint.h"

struct read_data {
    const char *buf, *data, *end;
    time_t mtime;
    int freep;
};

static ssize_t read_data(void *state, void *data, size_t len,
                         enum zip_source_cmd cmd);



struct zip_source *
zip_source_buffer(struct zip *za, const void *data, off_t len, int freep)
{
    struct read_data *f;
    struct zip_source *zs;

    if (za == NULL)
        return NULL;

    if (len < 0 || (data == NULL && len > 0)) {
        _zip_error_set(&za->error, ZIP_ER_INVAL, 0);
        return NULL;
    }

    if ((f=ZIP_ALLOC(sizeof(*f))) == NULL) {
        _zip_error_set(&za->error, ZIP_ER_MEMORY, 0);
        return NULL;
    }

    f->data = data;
    f->end = ((const char *)data)+len;
    f->freep = freep;
    f->mtime = time(NULL);

    if ((zs=zip_source_function(za, read_data, f)) == NULL) {
        ZIP_FREE(f);
        return NULL;
    }

    return zs;
}



static ssize_t
read_data(void *state, void *data, size_t len, enum zip_source_cmd cmd)
{
    struct read_data *z;
    char *buf;
    int n;

    z = (struct read_data *)state;
    buf = (char *)data;

    switch (cmd) {
    case ZIP_SOURCE_OPEN:
        z->buf = z->data;
        return 0;

    case ZIP_SOURCE_READ:
        n = z->end - z->buf;
        if (n > len)
            n = len;
        if (n < 0)
            n = 0;

        if (n) {
            memcpy(buf, z->buf, n);
            z->buf += n;
        }

        return n;

    case ZIP_SOURCE_CLOSE:
        return 0;

    case ZIP_SOURCE_STAT:
        {
            struct zip_stat *st;

            if (len < sizeof(*st))
                return -1;

            st = (struct zip_stat *)data;

            st->mtime = z->mtime;
            st->crc = 0;
            st->size = z->end - z->data;
            st->comp_size = -1;
            st->comp_method = ZIP_CM_STORE;

            return sizeof(*st);
        }

    case ZIP_SOURCE_ERROR:
        {
            int *e;

            if (len < sizeof(int)*2)
                return -1;

            e = (int *)data;
            e[0] = e[1] = 0;
        }
        return sizeof(int)*2;

    case ZIP_SOURCE_FREE:
        if (z->freep) {
            ZIP_FREE((void *)z->data);
            z->data = NULL;
        }
        ZIP_FREE(z);
        return 0;

    default:
        ;
    }

    return -1;
}
