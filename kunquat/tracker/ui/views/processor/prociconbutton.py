# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.views.iconbutton import IconButton


class ProcessorIconButton(IconButton):

    def __init__(self):
        super().__init__()

    def set_au_id(self, au_id):
        pass

    def set_proc_id(self, proc_id):
        pass


