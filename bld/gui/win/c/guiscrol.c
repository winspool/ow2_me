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


#include "guiwind.h"
#include "guiscale.h"
#include "guixutil.h"
#include "guiscrol.h"
#include "guidoscr.h"

guix_ord GUIGetScrollScreenSize( gui_window *wnd, int bar )
{
    WPI_RECTDIM left, top, right, bottom;

    _wpi_getclientrect( wnd->hwnd, &wnd->hwnd_client_rect );
    _wpi_getrectvalues( wnd->hwnd_client_rect, &left, &top, &right, &bottom );
    if( bar == SB_HORZ ) {
        return( right - left );
    } else {
        if( GUI_VSCROLL_ROWS( wnd ) ) {
            return( GUITextToScreenV( wnd->num_rows, wnd ) );
        } else {
            return( bottom - top );
        }
    }
}

guix_ord GUIGetScrollInc( gui_window *wnd, int bar )
{
    if( bar == SB_HORZ ) {
        if( GUI_HSCROLL_COLS( wnd ) ) {
            return( GUITextToScreenH( 1, wnd ) );
        } else {
            return( 1 );
        }
    } else {
        if( GUI_VSCROLL_ROWS( wnd ) ) {
            return( GUITextToScreenV( 1, wnd ) );
        } else {
            return( 1 );
        }
    }
}

void GUIScroll( gui_window *wnd, int bar, int change )
{
    guix_ord    new;
    guix_ord    old;
    guix_ord    range;
    guix_ord    screen_size;

    old = GUIGetScrollPos( wnd, bar );
    new = old + change;
    if( change < 0 ) {
        if( new < 0 ) {
            new = 0;
            change = new - old;
        }
    }
    screen_size = GUIGetScrollScreenSize( wnd, bar );
    if( change > 0  ) {
        range = GUIGetScrollRange( wnd, bar );
        if( new > range ) {
            new = range;
            if( new < 0 ) {
                new = 0;
            }
            change = new - old;
        }
    }
    if( old != new ) {
        GUISetScrollPos( wnd, bar, new, true );
        if( ( change > 0 ) && ( change >= screen_size )  ||
            ( change < 0 ) && ( -change >= screen_size ) ) {
            /* scrolled a page or more */
            GUIWndDirty( wnd );
        } else {
            GUIDoScroll( wnd, change, bar );
        }
    }
}

/*
 * GUISetVScrollRow -- Set the vertical scrolling position for the window
 */
void GUIAPI GUISetVScrollRow( gui_window *wnd, gui_text_ord vscroll_pos )
{
    if( IS_VSCROLL_ON( wnd ) ) {
        GUIScroll( wnd, SB_VERT, GUITextToScreenV( vscroll_pos, wnd ) - GUIGetScrollPos( wnd, SB_VERT ) );
    }
}

/*
 * GUISetHScrollCol -- Set the horizontal scrolling position for the window
 */
void GUIAPI GUISetHScrollCol( gui_window *wnd, gui_text_ord hscroll_pos )
{
    if( IS_HSCROLL_ON( wnd ) ) {
        GUIScroll( wnd, SB_HORZ, GUITextToScreenH( hscroll_pos, wnd ) - GUIGetScrollPos( wnd, SB_HORZ ) );
    }
}

/*
 * GUISetVScroll -- Set the vertical scrolling position for the window
 *                  call by user in user scale
 */
void GUIAPI GUISetVScroll( gui_window *wnd, gui_ord vscroll_pos )
{
    guix_ord    scr_y;

    if( IS_VSCROLL_ON( wnd ) ) {
        scr_y = GUIScaleToScreenV( vscroll_pos );
        if( ( vscroll_pos != 0 ) && ( scr_y == 0 ) ) {
            scr_y++;
        }
        GUIScroll( wnd, SB_VERT, scr_y - GUIGetScrollPos( wnd, SB_VERT ) );
    }
}

/*
 * GUISetHScroll -- Set the horizontal scrolling position for the window
 *                  call by user in user scale
 */
void GUIAPI GUISetHScroll( gui_window *wnd, gui_ord hscroll_pos )
{
    guix_ord    scr_x;

    if( IS_HSCROLL_ON( wnd ) ) {
        scr_x = GUIScaleToScreenH( hscroll_pos );
        if( ( hscroll_pos != 0 ) && ( scr_x == 0 ) ) {
            scr_x++;
        }
        GUIScroll( wnd, SB_HORZ, scr_x - GUIGetScrollPos( wnd, SB_HORZ ) );
    }
}

/*
 * GUIVScroll -- scroll vertically the given number of lines
 *               only call by library why library does scrolling
 */
static void GUIVScroll( int diff, gui_window *wnd, gui_event gui_ev )
{
    if( diff == 0 ) {
        return;
    }
    if( GUI_DO_VSCROLL( wnd ) ) {
        GUIScroll( wnd, SB_VERT, diff );
        GUIEVENT( wnd, GUI_VSCROLL_NOTIFY, NULL );
    } else {
        if( gui_ev == GUI_SCROLL_VERTICAL ) {
            diff /= GUIGetScrollInc( wnd, SB_VERT );
            GUIEVENT( wnd, gui_ev, &diff );
        } else {
            GUIEVENT( wnd, gui_ev, NULL );
        }
    }
}

/*
 * GUIHScroll -- scroll horizontally the given number of characters
 *               only call by library why library does scrolling
 */
static void GUIHScroll( int diff, gui_window *wnd, gui_event gui_ev )
{
    if( diff == 0 ) {
        return;
    }
    if( GUI_DO_HSCROLL( wnd ) ) {
        GUIScroll( wnd, SB_HORZ, diff );
        GUIEVENT( wnd, GUI_HSCROLL_NOTIFY, NULL );
    } else {
        if( gui_ev == GUI_SCROLL_HORIZONTAL ) {
            diff /= GUIGetScrollInc( wnd, SB_HORZ );
            GUIEVENT( wnd, gui_ev, &diff );
        } else {
            GUIEVENT( wnd, gui_ev, NULL );
        }
    }
}

void GUIProcessScrollMsg( gui_window *wnd, WPI_MSG msg, WPI_PARAM1 wparam, WPI_PARAM2 lparam )
{
    int         diff;
    WORD        param;
    int         mult;

    wparam = wparam;
    lparam = lparam;

    switch( msg ) {
    case WM_VSCROLL:
        mult = GUIGetScrollInc( wnd, SB_VERT );
        switch( GET_WM_VSCROLL_CMD( wparam, lparam ) ) {
        case SB_LINEUP:
            GUIVScroll( -mult, wnd, GUI_SCROLL_UP );
            break;
        case SB_PAGEUP:
            GUIVScroll( -mult * wnd->num_rows, wnd, GUI_SCROLL_PAGE_UP );
            break;
        case SB_LINEDOWN:
            GUIVScroll( mult, wnd, GUI_SCROLL_DOWN );
            break;
        case SB_PAGEDOWN:
            GUIVScroll( mult * wnd->num_rows, wnd, GUI_SCROLL_PAGE_DOWN );
            break;
#ifndef __OS2_PM__
        case SB_TOP:
            diff = -GUIGetScrollPos( wnd, SB_VERT );
            GUIVScroll( diff, wnd, GUI_SCROLL_TOP );
            break;
        case SB_BOTTOM:
            diff = GUIGetScrollRange( wnd, SB_VERT ) - GUIGetScrollPos( wnd, SB_VERT );
            GUIVScroll( diff, wnd, GUI_SCROLL_BOTTOM );
            break;
#endif
        case SB_THUMBTRACK:
            if( (wnd->scroll_style & GUI_VTRACK) == 0 )
                break;
            /* fall through */
        case SB_THUMBPOSITION:
            if( wnd->scroll_style & GUI_VDRAG ) {
                param = GET_WM_VSCROLL_POS( wparam, lparam );
                diff = param - GUIGetScrollPos( wnd, SB_VERT );
                GUIVScroll( diff, wnd, GUI_SCROLL_VERTICAL );
            }
            break;
        case SB_ENDSCROLL:
            GUIRedrawScroll( wnd, SB_VERT, false );
            break;
        }
        break;
    case WM_HSCROLL:
        mult = GUIGetScrollInc( wnd, SB_HORZ );
        switch( GET_WM_HSCROLL_CMD( wparam, lparam ) ) {
        case SB_LINELEFT:
            GUIHScroll( -mult, wnd, GUI_SCROLL_LEFT );
            break;
        case SB_PAGELEFT:
            GUIHScroll( -wnd->num_cols * mult, wnd, GUI_SCROLL_PAGE_LEFT );
            break;
        case SB_LINERIGHT:
            GUIHScroll( mult, wnd, GUI_SCROLL_RIGHT );
            break;
        case SB_PAGERIGHT:
            GUIHScroll( mult * wnd->num_cols, wnd, GUI_SCROLL_PAGE_RIGHT );
            break;
#ifndef __OS2_PM__
        case SB_LEFT:
            diff = -GUIGetScrollPos( wnd, SB_HORZ );
            GUIHScroll( diff, wnd, GUI_SCROLL_FULL_LEFT );
            break;
        case SB_RIGHT:
            diff = GUIGetScrollRange( wnd, SB_HORZ ) - GUIGetScrollPos( wnd, SB_HORZ );
            GUIHScroll( diff, wnd, GUI_SCROLL_FULL_RIGHT );
            break;
#endif
        case SB_THUMBTRACK:
            if( (wnd->scroll_style & GUI_HTRACK) == 0 )
                break;
            /* fall through */
        case SB_THUMBPOSITION:
            if( wnd->scroll_style & GUI_HDRAG ) {
                param = GET_WM_HSCROLL_POS( wparam, lparam );
                diff = param - GUIGetScrollPos( wnd, SB_HORZ );
                GUIHScroll( diff, wnd, GUI_SCROLL_HORIZONTAL );
            }
            break;
        case SB_ENDSCROLL:
            GUIRedrawScroll( wnd, SB_HORZ, false );
            break;
        }
    }
}

static void DoSetScroll( gui_window *wnd, int bar, bool range_set,
                         bool char_unit, unsigned *prange )
{
    guix_ord        range;
    guix_ord        pos;
    guix_ord        screen_size;

    screen_size = GUIGetScrollScreenSize( wnd, bar );
    if( range_set ) {
        /*
         * app explicitly set scroll range
         * need to adjust range if size changed
         */
        range = *prange;
        if( char_unit ) {
            range *= GUIGetScrollInc( wnd, bar );
        }
        if( range < screen_size ) {
            range = 0;
        } else {
            range -= screen_size;
        }
        pos = GUIGetScrollPos( wnd, bar );
        if( range < pos ) {
            GUIScroll( wnd, bar, range - pos );
        }
        if( !char_unit ) {
            *prange = range + screen_size;
        }
    } else {
        range = screen_size + GUIGetScrollPos( wnd, bar );
        *prange = range + screen_size;
    }
    GUISetScrollRange( wnd, bar, 0, range, true );
}

/*
 * GUISetSroll -- Set the scroll range for the given window
 */
void GUISetScroll( gui_window *wnd )
{
    if( GUI_DO_HSCROLL( wnd ) ) {
        DoSetScroll( wnd, SB_HORZ, GUI_HRANGE_SET( wnd ), GUI_HRANGE_CHAR_UNIT( wnd ), &wnd->hscroll_range );
    }
    if( GUI_DO_VSCROLL( wnd ) ) {
        DoSetScroll( wnd, SB_VERT, GUI_VRANGE_SET( wnd ), GUI_VRANGE_CHAR_UNIT( wnd ), &wnd->vscroll_range );
    }
}
