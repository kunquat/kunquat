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

from PyQt4 import QtGui, QtCore

from sample_map import SampleMap


class PitchMap(SampleMap):

    def __init__(self, project, key, parent=None):
        SampleMap.__init__(self, project, key, 'pitch', parent)


