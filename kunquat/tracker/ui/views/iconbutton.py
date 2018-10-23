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

    def __init__(self, size, padding):
        super().__init__()
        self._size = size
        self._padding = padding
        self._icon_image = None

        v = QVBoxLayout()
        self.setLayout(v)

    def _on_setup(self):
        super()._on_setup()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path(self._get_icon_name())

        self._icon_image = QImage()
        self._icon_image.load(icon_path)
        self._icon_image = self._icon_image.convertToFormat(
                QImage.Format_ARGB32_Premultiplied)
        self._icon_view = _IconView(self, self._icon_image)
        self.layout().addWidget(self._icon_view)

        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def setEnabled(self, enabled):
        update_icon_view = (enabled != self.isEnabled())
        super().setEnabled(enabled)
        if update_icon_view:
            self._icon_view.update()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        size = style_mgr.get_scaled_size(self._size)
        self.setFixedSize(QSize(size, size))

        margin = style_mgr.get_scaled_size(self._padding)
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().addWidget(self._icon_view)

    # Protected interface

    def _get_icon_name(self):
        raise NotImplementedError


