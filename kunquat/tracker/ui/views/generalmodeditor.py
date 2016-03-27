# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
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


class GeneralModEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._title = Title()

        self._mixing_volume = MixingVolume()

        ml = QGridLayout()
        ml.setMargin(0)
        ml.setHorizontalSpacing(4)
        ml.setVerticalSpacing(2)
        ml.addWidget(QLabel('Title:'), 0, 0)
        ml.addWidget(self._title, 0, 1)

        separator = QFrame()
        separator.setFrameShape(QFrame.HLine)
        separator.setFrameShadow(QFrame.Sunken)
        separator.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Maximum)
        separator.setMinimumHeight(2)

        gl = QGridLayout()
        gl.setMargin(0)
        gl.setHorizontalSpacing(4)
        gl.setVerticalSpacing(2)
        gl.setColumnStretch(1, 1)
        gl.addWidget(QLabel('Mixing volume:'), 0, 0)
        gl.addWidget(self._mixing_volume, 0, 1)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(4)
        v.addLayout(ml)
        v.addWidget(separator)
        v.addLayout(gl)
        v.addStretch(1)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._title.set_ui_model(ui_model)
        self._mixing_volume.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._mixing_volume.unregister_updaters()
        self._title.unregister_updaters()


class Title(QLineEdit):

    def __init__(self):
        QLineEdit.__init__(self)
        self._ui_model = None
        self._updater = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('textEdited(const QString&)'), self._change_title)

        self._update_title()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_title' in signals:
            self._update_title()

    def _update_title(self):
        module = self._ui_model.get_module()
        title = module.get_title() or u''

        if self.text() != title:
            old_block = self.blockSignals(True)
            self.setText(title)
            self.blockSignals(old_block)

    def _change_title(self, text_qstring):
        title = unicode(text_qstring)
        module = self._ui_model.get_module()
        module.set_title(title)


class MixingVolume(QDoubleSpinBox):

    def __init__(self):
        QDoubleSpinBox.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setDecimals(2)
        self.setRange(-384, 18)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('valueChanged(double)'), self._change_mixing_volume)

        self._update_mixing_volume()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_mixing_volume' in signals:
            self._update_mixing_volume()

    def _update_mixing_volume(self):
        module = self._ui_model.get_module()
        mix_vol = module.get_mixing_volume()
        if self.value() != mix_vol:
            old_block = self.blockSignals(True)
            self.setValue(mix_vol)
            self.blockSignals(old_block)

    def _change_mixing_volume(self, value):
        module = self._ui_model.get_module()
        module.set_mixing_volume(value)


