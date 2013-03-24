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

from launcher import QtLauncher


class Dummy():
    pass


class TestLauncher(unittest.TestCase):

    def test_setters(self):
        ui_launcher = QtLauncher()
        ui_launcher.set_frontend(Dummy())
        ui_launcher.set_queue_processor(Dummy())


if __name__ == '__main__':
    unittest.main()


