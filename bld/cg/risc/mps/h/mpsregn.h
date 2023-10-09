/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2023      The Open Watcom Contributors. All Rights Reserved.
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
* Description:  MIPS register numbers.
*
****************************************************************************/


typedef enum {
    GPR_IDX  = 0x01,    /* general purpose registers */
    FPR_IDX  = 0x02,    /* floating-point registers */
    QW_IDX   = 0x04,    /* 64-bit general purpose registers (MIPS 32-bit) */
} reg_cls;

typedef enum {
    #define pick(id,name) DW_REG_ ## id,
    #include "dwregmps.h"
    #undef pick
    DW_REG_END
} dw_regs;

/*
 * Dwarf debug information
 *
 * remaping code generator QWord pseudo registers
 * to first of 32-bit pair-register
 */
#define DW_REG_Q2   DW_REG_R2
#define DW_REG_Q4   DW_REG_R4
#define DW_REG_Q6   DW_REG_R6
#define DW_REG_Q8   DW_REG_R8
#define DW_REG_Q10  DW_REG_R10

typedef uint_8      reg_idx;

typedef struct arch_reg_info {
    hw_reg_set      hw_reg;
    reg_idx         idx;
    reg_cls         cls;
    dw_regs         dw_idx;
} arch_reg_info;

extern const arch_reg_info  RegsTab[];

extern reg_idx      RegTrans( hw_reg_set reg );
extern dw_regs      RegTransDW( hw_reg_set reg );
