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

from audiothread import AudioThread


class TestAudiothread(unittest.TestCase):

    def test_halt(self):
        class Fe():
            def update_drivers(self, drivers):
                pass
        audio_thread = AudioThread()
        audio_thread.set_frontend(Fe())
        audio_thread.halt()
        audio_thread.run()


if __name__ == '__main__':
    unittest.main()


