# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.views.helpview import HelpView


class SheetHelp(HelpView):

    def __init__(self):
        super().__init__()

    def _get_help_data(self):
        sheet_mgr = self._ui_model.get_sheet_manager()
        return sheet_mgr.get_help()


