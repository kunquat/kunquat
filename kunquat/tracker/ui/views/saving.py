# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2015-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import os.path

from PySide.QtCore import *
from PySide.QtGui import *

import kunquat.tracker.config as config


def get_module_save_path():
    default_dir = config.get_config().get_value('dir_modules') or ''
    module_path, _ = QFileDialog.getSaveFileName(
            caption='Save Kunquat composition',
            dir=default_dir,
            filter='Kunquat compositions (*.kqt *.kqt.gz *.kqt.bz2)')
    if not module_path:
        return None
    return module_path


def _get_suggested_au_base_file_name(au_name):
    return ''.join(c for c in au_name if c.isalnum() or c in ' ').strip()


def get_instrument_save_path(au_name, instruments_dir):
    suggested_path = instruments_dir
    if au_name:
        suggested_base_name = _get_suggested_au_base_file_name(au_name)
        if suggested_base_name:
            suggested_name = suggested_base_name + '.kqti.bz2'
            suggested_path = os.path.join(instruments_dir, suggested_name)
    au_path, _ = QFileDialog.getSaveFileName(
            caption='Save Kunquat instrument',
            dir=suggested_path,
            filter='Kunquat instruments (*.kqti *.kqti.gz *.kqti.bz2)')
    if not au_path:
        return None
    return au_path


def get_effect_save_path(au_name, effects_dir):
    suggested_path = effects_dir
    if au_name:
        suggested_base_name = _get_suggested_au_base_file_name(au_name)
        if suggested_base_name:
            suggested_name = suggested_base_name + '.kqte.bz2'
            suggested_path = os.path.join(effects_dir, suggested_name)
    au_path, _ = QFileDialog.getSaveFileName(
            caption='Save Kunquat effect',
            dir=suggested_path,
            filter='Kunquat effects (*.kqte *.kqte.gz *.kqte.bz2)')
    if not au_path:
        return None
    return au_path


