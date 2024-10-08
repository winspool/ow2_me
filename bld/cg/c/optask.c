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


#include "_cgstd.h"
#include "optwif.h"
#include "objout.h"
#include "feprotos.h"


static  label_handle DoAskForLabel( cg_sym_handle sym )
/**************************************************/
{
    label_handle    new_lbl;

  optbegin
    new_lbl = CGAlloc( sizeof( code_lbl ) );
    new_lbl->lbl.link    = Handles;
    Handles = new_lbl;
    new_lbl->lbl.sym     = sym;
    new_lbl->lbl.address = ADDR_UNKNOWN;
    new_lbl->lbl.patches = NULL;
    new_lbl->lbl.status  = 0;
    new_lbl->alias       = NULL;
    new_lbl->ins         = NULL;
    new_lbl->refs        = NULL;
#if OPTIONS & SHORT_JUMPS
    new_lbl->redirect    = NULL;
#endif
#if _TARGET_RISC
    new_lbl->ppc_alt_name = NULL;
    new_lbl->owl_symbol   = NULL;
#endif
#ifdef DEVBUILD
    new_lbl->useinfo.hdltype = NO_HANDLE;
    new_lbl->useinfo.used = false;
#endif
    optreturn( new_lbl );
}


label_handle AskForNewLabel( void )
/*********************************/
{
    label_handle    new_lbl;

  optbegin
    new_lbl = DoAskForLabel( NULL );
    /* the DYINGLABEL bit will cause unused labels to be scrapped*/
    /* without the LDONE travelling through the optimizer queue*/
    new_lbl->lbl.status = CODELABEL | DYINGLABEL;
    optreturn( new_lbl )
}


label_handle AskForRTLabel( rt_class rtindex )
/********************************************/
{
    label_handle    new_lbl;

  optbegin
    new_lbl = DoAskForLabel( RTIDX2SYM( rtindex ) );
    _SetStatus( new_lbl, RUNTIME );
    optreturn( new_lbl );
}


label_handle AskForLabel( cg_sym_handle sym )
/*******************************************/
{
    label_handle    new_lbl;
    fe_attr         attr;

  optbegin
    new_lbl = DoAskForLabel( sym );
    if( sym != NULL ) {
        attr = FEAttr( sym );
        if( attr & FE_UNIQUE ) {
            _SetStatus( new_lbl, UNIQUE );
        }
    }
    optreturn( new_lbl );
}


bool    AskIfReachedLabel( label_handle lbl )
/*******************************************/
{
  optbegin
    _ValidLbl( lbl );
    optreturn( _TstStatus( lbl, REACHED ) );
}


bool    AskIfRTLabel( label_handle lbl )
/**************************************/
{
  optbegin
    _ValidLbl( lbl );
    optreturn( _TstStatus( lbl, RUNTIME ) );
}


bool    AskIfUniqueLabel( label_handle lbl )
/******************************************/
{
  optbegin
    _ValidLbl( lbl );
    optreturn( _TstStatus( lbl, UNIQUE ) );
}


bool    AskIfCommonLabel( label_handle lbl )
/******************************************/
{
  optbegin
    _ValidLbl( lbl );
    optreturn( _TstStatus( lbl, COMMON_LBL ) );
}


offset  AskAddress( label_handle lbl )
/************************************/
{
  optbegin
    _ValidLbl( lbl );
    optreturn( lbl->lbl.address );
}


pointer AskLblPatches( label_handle lbl )
/***************************************/
{
  optbegin
    _ValidLbl( lbl );
    if( lbl->lbl.patches == NULL ) {
        lbl->lbl.patches = InitPatches();
    }
    optreturn( lbl->lbl.patches );
}


cg_sym_handle   AskForLblSym( label_handle lbl )
/**********************************************/
{
  optbegin
    _ValidLbl( lbl );
    optreturn( lbl->lbl.sym );
}
