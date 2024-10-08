/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2024      The Open Watcom Contributors. All Rights Reserved.
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
* Description:  DWARF reader location expression processing.
*
****************************************************************************/


#include <stdlib.h>
#include "drpriv.h"
#include "drutils.h"
#include "leb128rd.h"


#define ReadSLEB128(h)   DecodeSLEB128((void **)&(h), readLEB)
#define ReadULEB128(h)   DecodeULEB128((void **)&(h), readLEB)

static dw_locop_op const LocOpr[] = {
#define DW_LOC_OP( __n, __v )    __v,
#include "dwlocinf.h"
#undef DW_LOC_OP
};

static unsigned char readLEB( void **h )
/**************************************/
{
    return( *(*(unsigned char **)h)++ );
}

//TODO: check stack bounds
static void DoLocExpr( unsigned_8       *p,
                       int              length,
                       int              addr_size,
                       dr_loc_callbck   *callbck,
                       void             *d,
                       drmem_hdl        var )
{
    unsigned_8      *end;
    dw_op           op;
    dw_locop_op     opr;
    uint_32         op1;
    uint_32         op2;
    uint_32         stk1;
    uint_32         stk2;
    uint_64         utmp;
    int_64          stmp;
    uint_32         stack[101];
    uint_32         *top;
    uint_32         *stk_top;
    dr_loc_kind     kind;
    unsigned_32     utmp32;

#define Pop( a )     (++a)
#define Push( a )    (--a)
#define IsEmpty( a, b )  ( (a) == (b) )

    /* unused parameters */ (void)var;

    end = &p[length];
    stk_top = &stack[100];
    stack[100] = 0;
    top = stk_top;
    if( callbck->init!= NULL  ) {
        kind = callbck->init( d, &utmp32 );
        switch( kind ){
        case DR_LOC_NONE:
             kind = DR_LOC_ADDR;
             break;
        case DR_LOC_REG:
        case DR_LOC_ADDR:
            Push( top );
            top[0] = utmp32;
        }
    } else {
        kind = DR_LOC_ADDR;
    }
    op1 = 0;
    op2 = 0;
    stk1 = 0;
    stk2 = 0;
    while( p  < end ) {
        op = *p;
        ++p;
        opr = LocOpr[op];
        /* decode operands */
        switch( opr ) {
        case DW_LOP_NOOP:
            break;
        case DW_LOP_ADDR:
            if( addr_size == 4 ) {
                op1 = *(uint_32 _WCUNALIGNED *)p;
                if( DR_CurrNode->byte_swap ) {
                    SWAP_32( op1 );
                }
            } else if( addr_size == 2 ) {
                op1 = *(uint_16 _WCUNALIGNED *)p;
                if( DR_CurrNode->byte_swap ) {
                    SWAP_16( op1 );
                }
            } else if( addr_size == 1 ) {
                op1 = *(uint_8 _WCUNALIGNED *)p;
            } else {
                op1 = 0;
            }
            p += addr_size;
            break;
        case DW_LOP_OPU1:
            op1 = *(uint_8 *)p;
            p += sizeof( uint_8 );
            break;
        case DW_LOP_OPS1:
            op1 = (int_32)*(int_8 *)p;
            p += sizeof(int_8 );
            break;
        case DW_LOP_OPU2:
            op1 = *(uint_16 _WCUNALIGNED *)p;
            p += sizeof(uint_16 );
            break;
        case DW_LOP_OPS2:
            op1 = (int_32)*(int_16 _WCUNALIGNED *)p;
            p += sizeof(int_16);
            break;
        case DW_LOP_OPS4:
            op1 = *(int_32 _WCUNALIGNED *)p;
            p += sizeof(int_32);
            break;
        case DW_LOP_OPU4:
            op1 = *(uint_32 _WCUNALIGNED *)p;
            p += sizeof(uint_32);
            break;
        case DW_LOP_U128:
            utmp = ReadULEB128( p );
            op1 = utmp;
            break;
        case DW_LOP_S128:
            stmp = ReadSLEB128( p );
            op1 = stmp;
            break;
        case DW_LOP_U128_S128:
            utmp = ReadULEB128( p );
            op1 = utmp;
            stmp = ReadSLEB128( p );
            op2 = stmp;
            break;
        case DW_LOP_LIT1:
            op1 = op - DW_OP_lit0;
            op = DW_OP_lit0;
            break;
        case DW_LOP_REG1:
            op1 = op - DW_OP_reg0;
            op = DW_OP_reg0;
            break;
        case DW_LOP_BRG1:
            op1 = op - DW_OP_breg0;
            op = DW_OP_breg0;
            stmp = ReadSLEB128( p );
            op2 = stmp;
            break;
        case DW_LOP_STK2:
            stk2 = top[0];
            Pop( top );         /* drop thru */
        case DW_LOP_STK1:
            stk1 = top[0];
            break;
        default:
            DR_EXCEPT( DREXCEP_BAD_DBG_INFO );
        }
        switch( op ) {
        case DW_OP_reg0:
        case DW_OP_regx:
            /* reg is location */
            Push( top );
            top[0] = op1;
            kind = DR_LOC_REG;
            break;
        case DW_OP_addr: {
            bool isfar;

            /* get contents of reg op1 */
            Push( top );
            top[0] = op1;
            if( (p != end) && ((*p == DW_OP_xderef) || (*p == DW_OP_xderef_size)) ) {
                isfar = true;
            } else {
                isfar = false;
            }
            if( !callbck->acon( d, top, isfar ) ) {
                return;
            }
        }   break;
        case DW_OP_lit0:
        case DW_OP_const1u:
        case DW_OP_const1s:
        case DW_OP_const2u:
        case DW_OP_const2s:
        case DW_OP_const4s:
        case DW_OP_const4u:
        case DW_OP_const8u:
        case DW_OP_const8s:
        case DW_OP_constu:
        case DW_OP_consts:
            Push( top );
            top[0] = op1;
            break;
        case DW_OP_fbreg:
        /* We should look at DW_AT_frame_base here, however at this
         * point the debugger seems to be able to figure out the base
         * correctly without it - at least assuming that it's (E)BP on x86.
         */
            Push( top );
            if( !callbck->frame( d, top ) ) {
                return;
            }
            top[0] += op1;
            break;
        case DW_OP_breg0:
        case DW_OP_bregx:
            /* get contents of reg op1 */
            Push( top );
            if( !callbck->reg( d, top, (uint_16)op1 ) ) {
                return;
            }
            top[0] += op2;
            break;
        case DW_OP_dup:
            stk1 = top[0];
            Push( top );
            top[0] = stk1;
            break;
        case DW_OP_drop:
            Pop( top );
            break;
        case DW_OP_over:
            stk1 = top[1];
            Push( top );
            top[0] = stk1;
            break;
        case DW_OP_pick:
            stk1 = top[op1];
            Push( top );
            top[0] = stk1;
            break;
        case DW_OP_swap:
            stk1 = top[0];
            top[0] = top[1];
            top[1] = stk1;
            break;
        case DW_OP_rot:
            stk1 = top[0];
            top[0] = top[1];
            top[1] = top[2];
            top[2] = stk1;
            break;
        case DW_OP_deref:
            op1 = addr_size;
            /* fall through */
        case DW_OP_deref_size:
            /* dref addr */
            if( kind == DR_LOC_REG ) {  // indirect of reg name
                if( !callbck->reg( d, top, (uint_16)top[0] ) ) {
                    return;
                }
                kind = DR_LOC_ADDR;
            } else {
                if( !callbck->dref( d, top, top[0], op1 ) ) {
                    return;
                }
            }
            break;
        case DW_OP_xderef:
            op1 = addr_size;
            /* fall through */
        case DW_OP_xderef_size:
            /*  xdref addr */
            stk1 = top[0];
            Pop( top );
            if( !callbck->drefx( d, top, stk1, top[0], (uint_16)op1 ) ) {
                return;
            }
            break;
        case DW_OP_abs:
            if( (int_32)stk1 < 0 ) {
                stk1 = -(int_32)stk1;
                top[0] =  stk1;
            }
            break;
        case DW_OP_and:
            top[0] = stk2 & stk1;
            break;
        case DW_OP_div:
            top[0] = stk2 / stk1;
            break;
        case DW_OP_minus:
            top[0] = stk2 - stk1;
            break;
        case DW_OP_mod:
            top[0] = stk2 % stk1;
            break;
        case DW_OP_mul:
            top[0] = stk2 * stk1;
            break;
        case DW_OP_neg:
            top[0] = -(int_32)stk1;
            break;
        case DW_OP_not:
            top[0] = ~stk1;
            break;
        case DW_OP_or:
            top[0] = stk2 | stk1;
            break;
        case DW_OP_plus:
            top[0] = stk2 + stk1;
            break;
        case DW_OP_plus_uconst:
            top[0] += op1;
            break;
        case DW_OP_shl:
            top[0] = stk2 << stk1;
            break;
        case DW_OP_shr:
            top[0] = stk2 >> stk1;
            break;
        case DW_OP_shra:
            top[0] = (int_32)stk2 >> stk1;
            break;
        case DW_OP_xor:
            top[0] = stk2 ^ stk1;
            break;
        case DW_OP_eq:
            top[0] = (int_32)stk2 == (int_32)stk1;
            break;
        case DW_OP_ge:
            top[0] = (int_32)stk2 >= (int_32)stk1;
            break;
        case DW_OP_gt:
            top[0] = (int_32)stk2 > (int_32)stk1;
            break;
        case DW_OP_le:
            top[0] = (int_32)stk2 <= (int_32)stk1;
            break;
        case DW_OP_lt:
            top[0] = (int_32)stk2 < (int_32)stk1;
            break;
        case DW_OP_ne:
            top[0] = (int_32)stk2 != (int_32)stk1;
            break;
        case DW_OP_bra:
            stk1 = top[0];
            Pop( top );
            if( stk1 == 0 )
                break;
            /* fall through */
        case DW_OP_skip:
            p += op1;
            break;
        case DW_OP_nop:
            break;
        case DW_OP_piece:
            /* call got piece */
            if( !callbck->ref( d, top[0], op1, kind ) ) {
                return;
            }
            Pop( top );
            kind = DR_LOC_ADDR;
            break;
        default:
            DR_EXCEPT( DREXCEP_BAD_DBG_INFO );
        }
    }
    while( !IsEmpty( top, stk_top) ) {
        if( !callbck->ref( d, top[0], addr_size, kind ) ) {
            return;
        }
        Pop( top );
        kind = DR_LOC_ADDR;
    }
    return;
}

static drmem_hdl SearchLocList( uint_32 start, uint_32 context, uint addr_size )
/******************************************************************************/
// Search loc list for context return start of loc_expr block or NULL
{
    uint_32     low;
    uint_32     high;
    int         len;
    drmem_hdl   p;

    p =  DR_CurrNode->sections[DR_DEBUG_LOC].base;
    if( p == DRMEM_HDL_NULL ) {
        DR_EXCEPT( DREXCEP_BAD_DBG_INFO );
        return( DRMEM_HDL_NULL );
    }
    p += start;
    for( ;; ) {
        low = DR_ReadInt( p, addr_size );
        p += addr_size;
        high = DR_ReadInt( p, addr_size );
        p += addr_size;
        if( low == high && low == 0 ) {
            p = DRMEM_HDL_NULL;
            break;
        }
        if( low <= context && context < high )
            break;
        len = DR_VMReadWord( p );
        p += sizeof( uint_16 );
        p += len;
    }
    return( p );
}

static bool DR_LocExpr( drmem_hdl var, drmem_hdl abbrev, drmem_hdl info,
                                    dr_loc_callbck *callbck, void *d )
/**********************************************************************/
{
    dw_formnum  form;
    uint_32     size;
    uint_8      loc_buff[256];
    uint_8      *expr;
    bool        ret;
    uint_32     loclist;
    uint_32     context;
    int         addr_size;

    addr_size = DR_GetAddrSize( DR_FindCompileUnit( info ) );
    form = DR_VMReadULEB128( &abbrev );
    ret = true;
    for( ;; ) {
        switch( form ) {
        case DW_FORM_block1:
            size  = DR_VMReadByte( info );
            info += sizeof(unsigned_8);
            goto end_loop;
        case DW_FORM_block2:
            size = DR_VMReadWord( info );
            info += sizeof(unsigned_16);
            goto end_loop;
        case DW_FORM_block4:
            size = DR_VMReadDWord( info );
            info += sizeof(unsigned_32);
            goto end_loop;
        case DW_FORM_block:
            size = DR_VMReadULEB128( &info );
            goto end_loop;
        case DW_FORM_indirect:
            form = DR_VMReadULEB128( &info );
            break;
        case DW_FORM_ref_addr:
        case DW_FORM_data4:
            if( !callbck->live( d, &context ) ) {
                ret = false;
                goto exit;
            }
            loclist = DR_VMReadDWord( info );
            info = SearchLocList( loclist, context, addr_size );
            if( info == DRMEM_HDL_NULL ) {
                ret = false;
                goto exit;
            }
            form = DW_FORM_block2;
            break;
        case DW_FORM_data2: //TODO kludge
            size = DR_VMReadWord( info );
            ret = callbck->ref( d, size, addr_size, DR_LOC_ADDR );
            goto exit;
        default:
            ret = false;
            goto exit;
        }
    } end_loop:;
    if( size > sizeof( loc_buff ) ) {
        expr = DR_ALLOC( size );
    } else if( size > 0 ) {
        expr = loc_buff;
    } else {
        ret = false;
        goto exit;
    }
    ret = true;
    DR_VMRead( info, expr, size );
    DoLocExpr( expr, size, addr_size, callbck, d, var );
    if( size > sizeof( loc_buff ) ) {
        DR_FREE( expr );
    }
exit:
    return( ret );
}

bool DRENTRY DRLocBasedAT( drmem_hdl var, dr_loc_callbck *callbck, void *d )
/**************************************************************************/
{
    dw_tagnum       tag;
    dw_atnum        at;
    drmem_hdl       abbrev;
    drmem_hdl       sym = var;
    bool            ret;

    tag = DR_ReadTag( &var, &abbrev );
    abbrev++;   /* skip child flag */
    switch( tag ) {
    case DW_TAG_member:
    case DW_TAG_inheritance:
        at = DW_AT_data_member_location;
        break;
    case DW_TAG_subprogram:
        at = DW_AT_vtable_elem_location;
        break;
    default:
        return( false );
    }
    if( DR_ScanForAttrib( &abbrev, &var, at ) ) {
        ret = DR_LocExpr( sym, abbrev, var, callbck, d );
    } else {
        if( tag == DW_TAG_member ) {
            // For members of a union, it is valid not to have any
            // DW_AT_data_member_location attribute (we might want to check
            // that we are in fact dealing with a union here). Just create
            // a dummy location "+0".
            uint_8      dummy_loc[] = { DW_OP_lit0, DW_OP_plus };
            int         addr_size;

            addr_size = DR_GetAddrSize( DR_FindCompileUnit( var ) );
            DoLocExpr( dummy_loc, sizeof( dummy_loc ), addr_size, callbck, d, var );
            ret = true;
        } else {
            ret = false;
        }
    }
    return( ret );
}

bool DRENTRY DRLocationAT( drmem_hdl var, dr_loc_callbck *callbck, void *d )
/**************************************************************************/
{
    dw_tagnum       tag;
    dw_atnum        at;
    drmem_hdl       abbrev;
    drmem_hdl       sym = var;
    bool            ret;

    tag = DR_ReadTag( &var, &abbrev );
    abbrev++;   /* skip child flag */
    switch( tag ) {
    case DW_TAG_common_block:
    case DW_TAG_formal_parameter:
    case DW_TAG_variable:
    case DW_TAG_subprogram:
        at = DW_AT_location;
        break;
    case DW_TAG_string_type:
        at = DW_AT_string_length;
        break;
    default:
        return( false );
    }
    if( DR_ScanForAttrib( &abbrev, &var, at ) ) {
        ret = DR_LocExpr( sym, abbrev, var, callbck, d );
    } else {
        ret = false;
    }
    return( ret );
}

bool DRENTRY DRParmEntryAT( drmem_hdl var, dr_loc_callbck *callbck, void *d )
/***************************************************************************/
{
    drmem_hdl       abbrev;
    drmem_hdl       sym = var;
    bool            ret;

    abbrev = DR_SkipTag( &var ) + 1;
    if( DR_ScanForAttrib( &abbrev, &var, DW_AT_WATCOM_parm_entry ) ) {
        ret = DR_LocExpr( sym, abbrev, var, callbck, d );
    } else {
        ret = false;
    }
    return( ret );
}

drmem_hdl DRENTRY DRStringLengthAT( drmem_hdl str )
/*************************************************/
{
    drmem_hdl   abbrev;

    abbrev = DR_SkipTag( &str ) + 1;
    if( DR_ScanForAttrib( &abbrev, &str, DW_AT_string_length ) ) {
         return( str );
    } else {
         return( 0 );
    }
}

bool DRENTRY DRRetAddrLocation( drmem_hdl var, dr_loc_callbck *callbck, void *d )
/*******************************************************************************/
{
    drmem_hdl   abbrev;
    drmem_hdl   sym = var;
    bool        ret;

    abbrev = DR_SkipTag( &var ) + 1;
    if( DR_ScanForAttrib( &abbrev, &var, DW_AT_return_addr ) ) {
        ret = DR_LocExpr( sym, abbrev, var, callbck, d );
    } else {
        ret = false;
    }
    return( ret );
}

bool DRENTRY DRSegLocation( drmem_hdl var, dr_loc_callbck *callbck, void *d )
/***************************************************************************/
{
    drmem_hdl   abbrev;
    drmem_hdl   sym = var;
    bool        ret;

    abbrev = DR_SkipTag( &var ) + 1;
    if( DR_ScanForAttrib( &abbrev, &var, DW_AT_segment ) ) {
        ret = DR_LocExpr( sym, abbrev, var, callbck, d );
    } else {
        ret = false;
    }
    return( ret );
}
