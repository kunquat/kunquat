# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


'''
Format:
bind = [bind_entry]
bind_entry = [event_name, constraints, target_events]
'''


class Bindings():

    def __init__(self):
        self._controller = None
        self._session = None
        self._store = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._store = controller.get_store()

    def _get_data(self):
        return self._store.get('p_bind.json', [])

    def _set_data(self, data):
        self._store['p_bind.json'] = data

    def get_count(self):
        data = self._get_data()
        return len(data)

    def _get_binding_data(self, index):
        return self._get_data()[index]

    def _set_binding_data(self, index, binding):
        data = self._get_data()
        data[index] = binding
        self._set_data(data)

    def get_selected_binding_index(self):
        return self._session.get_selected_binding_index()

    def set_selected_binding_index(self, index):
        self._session.set_selected_binding_index(index)

    def get_binding(self, index):
        binding = Binding(self._get_binding_data, self._set_binding_data, index)
        return binding

    def add_binding(self):
        data = self._get_data()
        data.append(['c.ev', [], []])
        self._set_data(data)

    def remove_binding(self, index):
        data = self._get_data()
        del data[index]
        self._set_data(data)


class Binding():

    def __init__(self, get_data, set_data, index):
        self._get_data = get_data
        self._set_data = set_data
        self._index = index

    def get_source_event(self):
        return self._get_data(self._index)[0]

    def set_source_event(self, event_name):
        data = self._get_data(self._index)
        data[0] = event_name
        self._set_data(self._index, data)

    def _get_constraints_data(self):
        return self._get_data(self._index)[1]

    def _set_constraints_data(self, constraints):
        data = self._get_data(self._index)
        data[1] = constraints
        self._set_data(self._index, data)

    def get_constraints(self):
        constraints = Constraints(self._get_constraints_data, self._set_constraints_data)
        return constraints

    def _get_targets_data(self):
        return self._get_data(self._index)[2]

    def _set_targets_data(self, targets):
        data = self._get_data(self._index)
        data[2] = targets
        self._set_data(self._index, data)

    def get_targets(self):
        targets = Targets(self._get_targets_data, self._set_targets_data)
        return targets


'''
constraints = [constraint]
constraint = [event_name, expr]
'''

class Constraints():

    def __init__(self, get_data, set_data):
        self._get_data = get_data
        self._set_data = set_data

    def get_count(self):
        return len(self._get_data())

    def _get_constraint_data(self, index):
        return self._get_data()[index]

    def _set_constraint_data(self, index, constraint):
        data = self._get_data()
        data[index] = constraint
        self._set_data(data)

    def get_constraint(self, index):
        constraint = Constraint(
                self._get_constraint_data, self._set_constraint_data, index)
        return constraint

    def add_constraint(self):
        data = self._get_data()
        data.append(['.a', ''])
        self._set_data(data)

    def remove_constraint(self, index):
        data = self._get_data()
        del data[index]
        self._set_data(data)


class Constraint():

    def __init__(self, get_data, set_data, index):
        self._get_data = get_data
        self._set_data = set_data
        self._index = index

    def get_event_name(self):
        return self._get_data(self._index)[0]

    def set_event_name(self, event_name):
        data = self._get_data(self._index)
        data[0] = event_name
        self._set_data(self._index, data)

    def get_expression(self):
        return self._get_data(self._index)[1]

    def set_expression(self, expression):
        data = self._get_data(self._index)
        data[1] = expression
        self._set_data(self._index, data)


'''
target_events = [target_event]
target_event = [ch_offset, [event_name, maybe_expr]]
'''

class Targets():

    def __init__(self, get_data, set_data):
        self._get_data = get_data
        self._set_data = set_data

    def get_count(self):
        return len(self._get_data())

    def _get_target_data(self, index):
        return self._get_data()[index]

    def _set_target_data(self, index, target):
        data = self._get_data()
        data[index] = target
        self._set_data(data)

    def get_target(self, index):
        target = Target(self._get_target_data, self._set_target_data, index)
        return target

    def add_target(self):
        data = self._get_data()
        data.append([0, ['call', '']])
        self._set_data(data)

    def remove_target(self, index):
        data = self._get_data()
        del data[index]
        self._set_data(data)


class Target():

    def __init__(self, get_data, set_data, index):
        self._get_data = get_data
        self._set_data = set_data
        self._index = index

    def get_channel_offset(self):
        return self._get_data(self._index)[0]

    def set_channel_offset(self, offset):
        data = self._get_data(self._index)
        data[0] = offset
        self._set_data(self._index, data)

    def get_event_name(self):
        return self._get_data(self._index)[1][0]

    def set_event_name(self, event_name):
        data = self._get_data(self._index)
        data[1][0] = event_name
        self._set_data(self._index, data)

    def get_expression(self):
        return self._get_data(self._index)[1][1]

    def set_expression(self, expression):
        data = self._get_data(self._index)
        data[1][1] = expression
        self._set_data(self._index, data)


