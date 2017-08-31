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

from kunquat.tracker.ui.qt import *

from .config import *
from . import utils


class Header(QWidget):

    def __init__(self):
        super().__init__()

        self._config = DEFAULT_CONFIG

        self._col_width = DEFAULT_CONFIG['col_width']
        self._first_col = 0

        self._width = 0

        self._ui_model = None

        self._headers = []

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def unregister_updaters(self):
        for header in self._headers:
            header.unregister_updaters()

    def set_config(self, config):
        assert self._ui_model
        self._config = config
        for header in self._headers:
            header.set_config(self._config)
            header.set_ui_model(self._ui_model)
        self._update_contents()

    def set_column_width(self, width):
        self._col_width = width
        self._update_contents()

    def set_total_width(self, width):
        # We set our actual width manually because Qt assumes incorrect width
        self._width = width
        self._update_contents()

    def set_first_column(self, num):
        if num != self._first_col:
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
            header.set_ui_model(self._ui_model)
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
        module = self._ui_model.get_module()
        for i, header in enumerate(self._headers):
            header.set_width(self._col_width)
            header.set_column(self._first_col + i, module)
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

        self._ui_model = None

        self._menu = ColumnMenu(self)

        self.setMouseTracking(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._menu.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._menu.unregister_updaters()

    def set_config(self, config):
        self._config = config

        fm = QFontMetrics(self._config['header']['font'], self)
        self._text_height = fm.boundingRect('Ág').height() + 1
        self._baseline_offset = fm.tightBoundingRect('Á').height()

    def set_width(self, width):
        self._width = width - self._config['header']['padding_x'] * 2

    def set_column(self, num, module):
        self._num = num

        self._menu.set_column(self._num)

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

        max_width = self._width - self._config['border_width'] * 2

        assert self._num != None
        if self._au_name:
            full_text = '{}: {}'.format(self._num, self._au_name)
            text = fm.elidedText(full_text, Qt.ElideRight, max_width)
        else:
            text = str(self._num)

        rect = fm.tightBoundingRect(text)
        rect.setRight(rect.right() + 1)
        rect.setHeight(self._text_height)
        self._pixmap = QPixmap(rect.size())

        painter = QPainter(self._pixmap)
        painter.setBackground(self._config['header']['bg_colour'])
        painter.setPen(self._config['header']['fg_colour'])
        painter.setFont(self._config['header']['font'])
        painter.eraseRect(0, 0, self._pixmap.width(), self._pixmap.height())
        painter.drawText(QPoint(0, self._baseline_offset), text)

        self.update()

    def mousePressEvent(self, event):
        if event.buttons() == Qt.RightButton:
            self._menu.popup(self.mapToGlobal(QPoint(event.x(), event.y())))
            return

        if event.buttons() != Qt.LeftButton:
            return

        playback_mgr = self._ui_model.get_playback_manager()
        if event.modifiers() == Qt.ShiftModifier:
            solo = playback_mgr.get_channel_solo(self._num)
            playback_mgr.set_channel_solo(self._num, not solo)
        else:
            solo = playback_mgr.get_channel_solo(self._num)
            mute = playback_mgr.get_channel_mute(self._num)
            playback_mgr.set_channel_mute(self._num, not mute and not solo)

        self._menu.update_actions()

        updater = self._ui_model.get_updater()
        updater.signal_update('signal_channel_mute')

    def _get_border_colours(self):
        style_mgr = self._ui_model.get_style_manager()
        param = 'sheet_header_bg_colour'
        contrast = self._config['border_contrast']
        light = QColor(style_mgr.get_adjusted_colour(param, contrast))
        dark = QColor(style_mgr.get_adjusted_colour(param, -contrast))
        return light, dark

    def paintEvent(self, event):
        painter = QPainter(self)

        border_width = self._config['border_width']

        # Background
        painter.setBackground(self._config['header']['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        # Text
        text_width = self._pixmap.width()
        max_width = self.width() - border_width * 2
        x_offset = border_width + (max_width - text_width) // 2
        painter.drawPixmap(x_offset, 1, self._pixmap)

        # Border
        border_light, border_dark = self._get_border_colours()
        if border_width > 1:
            painter.fillRect(
                    QRect(QPoint(0, 0), QSize(border_width, self.height() - 1)),
                    border_light)
            painter.fillRect(
                    QRect(QPoint(self.width() - border_width, 0),
                        QSize(border_width, self.height() - 1)),
                    border_dark)
        else:
            painter.setPen(border_light)
            painter.drawLine(0, 0, 0, self.height() - 1)
            painter.setPen(border_dark)
            painter.drawLine(self.width() - 1, 0, self.width() - 1, self.height() - 1)

        # Apply solo/mute shade
        playback_mgr = self._ui_model.get_playback_manager()
        if playback_mgr.get_channel_solo(self._num):
            solo_shade = self._config['header']['solo_colour']
            painter.fillRect(0, 0, self.width(), self.height(), solo_shade)
        elif not playback_mgr.is_channel_active(self._num):
            mute_shade = QColor(self._config['canvas_bg_colour'])
            mute_shade.setAlpha(0x7f)
            painter.fillRect(0, 0, self.width(), self.height(), mute_shade)

        # Grey out if disabled
        if not self.isEnabled():
            painter.fillRect(
                    0, 0, self.width(), self.height(), self._config['disabled_colour'])

    def minimumSizeHint(self):
        fm = QFontMetrics(self._config['header']['font'], self)
        return QSize(10, self._text_height)


class ColumnMenu(QMenu):

    def __init__(self, parent):
        super().__init__(parent)
        self._num = None
        self._ui_model = None
        self._updater = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = self._ui_model.get_updater()

    def set_column(self, num):
        assert self._ui_model
        self._num = num
        self.update_actions()

    def unregister_updaters(self):
        pass

    def update_actions(self):
        self.clear()

        playback_mgr = self._ui_model.get_playback_manager()

        self.addAction(
                'Unmute' if playback_mgr.get_channel_mute(self._num) else 'Mute',
                self._mute_action)
        self.addAction(
                'Unsolo' if playback_mgr.get_channel_solo(self._num) else 'Solo',
                self._solo_action)

    def _mute_action(self):
        playback_mgr = self._ui_model.get_playback_manager()
        mute = playback_mgr.get_channel_mute(self._num)
        playback_mgr.set_channel_mute(self._num, not mute)
        self._updater.signal_update('signal_channel_mute')

    def _solo_action(self):
        playback_mgr = self._ui_model.get_playback_manager()
        solo = playback_mgr.get_channel_solo(self._num)
        playback_mgr.set_channel_solo(self._num, not solo)
        self._updater.signal_update('signal_channel_mute')


