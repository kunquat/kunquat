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

import kunquat as wrapper
from kunquat import Kunquat


class TestKunquat(unittest.TestCase):

    def test_handle_creation_succeeds(self):
        handle = Kunquat()
        self.assertEqual(type(handle), Kunquat)

    def test_handle_creation_raises_memory_error(self):
        wrapper.fake_out_of_memory()
        self.assertRaises(MemoryError, Kunquat)


if __name__ == '__main__':
    unittest.main()


