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

/*
 *  CMDRAW : command line parsing for the RAW load file format.
 *
*/

#include <string.h>
#include "linkstd.h"
#include "cmdutils.h"
#include "dbgall.h"
#include "cmdall.h"
#include "overlays.h"
#include "objcalc.h"
#include "cmdline.h"
#include "cmdraw.h"


#ifdef _RAW


/****************************************************************
 * "Format" Directive
 ****************************************************************/

static bool ProcRawBIN( void )
/****************************/
{
    FmtData.def_ext = E_BIN;
    FmtData.raw_hex_output = false;
    return( true );
}

static bool ProcRawHEX( void )
/****************************/
{
    FmtData.def_ext = E_HEX;
    FmtData.raw_hex_output = true;
    return( true );
}

static parse_entry  RawFormats[] = {
    "BIN",          ProcRawBIN,         MK_RAW, 0,
    "HEX",          ProcRawHEX,         MK_RAW, 0,
    NULL
};

bool ProcRawFormat( void )
/************************/
{
    LinkState |= LS_MAKE_RELOCS | LS_FMT_DECIDED;   // Make relocations;
    FmtData.base = 0;                               // Default offset
    ProcOneSuicide( RawFormats, SEP_NO );
    return( true );
}

#endif
