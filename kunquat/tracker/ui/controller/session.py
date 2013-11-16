# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import UserDict


class Session(UserDict.DictMixin):

    def __init__(self):
        self._content = dict()
        self._content['output_speed'] = 0
        self._content['render_speed'] = 0
        self._content['render_load'] = 0
        self._content['progress_position'] = 1
        self._content['progress_steps'] = 1
        self._content['audio_levels'] = (0, 0)

    def __getitem__(self, key):
        return self._content[key]

    def __setitem__(self, key, value):
        self._content[key] = value

    def __delitem__(self, key):
        del self._content[key]

    def keys(self):
        return self._content.keys()

