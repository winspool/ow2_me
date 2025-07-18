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
#define _DEBUG                  /* so prototypes are included */
#include <crtdbg.h>
#include "dbgdata.h"


_WCRTLINK int _CrtSetReportMode( int reporttype, int reportmode )
/***************************************************************/
{
    int                 prevmode;

    /*** Ensure reporttype is valid ***/
    if( reporttype < _CRT_WARN
      || reporttype > _CRT_ASSERT ) {
        return( -1 );
    }

    /*** Ok, now do something useful ***/
    prevmode = __DbgReportModes[reporttype];
    if( reportmode != _CRTDBG_REPORT_MODE ) {
        if( reportmode & ~(_CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW) ) {
            return( -1 );
        }
        #if !defined(__NT__) && !defined(__WINDOWS__) && !defined(__OS2__)
            /* do we want to return -1 if _CRTDBG_MODE_WNDW is on? */
            reportmode &= ~_CRTDBG_MODE_WNDW;
        #endif
        __DbgReportModes[reporttype] = reportmode;
    }

    return( prevmode );
}
