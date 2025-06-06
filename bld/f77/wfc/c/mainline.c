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
* Description:  FORTRAN compiler main line
*
****************************************************************************/


#include "ftnstd.h"
#if defined( __WATCOMC__ ) || !defined( __UNIX__ )
#include <process.h>
#endif
#include "global.h"
#include "cpopt.h"
#include "fcgbls.h"
#include "boot77.h"
#include "errutil.h"
#include "filescan.h"
#include "sdcline.h"
#include "fmacros.h"
#include "cioconst.h"
#include "inout.h"
#include "errcod.h"
#include "fmemmgr.h"

#include "clibint.h"
#include "clibext.h"


#if _CPU == 8086
#define WFC_NAME  "wfc"
#define WFC_ENV   "WFC"
#elif _CPU == 386
#define WFC_NAME  "wfc386"
#define WFC_ENV   "WFC386"
#elif _CPU == _AXP
#define WFC_NAME  "wfcaxp"
#define WFC_ENV   "WFCAXP"
#elif _CPU == _PPC
#define WFC_NAME  "wfcppc"
#define WFC_ENV   "WFCPPC"
#else
#error Unknown System
#endif

enum {
    MSG_USAGE_COUNT = 0
    #define pick(c,e,j) + 1
    #include "usage.gh"
    #undef pick
};

#if defined( _M_IX86 )

unsigned char   _8087   = 0;
unsigned char   _real87 = 0;

#endif

void    ShowUsage( void ) {
//===================

    unsigned    msg;
    const char  *str;

    TOutBanner();
    TOutNL( "" );
    for( msg = MSG_USAGE_BASE; msg < MSG_USAGE_BASE + MSG_USAGE_COUNT; msg++ ) {
        str = GetMsg( msg );
        TOutNL( str );
    }
}

static void FInit( void )
{
    char        imageName[_MAX_PATH];

    FMemInit();
    _cmdname( imageName );
    ErrorInit( imageName );
}

static void FFini( void )
{
    ErrorFini();
    FMemErrors();
    FMemFini();
}

int     main( int argc, char *argv[] )
//======================================
// FORTRAN compiler main line.
{
    int         ret_code;
    char        *opts[MAX_OPTIONS+1];
    char        *wfc_env;
    int         cmd_len;
    char        *cmd_line;

#if !defined( __WATCOMC__ )
    _argc = argc;
    _argv = argv;
#else
    /* unused parameters */ (void)argc; (void)argv;
#endif

    FInit();
    cmd_len = _bgetcmd( NULL, 0 ) + 1;
    wfc_env = getenv( WFC_ENV );
    if( wfc_env != NULL ) {
        size_t  len1;
        len1 = strlen( wfc_env );
        cmd_line = FMemAlloc( len1 + 1 + cmd_len );
        strcpy( cmd_line, wfc_env );
        cmd_line[len1++] = ' ';
        _bgetcmd( cmd_line + len1, cmd_len );
    } else {
        cmd_line = FMemAlloc( cmd_len );
        _bgetcmd( cmd_line, cmd_len );
    }
    ret_code = 0;
    InitCompMain();
    if( MainCmdLine( &SrcName, &CmdPtr, opts, cmd_line ) ) {
        SrcExtn = SDSplitSrcExtn( SrcName );    // parse the file name in case we get
        ProcOpts( opts );                       // an error in ProcOpts() so error
        InitPredefinedMacros();                 // file can be created
        ret_code = CompMain();
    } else {
        ShowUsage();
    }
    FiniCompMain();
    FMemFree( cmd_line );
    FFini();
    return( ret_code );
}
