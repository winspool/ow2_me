/*
   This file was generated automatically by ./make_zip_err_str.sh
   from ./zip.h; make changes there.

	NiH: make_zip_err_str.sh,v 1.8 2004/11/17 21:55:09 wiz Exp 
	NiH: zip.h,v 1.50 2005/07/14 14:08:11 dillo Exp
 */

#include "zip.h"
#include "zipint.h"



const char * const _zip_err_str[] = {
    "No error",
    "Multi-disk zip archives not supported",
    "Renaming temporary file failed",
    "Closing zip archive failed",
    "Seek error",
    "Read error",
    "Write error",
    "CRC error",
    "Containing zip archive was closed",
    "No such file",
    "File already exists",
    "Can't open file",
    "Failure to create temporary file",
    "Zlib error",
    "Malloc failure",
    "Entry has been changed",
    "Compression method not supported",
    "Premature EOF",
    "Invalid argument",
    "Not a zip archive",
    "Internal error",
    "Zip archive inconsistent",
    "Can't remove file",
    "Entry has been deleted",
};

const int _zip_nerr_str = sizeof(_zip_err_str)/sizeof(_zip_err_str[0]);

#define N ZIP_ET_NONE
#define S ZIP_ET_SYS
#define Z ZIP_ET_ZLIB

const int _zip_err_type[] = {
    N,
    N,
    S,
    S,
    S,
    S,
    S,
    N,
    N,
    N,
    N,
    S,
    S,
    Z,
    N,
    N,
    N,
    N,
    N,
    N,
    N,
    N,
    S,
    N,
};
