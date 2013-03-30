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


class Recorder(object):

    def __init__(self, put_record):
        self._put_record = put_record

    def __getattribute__(self, name):
        if name.startswith('_'):
            attr = object.__getattribute__(self, name)
            return attr
        recorder = self
        def newfunc(*args, **kwargs):
            record = (name, args, kwargs)
            recorder._put_record(name, args, kwargs)
        return newfunc



