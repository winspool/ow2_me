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
* Description:  Type management functions.
*
****************************************************************************/


#include "cvars.h"

extern  unsigned SymTypedef;

static TYPEPTR StructDecl(DATA_TYPE,bool);
//static TYPEPTR ComplexDecl(int,bool);
static void      SetPlainCharType( DATA_TYPE char_type );
static void CheckBitfieldType( TYPEPTR typ );

#if _CPU == 386
#define _CHECK_SIZE( s )
#else
#define _CHECK_SIZE( s ) \
        if( (s) > 0xffff ) {\
            CErr1( ERR_MAX_STRUCT_SIZE_IS_64K );\
        }
#endif

/*
 * matches enum DataType in ctypes.h
 */
static unsigned char  CTypeSizes[] = {
    #define pick1(type,dtype,cgtype,x86asmtype,name,size) size,
    #include "cdatatyp.h"
    #undef  pick1
};

TYPEPTR CTypeHash[DATA_TYPE_SIZE];
TYPEPTR PtrTypeHash[DATA_TYPE_SIZE];

TAGPTR  TagHash[ID_HASH_SIZE + 1];
FIELDPTR FieldHash[ID_HASH_SIZE];

typedef enum {
    M_NONE          = 0,
    M_CHAR          = 0x0001,
    M_INT           = 0x0002,
    M_SHORT         = 0x0004,
    M_LONG          = 0x0008,
    M_SIGNED        = 0x0010,
    M_UNSIGNED      = 0x0020,
    M_FLOAT         = 0x0040,
    M_DOUBLE        = 0x0080,
    M_LONG_LONG     = 0x0100,
    M_VOID          = 0x0200,
    M_COMPLEX       = 0x0400,
    M_IMAGINARY     = 0x0800,
    M_BOOL          = 0x1000,
} type_mask;

void InitTypeHashTables( void )
{
    parm_hash_idx   hash1;
    id_hash_idx     hash2;
    DATA_TYPE       base_type;

    for( hash1 = 0; hash1 <= MAX_PARM_LIST_HASH_SIZE; hash1++ ) {
        FuncTypeHead[hash1] = NULL;
    }
    for( base_type = 0; base_type < DATA_TYPE_SIZE; ++base_type ) {
        CTypeHash[base_type] = NULL;
        PtrTypeHash[base_type] = NULL;
    }
    for( hash2 = 0; hash2 < ID_HASH_SIZE; hash2++ ) {
        TagHash[hash2] = NULL;
        FieldHash[hash2] = NULL;
    }
    TagHash[ID_HASH_SIZE] = NULL;
}

void CTypeInit( void )
{
    DATA_TYPE   base_type;

    TypeHead = NULL;
    TagCount = 0;
    FieldCount = 0;
    EnumCount = 0;
    InitTypeHashTables();
    for( base_type = 0; base_type < DATA_TYPE_SIZE; ++base_type ) {
        CTypeCounts[base_type] = 0;
#if 0
        if ( base_type == TYP_FCOMPLEX || base_type == TYP_DCOMPLEX || base_type == TYP_LDCOMPLEX ) {
            BaseTypes[base_type] = ComplexDecl( TYP_STRUCT, false );
            BaseTypes[base_type]->decl_type = base_type;
        } else {
#endif
            if( CTypeSizes[base_type] != 0 || base_type == TYP_VOID || base_type == TYP_DOT_DOT_DOT ) {
                BaseTypes[base_type] = TypeNode( base_type, NULL );
            } else {
                BaseTypes[base_type] = NULL;
            }
#if 0
        }
#endif
    }
    SetPlainCharType( TYP_UCHAR );
    StringArrayType = NULL;
    VoidParmList[0] = BaseTypes[TYP_VOID];
    VoidParmList[1] = NULL;
}


TYPEPTR GetType( DATA_TYPE base_type )
{
    TYPEPTR     typ;

    typ = BaseTypes[base_type];
    if( typ == NULL ) {
        typ = TypeNode( base_type, NULL );
    }
    return( typ );
}

void WalkFuncTypeList( void (*func)(TYPEPTR,int) )
{
    TYPEPTR         typ;
    parm_hash_idx   hash;

    for( hash = 0; hash <= MAX_PARM_LIST_HASH_SIZE; hash++ ) {
        for( typ = FuncTypeHead[hash]; typ != NULL; typ = typ->next_type ) {
            func( typ, hash );
        }
    }
}

void WalkTypeList( void (*func)(TYPEPTR) )
{
    TYPEPTR     typ;
    DATA_TYPE   base_type;

    for( base_type = 0; base_type < DATA_TYPE_SIZE; ++base_type ) {
        for( typ = CTypeHash[base_type]; typ != NULL; typ = typ->next_type ) {
            func( typ );
        }
    }
    for( base_type = 0; base_type < DATA_TYPE_SIZE; ++base_type ) {
        for( typ = PtrTypeHash[base_type]; typ != NULL; typ = typ->next_type ) {
            func( typ );
        }
    }
}

static TYPEPTR DupType( TYPEPTR typ, enum type_state flags, bool force_duplicate )
{
    TYPEPTR     newtype;
    TYPEPTR     next;

    if( !force_duplicate ) {
        if( typ->decl_type == TYP_POINTER ) {
            next = PtrTypeHash[typ->object->decl_type];
        } else {
            next = CTypeHash[typ->decl_type];
        }
        for( ; next != NULL; next = next->next_type ) {
            if( next->decl_type == typ->decl_type
              && next->object == typ->object
              && next->u.tag == typ->u.tag
              && next->type_flags == flags ) {
                return( next );
            }
        }
    }
    newtype = TypeNode( typ->decl_type, typ->object );
    next = newtype->next_type;
    memcpy( newtype, typ, sizeof( TYPEDEFN ) );
    newtype->next_type = next;
    newtype->type_flags = flags;
    return( newtype );
}

static void SetPlainCharType( DATA_TYPE char_type )
{
    TYPEPTR     typ;

    typ = TypeNode( char_type, NULL );
    typ->type_flags = TF2_TYP_PLAIN_CHAR;
    BaseTypes[TYP_PLAIN_CHAR] = typ;
    StringType = PtrNode( typ, FLAG_NONE, SEG_DATA );
    ConstCharType =  typ;
}

void SetSignedChar( void )
{
    SetPlainCharType( TYP_CHAR );
}


type_modifiers TypeQualifier( void )
{
    type_modifiers   flags;
    type_modifiers   bit;

    flags = 0;
    bit = 0;
    for( ;; ) {
        switch( CurToken ) {
        case T_CONST:
            bit = FLAG_CONST;
            break;
        case T_VOLATILE:
            bit = FLAG_VOLATILE;
            break;
        case T_RESTRICT:
        case T___RESTRICT:
            bit = FLAG_RESTRICT;
            break;
        case T___UNALIGNED:
            bit = FLAG_UNALIGNED;
            break;
        default:
            return( flags );
        }
        if( (flags & bit)
          && ( CompVars.cstd < STD_C99 ) )
            CErr1( ERR_REPEATED_MODIFIER );
        flags |= bit;
        NextToken();
    }
}

static TYPEPTR GetScalarType( bool *plain_int, type_mask bmask, type_modifiers flags )
{
    DATA_TYPE   data_type;
    TYPEPTR     typ;

    if( (bmask & M_INT) && (bmask & (M_LONG_LONG | M_LONG | M_SHORT) ) ) {
        bmask &= ~M_INT;
    }
    switch( bmask ) {
    case M_CHAR:
        data_type = TYP_PLAIN_CHAR;
        break;
    case M_SIGNED | M_CHAR:
        data_type = TYP_CHAR;
        break;
    case M_UNSIGNED | M_CHAR:
        data_type = TYP_UCHAR;
        break;
    case M_SHORT:
    case M_SIGNED | M_SHORT:
        data_type = TYP_SHORT;
        break;
    case M_UNSIGNED | M_SHORT:
        data_type = TYP_USHORT;
        break;
    case M_NONE:
    case M_INT:
        *plain_int = true;
        /* fall through */
    case M_SIGNED:
    case M_SIGNED | M_INT:
        data_type = TYP_INT;
        break;
    case M_UNSIGNED:
    case M_UNSIGNED | M_INT:
        data_type = TYP_UINT;
        break;
    case M_LONG:
    case M_SIGNED | M_LONG:
        data_type = TYP_LONG;
        break;
    case M_UNSIGNED | M_LONG:
        data_type = TYP_ULONG;
        break;
    case M_LONG_LONG:
    case M_SIGNED | M_LONG_LONG:
        data_type = TYP_LONG64;
        break;
    case M_UNSIGNED | M_LONG_LONG:
        data_type = TYP_ULONG64;
        break;
    case M_BOOL:
        data_type = TYP_BOOL;
        break;
    case M_VOID:
        data_type = TYP_VOID;
        break;
    case M_FLOAT:
        data_type = TYP_FLOAT;
        break;
    case M_LONG | M_DOUBLE:
        if( CompFlags.use_long_double ) {
            data_type = TYP_LONG_DOUBLE;
            break;
        }
        /* fall through */
    case M_DOUBLE:
        data_type = TYP_DOUBLE;
        break;
    case M_COMPLEX | M_FLOAT:
        data_type = TYP_FCOMPLEX;
        break;
    case M_COMPLEX | M_LONG | M_DOUBLE:
        if( CompFlags.use_long_double ) {
            data_type = TYP_LDCOMPLEX;
            break;
        }
        /* fall through */
    case M_COMPLEX | M_DOUBLE:
        data_type = TYP_DCOMPLEX;
        break;
    case M_IMAGINARY | M_FLOAT:
        data_type = TYP_FIMAGINARY;
        break;
    case M_IMAGINARY | M_LONG | M_DOUBLE:
        if( CompFlags.use_long_double ) {
            data_type = TYP_LDIMAGINARY;
            break;
        }
        /* fall through */
    case M_IMAGINARY | M_DOUBLE:
        data_type = TYP_DIMAGINARY;
        break;
    default:
        CErr1( ERR_INV_TYPE );
        data_type = TYP_INT;
        break;
    }
    typ = GetType( data_type );
    if( flags & FLAG_SEGMENT )
        typ = DupType( typ, TF2_TYP_SEGMENT, false );

    return( typ );
}


static void AdvanceToken( void )
{
    if( CurToken == T_SAVED_ID ) {
        CMemFree( SavedId );
        SavedId = NULL;
        CurToken = LAToken;
    } else {
        NextToken();
    }
}

static void DeclSpecifiers( bool *plain_int, decl_info *info )
{
    TYPEPTR             typ;
    type_mask           bmask;
    type_mask           bit;
    type_modifiers      flags;
    bool                packed;
    SYM_HANDLE          sym_handle;
    stg_classes         stg_class;
    stg_classes         specified_stg_class;
    SYM_ENTRY           sym;
    declspec_class      decl;
    type_modifiers      modifier;

    *plain_int = false;
    info->mod = FLAG_NONE;
    info->decl_mod = FLAG_NONE;
    info->decl = DECLSPEC_NONE;
    info->naked = false;
    info->segid = SEG_NULL;
    bmask = M_NONE;
    flags = FLAG_NONE;
    packed = false;
    typ = NULL;
    specified_stg_class = SC_NONE;
    for(;;) {
        stg_class = SC_NONE;
        bit = M_NONE;
        switch( CurToken ) {
        case T_CHAR:
            bit = M_CHAR;
            break;
        case T_INT:
            bit = M_INT;
            break;
        case T_SHORT:
            bit = M_SHORT;
            break;
        case T_LONG:
            bit = M_LONG;
            break;
        case T___INT8:
            bit = M_CHAR;
            break;
        case T___INT16:
            bit = M_SHORT;
            break;
        case T___INT32:
#if _CPU == 8086
            bit = M_LONG;
#else
            bit = M_INT;
#endif
            break;
        case T___INT64:
            bit = M_LONG_LONG;
            break;
        case T_SIGNED:
            bit = M_SIGNED;
            break;
        case T_UNSIGNED:
            bit = M_UNSIGNED;
            break;
        case T_FLOAT:
            bit = M_FLOAT;
            break;
        case T_DOUBLE:
            bit = M_DOUBLE;
            break;
        case T_VOID:
            bit = M_VOID;
            break;
        case T__COMPLEX:
            bit = M_COMPLEX;
            break;
        case T__IMAGINARY:
            bit = M_IMAGINARY;
            break;
        case T__BOOL:
            bit = M_BOOL;
            break;
        case T_INLINE:
        case T___INLINE:
            flags |= FLAG_INLINE;
            break;
        case T__PACKED:
            if( packed )
                CErr1( ERR_REPEATED_MODIFIER );
            packed = true;
            break;
        case T_EXTERN:
            stg_class = SC_EXTERN;
            break;
        case T_STATIC:
            stg_class = SC_STATIC;
            break;
        case T_TYPEDEF:
            stg_class = SC_TYPEDEF;
            break;
        case T_AUTO:
            stg_class = SC_AUTO;
            break;
        case T_REGISTER:
            stg_class = SC_REGISTER;
            break;
        case T_STRUCT:
            if( typ != NULL )
                CErr1( ERR_INV_TYPE );
            typ = StructDecl( TYP_STRUCT, packed );
            packed = false;
            continue;
        case T_UNION:
            if( typ != NULL )
                CErr1( ERR_INV_TYPE );
            typ = StructDecl( TYP_UNION, packed );
            packed = false;
            continue;
        case T_ENUM:
            if( typ != NULL )
                CErr1( ERR_INV_TYPE );
            typ = EnumDecl( flags );
            continue;
        case T___SEGMENT:
            bit = M_UNSIGNED | M_SHORT;
            flags |= FLAG_SEGMENT;
            break;

        case T___DECLSPEC:
            AdvanceToken();                   // declspec( dllimport naked )
            MustRecog( T_LEFT_PAREN );
            while( CurToken != T_RIGHT_PAREN ) {
                modifier = 0;
                switch( CurToken ) {
                case T___WATCALL:
                    modifier = FLAG_WATCALL;
                    break;
                case T__CDECL:
                case T___CDECL:
                    modifier = FLAG_CDECL;
                    break;
                case T__PASCAL:
                case T___PASCAL:
                    modifier = FLAG_PASCAL;
                    break;
                case T___FORTRAN:
                    modifier = FLAG_FORTRAN;
                    break;
                case T__SYSCALL:
                case T___SYSCALL:
                case T__SYSTEM:
                    modifier = FLAG_SYSCALL;
                    break;
                case T___STDCALL:
                    modifier = FLAG_STDCALL;
                    break;
                case T__FASTCALL:
                case T___FASTCALL:
                    modifier = FLAG_FASTCALL;
                    break;
                case T__OPTLINK:
                    modifier = FLAG_OPTLINK;
                    break;
                case T_ID:
                    decl = DECLSPEC_NONE;
                    if( info->stg == SC_NONE ) {
                        CErr1( ERR_INVALID_DECLARATOR );
                        break;
                    }
                    if( strcmp( Buffer, "dllimport" ) == 0 ) {
                        decl = DECLSPEC_DLLIMPORT;
                    } else if( strcmp( Buffer, "overridable" ) == 0 ) {
                        decl = DECLSPEC_DLLIMPORT;
                    } else if( strcmp( Buffer, "dllexport" ) == 0 ) {
                        decl = DECLSPEC_DLLEXPORT;
                    } else if( strcmp( Buffer, "thread" ) == 0 ) {
                        decl = DECLSPEC_THREAD;
                    } else if( strcmp( Buffer, "naked" ) == 0 ) {
                        if( info->naked ) {
                            CErr1( ERR_INVALID_DECLSPEC );
                        } else {
                            info->naked = true;
                        }
                    } else if( strcmp( Buffer, "aborts" ) == 0 ) {
                        modifier = FLAG_ABORTS;
                    } else if( strcmp( Buffer, "noreturn" ) == 0 ) {
                        modifier = FLAG_NORETURN;
                    } else if( strcmp( Buffer, "farss" ) == 0 ) {
                        modifier = FLAG_FARSS;
                    } else {
                        CErr1( ERR_INVALID_DECLSPEC );
                    }
                    if( decl != DECLSPEC_NONE ) {
                        if( info->decl == DECLSPEC_NONE ) {
                            info->decl = decl;
                        } else {
                            CErr1( ERR_INVALID_DECLSPEC );
                        }
                    }
                    break;
                default:
                    CErr1( ERR_INVALID_DECLSPEC );
                    goto done;
                }
                if( modifier & MASK_LANGUAGES ) {
                    if( info->decl_mod & MASK_LANGUAGES ) {
                        CErr1( ERR_INVALID_DECLSPEC );
                    } else {
                        info->decl_mod |= modifier;
                    }
                }
                if( modifier & FLAG_ABORTS ) {
                    if( info->decl_mod & FLAG_ABORTS ) {
                        CErr1( ERR_INVALID_DECLSPEC );
                    } else {
                        info->decl_mod |= modifier;
                    }
                }
                if( modifier & FLAG_NORETURN ) {
                    if( info->decl_mod & FLAG_NORETURN ) {
                        CErr1( ERR_INVALID_DECLSPEC );
                    } else {
                        info->decl_mod |= modifier;
                    }
                }
                if( modifier & FLAG_FARSS ) {
                    if( info->decl_mod & FLAG_FARSS ) {
                        CErr1( ERR_INVALID_DECLSPEC );
                    } else {
                        info->decl_mod |= modifier;
                    }
                }
                NextToken();
            }
         done:
            MustRecog( T_RIGHT_PAREN );
            continue;
        case T_SAVED_ID:
        case T_ID:
            if( typ != NULL || bmask != M_NONE )
                goto got_specifier;
            /*
             * lookup id in symbol table
             * if valid type identifier then OK
             */
            if( CurToken == T_ID ) {
                sym_handle = SymLookTypedef( CalcHashID( Buffer ), Buffer, &sym );
            } else {    /* T_SAVED_ID */
                sym_handle = SymLookTypedef( CalcHashID( SavedId ), SavedId, &sym );
            }
            if( sym_handle == SYM_NULL )
                goto got_specifier;
            if( sym.attribs.stg_class != SC_TYPEDEF )
                goto got_specifier;
            if( SymLevel != 0 && flags == 0 ) {
                if( CurToken == T_ID ) {
                    LookAhead();
                    if( LAToken == T_COLON ) {
                        goto got_specifier;
                    }
                }
            }
            ++SymTypedef;
            typ = sym.sym_type;
            SymGet( &sym, sym_handle );  // get rest of sym from nutty sym table
            if( flags & sym.mods ) {
                CErr1( ERR_INV_TYPE );
            }
            flags |= sym.mods;
            if( sym.mods & FLAG_BASED ) {
                info->segid = sym.u.var.segid;
            }
            if( sym.attribs.declspec != DECLSPEC_NONE ) {
                if( info->decl == DECLSPEC_NONE ) {
                    info->decl = sym.attribs.declspec;
                } else {
                    CErr1( ERR_INVALID_DECLSPEC );
                }
            }
            if( sym.attribs.naked ) {
                if( info->naked ) {
                    CErr1( ERR_INVALID_DECLSPEC );
                } else {
                    info->naked = true;
                }
            }
            AdvanceToken();
            continue;
        default:
            modifier = TypeQualifier();
            if( modifier ) {
                flags |= modifier;
                continue;
            }
            goto got_specifier;
        }
        if( stg_class != SC_NONE ) {
            if( info->stg == SC_NONE )
                break;       // don't want any stg class
            if( specified_stg_class != SC_NONE ) {
                CErr1( ERR_TOO_MANY_STORAGE_CLASS_SPECIFIERS );
            }
            specified_stg_class = stg_class;
        }
        if( bmask & bit ) {
            if( bit == M_LONG ) {
                // long long found
                bmask &= ~M_LONG;
                bit = M_LONG_LONG;
            } else {
                CErr1( ERR_INV_TYPE );
            }
        }
        bmask |= bit;
        NextToken();
    }
got_specifier:
    info->stg = specified_stg_class;
    if( typ != NULL ) {
        /*
         * already have a type (TYP_STRUCT, TYP_UNION, TYP_ENUM)
         * or an ID that was a typedef name
         */
        if( bmask != M_NONE ) {
            CErr1( ERR_INV_TYPE );  // picked up an int
        }
    } else {
        if( flags != FLAG_NONE || bmask != M_NONE ) {  // not just id hanging there
            typ = GetScalarType( plain_int, bmask, flags );
        }
    }
    info->typ = typ;
    info->mod = flags;
}

void TypeSpecifier( decl_info *info )
{
    bool                plain_int;

    info->stg = SC_NONE;      // indicate don't want any storage class specifiers
    DeclSpecifiers( &plain_int, info );
}

static void GetFieldTypeSpecifier( bool *plain_int, decl_info *info )
{

    info->stg = SC_NONE;      // indicate don't want any storage class specifiers
    DeclSpecifiers( plain_int, info );
}

void FullDeclSpecifier( decl_info *info )
{
    bool        plain_int;

    info->stg = SC_FORWARD;    // indicate want storage class specifiers
    DeclSpecifiers( &plain_int, info );
}

TYPEPTR TypeDefault( void )
{
    return( GetType( TYP_INT ) );
}


static TAGPTR NewTag( id_hash_idx hash, const char *name )
{
    TAGPTR      tag;

    tag = (TAGPTR)CPermAlloc( sizeof( TAGDEFN ) + strlen( name ) );
    tag->level = (id_level_type)SymLevel;
    tag->hash = hash;
    tag->next_tag = TagHash[hash];
    TagHash[hash] = tag;
    strcpy( tag->name, name );
    ++TagCount;
    return( tag );
}


TAGPTR NullTag( void )
{
    return( NewTag( ID_HASH_SIZE, "" ) );
}


TAGPTR VfyNewTag( TAGPTR tag, DATA_TYPE tag_type )
{
    if( tag->sym_type != NULL ) {               /* tag already exists */
        if( !CheckEqSymLevel( tag ) ) {
            tag = NewTag( tag->hash, tag->name );
        } else if( tag->size != 0 || tag->sym_type->decl_type != tag_type ) {
            CErr2p( ERR_DUPLICATE_TAG, tag->name );
        }
    }
    return( tag );
}


static FIELDPTR NewField( FIELDPTR new_field, TYPEPTR decl )
{
    FIELDPTR    field;
    FIELDPTR    prev_field;
    TYPEPTR     typ;
    TAGPTR      tag;
    id_hash_idx hash;

    ++FieldCount;
    typ = new_field->field_type;
    if( typ != NULL ) {
        SKIP_TYPEDEFS( typ );
    }
    if( new_field->name[0] == '\0' ) {
        /*
         * allow nameless structs and unions
         */
        if( (typ->decl_type != TYP_STRUCT && typ->decl_type != TYP_UNION)
          || !CompFlags.extensions_enabled ) {
            CErr1( ERR_INVALID_DECLARATOR );
        }
    }
    if( typ == decl ) {
        CErr1( ERR_STRUCT_OR_UNION_INSIDE_ITSELF );
    } else if( SizeOfArg( typ ) == 0 ) {   /* was TypeSize(typ) */
        /*
         * can't have an array of incomplete type
         */
        if( typ->decl_type == TYP_ARRAY
          && ( SizeOfArg( typ->object ) == 0 || !CompFlags.extensions_enabled )
          || typ->decl_type != TYP_ARRAY ) {
            CErr2p( ERR_INCOMPLETE_TYPE, new_field->name );
        }
    }
    tag = decl->u.tag;
    hash = CalcHashID( new_field->name );
    new_field->hash = hash;
    if( new_field->name[0] != '\0' ) {  /* only check non-empty names */
        for( field = FieldHash[hash]; field != NULL; field = field->next_field_same_hash ) {
            /*
             * fields were added at the front of the hash linked list --
             * may as well stop if the level isn't the same anymore
             */
            if( field->level != new_field->level )
                break;
            if( strcmp( field->name, new_field->name ) == 0 ) {
                CErr2p( ERR_DUPLICATE_FIELD_NAME, field->name );
            }
        }
        new_field->next_field_same_hash = FieldHash[hash];
        FieldHash[hash] = new_field;
    }
    if( tag->u.field_list == NULL ) {
        tag->u.field_list = new_field;
    } else {
        prev_field = tag->u1.last_field;
        prev_field->next_field = new_field;
        if( SizeOfArg( prev_field->field_type ) == 0 ) {
            CErr2p( ERR_INCOMPLETE_TYPE, prev_field->name );
        }
    }
    tag->u1.last_field = new_field;
    return( new_field );
}


static TYPEPTR EnumFieldType( TYPEPTR ftyp, bool plain_int,
                    bitfield_width bits_start, bitfield_width bits_width )
{
    TYPEPTR     typ;
    DATA_TYPE   data_type;

    typ = TypeNode( TYP_FIELD, NULL );
    typ->u.f.field_start = bits_start;
    typ->u.f.field_width = bits_width;
    if( plain_int ) {
        data_type = TYP_INT;    /* default to signed bit fields */
    } else {
        SKIP_TYPEDEFS( ftyp );
        if( ftyp->decl_type == TYP_ENUM ) {
            ftyp = ftyp->object;
        }
        data_type = ftyp->decl_type;
    }
    typ->u.f.field_type = data_type;
    switch( data_type ) {
    case TYP_BOOL:
    case TYP_UCHAR:
    case TYP_USHORT:
    case TYP_UINT:
    case TYP_ULONG:
    case TYP_ULONG64:
        typ->decl_type = TYP_UFIELD;
        break;
    }
    return( typ );
}


align_type GetTypeAlignment( TYPEPTR typ )
{
    align_type  size;

    while( typ->decl_type == TYP_TYPEDEF || typ->decl_type == TYP_ARRAY ) {
        typ = typ->object;
    }
    if( typ->decl_type == TYP_STRUCT || typ->decl_type == TYP_UNION ) {
        size = typ->u.tag->alignment;
#if _CPU == _AXP
        if( CompFlags.align_structs_on_qwords ) {
            size = 8;
        }
#endif
    } else {
        size = (align_type)TypeSize( typ );
    }
    return( size );
}


static target_size FieldAlign( target_size next_offset, FIELDPTR field, align_type *worst_alignment )
{
    align_type  pack_adjustment;
    align_type  align;
    target_size old_offset;

    pack_adjustment = PackAmount;
    align = GetTypeAlignment( field->field_type );
    if( align > pack_adjustment ) { // can't be any bigger than pack( x )
        align = pack_adjustment;
    }
    if( align > *worst_alignment ) {
        *worst_alignment = align;
    }
    if( align != 1 ) {
        old_offset = next_offset;
        next_offset = _RoundUp( next_offset, align );
        if( CompFlags.slack_byte_warning && (next_offset - old_offset) ) {
            CWarn2( ERR_SLACK_ADDED, (unsigned)( next_offset - old_offset ) );
        }
    }
    field->offset = next_offset;
    return( next_offset );
}

static DATA_TYPE UnQualifiedType( TYPEPTR typ )
{
    // skip typedefs, go into enum base
    typ = SkipTypeFluff( typ );
    switch( typ->decl_type ) {
    case TYP_BOOL:
        return( TYP_BOOL );
    case TYP_CHAR:
    case TYP_UCHAR:
        return( TYP_CHAR );
    case TYP_SHORT:
    case TYP_USHORT:
        return( TYP_SHORT );
    case TYP_INT:
    case TYP_UINT:
        return( TYP_INT );
    case TYP_LONG:
    case TYP_ULONG:
        return( TYP_LONG );
    case TYP_LONG64:
    case TYP_ULONG64:
        return( TYP_LONG64 );
    default:
        break;
    }
    return( TYP_UNDEFINED );
}

static void ClearFieldHashTable( TAGPTR tag )
/********************************************
 * clear the hash table of all fields that were just defined
 * in the struct with tag tag
 */
{
    FIELDPTR field;
    FIELDPTR hash_field;
    FIELDPTR prev_field;

    for( field = tag->u.field_list; field != NULL; field = field->next_field ) {
        prev_field = NULL;
        hash_field = FieldHash[field->hash];
        if( hash_field == field ) {
            /*
             * first entry: easy kick out
             */
            FieldHash[field->hash] = field->next_field_same_hash;
        } else {
            while( hash_field != NULL ) {
                /*
                 * search for candidate to kick
                 */
                prev_field = hash_field;
                hash_field = hash_field->next_field_same_hash;
                if( hash_field == field ) {
                     hash_field = hash_field->next_field_same_hash;
                     prev_field->next_field_same_hash = hash_field;
                }
            }
        }
    }
}

static bool CheckAdjModsTypeNode( type_modifiers old_mod, type_modifiers decl_mod )
{
    bool        adjust;

    adjust = false;
    if( (decl_mod & MASK_LANGUAGES) && (old_mod & MASK_LANGUAGES) != (decl_mod & MASK_LANGUAGES) ) {
        if( old_mod & MASK_LANGUAGES ) {
            CErr1( ERR_INVALID_DECLSPEC );
        } else {
            adjust = true;
        }
    }
    if( (decl_mod & FLAG_ABORTS) && (old_mod & FLAG_ABORTS) == 0 ) {
        adjust = true;
    }
    if( (decl_mod & FLAG_NORETURN) && (old_mod & FLAG_NORETURN) == 0 ) {
        adjust = true;
    }
    if( (decl_mod & FLAG_FARSS) && (old_mod & FLAG_FARSS) == 0 ) {
        adjust = true;
    }
    return( adjust );
}

void AdjModsTypeNode( TYPEPTR *ptyp, type_modifiers decl_mod, SYMPTR sym )
{
    if( decl_mod ) {
        TYPEPTR     typ;

        if( sym != NULL ) {
            if( CheckAdjModsTypeNode( sym->mods, decl_mod ) ) {
                sym->mods |= decl_mod;
            }
        }
        typ = *ptyp;
        if( typ->decl_type == TYP_FUNCTION ) {
            if( CheckAdjModsTypeNode( typ->u.fn.decl_flags, decl_mod ) ) {
                *ptyp = FuncNode( typ->object, typ->u.fn.decl_flags | decl_mod, typ->u.fn.parms );
            }
        }
    }
}

static target_size GetFields( TYPEPTR decl )
{
    target_size         start;
    TYPEPTR             typ;
    decl_state          state;
    FIELDPTR            field;
    unsigned            scalar_size;
    unsigned            bits_available;
    unsigned            bits_total;
    target_size         struct_size;
    target_size         next_offset;
    unsigned            bits_width;
    align_type          worst_alignment;
    DATA_TYPE           unqualified_type;
    DATA_TYPE           prev_unqualified_type;
    bool                plain_int;
    decl_info           info;
    static field_level_stype struct_level = 0;
    const_val           cval;

    struct_level++;
    prev_unqualified_type = TYP_VOID;   /* so it doesn't match 1st time */
    start = 0;
    worst_alignment = 1;
    bits_available = 1;
    bits_total = 0;
    /*
     * assertion: bits_available != bits_total && ( bits_total / CHAR_BIT ) == 0
     */
    struct_size = start;
    next_offset = start;
    for(;;) {
        if( CurToken == T_SEMI_COLON && CompFlags.extensions_enabled ) {
            NextToken();
            if( CurToken == T_RIGHT_BRACE )
                break;
            continue;
        }
        GetFieldTypeSpecifier( &plain_int, &info );
        typ = info.typ;
        state = DECL_STATE_NONE;
        if( typ == NULL ) {
            state |= DECL_STATE_NOTYPE;
            CErr2p( ERR_MISSING_DATA_TYPE, Buffer );
            typ = TypeDefault();
            if( CurToken == T_ID ) {
                NextToken();
            }
        }
        unqualified_type = UnQualifiedType( typ );
        if( decl->decl_type == TYP_UNION ) {
            bits_total = TypeSize( typ ) * CHAR_BIT;
        } else if( bits_available == bits_total ) {
            bits_total = TypeSize( typ ) * CHAR_BIT;
            bits_available = bits_total;
        } else if( unqualified_type != prev_unqualified_type ) {
            next_offset += bits_total / CHAR_BIT;
            bits_total = TypeSize( typ ) * CHAR_BIT;
            bits_available = bits_total;
        }
        prev_unqualified_type = unqualified_type;
        for( ;; ) {
            if( decl->decl_type == TYP_UNION ) {
                next_offset = start;
                bits_available = bits_total;
            }
            field = NULL;
            if( CurToken != T_COLON ) {
                field = FieldDecl( typ, info.mod, state );
                field->level = struct_level;
                AdjModsTypeNode( &field->field_type, info.decl_mod, NULL );
                field = NewField( field, decl );
            }
            if( CurToken == T_COLON ) {
                if( field != NULL ) {
                    next_offset = FieldAlign( next_offset, field, &worst_alignment );
                }
                CheckBitfieldType( typ );
                NextToken();
                ConstExprAndType( &cval );
                bits_width = 0;
                if( (cval.value.u._32[I64HI32] | cval.value.u._32[I64LO32]) == 0
                  && field != NULL ) {
                    CErr1( ERR_WIDTH_0 );
                } else if( (int)cval.value.u._32[I64HI32] < 0 ) {
                    CErr1( ERR_WIDTH_NEGATIVE );
                } else if( cval.value.u._32[I64HI32] > 0
                  || cval.value.u._32[I64LO32] > ( TARGET_BITFIELD * CHAR_BIT )
                  || cval.value.u._32[I64LO32] > bits_total ) {
                    CErr1( ERR_FIELD_TOO_WIDE );
                    bits_width = TARGET_BITFIELD * CHAR_BIT;
                } else {
                    bits_width = cval.value.u._32[I64LO32];
                }
                if( bits_width > bits_available || bits_width == 0 ) {
                    scalar_size = TypeSize( typ );
                    if( bits_available != bits_total ) {
                        /*
                         * some bits have been used; abandon this unit
                         */
                        next_offset += bits_total / CHAR_BIT;
                    } else if( bits_width == 0 ) {
                        /*
                         * no bits have been used; align to base type
                         */
                        next_offset = _RoundUp( next_offset, scalar_size );
                    }
                    bits_total = scalar_size * CHAR_BIT;
                    bits_available = bits_total;
                }
                if( field != NULL ) {
                    field->offset = next_offset;
                    field->field_type = EnumFieldType( typ, plain_int,
                              (bitfield_width)( bits_total - bits_available ),
                              (bitfield_width)bits_width );
                }
                bits_available -= bits_width;
            } else {
                if( bits_available != bits_total ) { //changed from bit field to non
                    next_offset += bits_total / CHAR_BIT;
                    field->offset = next_offset;
                    bits_total = TypeSize( typ ) * CHAR_BIT;
                    bits_available = bits_total;
                }
                next_offset = FieldAlign( next_offset, field, &worst_alignment );
                next_offset += SizeOfArg( field->field_type );
            }
            if( next_offset > struct_size )
                struct_size = next_offset;
            if( CurToken != T_COMMA )
                break;
            NextToken();
        }
        if( CurToken == T_RIGHT_BRACE ) {
            CWarn1( ERR_MISSING_LAST_SEMICOLON );
        } else {
            MustRecog( T_SEMI_COLON );
        }
        if( CurToken == T_RIGHT_BRACE ) {
            break;
        }
    }
    if( bits_available != bits_total ) { /* if last field was bit field */
        next_offset += bits_total / CHAR_BIT;
        if( next_offset > struct_size ) {
            struct_size = next_offset;
        }
    }
    ClearFieldHashTable( decl->u.tag );
    decl->u.tag->alignment = worst_alignment;
    struct_size = _RoundUp( struct_size, worst_alignment );
    _CHECK_SIZE( struct_size );
    NextToken();
    struct_level--;
    return( struct_size );
}


static TYPEPTR StructDecl( DATA_TYPE decl_typ, bool packed )
{
    TYPEPTR     typ;
    TAGPTR      tag;
    align_type  saved_packamount;

    saved_packamount = PackAmount;
    if( packed )
        PackAmount = 1;
    NextToken();
    if( CurToken == T_LEFT_BRACE ) {
        tag = NullTag();
        NextToken();
    } else {
        for(;;) {
            if( CurToken == T_ID )
                break;
            if( CurToken == T_LEFT_BRACE )
                break;
            if( CurToken == T_EOF )
                break;
            ExpectStructUnionTag();
            NextToken();
        }
        if( CurToken == T_ID ) {        /* structure or union tag */
            tag = TagLookup( Buffer );
            NextToken();
        } else {
            tag = NullTag();
        }
        if( CurToken != T_LEFT_BRACE ) {
            if( CurToken == T_SEMI_COLON ) {
                if( !CheckEqSymLevel( tag ) ) {
                    tag = NewTag( tag->hash, tag->name );
                }
            }
            typ = tag->sym_type;
            if( typ == NULL ) {
                typ = TypeNode( decl_typ, NULL );
            } else {
                if( typ->decl_type != decl_typ ) {
                    CErr2p( ERR_DUPLICATE_TAG, tag->name );
                }
            }
            tag->sym_type = typ;
            typ->u.tag = tag;
            PackAmount = saved_packamount;
            return( typ );
        }
        NextToken();
        tag = VfyNewTag( tag, decl_typ );
    }
    typ = tag->sym_type;
    if( typ == NULL ) {
        typ = TypeNode( decl_typ, NULL );
        tag->sym_type = typ;
    }
    typ->u.tag = tag;
    tag->u.field_list = NULL;
    tag->size = GetFields( typ );
    PackAmount = saved_packamount;
    return( typ );
}

/*
 * Next three functions describe a struct that looks like
 * struct {
 *     double __ow_real;
 *     double _Imaginary __ow_imaginary;
 * }
 */
#if 0
static void GetComplexFieldTypeSpecifier( decl_info *info, DATA_TYPE data_type )
{
    info->stg = SC_NONE;      // indicate don't want any storage class specifiers
    info->mod = FLAG_NONE;
    info->decl_mod = FLAG_NONE;
    info->decl = DECLSPEC_NONE;
    info->naked = false;
    info->segid = SEG_NULL;
    info->typ = GetType( data_type );
}


static target_size GetComplexFields( TYPEPTR decl )
{
    target_size         start;
    TYPEPTR             typ;
    decl_state          state;
    FIELDPTR            field;
    target_size         struct_size;
    target_size         next_offset;
    align_type          worst_alignment;
    decl_info           info;

    start = 0;
    worst_alignment = 1;

    struct_size = start;
    next_offset = start;


    GetComplexFieldTypeSpecifier( &info, TYP_DOUBLE );

    field = FieldCreate( "__ow_real" );
    field->attrib = 0;
    field->field_type = info.typ;

    field = NewField( field, decl );

    typ = info.typ;
    state = DECL_STATE_NONE;

    next_offset = FieldAlign( next_offset, field, &worst_alignment );
    next_offset += SizeOfArg( field->field_type );
    if( next_offset > struct_size )
        struct_size = next_offset;

    GetComplexFieldTypeSpecifier( &info, TYP_DIMAGINARY );

    field = FieldCreate( "__ow_imaginary" );
    field->attrib = 0;
    field->field_type = info.typ;
    field = NewField( field, decl );

    typ = info.typ;
    state = DECL_STATE_NONE;

    next_offset = FieldAlign( next_offset, field, &worst_alignment );
    next_offset += SizeOfArg( field->field_type );
    if( next_offset > struct_size )
        struct_size = next_offset;

    decl->u.tag->alignment = worst_alignment;
    struct_size = _RoundUp( struct_size, worst_alignment );
    _CHECK_SIZE( struct_size );
    return( struct_size );
}


static TYPEPTR ComplexDecl( DATA_TYPE decl_typ, bool packed )
{
    TYPEPTR     typ;
    TAGPTR      tag;
    align_type  saved_packamount;

    saved_packamount = PackAmount;
    if( packed )
        PackAmount = 1;

    tag = NullTag();

    typ = tag->sym_type;
    if( typ == NULL ) {
        typ = TypeNode( decl_typ, NULL );
        tag->sym_type = typ;
    }

    typ->u.tag = tag;
    tag->u.field_list = NULL;
    tag->size = GetComplexFields( typ );
    PackAmount = saved_packamount;

    return( typ );
}
#endif

static void CheckBitfieldType( TYPEPTR typ )
{
    /*
     * skip typedefs
     */
    SKIP_TYPEDEFS( typ );
    if( CompFlags.extensions_enabled ) {
        /*
         * go into enum base
         */
        SKIP_ENUM( typ );
    }
    switch( typ->decl_type ) {
    case TYP_BOOL:
        if( !CompFlags.extensions_enabled
          && ( CompVars.cstd < STD_C99 ) ) {
            break;
        }
        /* fall through */
    case TYP_INT:
    case TYP_UINT:
        /*
         * ANSI C only allows int and unsigned [int]; C99 adds _Bool
         */
        return;
    case TYP_CHAR:
    case TYP_UCHAR:
    case TYP_SHORT:
    case TYP_USHORT:
    case TYP_LONG:
    case TYP_ULONG:
    case TYP_LONG64:
    case TYP_ULONG64:
        if( CompFlags.extensions_enabled ) {
            return;
        }
        break;
    default:
        break;
    }
    CErr1( ERR_INVALID_TYPE_FOR_FIELD );
}


void VfyNewSym( id_hash_idx hash, const char *name )
{
    SYM_HANDLE  sym_handle;
    SYM_ENTRY   sym;
    ENUMPTR     ep;

    ep = EnumLookup( hash, name );
    if( ep != NULL && CheckEqSymLevel( ep->parent ) ) {
        SetDiagEnum( ep );
        CErr2p( ERR_SYM_ALREADY_DEFINED, name );
        SetDiagPop();
    }
    sym_handle = SymLook( hash, name );
    if( sym_handle != SYM_NULL ) {
        SymGet( &sym, sym_handle );
        if( CheckEqSymLevel( &sym ) ) {
            SetDiagSymbol( &sym, sym_handle );
            CErr2p( ERR_SYM_ALREADY_DEFINED, name );
            SetDiagPop();
        }
    }
}


TAGPTR TagLookup( const char *name )
{
    TAGPTR      tag;
    id_hash_idx hash;

    hash = CalcHashID( name );
    for( tag = TagHash[hash]; tag != NULL; tag = tag->next_tag ) {
        if( strcmp( name, tag->name ) == 0 ) {
            return( tag );
        }
    }
    return( NewTag( hash, name ) );
}

void FreeTags( void )
{
    TAGPTR          tag;
    id_hash_idx     hash;

    for( hash = 0; hash <= ID_HASH_SIZE; hash++ ) {
        for( ; (tag = TagHash[hash]) != NULL; ) {
            if( CheckLtSymLevel( tag ) )
                break;
            TagHash[hash] = tag->next_tag;
            tag->next_tag = DeadTags;
            DeadTags = tag;
        }
    }
}

void WalkTagList( void (*func)(TAGPTR) )
{
    TAGPTR          tag;
    id_hash_idx     hash;

    for( hash = 0; hash <= ID_HASH_SIZE; hash++ ) {
        for( tag = TagHash[hash]; tag != NULL; tag = tag->next_tag ) {
            func( tag );
        }
    }
}

void AddTypeHash( TYPEPTR typ )
{
    if( typ->decl_type == TYP_POINTER ) {
        if( typ->object != NULL ) {
            typ->next_type = PtrTypeHash[typ->object->decl_type];
            PtrTypeHash[typ->object->decl_type] = typ;
        }
    } else if( typ->decl_type == TYP_FUNCTION ) {
        typ->next_type = NULL;
    } else {
        typ->next_type = CTypeHash[typ->decl_type];
        CTypeHash[typ->decl_type] = typ;
    }
}

void AddPtrTypeHash( TYPEPTR typ )
{
    typ->next_type = PtrTypeHash[typ->object->decl_type];
    PtrTypeHash[typ->object->decl_type] = typ;
}

TYPEPTR TypeNode( DATA_TYPE type_spec, TYPEPTR the_object )
{
    TYPEPTR     typ;

    typ = (TYPEPTR)CPermAlloc( sizeof( TYPEDEFN ) );
    typ->decl_type = type_spec;
    typ->object = the_object;
    typ->u.tag = NULL;
    AddTypeHash( typ );
    ++TypeCount;
    ++CTypeCounts[type_spec];
    return( typ );
}

TYPEPTR ArrayNode( TYPEPTR the_object )
{
    TYPEPTR     typ;

    typ = TypeNode( TYP_ARRAY, the_object );
    typ->u.array = CPermAlloc( sizeof( array_info ) );
    return( typ );
}


static TYPEPTR MkPtrNode( TYPEPTR typ, type_modifiers flags,
    segment_id segid, SYM_HANDLE base, BASED_KIND based_kind )
{
    TYPEPTR     ptrtyp;

    if( typ != NULL ) {
        ptrtyp = PtrTypeHash[typ->decl_type];
        for( ; ptrtyp != NULL; ptrtyp = ptrtyp->next_type ) {
            if( ptrtyp->decl_type == TYP_POINTER &&
                ptrtyp->object == typ &&
                ptrtyp->u.p.segid == segid &&
                ptrtyp->u.p.based_sym == base &&
                ptrtyp->u.p.based_kind == based_kind &&
                ptrtyp->u.p.decl_flags == flags ) {
                return( ptrtyp );
            }
        }
    }
    ptrtyp = TypeNode( TYP_POINTER, typ );
    ptrtyp->u.p.decl_flags = flags;
    ptrtyp->u.p.segid = segid;
    ptrtyp->u.p.based_sym = base;
    ptrtyp->u.p.based_kind = based_kind;
    return( ptrtyp );
}

TYPEPTR PtrNode( TYPEPTR typ, type_modifiers flags, segment_id segid )
{
    return( MkPtrNode( typ, flags, segid, SYM_NULL, BASED_NONE ) );
}

TYPEPTR BPtrNode( TYPEPTR typ, type_modifiers flags, segment_id segid, SYM_HANDLE base, BASED_KIND kind )
{
    return( MkPtrNode( typ, flags, segid, base, kind ) );
}

static parm_hash_idx FuncHeadIndex( TYPEPTR *parm_types )
{
    parm_hash_idx  hash;

    if( parm_types == NULL )
        return( 0 );
    for( hash = 0; hash < MAX_PARM_LIST_HASH_SIZE; hash++ ) {
        if( *parm_types == NULL ) {
            break;
        }
        ++parm_types;
    }
    return( hash );
}

TYPEPTR FuncNode( TYPEPTR return_typ, type_modifiers flag, TYPEPTR *parm_types )
{
    TYPEPTR         typ;
    parm_hash_idx   hash;

    hash = FuncHeadIndex( parm_types );
    if( return_typ != NULL ) {
        for( typ = FuncTypeHead[hash]; typ != NULL; typ = typ->next_type ) {
            if( typ->object == return_typ &&
                typ->u.fn.decl_flags == flag &&
                typ->u.fn.parms == parm_types ) {
                return( typ );
            }
        }
    }
    typ = TypeNode( TYP_FUNCTION, return_typ );
    typ->u.fn.decl_flags = flag;
    typ->u.fn.parms = parm_types;
    typ->next_type = FuncTypeHead[hash];
    FuncTypeHead[hash] = typ;
    return( typ );
}

target_size TypeSize( TYPEPTR typ )
{
    return( TypeSizeEx( typ, NULL ) );
}

target_size TypeSizeEx( TYPEPTR typ, bitfield_width *pFieldWidth )
{
    target_size size;

    if( typ == NULL )
        return( 0 );
    SKIP_TYPEDEFS( typ );
    switch( typ->decl_type ) {
    case TYP_CHAR:
    case TYP_UCHAR:
    case TYP_SHORT:
    case TYP_USHORT:
    case TYP_INT:
    case TYP_UINT:
    case TYP_LONG:
    case TYP_ULONG:
    case TYP_LONG64:
    case TYP_ULONG64:
    case TYP_FLOAT:
    case TYP_DOUBLE:
    case TYP_FUNCTION:
    case TYP_DOT_DOT_DOT:
    case TYP_PLAIN_CHAR:
    case TYP_LONG_DOUBLE:
    case TYP_FCOMPLEX:
    case TYP_DCOMPLEX:
    case TYP_LDCOMPLEX:
    case TYP_FIMAGINARY:
    case TYP_DIMAGINARY:
    case TYP_LDIMAGINARY:
    case TYP_BOOL:
        size = CTypeSizes[typ->decl_type];
        break;
    case TYP_VOID:
        if( CompFlags.unix_ext ) {
            size = 1;
        } else {
            size = 0;
        }
        break;
    case TYP_POINTER:
        if( typ->u.p.decl_flags & ( FLAG_FAR | FLAG_HUGE ) ) {
            size = TARGET_FAR_POINTER;
        } else if( typ->u.p.decl_flags & FLAG_NEAR ) {
            size = TARGET_POINTER;
        } else {
            typ = typ->object;
            SKIP_TYPEDEFS( typ );
            if( typ->decl_type == TYP_FUNCTION ) {
                size = CodePtrSize;
            } else {
                size = DataPtrSize;
            }
        }
        break;
    case TYP_ARRAY:
        size = typ->u.array->dimension;
        break;
    case TYP_STRUCT:
        size = typ->u.tag->size;
        if( typ->object != NULL ) {
            /*
             * structure has a zero length array as last field
             */
            typ = typ->object;  /* point to TYP_ARRAY entry */
            size += SizeOfArg( typ );
        }
        break;
    case TYP_UNION:
    case TYP_ENUM:
        size = typ->u.tag->size;
        break;
    case TYP_FIELD:
    case TYP_UFIELD:
        size = CTypeSizes[typ->u.f.field_type];
        if( pFieldWidth != NULL ) {
            *pFieldWidth = typ->u.f.field_width;
        }
        break;
    default:
        size = 0;
    }
    return( size );
}

/*
 * Return an integer type of specified size, or NULL in case of failure.
 * The type will be signed if 'sign' is true. The type will have exactly
 * requested size if 'exact' is true, or the next larger type will be
 * returned (eg. 64-bit integer if 6 byte size is requested).
 */
TYPEPTR GetIntTypeBySize( target_size size, bool sign, bool exact )
{
    static const DATA_TYPE  s_types[] = { TYP_CHAR, TYP_SHORT, TYP_INT, TYP_LONG, TYP_LONG64 };
    static const DATA_TYPE  u_types[] = { TYP_UCHAR, TYP_USHORT, TYP_UINT, TYP_ULONG, TYP_ULONG64 };
    const DATA_TYPE         *type_list;
    DATA_TYPE               type_id;
    TYPEPTR                 typ = NULL;
    unsigned                i;

    /*
     * Make sure the types are laid out the way we expect
     */
    assert( TYP_BOOL == 0 );
    assert( TYP_CHAR == 1 );
    assert( TYP_FLOAT == TYP_ULONG64 + 1 );

    if( size ) {
        type_list = ( sign ) ? s_types : u_types;
        for( i = 0; i < ARRAY_SIZE( s_types ); ++i ) {
            type_id = type_list[i];
            if( size == CTypeSizes[type_id] || !exact && ( size < CTypeSizes[type_id] ) ) {
                typ = GetType( type_id );
                assert( typ );
                break;
            }
        }
    }
    return( typ );
}

void TypesPurge( void )
{
#if 0
    /*
     * The type entries are in permanent memory, so they can't be freed
     */
    TYPEPTR     temp;

    while( TypeHead != NULL ) {
        temp = TypeHead->next_type;
        CMemFree( TypeHead );
        TypeHead = temp;
    }
#endif
}
