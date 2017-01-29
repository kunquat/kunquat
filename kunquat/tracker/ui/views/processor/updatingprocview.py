# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.views.audiounit.updatingauview import UpdatingAUView


class UpdatingProcView(UpdatingAUView):

    def __init__(self):
        self._proc_id = None

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        for widget in self._updating_children:
            widget.set_proc_id(proc_id)

    def add_updating_child(self, *widgets):
        if hasattr(self, '_proc_id') and self._proc_id != None:
            for widget in widgets:
                widget.set_proc_id(self._proc_id)
        super().add_updating_child(*widgets)


