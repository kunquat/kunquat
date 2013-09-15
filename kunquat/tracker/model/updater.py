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
        self._update_signal = False
        self._updaters = set()
        self._children = set()

    def _signal_update(self):
        self._update_signal = True

    def register_updater(self, updater):
        self._updaters.add(updater)

    def unregister_updater(self, updater):
        self._updaters.remove(updater)

    def register_child(self, child):
        self._children.add(child)

    def unregister_child(self, child):
        self._children.remove(child)

    def perform_updates(self):
        for child in self._children:
            child.perform_updates()
        if not self._update_signal:
            return
        for updater in self._updaters:
            updater()
        self._update_signal = False

