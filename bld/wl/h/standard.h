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
* Description:  Standard linker configuration.
*
****************************************************************************/


#ifndef _INCLUDE_STANDARD_H
#define _INCLUDE_STANDARD_H

#include <stddef.h>
#include <stdarg.h>
#include <time.h>
#include "bool.h"
#include "machtype.h"
#include "roundmac.h"
#include "alloc.h"
#include "iopath.h"
#include "wlinkcfg.h"


#define NLCHAR          '\n'

#ifdef _M_I86
#define CONSTU32(x)     x ## UL
#else
#define CONSTU32(x)     x ## U
#endif

// useful for stringizing tokens
#define __str( m )      #m
#define __xstr( l )     __str( l )

#ifdef DEVBUILD
    #define __location " (" __FILE__ "," __xstr(__LINE__) ")"
    #define DbgVerify( cond, msg )  if( !(cond) ) LnkFatal( msg __location )
    #define DbgDefault( msg )       default: LnkFatal( msg __location )
    #define DbgAssert( cond )       if( !(cond) ) LnkFatal( #cond __location )
#else
    #define DbgVerify( cond, msg )
    #define DbgDefault( msg )
    #define DbgAssert( cond )
#endif

#if defined( __UNIX__ )

    #define CASE_SENSITIVE

#else

//    #define CASE_SENSITIVE

#endif

#ifdef CASE_SENSITIVE
    #define FNAMECMPSTR      strcmp      /* for case  sensitive file systems */
#else
    #define FNAMECMPSTR      stricmp     /* for case insensitive file systems */
#endif

typedef int             f_handle;

#if defined( USE_VIRTMEM )
typedef unsigned_32     virt_mem;
#else
typedef char            *virt_mem;
#endif
typedef unsigned_32     virt_mem_size;

typedef union {
    virt_mem        vm_ptr;
    virt_mem_size   vm_offs;
} virt_mem_ptr;

#endif
