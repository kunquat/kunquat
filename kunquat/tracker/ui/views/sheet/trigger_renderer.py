# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from config import *


class TriggerRenderer():

    def __init__(self, config, trigger):
        assert trigger
        self._config = config
        self._trigger = trigger

        self._setup_fields()

    def get_field_count(self):
        return len(self._fields)

    def get_field_bounds(self, index):
        offset = self._fields[index]['offset']
        width = self._fields[index]['width']
        return (offset, width)

    def get_total_width(self):
        return self._total_width

    def draw_trigger(self, painter, include_line=True, select=None):
        # TODO: select colour based on event type
        evtype_fg_colour = self._config['trigger']['default_colour']

        # Draw fields
        for i, field in enumerate(self._fields):
            painter.save()

            # Set colours
            if select == i:
                painter.setBackgroundMode(Qt.OpaqueMode)
                painter.setBackground(QBrush(evtype_fg_colour))
                painter.setPen(self._config['bg_colour'])
            else:
                painter.setPen(evtype_fg_colour)

            painter.drawText(
                    QPoint(field['offset'], self._baseline_offset),
                    field['text'])

            painter.restore()

        # Draw line only if not obscured by cursor
        if include_line:
            painter.save()
            painter.setPen(evtype_fg_colour)
            painter.drawLine(QPoint(0, 0), QPoint(self._total_width - 2, 0))
            painter.restore()

    def _setup_fields(self): # TODO: note names
        evtype = self._trigger.get_type()
        expr = self._trigger.get_argument()

        metrics = self._config['font_metrics']
        padding = self._config['trigger']['padding']

        self._baseline_offset = metrics.tightBoundingRect('A').height()

        self._fields = []

        # Get field bounds, TODO: handle special cases for Note On, Hit and Note Off

        type_field = {
                'offset': padding,
                'width': metrics.boundingRect(evtype).width(),
                'text': evtype,
                }
        self._fields.append(type_field)

        if expr != None:
            arg_field = {
                    'offset': type_field['offset'] + type_field['width'] + padding,
                    'width': metrics.boundingRect(expr).width(),
                    'text': expr,
                    }
            self._fields.append(arg_field)

        # Width
        total_padding = padding * (len(self._fields) + 1)
        self._total_width = sum(f['width'] for f in self._fields) + total_padding


