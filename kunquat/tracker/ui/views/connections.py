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

import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *


DEFAULT_CONFIG = {
        'bg_colour': QColor(0x11, 0x11, 0x11),
    }


class Connections(QAbstractScrollArea):

    def __init__(self):
        QAbstractScrollArea.__init__(self)

        self.setViewport(ConnectionsView())
        self.viewport().setFocusProxy(None)

    def set_ui_model(self, ui_model):
        self.viewport().set_ui_model(ui_model)

    def unregister_updaters(self):
        self.viewport().unregister_updaters()

    def paintEvent(self, event):
        self.viewport().paintEvent(event)


class ConnectionsView(QWidget):

    def __init__(self, config={}):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._config = None
        self._set_config(config)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)
        self.setFocusPolicy(Qt.ClickFocus)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

    def _perform_updates(self, signals):
        pass

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        end = time.time()
        elapsed = end - start
        print('Connections view updated in {:.2f} ms'.format(elapsed * 1000))


