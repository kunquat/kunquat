# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2015
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

import kunquat.tracker.cmdline as cmdline
from editbutton import EditButton
from replacebutton import ReplaceButton
from restbutton import RestButton
from delselectionbutton import DelSelectionButton
from zoombutton import ZoomButton
from lengtheditor import LengthEditor


class Toolbar(QWidget):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None

        self._edit_button = EditButton()
        self._replace_button = ReplaceButton()
        self._rest_button = RestButton()
        self._del_selection_button = DelSelectionButton()
        self._zoom_buttons = [
                ZoomButton('out'),
                ZoomButton('original'),
                ZoomButton('in'),
                ZoomButton('shrink_w'),
                ZoomButton('original_w'),
                ZoomButton('expand_w'),
            ]
        self._grid_toggle = GridToggle()
        self._grid_editor_button = GridEditorButton()
        self._length_editor = LengthEditor()

        h = QHBoxLayout()
        h.setContentsMargins(4, 0, 4, 4)
        h.setSpacing(5)

        h.addWidget(self._edit_button)
        h.addWidget(self._replace_button)
        h.addWidget(HackSeparator())
        h.addWidget(self._rest_button)
        if cmdline.get_experimental():
            h.addWidget(self._del_selection_button)
        h.addWidget(HackSeparator())
        for button in self._zoom_buttons:
            h.addWidget(button)
        h.addWidget(HackSeparator())
        h.addWidget(self._grid_toggle)
        h.addWidget(self._grid_editor_button)

        spacer = QWidget()
        spacer.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        h.addWidget(spacer)

        h.addWidget(self._length_editor)

        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._edit_button.set_ui_model(ui_model)
        self._replace_button.set_ui_model(ui_model)
        self._rest_button.set_ui_model(ui_model)
        self._del_selection_button.set_ui_model(ui_model)
        for button in self._zoom_buttons:
            button.set_ui_model(ui_model)
        self._grid_toggle.set_ui_model(ui_model)
        self._grid_editor_button.set_ui_model(ui_model)
        self._length_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._edit_button.unregister_updaters()
        self._replace_button.unregister_updaters()
        self._rest_button.unregister_updaters()
        self._del_selection_button.unregister_updaters()
        for button in self._zoom_buttons:
            button.unregister_updaters()
        self._grid_toggle.unregister_updaters()
        self._grid_editor_button.unregister_updaters()
        self._length_editor.unregister_updaters()


class GridToggle(QCheckBox):

    def __init__(self):
        QCheckBox.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setText('Grid')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('clicked()'), self._set_grid_enabled)

        self._update_state()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_grid' in signals:
            self._update_state()

    def _update_state(self):
        sheet_manager = self._ui_model.get_sheet_manager()
        is_grid_enabled = sheet_manager.is_grid_enabled()

        old_block = self.blockSignals(True)
        self.setCheckState(Qt.Checked if is_grid_enabled else Qt.Unchecked)
        self.blockSignals(old_block)

    def _set_grid_enabled(self):
        enabled = (self.checkState() == Qt.Checked)

        sheet_manager = self._ui_model.get_sheet_manager()
        sheet_manager.set_grid_enabled(enabled)
        self._updater.signal_update(set(['signal_grid']))


class GridEditorButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setStyleSheet('padding: 4px 0 4px 0;')

        self.setAutoRaise(True)
        self.setText('Edit grid')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('clicked()'), self._open_grid_editor)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_grid' in signals:
            self._update_enabled()

    def _update_enabled(self):
        sheet_manager = self._ui_model.get_sheet_manager()
        is_grid_enabled = sheet_manager.is_grid_enabled()
        self.setEnabled(is_grid_enabled)

    def _open_grid_editor(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_grid_editor()


class HackSeparator(QFrame):

    def __init__(self):
        QFrame.__init__(self)
        self.setFrameShape(QFrame.VLine)
        self.setFrameShadow(QFrame.Sunken)


