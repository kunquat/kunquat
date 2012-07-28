# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2011-2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division, print_function

from PyQt4 import QtCore, QtGui

import kunquat.tracker.kqt_limits as lim
from env_time import EnvTime
from kunquat.tracker.envelope import Envelope


class EnvForceRel(EnvTime):

    def __init__(self, project, parent=None):
        key_base = 'ins_{0:02x}/p_envelope_force_release.json'
        EnvTime.__init__(self,
                         project,
                         key_base,
                         'Force release envelope',
                         Envelope(project,
                                  (0, float('inf')),
                                  (0, 1),
                                  (True, False),
                                  (False, True),
                                  [(0, 1), (1, 0)],
                                  32,
                                  key_base.format(0),
                                  'envelope'),
                         parent=parent)


