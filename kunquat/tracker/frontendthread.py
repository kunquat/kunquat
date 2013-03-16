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


class FrontendThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._q = Queue.Queue()
        self._frontend = Frontend()

    def set_command_processor(self, cp):
        self._frontend.set_command_processor(cp)

    def queue_event(self, event):
        self._q.put(event)

    def halt(self):
        self.queue_event(Command('halt', None))

    def run(self):
        event = self._q.get()
        while event.name != 'halt':
            self._frontend.process_event(event)
            event = self._q.get()


