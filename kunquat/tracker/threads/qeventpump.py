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

from PyQt4.QtCore import QThread


class QEventPump(QThread):

    def __init__(self):
        QThread.__init__(self)
        self._signaler = None
        self._blocker = None

    def set_signaler(self, signaler):
        self._signaler = signaler

    def set_blocker(self, blocker):
        self._blocker = blocker

    def run(self):
        while True:
            self._blocker()
            self._signaler()


