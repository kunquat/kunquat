# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from copy import deepcopy

import kunquat.kunquat.events as events


"""
Format:
bind = [bind_entry]
bind_entry = [event_name, constraints, target_events]
"""


class _Node():

    NEW = 'new'
    REACHED = 'reached'
    VISITED = 'visited'

    def __init__(self, name):
        self.name = name
        self.state = self.NEW
        self._connected = set()

    def add_connected(self, node):
        self._connected.add(node)

    def get_connected(self):
        return self._connected


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
        return deepcopy(self._store.get('p_bind.json', []))

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

    def _get_used_events(self, exclude):
        events = set()
        for i in range(self.get_count()):
            binding = self.get_binding(i)
            if i != exclude:
                events.add(binding.get_source_event())
            targets = binding.get_targets()
            for k in range(targets.get_count()):
                if (i, k) == exclude:
                    continue
                target = targets.get_target(k)
                events.add(target.get_event_name())
        return events

    def _test_graph_contains_cycle(self, edges):
        nodes = {}
        for (u, v) in edges:
            if u in nodes:
                src = nodes[u]
            else:
                src = _Node(u)
                nodes[u] = src
            if v in nodes:
                target = nodes[v]
            else:
                target = _Node(v)
                nodes[v] = target
            src.add_connected(target)

        def dfs(node):
            if node.state == node.REACHED:
                return True
            node.state = node.REACHED
            for next_node in node.get_connected():
                if dfs(next_node):
                    return True
            node.state = node.VISITED
            return False

        for node in nodes.values():
            if (node.state == node.NEW) and dfs(node):
                return True

        return False

    def _get_excluded_source_events(self, binding_index):
        used_events = self._get_used_events(exclude=binding_index)

        # TODO: This approach may turn out slow with complex bindings;
        #       test and optimise if needed

        # Get test graph without the binding at binding_index
        base_graph = set()
        for i in range(self.get_count()):
            if i != binding_index:
                binding = self.get_binding(i)
                targets = binding.get_targets()
                for k in range(targets.get_count()):
                    target = targets.get_target(k)
                    u = binding.get_source_event()
                    v = target.get_event_name()
                    base_graph.add((u, v))
        base_graph = frozenset(base_graph)

        excluded = set()

        # See if we get a cycle if we replace the binding source event
        for event in used_events:
            test_graph = set(base_graph)
            targets = self.get_binding(binding_index).get_targets()
            for k in range(targets.get_count()):
                target = targets.get_target(k)
                u = event
                v = target.get_event_name()
                test_graph.add((u, v))
            if self._test_graph_contains_cycle(test_graph):
                excluded.add(event)

        return excluded

    def _get_excluded_target_events(self, binding_index, target_index):
        used_events = self._get_used_events(exclude=(binding_index, target_index))

        # Get test graph without the binding target at target_index
        base_graph = set()
        for i in range(self.get_count()):
            binding = self.get_binding(i)
            targets = binding.get_targets()
            for k in range(targets.get_count()):
                if (i, k) != (binding_index, target_index):
                    target = targets.get_target(k)
                    u = binding.get_source_event()
                    v = target.get_event_name()
                    base_graph.add((u, v))
        base_graph = frozenset(base_graph)

        excluded = set()

        # See if we get a cycle if we replace the target event
        for event in used_events:
            test_graph = set(base_graph)
            binding = self.get_binding(binding_index)
            u = binding.get_source_event()
            v = event
            test_graph.add((u, v))
            if self._test_graph_contains_cycle(test_graph):
                excluded.add(event)

        return excluded

    def get_selected_binding_index(self):
        return self._session.get_selected_binding_index()

    def set_selected_binding_index(self, index):
        self._session.set_selected_binding_index(index)

    def get_binding(self, index):
        binding = Binding(
                self._get_binding_data,
                self._set_binding_data,
                self._get_excluded_source_events,
                self._get_excluded_target_events,
                index)
        return binding

    def has_selected_binding(self):
        return self.get_selected_binding_index() != None

    def get_selected_binding(self):
        return self.get_binding(self.get_selected_binding_index())

    def add_binding(self):
        data = self._get_data()
        data.append(['c.ev', [], []])
        self._set_data(data)

    def remove_binding(self, index):
        data = self._get_data()
        del data[index]
        self._set_data(data)


class Binding():

    def __init__(
            self,
            get_data,
            set_data,
            get_excluded_source_events,
            get_excluded_target_events,
            index):
        self._get_data = get_data
        self._set_data = set_data
        self._get_excluded_source_events = get_excluded_source_events
        self._get_excluded_target_events_at = get_excluded_target_events
        self._index = index

    def get_excluded_source_events(self):
        return self._get_excluded_source_events(self._index)

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

    def _get_excluded_target_events(self, target_index):
        return self._get_excluded_target_events_at(self._index, target_index)

    def _get_targets_data(self):
        return self._get_data(self._index)[2]

    def _set_targets_data(self, targets):
        data = self._get_data(self._index)
        data[2] = targets
        self._set_data(self._index, data)

    def get_targets(self):
        targets = Targets(
                self._get_targets_data,
                self._set_targets_data,
                self._get_excluded_target_events)
        return targets


"""
constraints = [constraint]
constraint = [event_name, expr]
"""

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


"""
target_events = [target_event]
target_event = [ch_offset, [event_name, maybe_expr]]
"""

class Targets():

    def __init__(self, get_data, set_data, get_excluded_target_events):
        self._get_data = get_data
        self._set_data = set_data
        self._get_excluded_target_events = get_excluded_target_events

    def get_count(self):
        return len(self._get_data())

    def _get_target_data(self, index):
        return self._get_data()[index]

    def _set_target_data(self, index, target):
        data = self._get_data()
        data[index] = target
        self._set_data(data)

    def get_target(self, index):
        target = Target(
                self._get_target_data,
                self._set_target_data,
                self._get_excluded_target_events,
                index)
        return target

    def add_target(self):
        data = self._get_data()

        all_events = events.all_events_by_name
        all_event_names = set(event['name'] for event in all_events.values())
        allowed_events = (
                all_event_names - self._get_excluded_target_events(self.get_count()))
        event_name = 'call' if 'call' in allowed_events else allowed_events.pop()
        expression = None if all_events[event_name]['arg_type'] == None else ''
        data.append([0, [event_name, expression]])

        self._set_data(data)

    def remove_target(self, index):
        data = self._get_data()
        del data[index]
        self._set_data(data)


class Target():

    def __init__(self, get_data, set_data, get_excluded_target_events, index):
        self._get_data = get_data
        self._set_data = set_data
        self._get_excluded_target_events = get_excluded_target_events
        self._index = index

    def get_channel_offset(self):
        return self._get_data(self._index)[0]

    def set_channel_offset(self, offset):
        data = self._get_data(self._index)
        data[0] = offset
        self._set_data(self._index, data)

    def get_excluded_events(self):
        return self._get_excluded_target_events(self._index)

    def get_event_name(self):
        return self._get_data(self._index)[1][0]

    def get_expression(self):
        return self._get_data(self._index)[1][1]

    def set_event_info(self, event_name, expression):
        data = self._get_data(self._index)
        data[1][0] = event_name
        data[1][1] = expression
        self._set_data(self._index, data)


