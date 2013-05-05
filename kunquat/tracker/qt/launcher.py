# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013
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

    def __init__(self, show=True):
        self._show = show
        self._ui_model = None
        self._queue_processor = None
        self._block = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def set_queue_processor(self, queue_processor, block):
        self._queue_processor = queue_processor
        self._block = block

    def halt_ui(self):
        self._ui.halt()

    def run_ui(self):
        self._ui = Ui()
        self._ui_model.set_ui(self._ui)
        self._ui.set_ui_model(self._ui_model)
        self._ui.set_queue_processor(self._queue_processor, self._block)
        if self._show == True:
            self._ui.show()
        self._ui.run()


