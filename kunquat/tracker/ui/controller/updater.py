# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections import OrderedDict


class Updater():

    def __init__(self):
        self._update_signals = ['signal_init']
        self._updaters = set()
        self._actions = OrderedDict()
        self._upcoming_actions = []
        self._removed_actor_ids = []
        self._is_updating = False

    def signal_update(self, *signals):
        assert not self._is_updating

        if not self._update_signals:
            self._update_signals.append('signal_change')

        for s in signals:
            assert type(s) == str
            if s not in self._update_signals:
                self._update_signals.append(s)

    def _update_actions(self):
        assert not self._is_updating

        for signal, action_info in self._upcoming_actions:
            if signal not in self._actions:
                self._actions[signal] = []
            assert action_info not in self._actions[signal]
            self._actions[signal].append(action_info)
        self._upcoming_actions = []

        if self._removed_actor_ids:
            new_actions = OrderedDict()
            for signal, action_list in self._actions.items():
                new_actions[signal] = [elem for elem in action_list
                        if elem[0] not in self._removed_actor_ids]
            self._actions = new_actions
            self._removed_actor_ids = []

    def register_actions(self, actions):
        if not actions:
            return

        self._upcoming_actions.extend(actions)
        if not self._is_updating:
            self._update_actions()

    def unregister_actions(self, actor_id):
        self._removed_actor_ids.append(actor_id)
        if not self._is_updating:
            self._update_actions()

    def register_updater(self, updater):
        self._updaters.add(updater)

    def unregister_updater(self, updater):
        self._updaters.remove(updater)

    def perform_updates(self):
        self._update_actions()

        self._is_updating = True

        try:
            self._perform_updates()
        finally:
            self._is_updating = False

    def _perform_updates(self):
        if not self._update_signals:
            return

        called_action_infos = set()
        for signal in self._update_signals:
            if signal in self._actions:
                for action_info in self._actions[signal]:
                    if action_info not in called_action_infos:
                        called_action_infos.add(action_info)
                        _, action, _ = action_info
                        action()

        iterator = set(self._updaters)
        while len(iterator) > 0:
            updater = iterator.pop()
            updater(set(self._update_signals))
        self._update_signals = []

    def verify_ready_to_exit(self):
        self._update_actions()

        live_actions = []
        last_signal = None
        for signal, actions in self._actions.items():
            for a in actions:
                if signal != last_signal:
                    live_actions.append('{}:'.format(signal))
                    last_signal = signal
                _, action, _ = a
                live_actions.append('    {}'.format(str(action)))
        if live_actions:
            actions_str = '\n'.join(a for a in live_actions)
            raise RuntimeError(
                    'Actions left on exit:\n{}'.format(actions_str))

        if self._updaters:
            updaters_str = '\n'.join(str(u) for u in self._updaters)
            raise RuntimeError(
                    'Updaters left on exit:\n{}'.format(updaters_str))


