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

from kunquat.kunquat.kunquat import Kunquat, KunquatFormatError


class KqtiValidator():

    def __init__(self, contents):
        self._validator = Kunquat()
        self._contents = contents
        self._validation_error = None

    def get_validation_steps(self):
        target_prefix = 'au_00'
        for (au_key, value) in self._contents.items():
            yield
            key = '/'.join((target_prefix, au_key))
            self._validator.set_data(key, value)

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


