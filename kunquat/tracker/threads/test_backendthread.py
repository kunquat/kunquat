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


class Recorder(object):

    def __init__(self):
        self._memory = {}

    def get_record(self, name):
        record = self._memory[name]
        return record

    def is_empty(self):
        empty = len(self._memory) < 1
        return empty

    def __getattribute__(self, name):
        try:
            attribute = object.__getattribute__(self, name)
            return attribute
        except:
            memory = self._memory
            def method(*args, **kwargs):
                record = (name, args, kwargs)
                memory[name] = record
            return method

class DummyFrontend():
    pass


class TestBackendthread(unittest.TestCase):

    def test_halt(self):
        backend_thread = BackendThread()
        backend_thread.halt()
        backend_thread.run()

    def test_argument_passing(self):
        calls = [
          ('set_frontend', (None,), {}),
          ('set_audio_output', (None,), {}),
          ('generate_audio', (123,), {}),
          ('set_data', ('pat_000/p_pattern.json', { 'length': [16, 0] }), {}),
          ('commit_data', (), {})
        ]
        backend_thread = BackendThread()
        recorder = Recorder()
        backend_thread.set_backend(recorder)
        for call in calls:
            (method, args, kwargs) = call
            getattr(backend_thread, method)(*args, **kwargs)
        self.assertTrue(recorder.is_empty())
        backend_thread.halt()
        backend_thread.run()
        for call in calls:
            (method, _, _) = call
            record = recorder.get_record(method)
            self.assertEqual(call, record)

if __name__ == '__main__':
    unittest.main()


