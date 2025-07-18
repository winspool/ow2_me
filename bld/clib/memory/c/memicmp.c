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


#include "variety.h"
#include <string.h>
#include <ctype.h>
#include "farfunc.h"


_WCRTLINK int _NEARFAR(_memicmp,_fmemicmp)( const void _FFAR *vs1, const void _FFAR *vs2, size_t len )
{
    const unsigned char _FFAR   *s1;
    const unsigned char _FFAR   *s2;
    unsigned char               c1;
    unsigned char               c2;

    for( s1 = vs1, s2 = vs2; len > 0; --len )  {
        c1 = *s1++;
        if( c1 >= 'A' && c1 <= 'Z' )
            c1 += 'a' - 'A';
        c2 = *s2++;
        if( c2 >= 'A' && c2 <= 'Z' )
            c2 += 'a' - 'A';
        if( c1 != c2 ) {
            return( c1 - c2 );
        }
    }
    return( 0 );    /* both operands are equal */
}
