/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2002-2024 The Open Watcom Contributors. All Rights Reserved.
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
* Description:  Internal memory management with optional tracking.
*
****************************************************************************/


/*  memory tracking levels */
#define   _NO_TRACKING     0
#define   _CHUNK_TRACKING  1
#define   _FULL_TRACKING   2

#ifdef _TRACK
    #define _MEMORY_TRACKING _FULL_TRACKING
#else
    #define _MEMORY_TRACKING _NO_TRACKING
//    #define _MEMORY_TRACKING _CHUNK_TRACKING
#endif

extern void             CGMemInit( void );
extern void             CGMemFini( void );
extern mem_out_action   SetMemOut( mem_out_action what );
extern pointer          CGAlloc( size_t size );
extern void             CGFree( pointer chunk );
#if _MEMORY_TRACKING & _FULL_TRACKING
extern void             DumpMem( void );
#endif
