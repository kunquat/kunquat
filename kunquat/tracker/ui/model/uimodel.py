# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2014
#          Toni Ruottu, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from stat_manager import StatManager
from controlmanager import ControlManager
from playback_manager import PlaybackManager
from typewritermanager import TypewriterManager
from eventhistory import EventHistory
from module import Module
from visibilitymanager import VisibilityManager
from selection import Selection
from sheetmanager import SheetManager
from notationmanager import NotationManager
from processmanager import ProcessManager
from iconbank import IconBank

class UiModel():
    """
    >>> ui_model = UiModel()
    >>> project = ui_model.get_project()
    >>> module = project.get_module()
    >>> module.instruments()
    []
    >>> module.create_instrument()
    >>> module.instruments()
    [0]
    >>> instrument = module.get_instrument(0)
    >>> instrument.generators()
    []
    >>> instrument.create_generator()
    >>> instrument.generators()
    [0]
    >>> generator = instrument.get_generator(0)
    >>> generator.set_type('debug')
    >>> instrument_output = instrument.get_output(0)
    >>> genrator_output = generator.get_output(0)
    >>> instrument.list_connections()
    []
    >>> instrument.connect(generator_output, instrument_master)
    >>> instrument.list_connections()
    [(<generator_output>, <instrument_output>)]
    >>> master_output = module.get_output()
    >>> module.connect(instrument_output, master_output)
    >>> module.track_count()
    0
    >>> module.create_track()
    >>> module.track_count()
    1
    >>> song = module.get_song_by_track(0)
    >>> song.system_count()
    1
    >>> pattern_instance = song.get_pattern_instance_by_system(0)
    >>> pattern = pattern_instance.get_pattern()

    Insert Notes
    >>> pattern.

    
    >>> song.insert_new_instance(pattern)
    >>> system_count = song.system_count()
    >>> new_instance_number = system_count - 1
    >>> new_instance = song.get_pattern_instance_by_system(new_instance_number)
    >>> target_system_number = 0
    >>> song.move_here(target_system_number, new_instance)


    >>> song.pattern_instance_count()
    0
    >>> song.create_pattern_instance()
    >>> song.pattern_instance_count()
    1
   

    """

    def __init__(self):
        self._stat_manager = None
        self._control_manager = None
        self._playback_manager = None
        self._typewriter_manager = None
        self._event_history = None
        self._module = None
        self._visibility_manager = None
        self._selection = None
        self._sheet_manager = None
        self._notation_manager = None
        self._process_manager = None
        self._icon_bank = None

    def set_ui(self, ui):
        self._ui = ui

    def set_controller(self, controller):
        self._controller = controller
        self._module.set_controller(controller)
        self._stat_manager.set_controller(self._controller)
        self._control_manager.set_controller(self._controller)
        self._playback_manager.set_controller(self._controller)
        self._typewriter_manager.set_controller(self._controller)
        self._visibility_manager.set_controller(self._controller)
        self._event_history.set_controller(self._controller)
        self._selection.set_controller(self._controller)
        self._sheet_manager.set_controller(self._controller)
        self._notation_manager.set_controller(self._controller)
        self._process_manager.set_controller(self._controller)
        self._icon_bank.set_controller(self._controller)

    def get_updater(self):
        updater = self._controller.get_updater()
        return updater

    def set_stat_manager(self, stat_manager):
        self._stat_manager = stat_manager

    def get_stat_manager(self):
        return self._stat_manager

    def set_control_manager(self, control_manager):
        self._control_manager = control_manager
        self._control_manager.set_ui_model(self)

    def get_control_manager(self):
        return self._control_manager

    def set_playback_manager(self, playback_manager):
        self._playback_manager = playback_manager

    def get_playback_manager(self):
        return self._playback_manager

    def set_typewriter_manager(self, typewriter_manager):
        self._typewriter_manager = typewriter_manager
        self._typewriter_manager.set_ui_model(self)

    def get_typewriter_manager(self):
        return self._typewriter_manager

    def set_event_history(self, event_history):
        self._event_history = event_history

    def get_event_history(self):
        return self._event_history

    def set_module(self, module):
        self._module = module
        self._module.set_ui_model(self)

    def get_module(self):
        return self._module

    def set_visibility_manager(self, visibility_manager):
        self._visibility_manager = visibility_manager

    def get_visibility_manager(self):
        return self._visibility_manager

    def set_selection(self, selection):
        self._selection = selection
        self._selection.set_ui_model(self)

    def get_selection(self):
        return self._selection

    def set_sheet_manager(self, sheet_manager):
        self._sheet_manager = sheet_manager
        self._sheet_manager.set_ui_model(self)

    def get_sheet_manager(self):
        return self._sheet_manager

    def set_notation_manager(self, notation_manager):
        self._notation_manager = notation_manager

    def get_notation_manager(self):
        return self._notation_manager

    def set_process_manager(self, process_manager):
        self._process_manager = process_manager

    def get_process_manager(self):
        return self._process_manager

    def set_icon_bank(self, icon_bank):
        self._icon_bank = icon_bank

    def get_icon_bank(self):
        return self._icon_bank

    def play(self):
        self._controller.play()

    def play_pattern(self):
        module = self.get_module()
        album = module.get_album()
        if not album:
            return

        selection = self.get_selection()
        location = selection.get_location()
        if not location:
            return

        track = location.get_track()
        if track >= album.get_track_count():
            return

        song = album.get_song_by_track(track)

        system = location.get_system()
        if system >= song.get_system_count():
            return

        pi = song.get_pattern_instance(system)
        self._controller.play_pattern((pi.get_pattern_num(), pi.get_instance_num()))

    def silence(self):
        self._controller.silence()

    def clock(self):
        self._controller.send_queries()

def create_ui_model():
    stat_manager = StatManager()
    control_manager = ControlManager()
    playback_manager = PlaybackManager()
    typewriter_manager = TypewriterManager()
    event_history = EventHistory()
    module = Module()
    visibility_manager = VisibilityManager()
    selection = Selection()
    sheet_manager = SheetManager()
    notation_manager = NotationManager()
    process_manager = ProcessManager()
    icon_bank = IconBank()
    ui_model = UiModel()
    ui_model.set_stat_manager(stat_manager)
    ui_model.set_control_manager(control_manager)
    ui_model.set_playback_manager(playback_manager)
    ui_model.set_typewriter_manager(typewriter_manager)
    ui_model.set_event_history(event_history)
    ui_model.set_module(module)
    ui_model.set_visibility_manager(visibility_manager)
    ui_model.set_selection(selection)
    ui_model.set_sheet_manager(sheet_manager)
    ui_model.set_notation_manager(notation_manager)
    ui_model.set_process_manager(process_manager)
    ui_model.set_icon_bank(icon_bank)
    return ui_model

