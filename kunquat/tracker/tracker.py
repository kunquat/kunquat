# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013
#          Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import time

from kunquat.tracker.threading.audiothread import AudioThread
from kunquat.tracker.threading.backendthread import BackendThread
from kunquat.tracker.threading.frontendthread import FrontendThread


def main():
    audio_thread = AudioThread()
    backend_thread = BackendThread()
    frontend_thread = FrontendThread()

    audio_thread.set_backend(backend_thread)
    audio_thread.set_frontend(frontend_thread)
    backend_thread.set_frontend(frontend_thread)
    backend_thread.set_audio_output(audio_thread)
    frontend_thread.set_backend(backend_thread)
    frontend_thread.set_audio_output(audio_thread)

    audio_thread.start()
    backend_thread.start()
    frontend_thread.start()

    try:
        while frontend_thread.is_alive():
            print('main')
            time.sleep(1)
    except:
        pass

    frontend_thread.halt()
    backend_thread.halt()
    audio_thread.halt()
    frontend_thread.join()
    backend_thread.join()
    audio_thread.join()
    time.sleep(0.1)


