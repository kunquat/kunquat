# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.kunquat.kunquat import Kunquat, KunquatFormatError
import kunquat.tracker.cmdline as cmdline
from .dataconverters import VersionError, UnsupportedVersionError


class KqtiValidator():

    def __init__(self, contents, data_converters):
        self._validator = Kunquat()
        self._validator.set_loader_thread_count(cmdline.get_default_thread_count())
        self._contents = contents
        self._data_converters = data_converters
        self._validation_error = None
        self._progress = 0

    def get_progress(self):
        return self._progress

    def get_validation_steps(self):
        target_prefix = 'au_00'
        step_count = len(self._contents.items())
        for i, (au_key, value) in enumerate(self._contents.items()):
            yield
            key = '/'.join((target_prefix, au_key))

            try:
                self._data_converters.convert_key_and_data(key, value)
            except UnsupportedVersionError as e:
                version_data = self._contents.get('m_editor_version.json')
                self._validation_error = e.get_message('audio unit', version_data)
                break
            except VersionError as e:
                self._validation_error = e.args[0]
                break

            self._validator.set_data(key, value)
            self._progress = i / step_count

    def is_valid(self):
        if self._validation_error:
            return False

        try:
            self._validator.validate()
        except KunquatFormatError as e:
            self._validation_error = e['message']
            return False
        return True

    def get_validation_error(self):
        return self._validation_error


