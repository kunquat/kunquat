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

from kunquat import Handle


class TestKunquat(unittest.TestCase):

    def test_handle_creation(self):
        handle = Handle()
        self.assertEqual(type(handle), Handle)


if __name__ == '__main__':
    unittest.main()


