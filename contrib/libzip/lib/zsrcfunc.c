/*
  $NiH: zip_source_function.c,v 1.3 2004/12/22 16:32:00 dillo Exp $

  zip_source_function.c -- create zip data source from callback function
  Copyright (C) 1999, 2003, 2004 Dieter Baron and Thomas Klausner

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

#include "zip.h"
#include "zipint.h"



struct zip_source *
zip_source_function(struct zip *za, zip_source_callback zcb, void *ud)
{
    struct zip_source *zs;

    if (za == NULL)
        return NULL;

    if ((zs=ZIP_ALLOC(sizeof(*zs))) == NULL) {
        _zip_error_set(&za->error, ZIP_ER_MEMORY, 0);
        return NULL;
    }

    zs->f = zcb;
    zs->ud = ud;

    return zs;
}
