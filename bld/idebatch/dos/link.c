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
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "wdebug.h"
#ifdef __WINDOWS__
#include <windows.h>
#endif
#include "osidle.h"
#include "link.h"


_dword __ConversationId;

static char linkName[128];

static void _farstrcpy( char __far *dest, const char __far *src )
{
    while( *dest++ = *src++ );
}

/*
 * VxDPresent - check if WGOD is present
 */
int __pascal VxDPresent( void )
{
    if( CheckWin386Debug() != WGOD_VERSION ) {
        return( 0 );
    }
    return( 1 );

} /*VxDPresent */

#ifdef __WINDOWS__
static void messageLoop( void )
{
    MSG         msg;

    Yield();
    while( PeekMessage( &msg, (HWND) NULL, (UINT) NULL, (UINT) NULL,
                    PM_NOYIELD | PM_NOREMOVE ) ) {
        GetMessage( &msg, NULL, 0, 0 );
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
    Yield();
    ReleaseVMTimeSlice();
    Yield();

} /* messageLoop */
#endif


#ifdef __WINDOWS__
/*
 * VxDRaiseInterrupt - raise an interrupt in the client VM
 */
void __pascal VxDRaiseInterrupt( unsigned intr )
{
    RaiseInterruptInVM( __ConversationId, intr );

} /* VxDRaiseInterrupt */
#endif

/*
 * VxDGet - get some data from a client/server
 */
int __pascal VxDGet( void __far *rec, unsigned len )
{
    _dword      rc;

    /*
     * reserve space for null terminate character
     */
#ifdef __WINDOWS__
    for( ;; ) {
        rc = ConversationGet( __ConversationId, rec, len - 1, NO_BLOCK );
        if( (rc & 0xffff) == BLOCK ) {
            messageLoop();
        } else {
            break;
        }
    }
#else
    rc = ConversationGet( __ConversationId, rec, len - 1, BLOCK );
#endif
    if( (signed short)rc < 0 )
        return( (signed short)rc );
    len = rc >> 16;
    /*
     * add null terminate character
     */
    ((char __far *)rec)[len] = '\0';
    return( len );

} /* VxDGet */

/*
 * VxDPutPending - tests if a server is trying to put to us
 */
int __pascal VxDPutPending( void )
{
    return( ConversationPutPending() );

} /* VxDPutPending */

/*
 * VxDPut - put some data to client/server
 */
void __pascal VxDPut( const void __far *rec, unsigned len )
{
#ifdef __WINDOWS__
    int rc;

    for( ;; ) {
        rc = ConversationPut( __ConversationId, rec, len, NO_BLOCK );
        if( rc == BLOCK ) {
            messageLoop();
        } else {
            break;
        }
    }
#else
    ConversationPut( __ConversationId, rec, len, BLOCK );
#endif

} /* VxDPut */


/*
 * VxDConnect - connect to a client/server
 */
char __pascal VxDConnect( void )
{
    int rc;
#ifdef SERVER
    rc = LookForConversation( &__ConversationId );
    if( rc == 1 ) {
        return( 1 );
    }
    if( rc == 0 ) {
        ReleaseVMTimeSlice();
        return( 0 );
    }
    return 0;           //added by TW (no return value otherwise)??
#else
    static int _first=1;
    if( _first ) {
        _first = 0;
        rc = StartConversation( __ConversationId );
    }
    for( ;; ) {
        rc = IsConversationAck( __ConversationId );
        if( !rc ) {
            ReleaseVMTimeSlice();
        } else if( rc < 0 ) {
        } else {
            return( 1 );
        }
    }
#endif

} /* VxDConnect */

/*
 * VxDDisconnect - stop a conversation with a server
 */
int __pascal VxDDisconnect( void )
{
#ifndef SERVER
    return( EndConversation( __ConversationId ) );
#else
    return( 0 );
#endif

} /* VxDDisconnect */

/*
 * VxDLink - start a lnk
 */
const char * __pascal VxDLink( const char __far *name )
{
    int rc;

    _farstrcpy( linkName, name );
#ifdef SERVER
    rc = RegisterName( linkName );
    if( rc < 0 ) {
        return( "Could not register server!" );
    }
#else
    rc = AccessName( linkName, &__ConversationId );
    if( rc < 0 ) {
        return( "Could not find server!" );
    }
#endif
    return( NULL );

} /* VxDLink */

/*
 * VxDUnLink - terminate link
 */
int __pascal VxDUnLink( void )
{
#ifdef SERVER
#ifdef __WINDOWS__
    messageLoop();
#endif
    return( UnregisterName( linkName ) );
#else
    return( UnaccessName( linkName ) );
#endif

} /* VxDUnLink */
