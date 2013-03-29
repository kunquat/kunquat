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

    def __getattribute__(self, name):
        put_record = object.__getattribute__(self, '_put_record')
        try:
            attribute = object.__getattribute__(self, name)
            return attribute
        except AttributeError:
            def method(*args, **kwargs):
                record = (name, args, kwargs)
                put_record(record)
            return method


class DummyFrontend(threading.Thread):
    pass


def public_interface(some_class):
    members = inspect.getmembers(Backend)
    interface = [name for (name, _) in members if not name.startswith('_')]
    return interface


class TestBackendthread(unittest.TestCase):

    def setUp(self):
        self._records = {}

    def _put_record(self, record):
        (name, _, _) = record
        self._records[name] = record

    def test_halt(self):
        backend_thread = BackendThread()
        backend_thread.halt()
        backend_thread.run()

    def test_interface(self):
        interface = set(public_interface(Backend))
        implementation = set(public_interface(BackendThread))
        missing_members = interface - implementation
        self.assertEqual(missing_members, set())

    def test_argument_passing(self):
        backend_calls = [
          ('set_frontend', (DummyFrontend(),), {}),
          ('set_audio_output', (None,), {}),
          ('generate_audio', (123,), {}),
          ('set_data', ('pat_000/p_pattern.json', { 'length': [16, 0] }), {}),
          ('commit_data', (), {})
        ]
        backend_thread = BackendThread()
        recorder = Recorder(self._put_record)
        backend_thread.set_backend(recorder)
        for call in backend_calls:
            (method, args, kwargs) = call
            getattr(backend_thread, method)(*args, **kwargs)
        self.assertTrue(len(self._records) < 1)
        backend_thread.halt()
        backend_thread.run()
        for call in backend_calls:
            (name, _, _) = call
            record = self._records[name]
            self.assertEqual(call, record)


if __name__ == '__main__':
    unittest.main()


