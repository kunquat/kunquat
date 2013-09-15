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

import doctest
import unittest
import command

from command import Command
from Queue import Queue
from threading import Thread


def load_tests(loader, tests, ignore):
    tests.addTests(doctest.DocTestSuite(command))
    return tests


class TestCommand(unittest.TestCase):

    def test_valid_types(self):
        name = 'foo'
        args = ['', None, Thread(), 2, long(2), ([0],[0]), Queue]
        command = Command(name, *args)
        self.assertEqual(command.name, name)
        for (received, expected) in zip(command.args, args):
            self.assertEqual(received, expected)
        

if __name__ == '__main__':
    unittest.main()

