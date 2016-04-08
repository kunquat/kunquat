# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2016
#          Toni Ruottu, Finland 2014
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

from kunquat.kunquat.limits import *


class OpenButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None

        self.setText('Open')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        pass

    def _clicked(self):
        file_path_qstring = QFileDialog.getOpenFileName(
                caption='Open Kunquat Composition',
                filter='All Kunquat files'
                        ' (*.kqt *.kqt.gz *.kqt.bz2'
                        ' *.kqti *.kqti.gz *.kqti.bz2'
                        ' *.kqte *.kqte.gz *.kqte.bz2)'
                    ';;Kunquat compositions (*.kqt *.kqt.gz *.kqt.bz2)'
                    ';;Kunquat instruments (*.kqti *.kqti.gz *.kqti.bz2)'
                    ';;Kunquat effects (*.kqte *.kqte.gz *.kqte.bz2)')
        if file_path_qstring:
            file_path = str(file_path_qstring.toUtf8())

            if file_path.endswith(('.kqt', '.kqt.gz', 'kqt.bz2')):
                process_manager = self._ui_model.get_process_manager()
                process_manager.new_kunquat(file_path)

            elif file_path.endswith(('.kqti', '.kqti.gz', '.kqti.bz2')):
                module = self._ui_model.get_module()
                au_id = module.get_free_au_id()
                if au_id == None:
                    dialog = OutOfIDsErrorDialog(self._ui_model.get_icon_bank(), 'au')
                    dialog.exec_()
                    return
                control_id = module.get_free_control_id()
                if control_id == None:
                    dialog = OutOfIDsErrorDialog(
                            self._ui_model.get_icon_bank(), 'control')
                    dialog.exec_()
                    return
                module.start_import_au(file_path, au_id, control_id)

            elif file_path.endswith(('.kqte', '.kqte.gz', '.kqte.bz2')):
                module = self._ui_model.get_module()
                au_id = module.get_free_au_id()
                if au_id == None:
                    dialog = OutOfIDsErrorDialog(self._ui_model.get_icon_bank(), 'au')
                    dialog.exec_()
                    return
                module.start_import_au(file_path, au_id)


class OutOfIDsErrorDialog(QDialog):

    def __init__(self, icon_bank, id_type):
        QDialog.__init__(self)

        error_img_path = icon_bank.get_icon_path('error')
        error_label = QLabel()
        error_label.setPixmap(QPixmap(error_img_path))

        self._message = QLabel()
        self._message.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

        h = QHBoxLayout()
        h.setMargin(8)
        h.setSpacing(16)
        h.addWidget(error_label)
        h.addWidget(self._message)

        self._button_layout = QHBoxLayout()

        v = QVBoxLayout()
        v.addLayout(h)
        v.addLayout(self._button_layout)

        self.setLayout(v)

        # Dialog contents

        if id_type == 'au':
            title = 'Out of instrument/effect IDs'
            message = ('<p>The composition has reached the maximum of {}'
                ' instruments and/or effects.</p>'.format(AUDIO_UNITS_MAX))
        elif id_type == 'control':
            title = 'Out of instrument control IDs'
            message = ('<p>The composition has reached the maximum of {}'
                ' instrument controls.</p>'.format(CONTROLS_MAX))

        self.setWindowTitle(title)

        self._message.setText(message)

        ok_button = QPushButton('OK')
        self._button_layout.addStretch(1)
        self._button_layout.addWidget(ok_button)
        self._button_layout.addStretch(1)

        QObject.connect(ok_button, SIGNAL('clicked()'), self.close)


