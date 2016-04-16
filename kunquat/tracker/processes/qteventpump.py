# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4.QtCore import *


class QtEventPump(QThread):

    process_queue = pyqtSignal(name='process_queue')

    def __init__(self):
        super().__init__()
        self._blocker = None

    def set_blocker(self, blocker):
        self._blocker = blocker

    def run(self):
        while True:
            self._blocker()
            self.emit(SIGNAL('process_queue()'))


