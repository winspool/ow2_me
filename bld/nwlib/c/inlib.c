/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2024-2024 The Open Watcom Contributors. All Rights Reserved.
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


#include "wlib.h"

#include "clibext.h"


static input_lib *InputLibs;

void AddInputLib( libfile io )
{
    input_lib   *new;

    new = MemAlloc( sizeof( *new ) + strlen( io->name ) );
    new->next = InputLibs;
    InputLibs = new;
    new->io = io;
    strcpy( new->name, io->name );
}

void CloseOneInputLib( void )
{
    input_lib   *curr;

    for( curr = InputLibs; curr != NULL; curr = curr->next ) {
        if( curr->io != NULL ) {
            LibClose( curr->io );
            curr->io = NULL;
            return;
        }
    }
}

void CloseInputLibs( void )
{
    input_lib   *curr;

    for( curr = InputLibs; curr != NULL; curr = curr->next ) {
        if( curr->io != NULL ) {
            LibClose( curr->io );
            curr->io = NULL;
        }
    }
}

void InitInputLibs( void )
{
    InputLibs = NULL;
}

void FiniInputLibs( void )
{
    input_lib   *curr;

    while( (curr = InputLibs) != NULL ) {
        InputLibs = curr->next;
        if( curr->io != NULL ) {
            LibClose( curr->io );
        }
        MemFree( curr );
    }
}

input_lib *FindInLib( libfile io )
{
    input_lib   *curr;

    for( curr = InputLibs; curr != NULL; curr = curr->next ) {
        if( curr->io == io ) {
            return( curr );
        }
    }
    return( NULL );
}

libfile InLibHandle( input_lib *curr )
{
    if( curr->io == NULL ) {
        curr->io = LibOpen( curr->name, LIBOPEN_READ );
    }
    return( curr->io );
}
