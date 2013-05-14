# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import unittest

import benchaudio
from benchaudio import Benchaudio
from test_abstract_driver import TestAbstractDriver


class TestBenchaudio(TestAbstractDriver, unittest.TestCase):

    def setUp(self):
        self._DriverClass = Benchaudio


if __name__ == '__main__':
    unittest.main()


