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
#include <mbstring.h>
#include "farfunc.h"
#include "mbyte.h"



/****
***** Differs from _mbsninc in that a null byte counts as a character.
****/

_WCRTLINK unsigned char _FFAR *_NEARFAR(__mbsninc,__fmbsninc)( unsigned char const _FFAR *s, int nm )
{
    if( __IsDBCS ) {
        while( nm > 0 ) {
            s += _ismbblead(*s) ? 2 : 1;
            nm--;
        }
        return( (unsigned char _FFAR *)s );
    } else {
        return( (unsigned char _FFAR *)(s + nm) );
    }
}
