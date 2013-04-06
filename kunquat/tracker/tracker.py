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

from kunquat.tracker.frontend.drivers import Drivers
from kunquat.tracker.frontend.uimodel import UiModel
from kunquat.tracker.audio.audio import Audio
from kunquat.tracker.backend.backend import Backend
from kunquat.tracker.frontend.frontend import Frontend
from kunquat.tracker.qt.launcher import QtLauncher
from kunquat.tracker.threads.audiothread import AudioThread
from kunquat.tracker.threads.backendthread import BackendThread
from kunquat.tracker.threads.frontendthread import FrontendThread

from kunquat.tracker.audio.drivers.pulseaudio import Pulseaudio
from kunquat.tracker.audio.drivers.pushaudio import Pushaudio


def main():
    drivers = [Pulseaudio, Pushaudio]
    driver_manager = Drivers()
    driver_manager.set_drivers(drivers)
    ui_model = UiModel()
    ui_model.set_driver_manager(driver_manager)

    audio_output = Audio()
    backend = Backend()
    frontend = Frontend(ui_model)

    audio_thread = AudioThread()
    backend_thread = BackendThread()
    frontend_thread = FrontendThread()

    audio_thread.set_handler(audio_output)
    backend_thread.set_handler(backend)
    frontend_thread.set_handler(frontend)

    ui_launcher = QtLauncher()
    ui_launcher.set_ui_model(ui_model)
    frontend_thread.set_ui_launcher(ui_launcher)

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


