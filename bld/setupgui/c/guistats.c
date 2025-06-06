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
* Description:  Installer GUI status bar.
*
****************************************************************************/


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "setup.h"
#include "guiutil.h"
#include "guikey.h"
#include "dlggen.h"
#include "setupwpi.h"
#include "setupinf.h"
#include "guistr.h"
#include "guidlg.h"
#include "dlgbutn.h"
#include "genvbl.h"
#include "utils.h"
#if !defined( GUI_IS_GUI )
    #include "stdui.h"
    #include "uigchar.h"
#endif
#include "guistats.h"

#include "clibext.h"


#define LINE0_ROW       1
#define LINE1_ROW       2
#define LINE0_COL       1
#define LINE1_COL       1
#define STATUS_WIDTH    70
#define STATUS_HEIGHT   10
#define CANNERY_SIZE    10
#define CANNERY_ROW     6
#define STATUS_ROW      4
#define BAR_INDENT      4

int                     MsgLine0 = STAT_BLANK;
bool                    CancelSetup = false;
gui_colour_set          ToolPlain = { GUI_BLACK, GUI_BLUE, };
gui_colour_set          ToolStandout = { GUI_BR_WHITE, GUI_BLUE };

static gui_window       *StatusWnd;
static gui_rect         StatusBarRect;
static gui_coord        CharSize;
static unsigned         Percent;
static unsigned         Parts_Injob;
static unsigned         Parts_Complete;
static char             StatusLine1[_MAX_PATH];
static gui_ord          StatusBarLen;
static gui_rect         StatusRect;
static char             StatusBarBuf[256];

static const char *Messages[] = {
    #define pick( x, y ) y,
    #include "status.h"
    #undef pick
};

static gui_control_info Cancel = {
    GUI_DEFPUSH_BUTTON, NULL, {0, 0, 0, 0}, NULL, GUI_NOSCROLL, GUI_STYLE_CONTROL_TAB_GROUP | GUI_STYLE_CONTROL_AUTOMATIC, CTL_CANCEL
};

static gui_colour_set StatusColours[] = {
#if !defined( GUI_IS_GUI )
    /* Fore              Back        */
    { GUI_WHITE,        GUI_BLACK },        /* GUI_MENU_PLAIN           */
    { GUI_BLUE,         GUI_BR_WHITE },     /* GUI_MENU_STANDOUT        */
    { GUI_GREY,         GUI_BLACK },        /* GUI_MENU_GRAYED          */
    { GUI_BR_YELLOW,    GUI_BLACK },        /* GUI_MENU_ACTIVE          */
    { GUI_BR_YELLOW,    GUI_BR_WHITE },     /* GUI_MENU_ACTIVE_STANDOUT */
    { GUI_BR_WHITE,     GUI_BLUE },         /* GUI_BACKGROUND           */
    { GUI_BR_WHITE,     GUI_BLUE },         /* GUI_MENU_FRAME           */
    { GUI_GREY,         GUI_BLUE },         /* GUI_TITLE_INACTIVE       */
    { GUI_BLUE,         GUI_CYAN },         /* GUI_FRAME_ACTIVE         */
    { GUI_GREY,         GUI_BLACK },        /* GUI_FRAME_INACTIVE       */
    { GUI_BR_WHITE,     GUI_RED },          /* GUI_ICON                 */
    { GUI_GREY,         GUI_BR_WHITE },     /* GUI_MENU_GRAYED_ACTIVE   */
    { GUI_GREY,         GUI_BR_WHITE },     /* GUI_FRAME_RESIZE         */
    { GUI_BLUE,         GUI_WHITE },        /* GUI_CONTROL_BACKGROUND   */

    { GUI_GREEN,        GUI_BLACK },        /* WND_PLAIN                */
    { GUI_BLACK,        GUI_GREEN },        /* WND_TABSTOP              */
    { GUI_BLUE,         GUI_BR_WHITE },     /* WND_SELECTED             */
    { GUI_BLACK,        GUI_RED },          /* WND_HOTSPOT              */
    { GUI_GREY,         GUI_BLACK },        /* WND_CENSORED             */
    { GUI_BLACK,        GUI_WHITE },        /* WND_STATUS_BAR           */
    { GUI_WHITE,        GUI_BLUE },         /* WND_STATUS_TEXT          */
    { GUI_WHITE,        GUI_BLUE },         /* WND_STATUS_FRAME         */
#else                   // win or winnt or OS/2
    /* Fore              Back        */
    { GUI_BR_WHITE,     GUI_BLACK },        /* GUI_MENU_PLAIN           */
    { GUI_BLACK,        GUI_BR_WHITE },     /* GUI_MENU_STANDOUT        */
    { GUI_GREY,         GUI_BLACK },        /* GUI_MENU_GRAYED          */
    { GUI_BR_YELLOW,    GUI_BLACK },        /* GUI_MENU_ACTIVE          */
    { GUI_BR_YELLOW,    GUI_BR_WHITE },     /* GUI_MENU_ACTIVE_STANDOUT */
    { GUI_BLACK,        GUIEX_DLG_BKGRND }, /* GUI_BACKGROUND           */
    { GUI_BR_WHITE,     GUI_BLUE },         /* GUI_MENU_FRAME           */
    { GUI_GREY,         GUI_BLACK },        /* GUI_TITLE_INACTIVE       */
    { GUI_BLUE,         GUI_CYAN },         /* GUI_FRAME_ACTIVE         */
    { GUI_GREY,         GUI_BLACK },        /* GUI_FRAME_INACTIVE       */
    { GUI_BR_WHITE,     GUI_RED },          /* GUI_ICON                 */
    { GUI_GREY,         GUI_BR_WHITE },     /* GUI_MENU_GRAYED_ACTIVE   */
    { GUI_GREY,         GUI_BR_WHITE },     /* GUI_FRAME_RESIZE         */
    { GUI_BLACK,        GUIEX_WND_BKGRND }, /* GUI_CONTROL_BACKGROUND   */

    { GUI_GREEN,        GUI_BLACK },        /* WND_PLAIN                */
    { GUI_BLACK,        GUI_GREEN },        /* WND_TABSTOP              */
    { GUI_BLACK,        GUI_BR_WHITE },     /* WND_SELECTED             */
    { GUI_BLACK,        GUI_RED },          /* WND_HOTSPOT              */
    { GUI_GREY,         GUI_BLACK },        /* WND_CENSORED             */
    { GUIEX_DLG_BKGRND, GUI_BLACK },        /* WND_STATUS_BAR           */
    { GUI_BLACK,        GUIEX_DLG_BKGRND }, /* WND_STATUS_TEXT          */
    { GUI_BLACK,        GUIEX_DLG_BKGRND }  /* WND_STATUS_FRAME         */
#endif
};

static bool GUICALLBACK StatusGUIEventProc( gui_window *gui, gui_event gui_ev, void *parm )
/*****************************************************************************************/
{
    static bool         button_pressed = false;
    gui_ctl_id          id;
    gui_key             key;
    gui_keystate        state;
    const char          *msg;

    /* unused parameters */ (void)parm;

    if( gui == NULL )
        return( false );

    switch( gui_ev ) {
    case GUI_INIT_WINDOW:
        return( true );
    case GUI_PAINT:
        {
            if( StatusBarLen == 0 ) {
                break;
            }
            msg = GetVariableStrVal( Messages[MsgLine0] );
            GUIDrawTextExtent( gui, msg, strlen( msg ), LINE0_ROW,
                               LINE0_COL * CharSize.x, WND_STATUS_TEXT, GUI_NO_EXTENT );
            GUIDrawTextExtent( gui, StatusLine1, strlen( StatusLine1 ), LINE1_ROW,
                               LINE1_COL * CharSize.x, WND_STATUS_TEXT, GUI_NO_EXTENT );
#if !defined( GUI_IS_GUI )
            {
                int         len1, len2;
                char        num[20];

                memset( StatusBarBuf, ' ', StatusBarLen );
                StatusBarBuf[StatusBarLen] = '\0';
                sprintf( num, "%u%%", Percent );
                memcpy( StatusBarBuf + StatusBarLen / 2 - 1, num, strlen( num ) );
                // draw bar in two parts
                len1 = (StatusBarLen * Percent) / 100;
                if( len1 < 0 ) {
                    len1 = 0;
                } else if( len1 > StatusBarLen ) {
                    len1 = StatusBarLen;
                }
                len2 = StatusBarLen - len1;
                if( len1 > 0 ) {
                    GUIDrawText( gui, StatusBarBuf, len1, STATUS_ROW,
                                 StatusBarRect.x, WND_STATUS_BAR );
                }
                if( len2 > 0 ) {
                    GUIDrawText( gui, StatusBarBuf + len1, len2, STATUS_ROW,
                                 StatusBarRect.x + len1 * CharSize.x, WND_STATUS_TEXT );
                }
                memset( StatusBarBuf, UiGChar[UI_SBOX_TOP_LINE], StatusBarLen );
                GUIDrawText( gui, StatusBarBuf, StatusBarLen, STATUS_ROW - 1,
                             StatusBarRect.x, WND_STATUS_TEXT );
                GUIDrawText( gui, StatusBarBuf, StatusBarLen, STATUS_ROW + 1,
                             StatusBarRect.x, WND_STATUS_TEXT );
            }
#else
            {
                gui_coord   coord;
                int         width, height;
                size_t      str_len;
                size_t      len1;
                size_t      len2;
                int         bar_width, divider;
                gui_point   start, end;
                gui_rect    rStatusBar;

//              sprintf( StatusBarBuf, "%u%%", Percent );
                // clear whole bar
                GUIFillRect( gui, &StatusBarRect, WND_STATUS_BAR );
                // calculate where divider splits rectangle
                bar_width = StatusBarRect.width;
                divider = (bar_width * Percent) / 100;
                if( divider < 0 ) {
                    divider = 0;
                } else if( divider > bar_width ) {
                    divider = bar_width;
                }
                rStatusBar = StatusBarRect;
                rStatusBar.width = divider;
                // calculate position for text (centre it)
                str_len = strlen( StatusBarBuf );
                width = GUIGetExtentX( gui, StatusBarBuf, str_len );
                height = GUIGetExtentY( gui, StatusBarBuf );
                coord.y = StatusBarRect.y + (StatusBarRect.height - height) / 2;
                coord.x = StatusBarRect.x + (StatusBarRect.width - width) / 2;
                divider += StatusBarRect.x;
                if( coord.x > divider ) {
                    // text is completely to right of divider
                    GUIFillRect( gui, &rStatusBar, WND_STATUS_TEXT );
                    GUIDrawTextPos( gui, StatusBarBuf, str_len, &coord, WND_STATUS_TEXT );
                } else if( coord.x + width < divider ) {
                    // text is completely to left of divider
                    GUIFillRect( gui, &rStatusBar, WND_STATUS_TEXT );
                    GUIDrawTextPos( gui, StatusBarBuf, str_len, &coord, WND_STATUS_BAR );
                } else {
                    // text is over divider
                    // coord.x <= divider <= coord.x + width
                    // need to split text
                    len1 = ((size_t)(divider - coord.x) * str_len) / (size_t)width;
                    if( len1 > str_len ) {
                        len1 = str_len;
                    }
                    len2 = str_len - len1;
                    // recalc divider, so it falls on a character boundary
                    divider = coord.x + GUIGetExtentX( gui, StatusBarBuf, len1 );
                    rStatusBar.width = divider - StatusBarRect.x;
                    GUIFillRect( gui, &rStatusBar, WND_STATUS_TEXT );
                    if( len1 > 0 ) {
                        GUIDrawTextPos( gui, StatusBarBuf, len1, &coord, WND_STATUS_BAR );
                    }
                    if( len2 > 0 ) {
                        coord.x = divider;
                        GUIDrawTextPos( gui, StatusBarBuf + len1, len2, &coord, WND_STATUS_TEXT );
                    }
                }
                // draw frame
                start.x = StatusBarRect.x;
                start.y = StatusBarRect.y;
                end.x = StatusBarRect.width + StatusBarRect.x;
                end.y = start.y;                                // top line
                GUIDrawLine( gui, &start, &end, GUI_PEN_SOLID, 1, WND_STATUS_FRAME );
                start.y = StatusBarRect.y + StatusBarRect.height; // bottom line
                end.y = start.y;
                GUIDrawLine( gui, &start, &end, GUI_PEN_SOLID, 1, WND_STATUS_FRAME );
                end.y = StatusBarRect.y;
                start.x = StatusBarRect.x;
                end.x = start.x;                            // left side
                GUIDrawLine( gui, &start, &end, GUI_PEN_SOLID, 1, WND_STATUS_FRAME );
                start.x = StatusBarRect.x + StatusBarRect.width;
                end.x = start.x;                           // right side
                GUIDrawLine( gui, &start, &end, GUI_PEN_SOLID, 1, WND_STATUS_FRAME );
            }
#endif
            return( true );
        }
    case GUI_DESTROY:
        StatusWnd = NULL;
        return( true );
    case GUI_CONTROL_CLICKED:
        GUIGetFocus( gui, &id );
        GUI_GETID( parm, id );
        switch( id ) {
        case CTL_CANCEL:
            if( !button_pressed ) {
                button_pressed = true;
                if( MsgBox( gui, "IDS_QUERYABORT", GUI_YES_NO ) == GUI_RET_YES ) {
                    CancelSetup = true;
                }
                button_pressed = false;
            }
            return( true );
        case CTL_DONE:
            if( !button_pressed ) {
                CancelSetup = true;
            }
            return( true );
        }
        break;
    case GUI_KEYDOWN:
        GUI_GET_KEY_STATE( parm, key, state );
        switch( key ) {
        case GUI_KEY_ESCAPE:
            if( !button_pressed ) {
                button_pressed = true;
                if( MsgBox( gui, "IDS_QUERYABORT", GUI_YES_NO ) == GUI_RET_YES ) {
                    CancelSetup = true;
                }
                button_pressed = false;
                break;
            }
        default:
            break;
        }
        return( true );
    default:
        break;
    }
    return( false );
}

static bool OpenStatusWindow( const char *title )
/***********************************************/
{
    gui_text_metrics    metrics;
//    int                 i;
    gui_rect            rect;
    gui_create_info     init;

//    for( i = STAT_BLANK; i < sizeof( Messages ) / sizeof( Messages[0] ); ++i ) {
//      Messages[i] = GetVariableStrVal( Messages[i] );
//    }
    GUIGetDlgTextMetrics( &metrics );
    CharSize.x = metrics.avg.x;
    CharSize.y = 5 * metrics.avg.y / 4;
    GUITruncToPixel( &CharSize );

    memset( &init, 0, sizeof( init ) );
    init.title = title;
    init.rect.width = STATUS_WIDTH * CharSize.x;
    init.rect.height = STATUS_HEIGHT * CharSize.y;
    GUIGetClientRect( MainWnd, &rect );
    if( GUIIsGUI() ) {
        init.rect.y = BitMapBottom;
    } else {
        init.rect.y = (GUIScale.y - init.rect.height) / 2;
    }
    if( init.rect.y > rect.height - init.rect.height ) {
        init.rect.y = rect.height - init.rect.height;
    }
    init.rect.x = (GUIScale.x - init.rect.width) / 2;
    init.scroll_style = GUI_NOSCROLL;
    init.style = GUI_VISIBLE | GUI_SYSTEM_MENU;
    init.parent = MainWnd;
    init.colours.num_items = GUI_ARRAY_SIZE( StatusColours );
    init.colours.colour = StatusColours;
    init.gui_call_back = StatusGUIEventProc;

    StatusBarLen = 0;

    StatusWnd = GUICreateWindow( &init );

    GUIGetClientRect( StatusWnd, &StatusRect );

    Cancel.parent = StatusWnd;
    Cancel.text = LIT_GUI( Cancel );
    Cancel.rect.height = 7 * CharSize.y / 4;
    Cancel.rect.width = (strlen( Cancel.text ) + 4) * CharSize.x;
    Cancel.rect.x = (StatusRect.width - Cancel.rect.width) / 2;
    Cancel.rect.y = CANNERY_ROW * CharSize.y;

    StatusBarRect.x = BAR_INDENT * CharSize.x;
    StatusBarRect.width = StatusRect.width - 2 * BAR_INDENT * CharSize.x;
    StatusBarRect.y = STATUS_ROW * CharSize.y;
    StatusBarRect.height = CharSize.y;
#if defined( GUI_IS_GUI )
    StatusBarRect.y -= CharSize.y / 2;
    StatusBarRect.height += CharSize.y;
#endif

    StatusBarLen = StatusBarRect.width / CharSize.x;

    if( !GUIAddControl( &Cancel, &ToolPlain, &ToolStandout ) ) {
        SetupError( "IDS_CONTROLERROR" );
        return( false );
    }
    return( true );
}

void StatusShow( bool show )
/**************************/
{
    if( StatusWnd != NULL ) {
        if( show ) {
            GUIShowWindow( StatusWnd );
        } else {
            GUIHideWindow( StatusWnd );
        }
        GUIWndDirty( StatusWnd );
    }
}

void StatusLines( int msg0, const char *message1 )
/************************************************/
{
    if( StatusWnd != NULL ) {
        if( message1 != NULL ) {
            if( strcmp( message1, StatusLine1 ) != 0 ) {
                strcpy( StatusLine1, message1 );
                GUIWndDirtyRow( StatusWnd, LINE1_ROW );
            }
        }
        if( msg0 != STAT_SAME ) {
            if( msg0 != MsgLine0 ) {
                MsgLine0 = msg0;
                GUIWndDirty( StatusWnd );
            }
        }
    }
}

void BumpStatus( unsigned by )
/****************************/
{
    StatusAmount( Parts_Complete + by, Parts_Injob );
}

void StatusAmount( unsigned parts_complete, unsigned parts_injob )
/****************************************************************/
// Display slider bar indicating percentage complete
{
    unsigned        old_percent;

    Parts_Injob = parts_injob;
    Parts_Complete = parts_complete;
    old_percent = Percent;
    if( parts_injob == 0 ) {
        if( parts_complete == 0 ) {
            Percent = 0;
        } else {
            Percent = 100;
        }
    } else {
        if( Parts_Complete > Parts_Injob ) {
            Parts_Complete = Parts_Injob;
        }
        if( Parts_Injob > 100000 ) {
            Percent = Parts_Complete / (Parts_Injob / 100);
        } else {
            Percent = (100 * Parts_Complete) / Parts_Injob;
        }
        if( Percent > 100 ) {
            Percent = 100;
        }
    }
    if( old_percent == Percent )
        return;
    if( Percent != 0 && Percent < old_percent ) {
        Percent = old_percent;
        return;
    }
    if( StatusWnd == NULL )
        return;
#if !defined( GUI_IS_GUI )
    GUIWndDirty( StatusWnd );
#else
    {
        gui_ord         bar_width, old_divider, divider;
        gui_rect        rect;

        sprintf( StatusBarBuf, "%u%%", Percent );

        // calculate where divider splits rectangle
        bar_width = StatusBarRect.width;
        divider = (bar_width * Percent) / 100;
        if( divider < 0 ) {
            divider = 0;
        } else if( divider > bar_width ) {
            divider = bar_width;
        }
        old_divider = (bar_width * old_percent) / 100;
        if( old_divider < 0 ) {
            old_divider = 0;
        } else if( old_divider > bar_width ) {
            old_divider = bar_width;
        }
        if( divider <= old_divider ) {
            GUIWndDirty( StatusWnd );
        } else {
            // dirty new bit of bar
            divider += StatusBarRect.x;
            old_divider += StatusBarRect.x;
            rect = StatusBarRect;
            rect.width = GUIGetExtentX( StatusWnd, StatusBarBuf, strlen( StatusBarBuf ) );
            rect.x = StatusBarRect.x + (StatusBarRect.width - rect.width) / 2;
            rect.x -= CharSize.x / 2;
            rect.width += CharSize.x;
            GUIWndDirtyRect( StatusWnd, &rect ); // dirty text
            rect.x = old_divider - CharSize.x;
            rect.width = divider - old_divider + 2 * CharSize.x;
            GUIWndDirtyRect( StatusWnd, &rect ); // dirty new bit of bar
        }
    }
#endif
}

bool StatusCancelled( void )
/**************************/
{
    // update windows and let other apps execute
    GUIDrainEvents();
    return( CancelSetup );
}

void StatusInit( void )
/*********************/
{
    VBUF    buff;

    if( StatusWnd == NULL && !Invisible ) {
        VbufInit( &buff );
        ReplaceVars( &buff, GetVariableStrVal( "AppName" ) );
        if( OpenStatusWindow( VbufString( &buff ) ) ) {
            GUIHideWindow( StatusWnd );
        }
        VbufFree( &buff );
    }
}

void StatusFini( void )
/*********************/
{
    if( StatusWnd != NULL ){
        GUIDestroyWnd( StatusWnd );
        StatusWnd = NULL;
    }
}
