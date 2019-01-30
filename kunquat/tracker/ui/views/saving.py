# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2015-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import os.path

from kunquat.tracker.ui.qt import *

from .filedialog import FileDialog
import kunquat.tracker.config as config


def try_save_module(ui_model, save_as=False):
    module = ui_model.get_module()

    if (not module.get_path()) or save_as:
        module_path = get_module_save_path(ui_model)
        if not module_path:
            return
        module.set_path(module_path)

    module.start_save()


def get_module_save_path(ui_model):
    default_dir = config.get_config().get_value('dir_modules') or ''
    caption = 'Save Kunquat composition'
    filters = FileDialog.FILTER_ALL_KQT | FileDialog.FILTER_ALL_PCM
    dialog = FileDialog(ui_model, FileDialog.MODE_SAVE, caption, default_dir, filters)
    module_path = dialog.get_path()
    if not module_path:
        #print('Not saving')
        return None

    #print('Saving {}'.format(module_path))
    return module_path


def _get_suggested_au_base_file_name(au_name):
    return ''.join(c for c in au_name if c.isalnum() or c in ' ').strip()


def get_instrument_save_path(au_name):
    default_dir = config.get_config().get_value('dir_instruments') or ''
    suggested_path = default_dir
    if au_name:
        suggested_base_name = _get_suggested_au_base_file_name(au_name)
        if suggested_base_name:
            suggested_name = suggested_base_name + '.kqti'
            suggested_path = os.path.join(default_dir, suggested_name)
    au_path, _ = QFileDialog.getSaveFileName(
            None,
            'Save Kunquat instrument',
            suggested_path,
            'Kunquat instruments (*.kqti)')
    if not au_path:
        return None

    if not au_path.endswith('.kqti'):
        au_path += '.kqti'
    return au_path


def get_effect_save_path(au_name):
    default_dir = config.get_config().get_value('dir_effects') or ''
    suggested_path = default_dir
    if au_name:
        suggested_base_name = _get_suggested_au_base_file_name(au_name)
        if suggested_base_name:
            suggested_name = suggested_base_name + '.kqte'
            suggested_path = os.path.join(default_dir, suggested_name)
    au_path, _ = QFileDialog.getSaveFileName(
            None,
            'Save Kunquat effect',
            suggested_path,
            'Kunquat effects (*.kqte)')
    if not au_path:
        return None

    if not au_path.endswith('.kqte'):
        au_path += '.kqte'
    return au_path


