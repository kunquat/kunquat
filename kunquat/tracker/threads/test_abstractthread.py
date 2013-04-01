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


class TestAbstractThread():
    """
    Abstract thread tester class. The class inheriting this class also
    needs to inherit unittest.TestCase The abstract class can not inherit
    TestCase itself since it does not implement a standalone testcase.
    """

    def setUp(self):
        raise NotImplementedError

    def test_halt(self):
        self._thread.halt()
        self._thread.run()


if __name__ == '__main__':
    unittest.main()


