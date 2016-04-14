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

import sys
import threading


class MonitoringThread(threading.Thread):

    def __init__(self, group=None, target=None, name=None, args=(), kwargs={}):
        super().__init__(group, target, name, args, kwargs)

    def run(self):
        try:
            self.run_monitored()
        except Exception:
            #print('exception caught by MonitoringThread {}:'.format(self))
            sys.excepthook(*sys.exc_info())

    def run_monitored(self):
        raise NotImplementedError(
                'MonitoringThread {} does not implement run_monitored'.format(self.name))


