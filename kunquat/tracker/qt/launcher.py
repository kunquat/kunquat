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

from ui import Ui


class QtLauncher():

    def __init__(self):
        self._frontend = None
        self._queue_processor = None

    def set_frontend(self, frontend):
        self._frontend = frontend

    def set_queue_processor(self, queue_processor):
        self._queue_processor = queue_processor

    def halt_ui(self):
        self._ui.halt()

    def run_ui(self):
        self._ui = Ui()
        self._ui.set_frontend(self._frontend)
        self._ui.set_queue_processor(self._queue_processor)
        self._ui.run()


