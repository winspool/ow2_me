/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2025      The Open Watcom Contributors. All Rights Reserved.
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#define __FUNCTION_DATA_ACCESS
#include "variety.h"
#ifdef __NT__
    #include <windows.h>
    #include <winnls.h>
#endif
#include <mbstring.h>

/**
 * Determine if a multibyte character is legal.
 */

_WCRTLINK int _ismbclegal( unsigned int ch )
{
#ifdef __NT__
    int                 len;
    unsigned char       mbc[MB_LEN_MAX + 1];
    WORD                char_type[MB_LEN_MAX + 1];

    _mbvtop( ch, mbc );
    len = _mbclen( mbc );
    mbc[len] = '\0';
    if( len <= 1 )
        return( 0 );
    if( GetStringTypeEx( LOCALE_USER_DEFAULT, CT_CTYPE1, (char *)mbc, -1, char_type ) == 0 )
        return( 0 );
    if( char_type[0] == 0 )
        return( 0 );
    return( 1 );
#else                                   /* OS/2 and others */
    return( _ismbblead( ch >> 8 ) && _ismbbtrail( ch & 0xFF ) );
#endif
}
