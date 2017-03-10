# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PySide.QtCore import *
from PySide.QtGui import *

from .config import *
from . import utils


class Header(QWidget):

    def __init__(self):
        super().__init__()

        self._config = DEFAULT_CONFIG

        self._col_width = DEFAULT_CONFIG['col_width']
        self._first_col = 0

        self._width = 0

        self._module = None

        self._headers = []

    def set_config(self, config):
        self._config = config
        for header in self._headers:
            header.set_config(self._config)
        self._update_contents()

    def set_module(self, module):
        self._module = module

    def set_column_width(self, width):
        self._col_width = width
        self._update_contents()

    def set_total_width(self, width):
        # We set our actual width manually because Qt assumes incorrect width
        self._width = width
        self._update_contents()

    def set_first_column(self, num):
        self._first_col = num
        self._update_contents()

    def update_header_aus(self):
        self._update_contents()

    def resizeEvent(self, ev):
        self._update_contents()

    def minimumSizeHint(self):
        w = self._headers[0] if self._headers else ColumnHeader()
        sh = w.minimumSizeHint()
        return QSize(self._col_width * 3, sh.height())

    def _resize_layout(self, max_visible_cols):
        visible_cols = utils.get_visible_cols(self._first_col, max_visible_cols)

        for i in range(len(self._headers), visible_cols):
            header = ColumnHeader()
            header.set_config(self._config)
            header.setParent(self)
            header.show()
            self._headers.append(header)
        for i in range(visible_cols, len(self._headers)):
            h = self._headers.pop()
            h.hide()

    def _update_contents(self):
        max_visible_cols = utils.get_max_visible_cols(self._width, self._col_width)

        self._first_col = utils.clamp_start_col(self._first_col, max_visible_cols)

        self._resize_layout(max_visible_cols)

        # Update headers
        for i, header in enumerate(self._headers):
            header.set_width(self._col_width)
            header.set_column(self._first_col + i, self._module)
            header.move(i * self._col_width, 0)
            header.setFixedWidth(self._col_width)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setBackground(self._config['canvas_bg_colour'])
        x_offset = (self._width // self._col_width) * self._col_width - 1
        painter.eraseRect(x_offset + 1, 0, self._width, self.height())

        if not self.isEnabled():
            painter.fillRect(
                    0, 0, self.width(), self.height(), self._config['disabled_colour'])


class ColumnHeader(QWidget):

    def __init__(self):
        super().__init__()
        self._config = DEFAULT_CONFIG
        self._num = None
        self._au_name = None
        self._pixmap = None
        self._width = 0

        self._text_height = 0
        self._baseline_offset = 0

    def set_config(self, config):
        self._config = config

        fm = QFontMetrics(self._config['header']['font'], self)
        self._text_height = fm.boundingRect('Ág').height()
        self._baseline_offset = fm.tightBoundingRect('Á').height()

    def set_width(self, width):
        self._width = width - self._config['header']['padding_x'] * 2

    def set_column(self, num, module):
        self._num = num

        self._au_name = None
        chd = module.get_channel_defaults()
        if chd:
            control_id = chd.get_default_control_id(self._num)
            control = module.get_control(control_id)
            if control.get_existence():
                au = control.get_audio_unit()
                if au.get_existence():
                    self._au_name = au.get_name()

        self._update_pixmap()

    def _update_pixmap(self):
        fm = QFontMetrics(self._config['header']['font'], self)

        assert self._num != None
        if self._au_name != None:
            vis_au_name = self._au_name or '-'
            full_text = '{}: {}'.format(self._num, vis_au_name)
            text = fm.elidedText(full_text, Qt.ElideRight, self._width)
        else:
            text = str(self._num)

        rect = fm.tightBoundingRect(text)
        rect.setHeight(self._text_height)
        self._pixmap = QPixmap(rect.size())

        painter = QPainter(self._pixmap)
        painter.setBackground(self._config['header']['bg_colour'])
        painter.setPen(self._config['header']['fg_colour'])
        painter.setFont(self._config['header']['font'])
        painter.eraseRect(0, 0, self._pixmap.width(), self._pixmap.height())
        painter.drawText(QPoint(0, self._baseline_offset), text)

        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)

        # Background
        painter.setBackground(self._config['header']['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        # Number
        num_width = self._pixmap.width()
        x_offset = (self.width() - num_width) // 2
        painter.drawPixmap(x_offset, 1, self._pixmap)

        # Border
        painter.setPen(self._config['header']['border_colour'])
        painter.drawLine(self.width() - 1, 0, self.width() - 1, self.height() - 1)

        # Grey out if disabled
        if not self.isEnabled():
            painter.fillRect(
                    0, 0, self.width(), self.height(), self._config['disabled_colour'])

    def minimumSizeHint(self):
        fm = QFontMetrics(self._config['header']['font'], self)
        return QSize(10, self._text_height)


