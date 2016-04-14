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
import time
import signal

from . import cmdline
from kunquat.tracker.errorbase import *
from kunquat.tracker.threads.audiothread import create_audio_thread
from kunquat.tracker.threads.uithread import create_ui_thread


class Tracker():

    def __init__(self):
        pass

    def main(self):
        cmdline.parse_arguments()

        setup_basic_error_handler()
        audio_thread = create_audio_thread()
        ui_thread = create_ui_thread()
        audio_thread.set_ui_engine(ui_thread)
        ui_thread.set_audio_engine(audio_thread)
        audio_thread.start()
        ui_thread.start()

        try:
            while ui_thread.is_alive():
                time.sleep(1)
        except KeyboardInterrupt:
            pass

        ui_thread.halt()
        audio_thread.halt()
        ui_thread.join()
        audio_thread.join()
        time.sleep(0.1)
        pgrp = os.getpgrp()
        #os.killpg(pgrp, signal.SIGHUP)
        #time.sleep(1)
        #os.killpg(pgrp, signal.SIGKILL)

def main():
    t = Tracker()
    t.main()

if __name__ == '__main__':
    main()


