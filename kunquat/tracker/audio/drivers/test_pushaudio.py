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

import unittest

from pushaudio import Pushaudio
from test_abstract_driver import TestAbstractDriver


class TestPushaudio(TestAbstractDriver):

    def test_pushaudio(self):
        self.run_tests(Pushaudio)


if __name__ == '__main__':
    unittest.main()


