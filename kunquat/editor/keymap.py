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

from __future__ import print_function
from collections import defaultdict

from PyQt4 import QtCore


class KeyMap(QtCore.QObject):

    """A keyboard mapping."""

    def __init__(self, name, keys={}, parent=None):
        """Create a new KeyMap.

        Arguments:
        name -- The name (category) of the KeyMap.
        keys -- Initial mapping.

        """
        QtCore.QObject.__init__(self, parent)
        self._name = name
        self._map = defaultdict(list)
        self._calls = defaultdict(list)
        if keys:
            for key, modifier in keys:
                ambiguous = self.bind(key, modifier, keys[(key, modifier)])
                assert not ambiguous

    def bind(self, key, modifier, call):
        """Bind a key combination to a call.

        Arguments:
        key      -- The key pressed.
        modifier -- The key modifier combination used with key.
        call     -- The method called with the key combination.

        Return value:
        True if and only if the bind resulted in ambiguity for this
        key combination.

        """
        ambiguous = False
        if modifier != None:
            modifier = int(modifier)
        if (key, modifier) in self._map:
            ambiguous = any(kcall != call for kcall
                                          in self._map[(key, modifier)])
        if call in self._calls:
            for kspec in self._calls[call]:
                self._map[kspec].remove(call)
            del self._calls[call]
        self._map[(key, modifier)].append(call)
        self._calls[call].append((key, modifier))
        return ambiguous

    def call(self, ev):
        """Call a bound method based on a QKeyEvent.

        Arguments:
        ev -- The QKeyEvent. This will be set to ignored if the key
              combination is not found in the mapping.

        Exceptions:
        TypeError    -- The key combination is set to a guide.
        RuntimeError -- The key combination is ambiguous (bound to
                        more than one call or guide).

        """
        kspec = ev.key(), int(ev.modifiers())
        altspec = ev.key(), None
        if kspec in self._map:
            pass
        elif altspec in self._map:
            kspec = altspec
        else:
            ev.ignore()
            return
        assert len(self._map[kspec]) > 0
        if len(self._map[kspec]) > 1:
            raise RuntimeError(
                    'Ambiguous key specification: {0:02x}:{0:08x}'.format(
                            ev.key(), ev.modifiers()))
        self._map[kspec][0](ev)

    def set_guide(self, key, modifier, guide):
        """Set a guide for a key combination.

        This method should be used instead of bind if and only if the
        action triggered is not bound to a single object.

        Arguments:
        key      -- The key pressed.
        modifier -- The key modifier combination used with key.
        guide    -- A textual description of the action.

        Return value:
        True if and only if setting the guide resulted in ambiguity
        for this key combination.

        """
        assert type(guide) == str
        return self.bind(key, modifier, guide)

    def get_guide(self, ev):
        """Get a guide based on a QKeyEvent.

        Arguments:
        ev -- The QKeyEvent. This will be set to ignored if the key
              combination is not found in the mapping.

        Exceptions:
        TypeError    -- The key combination is set to a call.
        RuntimeError -- The key combination is ambiguous (bound to
                        more than one call or guide).

        Return value:
        The guide if the key combination was found, otherwise None.

        """
        kspec = ev.key(), int(ev.modifiers())
        if kspec in self._map:
            assert len(self._map[kspec]) > 0
            if len(self._map[kspec]) > 1:
                raise RuntimeError(
                        'Ambiguous key specification: {0:02x}:{0:08x}'.format(
                                ev.key(), ev.modifiers()))
            s = self._map[kspec][0]
            if type(s) != str:
                raise TypeError('Key combination is bound to a callable')
            return s
        else:
            ev.ignore()
        return None

    @property
    def name(self):
        return self._name


