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

from kunquat.tracker.ui.views.updatingview import UpdatingView


class UpdatingAUView(UpdatingView):

    def __init__(self):
        self._au_id = None

    def set_au_id(self, au_id):
        self._au_id = au_id
        for widget in self._updating_children:
            widget.set_au_id(au_id)

    def add_updating_child(self, *widgets):
        if hasattr(self, '_au_id') and self._au_id != None:
            for widget in widgets:
                widget.set_au_id(self._au_id)
        super().add_updating_child(*widgets)


