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

from recorder import Recorder


class TestCommand(unittest.TestCase):

    def _put(self, name, args, kwargs):
        record = (name, args, kwargs)
        self._records[name] = record

    def test_recording(self):
        self._records = {}
        recorder = Recorder(self._put)
        recorder.test('foo', bar='baz')
        recorded = self._records['test']
        expected = ('test', ('foo',), {'bar': 'baz'})
        self.assertEqual(recorded, expected)
        

if __name__ == '__main__':
    unittest.main()

