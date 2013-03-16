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

import Queue
import threading

from command import Command
from frontend.frontend import Frontend
from qt.ui import Ui


class FrontendThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._q = Queue.Queue()
        self._frontend = Frontend()
        self._ui = None

    def set_command_processor(self, cp):
        self._frontend.set_command_processor(cp)

    def queue_event(self, event):
        self._q.put(event)

    def halt(self):
        self.queue_event(Command('halt', None))

    def _process_queue(self):
        count_estimate = self._q.qsize()
        for _ in range(count_estimate):
            try:
                event = self._q.get_nowait()
                if event.name == 'halt':
                    self._ui.halt()
                else:
                    self._frontend.process_event(event)
            except Queue.Empty:
                break

    def run(self):
        self._ui = Ui()
        self._ui.set_frontend(self._frontend)
        self._ui.set_queue_processor(self._process_queue)
        self._ui.run()


