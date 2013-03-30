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

import inspect
import unittest
import threading

from kunquat.tools.recorder import Recorder


def public_interface(some_class):
    members = inspect.getmembers(some_class)
    interface = [name for (name, _) in members if not name.startswith('_')]
    return interface


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

    def test_interface(self):
        interface_spec = set(public_interface(self._InterfaceClass))
        implementation = set(public_interface(self._TestClass))
        missing_members = interface_spec - implementation
        self.assertEqual(missing_members, set())

    def _put_record(self, record):
        (name, _, _) = record
        self._records[name] = record

    def broken_test_argument_passing(self):
        self._records = {}
        recorder = Recorder(self._put_record)
        self._thread.set_handler(recorder)
        for call in self._test_calls:
            (method, args, kwargs) = call
            getattr(self._thread, method)(*args, **kwargs)
        self.assertTrue(len(self._records) < 1)
        self._thread.halt()
        self._thread.run()
        for call in self._test_calls:
            (name, _, _) = call
            record = self._records[name]
            self.assertEqual(call, record)
        called_methods = set(self._records.keys())
        interface_spec = set(public_interface(self._InterfaceClass))
        not_tested = interface_spec - called_methods
        self.assertEqual(not_tested, set())


if __name__ == '__main__':
    unittest.main()


