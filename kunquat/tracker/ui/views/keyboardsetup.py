# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.model.keymapmanager import KeyboardAction, KeyboardNoteAction

_has_xlib = False
try:
    import Xlib.display
    import Xlib.X
    _has_xlib = True
except ImportError:
    pass


def _get_scancode_rows():
    scancode_rows = [
        [10 + x for x in range(11)],
        [24 + x for x in range(11)],
        [38 + x for x in range(11)],
        [52 + x for x in range(10)],
    ]
    return scancode_rows


def setup(ui_model):
    keymap_mgr = ui_model.get_keymap_manager()

    scancode_rows = _get_scancode_rows()

    # Scancodes
    scancode_locs = {}
    for row, codes in enumerate(scancode_rows):
        for index, code in enumerate(codes):
            scancode_locs[code] = (row, index)
    keymap_mgr.set_scancode_locations(scancode_locs)

    if _has_xlib:
        display = Xlib.display.Display()

        key_names = {}
        for row, codes in enumerate(scancode_rows):
            for index, code in enumerate(codes):
                keysym = display.keycode_to_keysym(code, 0)
                if keysym != Xlib.X.NoSymbol:
                    name = display.lookup_string(keysym)
                    if name:
                        key_names[(row, index)] = name.upper()

        display.close()

        keymap_mgr.set_key_names(key_names)

    # TODO: set key ID locations for systems that need them

    # Actions
    key_actions = {
        (0, 0):     KeyboardAction(KeyboardAction.REST),
        (2, 9):     KeyboardAction(KeyboardAction.OCTAVE_DOWN),
        (2, 10):    KeyboardAction(KeyboardAction.OCTAVE_UP),
        (3, 7):     KeyboardAction(KeyboardAction.PLAY),
        (3, 8):     KeyboardAction(KeyboardAction.SILENCE),
    }
    tw_sizes = keymap_mgr.get_typewriter_row_sizes()
    tw_offsets = keymap_mgr.get_typewriter_row_offsets()
    for row, (size, offset) in enumerate(zip(tw_sizes, tw_offsets)):
        for index in range(size):
            loc = (row, index + offset)
            assert loc not in key_actions
            key_actions[loc] = KeyboardNoteAction(row, index)
    keymap_mgr.set_key_actions(key_actions)


