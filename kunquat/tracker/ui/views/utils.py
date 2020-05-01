# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2020
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *


def get_abs_window_size(width_norm, height_norm):
    screen_rect = QApplication.desktop().screenGeometry()
    return QSize(
            int(screen_rect.width() * width_norm),
            int(screen_rect.height() * height_norm))

def update_ref_font_height(font, style_mgr):
    for w in QApplication.topLevelWidgets():
        widget = w
        break
    else:
        raise RuntimeError('Main window not found')

    fm = QFontMetrics(QFont(*font), widget)
    style_mgr.set_reference_font_height(fm.tightBoundingRect('E').height())

def get_default_font():
    screen = QApplication.primaryScreen()

    # Get default true DPI based on screen size (within reason)
    min_true_dpi = 7.2
    max_true_dpi = 12
    min_dpi_at_width = 350
    max_dpi_at_width = 675

    physical_width = screen.physicalSize().width()
    dpi_add = (physical_width - min_dpi_at_width)
    t = min(max(0, dpi_add / (max_dpi_at_width - min_dpi_at_width)), 1)
    true_dpi = lerp_val(min_true_dpi, max_true_dpi, t)

    # Scale true DPI to compensate for misconfiguration in the system
    ldpi = screen.logicalDotsPerInch()
    pdpi = screen.physicalDotsPerInch()
    size = int(round(true_dpi * pdpi / ldpi))

    return QFont(QFont().defaultFamily(), size)

def get_default_font_info(style_mgr):
    df = get_default_font()
    def_font_family = style_mgr.get_style_param('def_font_family') or df.family()
    def_font_size = style_mgr.get_style_param('def_font_size') or df.pointSize()
    return (def_font_family, def_font_size)

def get_scaled_font(style_mgr, scale, *attrs):
    ref_font_family, ref_font_size = get_default_font_info(style_mgr)
    scaled_font = QFont(ref_font_family, int(round(ref_font_size * scale)), *attrs)
    scaled_font.setPointSizeF(ref_font_size * scale)
    return scaled_font

def lerp_val(v1, v2, t):
    assert 0 <= t <= 1, 'lerp value {} is not within valid range [0, 1]'.format(t)
    return v1 + (v2 - v1) * t

def lerp_colour(c1, c2, t):
    assert 0 <= t <= 1, 'lerp value {} is not within valid range [0, 1]'.format(t)
    return QColor(
            int(lerp_val(c1.red(), c2.red(), t)),
            int(lerp_val(c1.green(), c2.green(), t)),
            int(lerp_val(c1.blue(), c2.blue(), t)))

def get_colour_from_str(s):
    if len(s) == 4:
        cs = [s[1], s[2], s[3]]
        cs = [c + c for c in cs]
    elif len(s) == 7:
        cs = [s[1:3], s[3:5], s[5:7]]
    else:
        assert False
    colour = [int(c, 16) for c in cs]
    return QColor(colour[0], colour[1], colour[2])

def get_str_from_colour(colour):
    components = [colour.red(), colour.green(), colour.blue()]
    cs = ['{:02x}'.format(c) for c in components]
    s = '#' + ''.join(cs)
    assert len(s) == 7
    return s

def get_glyph_rel_width(font, widget_cls, ref_str):
    fm = QFontMetricsF(font, widget_cls())
    rect = fm.tightBoundingRect(ref_str)
    return rect.width() / rect.height()

def set_glyph_rel_width(font, widget_cls, ref_str, rel_width):
    cur_rel_width = get_glyph_rel_width(font, widget_cls, ref_str)
    stretch = int(round(100 * rel_width / cur_rel_width))
    font.setStretch(stretch)


