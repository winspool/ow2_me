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
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#include "variety.h"
#include <stddef.h>
#include <stdio.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include "fileacc.h"
#include <wos2.h>
#include "openmode.h"
#include "iomode.h"
#include "seterrno.h"


_WCRTLINK unsigned _dos_creatnew( const char *name, unsigned attribute, int *phandle )
{
    APIRET  rc;
    OS_UINT actiontaken;
    HFILE   handle;

    while( *name == ' ' )
        ++name;
    rc = DosOpen( (PSZ)name,
                     &handle,
                     &actiontaken,
                     0,
                     attribute,
                     OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_FAIL_IF_EXISTS,
                     OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE,
                     0 );
    if( rc ) {
        return( __set_errno_dos_reterr( rc ) );
    }
    *phandle = handle;
    __SetIOMode_grow( handle, _READ | _WRITE );
    return( 0 );
}
