# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2010-2012
#          Toni Ruottu,       Finland 2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

class History(object):

    def __init__(self, project):
        """Create a new History.

        Arguments:
        project -- The Kunquat Project associated with this History.

        """
        self._project = project
        self._root = Step()
        self._current = self._root
        self._commit = self._root
        self._group = 0
        self._pending = {}

    def at_commit(self):
        assert not self._group
        return self._commit == self._current

    def set_commit(self):
        assert not self._group
        self._commit = self._current

    def show_latest_branch(self):
        cur = self._root
        while cur:
            print(cur.name, '<-' if cur == self._current else '')
            cur = cur.child()

    #'Import composition {0}'.format(src)
    #'Load {0} into instrument {1:d}'.format(src, index)
    #'Load {0} into effect {1:d} of instrument {2:d}'.format(src, index, ins_num)
    #'Load {0} into effect {1:d}'.format(src, index)

    def start_group(self, name=''):
        """Start a group of changes.

        Only the methods step() and end_group() may be called when a
        group of changes is being made.

        """
        self._group += 1
        if self._group > 1:
            return
        self._current = Step(self._current, name)

    def end_group(self):
        """End a group of changes."""
        assert self._group > 0
        self._group -= 1

    def rollback(self):
        assert self._group
        # TODO: rollback
        self._group = 0

    def parents(self):
        """Generate a sequence containing all ancestors of
        the current state.

        """
        #assert not self._group
        node = self._current.parent
        while node:
            yield node
            node = node.parent

    def step(self, key, new_data, name='', immediate=True):
        if not immediate:
            if key in self._pending:
                old_value = self._pending[key][1]
            else:
                old_value = self._project[key]
                if old_value == None:
                    old_value = ''
                elif key[key.index('.'):].startswith('.json'):
                    old_value = json.dumps(old_value)
            self._pending[key] = new_data, old_value, name
            #print('added pending change', key, '-',
            #      new_data if len(new_data) < 100 else new_data[:97] + '...')
            return

        old_value = self._project[key]
        if old_value == None:
            old_value = ''
        elif key[key.index('.'):].startswith('.json'):
            old_value = json.dumps(old_value)

        if key in self._pending:
            old_value = self._pending[key][1]
            del self._pending[key]
        """
        if self._pending:
            for k, value in self._pending.iteritems():
                if k == key:
                    continue
                pdata, pold, pname = value
                self._step(k, pdata, pold, pname)
                #print('changed pending', k, '-',
                #      pdata if len(pdata) < 100 else pdata[:97] + '...')
            if key in self._pending:
                old_value = self._pending[key][1]
            self._pending = {}
        """
        self._step(key, new_data, old_value, name)
        #print('added immediate change', key, '-',
        #      new_data if len(new_data) < 100 else new_data[:97] + '...')
        #for p in self.parents():
        #    print(p.name)

    def _step(self, key, new_data, old_data, name=''):
        if not self._group:
            self._current = Step(self._current, name)
        self._current.add_change(Change(key, old_data, new_data))

    def flush(self, key):
        if key not in self._pending:
            return
        new_data, old_data, name = self._pending.pop(key)
        self._step(key, new_data, old_data, name)
        for k in [c for c in self._pending.iterkeys()
                          if c.endswith('p_connections.json')]:
            new_data, old_data, name = self._pending.pop(k)
            self._step(k, new_data, old_data, name)

    def cancel(self, key):
        old_data = ''
        if key in self._pending:
            new_data, old_data, name = self._pending.pop(key)
        for k in [c for c in self._pending.iterkeys()
                          if c.endswith('p_connections.json')]:
            del self._pending[k]
        return old_data

    def undo(self, step_signaller=None):
        """Undoes a step."""
        assert not self._group
        if self._pending:
            if len(self._pending) > 1:
                self.start_group()
            while self._pending:
                k, value = self._pending.popitem()
                pdata, pold, pname = value
                self._step(k, pdata, pold, pname)
            if self._group:
                self.end_group()
            self.undo(step_signaller)
            return
        if not self._current.parent:
            return
            #raise RuntimeError('Nothing to undo')
        #print('undoing', self._current.name)
        if step_signaller:
            QtCore.QObject.emit(step_signaller,
                                QtCore.SIGNAL('startTask(int)'),
                                len(self._current.changes))
        try:
            for change in self._current.changes:
                #print('undoing', change.key)
                if step_signaller:
                    QtCore.QObject.emit(step_signaller,
                                        QtCore.SIGNAL('step(QString)'),
                                        'Undoing {0} ({1}) ...'.format(
                                            self._current.name, change.key))
                old_data = None
                if change.old_data:
                    if change.key[change.key.index('.'):].startswith('.json'):
                        old_data = json.loads(change.old_data)
                    else:
                        old_data = change.old_data
                self._project.set_raw(change.key, old_data)
                #self._project.handle[change.key] = change.old_data
        finally:
            if step_signaller:
                QtCore.QObject.emit(step_signaller,
                                    QtCore.SIGNAL('endTask()'))
        self._current = self._current.parent

    def redo(self, branch=None, step_signaller=None):
        """Redoes a step.

        Arguments:
        branch -- The index of the branch to be used, or None for
                  the last used branch in the current node.

        """
        assert not self._group
        if self._pending:
            # XXX: should we step() here?
            return
        child = self._current.child(branch)
        if not child:
            return
            #raise RuntimeError('Nothing to redo')
        #print('redoing', child.name)
        if step_signaller:
            QtCore.QObject.emit(step_signaller,
                                QtCore.SIGNAL('startTask(int)'),
                                len(child.changes))
        try:
            for change in child.changes:
                if step_signaller:
                    QtCore.QObject.emit(step_signaller,
                                        QtCore.SIGNAL('step(QString)'),
                                        'Redoing {0} ({1}) ...'.format(
                                            child.name, change.key))
                self._project.set_raw(change.key, change.new_data)
                #self._project.handle[change.key] = change.new_data
        finally:
            if step_signaller:
                QtCore.QObject.emit(step_signaller,
                                    QtCore.SIGNAL('endTask()'))
        self._current = child


class Step(object):

    """An editing step in a Kunquat Project."""

    def __init__(self, parent=None, name=''):
        """Create a new step.

        Optional arguments:
        parent  -- The step that precedes this step.
        name    -- The name of this step.

        """
        self._changes = []
        self._children = []
        self._last_child = None
        self._parent = parent
        if parent:
            parent._add_step(self)
        self._name = name

    def add_change(self, change):
        """Adds a change into the step.

        Arguments:
        change -- The change.

        """
        for i, ch in enumerate(self._changes):
            if ch.key == change.key:
                self._changes[i] = Change(ch.key, ch.old_data,
                                          change.new_data)
                break
        else:
            self._changes.append(change)

    @property
    def changes(self):
        return self._changes

    def child(self, index=None):
        if index == None:
            return self._last_child
        return self._children[index]

    @property
    def name(self):
        """Return the name of the step.

        Return value:
        The name of the step.  The default name is generated based on
        the modified keys.

        """
        if self._name:
            return self._name
        if not self._changes:
            return 'No change'
        prefix = self._changes[0].key
        for change in self._changes[1:]:
            common_len = sum(1 for _ in takewhile(lambda x: x[0] == x[1],
                                                  izip(prefix, change.key)))
            prefix = prefix[:common_len]
            """
            for i, ch in enumerate(prefix):
                if ch != key[i]:
                    prefix = prefix[:i]
                    break
            """
        if len(prefix) < len(self._changes[0].key):
            prefix = prefix + '*'
        return prefix

    @name.setter
    def name(self, value):
        """Set a name for the step."""
        self._name = value

    @property
    def parent(self):
        return self._parent

    def _add_step(self, step):
        self._children.append(step)
        self._last_child = step


class Change(object):

    """A value change of a key inside a Kunquat Project."""

    def __init__(self, key, old_data, new_data):
        """Create a new Change.

        Arguments:
        key      -- The key modified.
        old_data -- The value of the key before modification.
        new_data -- The value of the key after modification.

        """
        self._key = key
        self._old_data = old_data if old_data != None else ''
        self._new_data = new_data if new_data != None else ''

    @property
    def key(self):
        """Return the key of the change."""
        return self._key

    @property
    def old_data(self):
        """Return the value of the key before this change."""
        return self._old_data

    @property
    def new_data(self):
        """Return the value of the key after this change."""
        return self._new_data



