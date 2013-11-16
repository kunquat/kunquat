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

import threading


class EventPump(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self, name="EventPump")
        self.daemon = True
        self._signaler = None
        self._blocker = None

    def set_signaler(self, signaler):
        self._signaler = signaler

    def set_blocker(self, blocker):
        self._blocker = blocker

    def run(self):
        while True:
            print 'ep'
            self._blocker()
            if self._signaler:
                self._signaler()

