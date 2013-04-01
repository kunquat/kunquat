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

from backend import Backend


class AudioDummy():

    def __init__(self):
        self._audio = None

    def put_audio(self, data):
        self._audio = data


class TestBackend(unittest.TestCase):

    def setUp(self):
        self._backend = Backend()

    def test_creating_two_frames_of_silence_succeeds(self):
        audio_dummy = AudioDummy()
        self._backend.set_audio_output(audio_dummy)
        self._backend.set_data('album/p_manifest.json', {})
        self._backend.set_data('album/p_tracks.json', [0])
        self._backend.set_data('song_00/p_manifest.json', {})
        self._backend.set_data('song_00/p_order_list.json', [ [0, 0] ])
        self._backend.set_data('pat_000/p_manifest.json', {})
        self._backend.set_data('pat_000/p_pattern.json', { 'length': [16, 0] })
        self._backend.set_data('pat_000/instance_000/p_manifest.json', {})
        self._backend.commit_data()
        self._backend.generate_audio(2)
        result = audio_dummy._audio
        expected = ([0.0, 0.0], [0.0, 0.0])
        self.assertEqual(result, expected)


if __name__ == '__main__':
    unittest.main()


