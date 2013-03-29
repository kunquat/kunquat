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

from kunquat.tracker.backend.backend import Backend
from backendthread import BackendThread


class Recorder(object):

    def __init__(self, put_record):
        self._put_record = put_record

    def __call__(self, name, *args, **kwargs):
        record = (name, args, kwargs)
        self._put_record(record)


class DummyFrontend(threading.Thread):
    pass


class DummyBackend():
    pass


def public_interface(some_class):
    members = inspect.getmembers(some_class)
    interface = [name for (name, _) in members if not name.startswith('_')]
    return interface


class TestBackendthread(unittest.TestCase):

    def setUp(self):
        self._test_calls = [
          ('set_frontend', (DummyFrontend(),), {}),
          ('set_audio_output', (None,), {}),
          ('generate_audio', (123,), {}),
          ('set_data', ('pat_000/p_pattern.json', { 'length': [16, 0] }), {}),
          ('commit_data', (), {})
        ]
        self._InterfaceClass = Backend
        self._TestClass = BackendThread

    def test_halt(self):
        thread = self._TestClass(None)
        thread.halt()
        thread.run()

    def test_interface(self):
        interface_spec = set(public_interface(self._InterfaceClass))
        implementation = set(public_interface(self._TestClass))
        missing_members = interface_spec - implementation
        self.assertEqual(missing_members, set())

    def _put_record(self, record):
        (name, _, _) = record
        self._records[name] = record

    def test_argument_passing(self):
        self._records = {}
        recorder = Recorder(self._put_record)
        thread = self._TestClass(recorder)
        for call in self._test_calls:
            (method, args, kwargs) = call
            getattr(thread, method)(*args, **kwargs)
        self.assertTrue(len(self._records) < 1)
        thread.halt()
        thread.run()
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


