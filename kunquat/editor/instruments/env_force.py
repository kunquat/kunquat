# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2011
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

import kunquat.editor.kqt_limits as lim
from env_time import EnvTime
from kunquat.editor.envelope import Envelope


class EnvForce(EnvTime):

    def __init__(self, project, parent=None):
        key_base = 'ins_{{0:02x}}/kqti{0}/p_envelope_force.json'.format(
                           lim.FORMAT_VERSION)
        EnvTime.__init__(self,
                         project,
                         key_base,
                         'Force envelope',
                         Envelope(project,
                                  (0, float('inf')),
                                  (0, 1),
                                  (True, False),
                                  (False, False),
                                  [(0, 1), (1, 1)],
                                  32,
                                  key_base.format(0),
                                  'envelope'),
                         parent)


