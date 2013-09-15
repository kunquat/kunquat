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

incorrect_arg = ('foo', 1, 2, 3)


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
        q.block()
        self.assertEqual(q.get(), foo)
        q.block()
        self.assertEqual(q.get(), bar)
        q.block()
        self.assertEqual(q.get(), baz)

    def test_valid_types(self):
        q = CommandQueue()
        values = ['', None, Thread(), 2, long(2)]
        for value in values:
            command = Command('foo', value)
            q.put(command)

    def test_push(self):
        q = CommandQueue()
        name = 'test'
        args = [1, 2, 3]
        q.push(name, *args)
        q.block()
        command = q.get()
        self.assertEqual(command.name, 'test')
        self.assertEqual(command.args, args)


if __name__ == '__main__':
    unittest.main()


