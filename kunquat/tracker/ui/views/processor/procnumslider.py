# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.tracker.ui.views.audiounit.aunumslider import AuNumSlider
from .updatingprocview import UpdatingProcView


class ProcNumSlider(AuNumSlider, UpdatingProcView):

    def __init__(self, decimal_count, min_val, max_val, title='', width_txt=''):
        super().__init__(decimal_count, min_val, max_val, title, width_txt)


