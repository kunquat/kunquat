# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .updater import Updater


class _IconView(QWidget):

    def __init__(self, button, image):
        super().__init__()
        self._button = button
        self._image = image

    def paintEvent(self, event):
        if not self._image:
            return

        width = self.width()
        height = self.height()

        painter = QPainter(self)
        painter.setRenderHint(QPainter.SmoothPixmapTransform)

        if not self._button.isEnabled():
            painter.setOpacity(0.5)

        img_rect = QRectF(0, 0, width, height)
        painter.drawImage(img_rect, self._image)

        painter.end()


class IconButton(QPushButton, Updater):

    def __init__(self, flat=False):
        super().__init__()
        self._size = None
        self._padding = None

        self.setFlat(flat)

        v = QVBoxLayout()
        self.setLayout(v)

    def _on_setup(self):
        super()._on_setup()
        self.register_action('signal_style_changed', self._update_style)

        style_mgr = self._ui_model.get_style_manager()
        if not self._size:
            self._size = style_mgr.get_style_param('tool_button_size')
        if not self._padding:
            self._padding = style_mgr.get_style_param('tool_button_padding')

        self._update_style()

    def set_sizes(self, size, padding):
        self._size = size
        self._padding = padding

        if self._ui_model:
            self._update_style()

    def set_icon(self, icon_name):
        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path(icon_name)

        icon_image = QImage()
        icon_image.load(icon_path)
        icon_image = icon_image.convertToFormat(QImage.Format_ARGB32_Premultiplied)
        icon_view = _IconView(self, icon_image)

        old_item = self.layout().takeAt(0)
        if old_item and old_item.widget():
            old_item.widget().deleteLater()
        self.layout().addWidget(icon_view)

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        size = style_mgr.get_scaled_size(self._size)
        self.setFixedSize(QSize(size, size))

        margin = style_mgr.get_scaled_size(self._padding)
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.update()


