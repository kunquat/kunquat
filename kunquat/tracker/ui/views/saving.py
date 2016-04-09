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

from PyQt4.QtCore import *
from PyQt4.QtGui import *


def get_module_save_path():
    module_path_qstring = QFileDialog.getSaveFileName(
            caption='Save Kunquat composition',
            filter='Kunquat compositions (*.kqt *.kqt.gz *.kqt.bz2)')
    if not module_path_qstring:
        return None
    module_path = str(module_path_qstring.toUtf8())
    return module_path


def _get_suggested_au_base_file_name(au_name):
    return ''.join(c for c in au_name if c.isalnum() or c in ' ').strip()


def get_instrument_save_path(au_name, instruments_dir):
    suggested_name = _get_suggested_au_base_file_name(au_name) + '.kqti.bz2'
    suggested_path = os.path.join(instruments_dir, suggested_name)
    au_path_qstring = QFileDialog.getSaveFileName(
            caption='Save Kunquat instrument',
            directory=suggested_path,
            filter='Kunquat instruments (*.kqti *.kqti.gz *.kqti.bz2)')
    if not au_path_qstring:
        return None
    au_path = str(au_path_qstring.toUtf8())
    return au_path


def get_effect_save_path(au_name, effects_dir):
    suggested_name = _get_suggested_au_base_file_name(au_name) + '.kqte.bz2'
    suggested_path = os.path.join(effects_dir, suggested_name)
    au_path_qstring = QFileDialog.getSaveFileName(
            caption='Save Kunquat effect',
            directory=suggested_path,
            filter='Kunquat effects (*.kqte *.kqte.gz *.kqte.bz2)')
    if not au_path_qstring:
        return None
    au_path = str(au_path_qstring.toUtf8())
    return au_path


