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

from backendthread import BackendThread


class TestBackendthread(unittest.TestCase):

    def test_halt(self):
        backend_thread = BackendThread()
        backend_thread.halt()
        backend_thread.run()


if __name__ == '__main__':
    unittest.main()


