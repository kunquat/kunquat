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
import commandqueue

from Queue import Empty
from command import Command
from threading import Thread
from commandqueue import CommandQueue

foo = Command('foo', None)
bar = Command('bar', None)
baz = Command('baz', None)

incorrect_arg = Command('foo', [])


def load_tests(loader, tests, ignore):
    tests.addTests(doctest.DocTestSuite(commandqueue))
    return tests


class TestCommandQueue(unittest.TestCase):

    def test_mutable_put(self):
        q = CommandQueue()
        self.assertRaises(TypeError, q.put, incorrect_arg)

    def test_order(self):
        q = CommandQueue()
        q.put(foo)
        q.put(bar)
        q.put(baz)
        self.assertEqual(q.get(), foo)
        self.assertEqual(q.get(), bar)
        self.assertEqual(q.get(), baz)

    def test_valid_types(self):
        q = CommandQueue()
        values = ['', None, Thread(), 2, long(2)]
        for value in values:
            command = Command('foo', value)
            q.put(command)

    def test_nowait(self):
        q = CommandQueue()
        self.assertRaises(Empty, q.get_nowait)
        
    def test_qsize(self):
        q = CommandQueue()
        self.assertEqual(q.qsize(), 0)
        q.put(foo)
        self.assertEqual(q.qsize(), 1)
        q.get()
        self.assertEqual(q.qsize(), 0)
        

if __name__ == '__main__':
    unittest.main()


