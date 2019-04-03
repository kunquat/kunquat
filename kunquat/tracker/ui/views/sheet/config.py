# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import string

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.views.utils import set_glyph_rel_width, get_scaled_font


_HEADER_FONT = QFont(QFont().defaultFamily(), 11, QFont.Bold)
set_glyph_rel_width(_HEADER_FONT, QWidget, string.ascii_lowercase, 14.79)
_HEADER_FONT.setPointSizeF(10.5)


DEFAULT_CONFIG = {
    'ruler': {
        'bg_colour'         : QColor(0x11, 0x22, 0x55),
        'fg_colour'         : QColor(0xaa, 0xcc, 0xff),
        'canvas_bg_colour'  : QColor(0x11, 0x11, 0x11),
        'play_cursor_colour': QColor(0x66, 0xee, 0x66),
        'play_marker_colour': QColor(0xff, 0, 0xff),
        'disabled_colour'   : QColor(0x88, 0x88, 0x88, 0x7f),
        'font'              : QFont(QFont().defaultFamily(), 9),
        'line_min_dist'     : 3,
        'line_len_short'    : 2,
        'line_len_long'     : 4,
        'line_width'        : 1,
        'num_min_dist'      : 48,
        'num_padding_left'  : 8,
        'num_padding_right' : 2,
        'inactive_dim'      : 0.6,
    },
    'header': {
        'bg_colour'    : QColor(0x22, 0x44, 0x22),
        'fg_colour'    : QColor(0xcc, 0xee, 0xaa),
        'border_colour': QColor(0x55, 0x77, 0x55),
        'solo_colour'  : QColor(0x77, 0xee, 0x66, 0x7f),
        'font'         : _HEADER_FONT,
        'padding_x'    : 2,
    },
    'col_width'         : 12, # unit is em
    'trs_per_beat'      : 4,
    'zoom_factor'       : 1.25,
    'inactive_dim'      : 0.6,
    'canvas_bg_colour'  : QColor(0x11, 0x11, 0x11),
    'bg_colour'         : QColor(0, 0, 0),
    'border_colour'     : QColor(0x22, 0x22, 0x22),
    'border_contrast'   : 0.25,
    'border_width'      : 1, # px per side -> effective border width is double
    'font'              : QFont(QFont().defaultFamily(), 12),
    'disabled_colour'   : QColor(0x88, 0x88, 0x88, 0x7f),
    'play_cursor_colour': QColor(0x66, 0xee, 0x66),
    'trigger': {
        'default_colour'   : QColor(0xcc, 0xdd, 0xee),
        'note_on_colour'   : QColor(0xff, 0xdd, 0xbb),
        'hit_colour'       : QColor(0xbb, 0xee, 0x88),
        'note_off_colour'  : QColor(0xcc, 0x99, 0x66),
        'warning_bg_colour': QColor(0xee, 0x33, 0x11),
        'warning_fg_colour': QColor(0xff, 0xff, 0xcc),
        'padding_x'        : 6,
        'padding_y'        : 3,
    },
    'edit_cursor': {
        'view_line_colour': QColor(0xdd, 0xee, 0xff),
        'edit_line_colour': QColor(0xff, 0x88, 0x44),
        'min_snap_dist'   : 64,
        'guide_colour'    : QColor(0xff, 0x88, 0x44, 0x7f),
    },
    'area_selection': {
        'border_colour': QColor(0x88, 0xaa, 0xcc),
        'fill_colour'  : QColor(0x88, 0xaa, 0xcc, 0x7f),
    },
    'grid': {
        'styles': {
            0: QPen(QBrush(QColor(0xa0, 0xa0, 0xa0)), 1, Qt.SolidLine),
            1: QPen(QBrush(QColor(0xa0, 0xa0, 0xa0)), 1, Qt.DashLine),
            2: QPen(QBrush(QColor(0xa0, 0xa0, 0xa0)), 1, Qt.DotLine),
            3: QPen(QBrush(QColor(0x60, 0x60, 0x60)), 1, Qt.SolidLine),
            4: QPen(QBrush(QColor(0x60, 0x60, 0x60)), 1, Qt.DashLine),
            5: QPen(QBrush(QColor(0x60, 0x60, 0x60)), 1, Qt.DotLine),
            6: QPen(QBrush(QColor(0x40, 0x40, 0x40)), 1, Qt.SolidLine),
            7: QPen(QBrush(QColor(0x40, 0x40, 0x40)), 1, Qt.DashLine),
            8: QPen(QBrush(QColor(0x40, 0x40, 0x40)), 1, Qt.DotLine),
        },
        'edit_cursor': {
            'height': 11,
            'width' : 13,
            'colour': QColor(0xee, 0x77, 0x33),
        },
    },
}


def get_config_with_custom_style(style_mgr):
    config = {}
    config['ruler'] = {}
    config['header'] = {}
    config['trigger'] = {}
    config['edit_cursor'] = {}
    config['area_selection'] = {}
    config['grid'] = {}

    def _get_colour(s):
        if isinstance(s, QColor):
            return QColor(s)
        if len(s) == 4:
            cs = [s[1], s[2], s[3]]
            cs = [c + c for c in cs]
        elif len(s) == 7:
            cs = [s[1:3], s[3:5], s[5:7]]
        else:
            assert False
        colour = [int(c, 16) for c in cs]
        return QColor(colour[0], colour[1], colour[2])

    config['font'] = get_scaled_font(style_mgr, 1)

    canvas_bg_colour = _get_colour(style_mgr.get_style_param('sheet_canvas_bg_colour'))
    config['canvas_bg_colour'] = canvas_bg_colour

    disabled_colour = _get_colour(style_mgr.get_style_param('bg_colour'))
    disabled_colour.setAlpha(0x7f)
    config['disabled_colour'] = disabled_colour

    config['border_contrast'] = style_mgr.get_style_param('border_contrast')

    # Columns
    config['bg_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_column_bg_colour'))
    config['border_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_column_border_colour'))
    config['border_width'] = style_mgr.get_scaled_size(
            style_mgr.get_style_param('border_thin_width'))

    # Ruler
    ruler = config['ruler']
    ruler['font'] = get_scaled_font(style_mgr, 0.75)
    ruler['canvas_bg_colour'] = canvas_bg_colour
    ruler['bg_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_ruler_bg_colour'))
    ruler['fg_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_ruler_fg_colour'))
    ruler['play_cursor_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_playback_cursor_colour'))
    ruler['play_marker_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_ruler_playback_marker_colour'))
    ruler['disabled_colour'] = disabled_colour
    ruler['line_min_dist'] = style_mgr.get_scaled_size(0.3)
    ruler['line_len_short'] = style_mgr.get_scaled_size(0.3)
    ruler['line_len_long'] = style_mgr.get_scaled_size(0.6)
    ruler['line_width'] = style_mgr.get_scaled_size(0.1)
    ruler['num_min_dist'] = style_mgr.get_scaled_size(3.0)
    ruler['num_padding_left'] = style_mgr.get_scaled_size(0.8)
    ruler['num_padding_right'] = style_mgr.get_scaled_size(0.2)

    # Column headers
    header_font = get_scaled_font(style_mgr, 1, QFont.Bold)
    set_glyph_rel_width(header_font, QWidget, string.ascii_lowercase, 14.79)
    config['header']['font'] = header_font
    config['header']['bg_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_header_bg_colour'))
    config['header']['fg_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_header_fg_colour'))
    solo_colour = _get_colour(style_mgr.get_style_param('sheet_header_solo_colour'))
    solo_colour.setAlpha(0x7f)
    config['header']['solo_colour'] = solo_colour
    config['header']['padding_x'] = style_mgr.get_scaled_size(0.2)

    # Triggers
    config['trigger']['default_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_trigger_default_colour'))
    config['trigger']['note_on_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_trigger_note_on_colour'))
    config['trigger']['hit_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_trigger_hit_colour'))
    config['trigger']['note_off_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_trigger_note_off_colour'))
    config['trigger']['warning_bg_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_trigger_warning_bg_colour'))
    config['trigger']['warning_fg_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_trigger_warning_fg_colour'))
    config['trigger']['padding_x'] = style_mgr.get_scaled_size(0.5, 0)
    config['trigger']['padding_y'] = style_mgr.get_scaled_size(0.3, 0)

    # Cursor
    config['edit_cursor']['view_line_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_cursor_view_line_colour'))
    elc = _get_colour(style_mgr.get_style_param('sheet_cursor_edit_line_colour'))
    config['edit_cursor']['edit_line_colour'] = elc
    guide_colour = QColor(elc.red(), elc.green(), elc.blue(), 0x7f)
    config['edit_cursor']['guide_colour'] = guide_colour
    config['play_cursor_colour'] = _get_colour(
            style_mgr.get_style_param('sheet_playback_cursor_colour'))
    config['edit_cursor']['min_snap_dist'] = style_mgr.get_scaled_size(10.0)

    # Area selection
    asc = _get_colour(style_mgr.get_style_param('sheet_area_selection_colour'))
    as_fill_colour = QColor(asc.red(), asc.green(), asc.blue(), 0x7f)
    config['area_selection']['border_colour'] = asc
    config['area_selection']['fill_colour'] = as_fill_colour

    # Grid lines
    grid_styles = {}
    for i in range(9):
        grid_styles[i] = QPen(DEFAULT_CONFIG['grid']['styles'][i])
    grid_colours = [
        _get_colour(style_mgr.get_style_param('sheet_grid_level_1_colour')),
        _get_colour(style_mgr.get_style_param('sheet_grid_level_2_colour')),
        _get_colour(style_mgr.get_style_param('sheet_grid_level_3_colour')),
    ]
    grid_line_width = style_mgr.get_scaled_size_param('sheet_grid_line_width')
    for i in range(9):
        grid_styles[i].setColor(grid_colours[i // 3])
        grid_styles[i].setWidthF(grid_line_width)
    grid_edit_cursor = DEFAULT_CONFIG['grid']['edit_cursor'].copy()
    grid_edit_cursor['height'] = style_mgr.get_scaled_size(0.9)
    grid_edit_cursor['width'] = style_mgr.get_scaled_size(1.1)
    grid_edit_cursor['colour'] = elc
    config['grid']['styles'] = grid_styles
    config['grid']['edit_cursor'] = grid_edit_cursor

    return config


