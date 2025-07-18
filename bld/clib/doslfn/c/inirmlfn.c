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
* Description:  Real mode selector setup for DOS extenders and Win386.
*
****************************************************************************/


#define __WATCOM_LFN__
#include "variety.h"
#include "widechar.h"
#include <stdlib.h>
#include <string.h>
#include <mbstring.h>
#include <dos.h>
#include "watcom.h"
#include "extender.h"
#include "roundmac.h"
#include "exitwmsg.h"
#include "rtinit.h"
#include "tinyio.h"
#include "_doslfn.h"


#define TOTAL_RM_TB_SIZE_PARA   __ROUND_UP_SIZE_TO_PARA( RM_TB_PARM1_SIZE + RM_TB_PARM2_SIZE )

char                    * __lfn_rm_tb_linear = 0;
unsigned short          __lfn_rm_tb_segment = 0;

static unsigned short   __lfn_rm_tb_selector = 0;

static void init( void )
/**********************/
{
    dpmi_dos_mem_block  dos_mem;

    if( _DPMI ) {
        dos_mem = DPMIAllocateDOSMemoryBlock( TOTAL_RM_TB_SIZE_PARA );
        __lfn_rm_tb_segment = dos_mem.rm;
        __lfn_rm_tb_selector = dos_mem.pm;
    }
    if( __lfn_rm_tb_selector == 0 ) {
        __fatal_runtime_error( "Unable to allocate LFN real mode transfer buffer", -1 );
        // never return
    }
    __lfn_rm_tb_linear = (char *)DPMIGetSegmentBaseAddress( __lfn_rm_tb_selector );
}

static void fini( void )
/**********************/
{
    if( _DPMI ) {
        DPMIFreeDOSMemoryBlock( __lfn_rm_tb_selector );
    }
}

AXI( init, INIT_PRIORITY_RUNTIME )
AYI( fini, INIT_PRIORITY_RUNTIME )
