# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class Updater(object):

    def __init__(self):
        self._update_signals = set()
        self._updaters = set()

    def signal_update(self, signals = set()):
        self._update_signals.add('signal_change')
        self._update_signals |= signals

    def register_updater(self, updater):
        self._updaters.add(updater)

    def unregister_updater(self, updater):
        self._updaters.remove(updater)

    def perform_updates(self):
        if not self._update_signals:
            return
        for updater in self._updaters:
            updater(self._update_signals)
        self._update_signals = set()

