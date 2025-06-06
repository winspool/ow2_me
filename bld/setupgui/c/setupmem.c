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
* Description:  Memory Management functions with included trmem memory tracker
*
****************************************************************************/


#include <stdlib.h>
#include <string.h>
#include "bool.h"
#include "guimem.h"
#if defined( GUI_IS_GUI )
    #include "wpimem.h"
#else
    #include "stdui.h"
    #include "helpmem.h"
#endif
#ifdef TRMEM
    #include "trmem.h"
#endif
#ifdef USE_WRESLIB
    #include "wresmem.h"
#endif
#include "zipmem.h"


#ifdef TRMEM
static _trmem_hdl  GUIMemHandle;

static FILE *GUIMemFP = NULL;           /* stream to put output on */
static bool GUIMemOpened = false;

static void GUIMemPrintLine( void *parm, const char *buff, size_t len )
/*********************************************************************/
{
    /* unused parameters */ (void)parm; (void)len;

    if( GUIMemFP != NULL ) {
        fprintf( GUIMemFP, "%s\n", buff );
    }
}
#if 0
static void UIMemPrintLine( void *parm, const char *buff, size_t len )
{
    /* unused parameters */ (void)parm; (void)buff; (void)len;
}
#endif
#endif

void GUIMemPrtUsage( void )
/*************************/
{
#ifdef TRMEM
    _trmem_prt_usage( GUIMemHandle );
#endif
}

void GUIMemRedirect( FILE *fp )
/*****************************/
{
#ifdef TRMEM
    GUIMemFP = fp;
#else
    /* unused parameters */ (void)fp;
#endif
}
#if 0
void UIMemRedirect( FILE *fp )
{
#ifdef TRMEM
    GUIMemFP = fp;
#else
    /* unused parameters */ (void)fp;
#endif
}
#endif


void GUIMemOpen( void )
/*********************/
{
#ifdef TRMEM
    char * tmpdir;

    if( !GUIMemOpened ) {
        GUIMemFP = stderr;
        GUIMemHandle = _trmem_open( malloc, free, realloc, NULL,
            NULL, GUIMemPrintLine,
            _TRMEM_ALLOC_SIZE_0 | _TRMEM_REALLOC_SIZE_0 |
            _TRMEM_OUT_OF_MEMORY | _TRMEM_CLOSE_CHECK_FREE );

        tmpdir = getenv( "TRMEMFILE" );
        if( tmpdir != NULL ) {
            GUIMemFP = fopen( tmpdir, "w" );
        }
        GUIMemOpened = true;
    }
#endif
}
#if !defined( GUI_IS_GUI )
void UIAPI UIMemOpen( void ) {}
void HelpMemOpen( void ) {}
#endif

void GUIMemClose( void )
/**********************/
{
#ifdef TRMEM
    _trmem_prt_list( GUIMemHandle );
    _trmem_close( GUIMemHandle );
    if( GUIMemFP != stderr ) {
        fclose( GUIMemFP );
        GUIMemFP = NULL;
    }
#endif
}
#if !defined( GUI_IS_GUI )
void UIAPI UIMemClose( void ) {}
void HelpMemClose( void ) {}
#endif


/*
 * Alloc functions
 */

void *GUIMemAlloc( size_t size )
/******************************/
{
#ifdef TRMEM
    return( _trmem_alloc( size, _trmem_guess_who(), GUIMemHandle ) );
#else
    return( malloc( size ) );
#endif
}
void *zip_alloc( size_t size )
/****************************/
{
#ifdef TRMEM
    return( _trmem_alloc( size, _trmem_guess_who(), GUIMemHandle ) );
#else
    return( malloc( size ) );
#endif
}
#if defined( GUI_IS_GUI )
#if defined( __OS2__ )
void * _wpi_malloc( size_t size )
{
#ifdef TRMEM
    return( _trmem_alloc( size, _trmem_guess_who(), GUIMemHandle ) );
#else
    return( malloc( size ) );
#endif
}
#endif
#else
void * UIAPI uimalloc( size_t size )
{
#ifdef TRMEM
    return( _trmem_alloc( size, _trmem_guess_who(), GUIMemHandle ) );
#else
    return( malloc( size ) );
#endif
}
void *HelpMemAlloc( size_t size )
{
#ifdef TRMEM
    return( _trmem_alloc( size, _trmem_guess_who(), GUIMemHandle ) );
#else
    return( malloc( size ) );
#endif
}
#endif
#ifdef USE_WRESLIB
void *wres_alloc( size_t size )
{
#ifdef TRMEM
    return( _trmem_alloc( size, _trmem_guess_who(), GUIMemHandle ) );
#else
    return( malloc( size ) );
#endif
}
#endif

/*
 * Free functions
 */

void GUIMemFree( void *ptr )
/**************************/
{
#ifdef TRMEM
    _trmem_free( ptr, _trmem_guess_who(), GUIMemHandle );
#else
    free( ptr );
#endif
}
void zip_free( void *ptr )
/************************/
{
#ifdef TRMEM
    _trmem_free( ptr, _trmem_guess_who(), GUIMemHandle );
#else
    free( ptr );
#endif
}
#if defined( GUI_IS_GUI )
#if defined( __OS2__ )
void _wpi_free( void *ptr )
{
#ifdef TRMEM
    _trmem_free( ptr, _trmem_guess_who(), GUIMemHandle );
#else
    free( ptr );
#endif
}
#endif
#else
void UIAPI uifree( void *ptr )
{
#ifdef TRMEM
    _trmem_free( ptr, _trmem_guess_who(), GUIMemHandle );
#else
    free( ptr );
#endif
}
void HelpMemFree( void *ptr )
{
#ifdef TRMEM
    _trmem_free( ptr, _trmem_guess_who(), GUIMemHandle );
#else
    free( ptr );
#endif
}
#endif
#ifdef USE_WRESLIB
void wres_free( void *ptr )
{
#ifdef TRMEM
    _trmem_free( ptr, _trmem_guess_who(), GUIMemHandle );
#else
    free( ptr );
#endif
}
#endif


/*
 * Realloc functions
 */

void *GUIMemRealloc( void *ptr, size_t size )
/*******************************************/
{
#ifdef TRMEM
    return( _trmem_realloc( ptr, size, _trmem_guess_who(), GUIMemHandle ) );
#else
    return( realloc( ptr, size ) );
#endif
}
void *zip_realloc( void *ptr, size_t size )
/*****************************************/
{
#ifdef TRMEM
    return( _trmem_realloc( ptr, size, _trmem_guess_who(), GUIMemHandle ) );
#else
    return( realloc( ptr, size ) );
#endif
}
#if defined( GUI_IS_GUI )
#if defined( __OS2__ )
void * _wpi_realloc( void *ptr, size_t size )
{
#ifdef TRMEM
    return( _trmem_realloc( ptr, size, _trmem_guess_who(), GUIMemHandle ) );
#else
    return( realloc( ptr, size ) );
#endif
}
#endif
#else
void * UIAPI uirealloc( void *old, size_t size )
{
#ifdef TRMEM
    return( _trmem_realloc( old, size, _trmem_guess_who(), GUIMemHandle ) );
#else
    return( realloc( old, size ) );
#endif
}
void *HelpMemRealloc( void *ptr, size_t size )
{
#ifdef TRMEM
    return( _trmem_realloc( ptr, size, _trmem_guess_who(), GUIMemHandle ) );
#else
    return( realloc( ptr, size ) );
#endif
}
#endif
