/*
  $NiH: zip_entry_new.c,v 1.1 2004/11/30 21:42:22 wiz Exp $

  zip_entry_new.c -- create and init struct zip_entry
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



struct zip_entry *
_zip_entry_new(struct zip *za)
{
    struct zip_entry *ze;
    int increment = 64;
    if (!za) {
        ze = (struct zip_entry *)ZIP_ALLOC(sizeof(struct zip_entry));
        if (!ze) {
            _zip_error_set(&za->error, ZIP_ER_MEMORY, 0);
            return NULL;
        }
    }
    else {
        if (za->nentry >= za->nentry_alloc-1) {
            za->nentry_alloc += increment;
            increment *= 2;
            za->entry = (struct zip_entry *)ZIP_REALLOC(za->entry,
                                                    sizeof(struct zip_entry)
                                                    * za->nentry_alloc);
            if (!za->entry) {
                _zip_error_set(&za->error, ZIP_ER_MEMORY, 0);
                return NULL;
            }
        }
        ze = za->entry+za->nentry;
    }

    ze->state = ZIP_ST_UNCHANGED;

    ze->ch_filename = NULL;
    ze->source = NULL;

    if (za)
        za->nentry++;

    return ze;
}
