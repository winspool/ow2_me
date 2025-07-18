/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2002-2025 The Open Watcom Contributors. All Rights Reserved.
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
* Description:  Implementation of _fstrnicmp() - far _strnicmp().
*
****************************************************************************/


#include "variety.h"
#include <ctype.h>
#include <string.h>

/* return <0 if s<t, 0 if s==t, >0 if s>t */

_WCRTLINK int _fstrnicmp( const char _WCFAR *s, const char _WCFAR *t, size_t n )
{
    unsigned char   c1;
    unsigned char   c2;

    for( ; n > 0; --n ) {
        c1 = *s++;
        if( c1 >= 'A' && c1 <= 'Z' )
            c1 += 'a' - 'A';
        c2 = *t++;
        if( c2 >= 'A' && c2 <= 'Z' )
            c2 += 'a' - 'A';
        if( c1 != c2 )
            break;      /* less than or greater than */
        if( c1 == '\0' ) {
            break;      /* equal */
        }
    }
    return( c1 - c2 );
}
