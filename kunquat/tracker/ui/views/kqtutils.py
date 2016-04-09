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

from kunquat.kunquat.limits import *


def get_kqt_file_path(types):
    filters = []
    if types == set(['kqt', 'kqti', 'kqte']):
        caption = 'Open Kunquat file'
        filters.append('All Kunquat files'
            ' (*.kqt *.kqt.gz *.kqt.bz2'
            ' *.kqti *.kqti.gz *.kqti.bz2'
            ' *.kqte *.kqte.gz *.kqte.bz2)')
    elif types == set(['kqti', 'kqte']):
        caption = 'Open Kunquat instrument/effect'
        filters.append('Kunquat instruments and effects'
            ' (*.kqti *.kqti.gz *.kqti.bz2 *.kqte *.kqte.gz *.kqte.bz2)')
    elif types == set(['kqte']):
        caption = 'Open Kunquat effect'
    else:
        assert False

    if 'kqt' in types:
        filters.append('Kunquat compositions (*.kqt *.kqt.gz *.kqt.bz2)')
    if 'kqti' in types:
        filters.append('Kunquat instruments (*.kqti *.kqti.gz *.kqti.bz2)')
    if 'kqte' in types:
        filters.append('Kunquat effects (*.kqte *.kqte.gz *.kqte.bz2)')

    file_path_qstring = QFileDialog.getOpenFileName(
            caption=caption, filter=';;'.join(filters))
    if file_path_qstring:
        return str(file_path_qstring.toUtf8())
    return None


def open_kqt_au(au_path, ui_model, container):
    is_inside_instrument = not (container is ui_model.get_module())

    if au_path.endswith(('.kqti', '.kqti.gz', '.kqti.bz2')):
        au_id = container.get_free_au_id()
        if au_id == None:
            dialog = OutOfIDsErrorDialog(ui_model.get_icon_bank(), 'au')
            dialog.exec_()
            return
        if not is_inside_instrument:
            control_id = container.get_free_control_id()
            if control_id == None:
                dialog = OutOfIDsErrorDialog(ui_model.get_icon_bank(), 'control')
                dialog.exec_()
                return
        else:
            control_id = None
        container.start_import_au(au_path, au_id, control_id)

    elif au_path.endswith(('.kqte', '.kqte.gz', '.kqte.bz2')):
        au_id = container.get_free_au_id()
        if au_id == None:
            dialog = OutOfIDsErrorDialog(
                    ui_model.get_icon_bank(), 'au', is_inside_instrument)
            dialog.exec_()
            return
        container.start_import_au(au_path, au_id)

    else:
        assert False


class OutOfIDsErrorDialog(QDialog):

    def __init__(self, icon_bank, id_type, is_inside_instrument=False):
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
            if is_inside_instrument:
                message = ('<p>The containing instrument has reached the maximum of'
                    ' {} effects.</p>'.format(AUDIO_UNITS_MAX))
            else:
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


