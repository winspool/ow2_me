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
* Description:  Implementation of atoll() - convert string to long long.
*
****************************************************************************/


#include "variety.h"
#include "widechar.h"
#include "watcom.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#ifdef __WIDECHAR__
    #include <wctype.h>
#else
    #include <ctype.h>
#endif


_WCRTLINK long long __F_NAME(atoll,_wtoll)( const CHAR_TYPE *p )  /* convert ASCII string to long long */
{
    unsigned long long  value = 0;
    bool                minus;

    __ptr_check( p, 0 );

    while( __F_NAME(isspace,iswspace)( (UCHAR_TYPE)*p ) )
        ++p;
    minus = false;
    switch( *p ) {
    case STRING( '-' ):
        minus = true;
        // fall down
    case STRING( '+' ):
        ++p;
        break;
    }
    while( __F_NAME(isdigit,iswdigit)( (UCHAR_TYPE)*p ) ) {
        value = value * 10 + *p - STRING( '0' );
        ++p;
    }
    if( minus )
        value = -value;
    return( value );
}
