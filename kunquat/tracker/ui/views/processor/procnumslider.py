# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from kunquat.tracker.ui.views.audio_unit.aunumslider import AuNumSlider


class ProcNumSlider(AuNumSlider):

    def __init__(self, decimal_count, min_val, max_val, title='', width_txt=''):
        super().__init__(decimal_count, min_val, max_val, title, width_txt)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id


