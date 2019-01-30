# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .filedialog import FileDialog
from kunquat.kunquat.limits import *
import kunquat.tracker.config as config


def get_kqt_file_path(ui_model, types):
    if types == set(['kqt']):
        caption = 'Open Kunquat module'
        def_dir_conf_key = 'dir_modules'
    elif types == set(['kqti', 'kqte']):
        caption = 'Open Kunquat instrument/effect'
        def_dir_conf_key = 'dir_instruments'
    elif types == set(['kqte']):
        caption = 'Open Kunquat effect'
        def_dir_conf_key = 'dir_effects'
    else:
        assert False

    filters = 0
    if 'kqt' in types:
        filters |= FileDialog.FILTER_KQT
    if 'kqti' in types:
        filters |= FileDialog.FILTER_KQTI
    if 'kqte' in types:
        filters |= FileDialog.FILTER_KQTE

    default_dir = config.get_config().get_value(def_dir_conf_key) or ''

    dialog = FileDialog(ui_model, FileDialog.MODE_OPEN, caption, default_dir, filters)
    paths = dialog.get_paths()
    return paths[0] if paths else None


def try_open_kqt_module(ui_model):
    file_path = get_kqt_file_path(ui_model, set(['kqt']))
    if file_path:
        process_mgr = ui_model.get_process_manager()
        process_mgr.new_kunquat(file_path)


def try_open_kqt_au(ui_model):
    file_path = get_kqt_file_path(ui_model, set(['kqti', 'kqte']))
    if file_path:
        open_kqt_au(file_path, ui_model, ui_model.get_module())


def open_kqt_au(au_path, ui_model, container):
    is_inside_instrument = not (container is ui_model.get_module())

    if au_path.endswith('.kqti'):
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

    elif au_path.endswith('.kqte'):
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
        super().__init__()

        error_img_path = icon_bank.get_icon_path('error')
        error_label = QLabel()
        error_label.setPixmap(QPixmap(error_img_path))

        self._message = QLabel()
        self._message.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

        h = QHBoxLayout()
        h.setContentsMargins(8, 8, 8, 8)
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

        ok_button.clicked.connect(self.close)


