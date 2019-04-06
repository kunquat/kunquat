# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

import kunquat.kunquat.events as events
from .config import *

import math


class TriggerRenderer():

    def __init__(self, cache, config, trigger, notation, force_shift):
        assert trigger
        self._cache = cache
        self._config = config
        self._trigger = trigger
        self._notation = notation
        self._inactive = False
        self._force_shift = force_shift

        self._setup_fields()

    def set_inactive(self):
        self._inactive = True

    def get_field_count(self):
        return len(self._fields)

    def get_field_bounds(self, index):
        offset = self._fields[index]['offset']
        width = self._fields[index]['width']
        return (offset, width)

    def get_total_width(self):
        return self._total_width

    def get_trigger_image(self):
        self._check_create_trigger_image()
        key = self._get_trigger_image_key()
        image = self._cache.get_trigger_image(key)
        assert image
        return image

    def draw_trigger(self, painter, include_line=True, select=False):
        use_cache = include_line and not select

        if use_cache:
            self._check_create_trigger_image()
            key = self._get_trigger_image_key()
            image = self._cache.get_trigger_image(key)
            assert image
            lw_add = self._config['trigger']['line_width'] // 2
            painter.drawImage(0, -lw_add, image)
        else:
            self._draw_trigger(painter, include_line, select)

    def _get_final_colour(self, colour):
        if self._inactive:
            dim_factor = self._config['inactive_dim']
            new_colour = QColor(colour)
            new_colour.setRed(colour.red() * dim_factor)
            new_colour.setGreen(colour.green() * dim_factor)
            new_colour.setBlue(colour.blue() * dim_factor)
            return new_colour
        return colour

    def _get_trigger_image_key(self):
        evtype = self._trigger.get_type()
        expr = self._trigger.get_argument()
        texts = [f['text'] for f in self._fields]
        warn = self._use_warning_colours()
        return tuple([evtype, expr, self._inactive, warn] + texts)

    def _check_create_trigger_image(self):
        key = self._get_trigger_image_key()
        if self._cache.get_trigger_image(key):
            return

        lw_add = self._config['trigger']['line_width'] // 2
        image = QImage(
                self._total_width,
                self._config['tr_height'] + lw_add,
                QImage.Format_ARGB32_Premultiplied)
        image.fill(0)
        painter = QPainter(image)
        painter.translate(0, lw_add)
        painter.setCompositionMode(QPainter.CompositionMode_Plus)
        self._draw_trigger(painter, include_line=True, select=False)
        painter.end()
        self._cache.set_trigger_image(key, image)

    def _use_warning_colours(self):
        evtype = self._trigger.get_type()
        if evtype in ('.f', '/f'):
            arg = self._trigger.get_argument()
            # Use warning colour at high force levels
            try:
                value = float(arg)
                if (not math.isinf(value) and
                        not math.isnan(value) and
                        value > -self._force_shift):
                    return True
            except ValueError:
                pass

        elif evtype in ('m.v', 'm/v'):
            arg = self._trigger.get_argument()
            try:
                value = float(arg)
                if (not math.isinf(value) and
                        not math.isnan(value) and
                        value > 0):
                    return True
            except ValueError:
                pass

        return False

    def _draw_trigger(self, painter, include_line=True, select=False):
        # Select colour based on event type
        evtype = self._trigger.get_type()
        evtype_bg_colour = None
        evtype_fg_colour = self._config['trigger']['default_colour']
        if evtype == 'n+':
            evtype_fg_colour = self._config['trigger']['note_on_colour']
        elif evtype == 'h':
            evtype_fg_colour = self._config['trigger']['hit_colour']
        elif evtype == 'n-':
            evtype_fg_colour = self._config['trigger']['note_off_colour']
        elif self._use_warning_colours():
            evtype_bg_colour = self._config['trigger']['warning_bg_colour']
            evtype_fg_colour = self._config['trigger']['warning_fg_colour']

        if evtype_bg_colour:
            evtype_bg_colour = self._get_final_colour(evtype_bg_colour)
        evtype_fg_colour = self._get_final_colour(evtype_fg_colour)

        lw = self._config['trigger']['line_width']

        # Set colours
        painter.save()
        if select:
            evtype_fg_colour, evtype_bg_colour = evtype_bg_colour, evtype_fg_colour
        if evtype_bg_colour:
            height = self._config['tr_height']
            painter.fillRect(
                    QRect(0, 0, self._total_width - (lw // 2) - 1, height + lw - 1),
                    evtype_bg_colour)
        painter.setPen(evtype_fg_colour or self._config['bg_colour'])

        # Draw fields
        painter.setFont(self._config['font'])
        for i, field in enumerate(self._fields):
            painter.drawText(
                    QPoint(field['offset'], self._baseline_offset),
                    field['text'])

        painter.restore()

        # Draw line only if not obscured by cursor
        if include_line:
            painter.save()
            pen = QPen(evtype_fg_colour)
            pen.setWidthF(self._config['trigger']['line_width'])
            painter.setPen(pen)
            painter.drawLine(QPoint(0, 0), QPoint(self._total_width - 2, 0))
            painter.restore()

    def _make_field_data(self, offset, vis_text):
        metrics = self._config['font_metrics']
        return {
                'offset': offset,
                'width': metrics.boundingRect(vis_text).width(),
                'text': vis_text,
                }

    def _get_note_vis_name(self, expr):
        try:
            cents = float(expr)
            name = self._notation.get_full_name(cents)
        except ValueError:
            return expr

        if name:
            return name
        else:
            return expr

    def _setup_fields(self):
        evtype = self._trigger.get_type()
        expr = self._trigger.get_argument()

        metrics = self._config['font_metrics']
        padding_x = self._config['trigger']['padding_x']
        padding_y = self._config['trigger']['padding_y']

        self._baseline_offset = metrics.tightBoundingRect('A').height() + padding_y

        self._fields = []

        # Get field bounds
        if evtype == 'n+':
            note_name = self._get_note_vis_name(expr)
            note_field = self._make_field_data(padding_x, note_name)
            self._fields.append(note_field)
        elif evtype == 'h':
            hit_name = self._trigger.get_hit_name()
            hit_field = self._make_field_data(padding_x, hit_name)
            self._fields.append(hit_field)
        elif evtype == 'n-':
            vis_text = '══'
            note_off_field = self._make_field_data(padding_x, vis_text)
            self._fields.append(note_off_field)
        else:
            type_field = self._make_field_data(padding_x, evtype)
            self._fields.append(type_field)

            if expr != None:
                if self._trigger.get_argument_type() == events.EVENT_ARG_PITCH:
                    vis_text = self._get_note_vis_name(expr)
                else:
                    vis_text = expr

                arg_field = self._make_field_data(
                        type_field['offset'] + type_field['width'] + padding_x,
                        vis_text)
                self._fields.append(arg_field)

        # Width
        total_padding = padding_x * (len(self._fields) + 1)
        self._total_width = sum(f['width'] for f in self._fields) + total_padding


