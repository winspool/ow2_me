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
* Description:  Linux lstat64() implementation.
*
****************************************************************************/


#include "variety.h"
#include <unistd.h>
#include <sys/stat.h>
#include "linuxsys.h"
#include "kstat.h"


_WCRTLINK int lstat64( const char *filename, struct stat64 * __buf )
{
#if 0
    struct kstat64  ks;

    syscall_res res = sys_call2( SYS_lstat64, (u_long)filename, (u_long)&ks );
    if( !__syscall_iserror( res ) ) {
        COPY_STAT( __buf, ks );
    }
#else
    syscall_res res = sys_call2( SYS_lstat64, (u_long)filename, (u_long)__buf );
#endif
    __syscall_return( int, res );
}
