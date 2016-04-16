# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2016
#          Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import os
import signal

from . import cmdline
from .errorbase import *
from .processes.audioprocess import create_audio_process
from .processes.uiprocess import create_ui_process


class Tracker():

    def __init__(self):
        pass

    def main(self):
        cmdline.parse_arguments()

        setup_basic_error_handler()

        audio_process = create_audio_process()
        ui_process = create_ui_process()
        audio_process.set_ui_engine(ui_process)
        ui_process.set_audio_engine(audio_process)
        audio_process.start()
        ui_process.start()

        try:
            ui_process.join()
        except KeyboardInterrupt:
            pass

        ui_process.halt()
        audio_process.halt()
        ui_process.join()
        audio_process.join()

def main():
    t = Tracker()
    t.main()

if __name__ == '__main__':
    main()


