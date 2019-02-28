# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import kunquat.kunquat.events as events
import kunquat.tracker.config as config
from .trigger import Trigger
from .triggerposition import TriggerPosition


class TypewriterButtonModel():

    def __init__(self, row, index):
        self._controller = None
        self._session = None
        self._ui_model = None
        self._control_mgr = None
        self._typewriter_mgr = None
        self._notation_mgr = None
        self._sheet_mgr = None

        self._row = row
        self._index = index

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._control_mgr = ui_model.get_control_manager()
        self._typewriter_mgr = ui_model.get_typewriter_manager()
        self._notation_mgr = ui_model.get_notation_manager()
        self._sheet_mgr = ui_model.get_sheet_manager()

    def get_name(self):
        pitch = self._get_pitch()
        if pitch != None:
            notation = self._notation_mgr.get_selected_notation()
            name = notation.get_full_name(pitch)
            return name

        hit_index = self._get_hit()
        if hit_index != None:
            control = self._control_mgr.get_selected_control()
            au = control.get_audio_unit()
            if au.get_existence():
                hit = au.get_hit(hit_index)
                if hit.get_existence():
                    return hit.get_name()

        return None

    def get_background_image(self, size, enabled, pressed):
        images = self._session.typewriter_button_images[size]
        return images.backgrounds.get((enabled, pressed), None)

    def set_background_image(self, size, enabled, pressed, image):
        images = self._session.typewriter_button_images[size]
        images.backgrounds[(enabled, pressed)] = image

    def get_led_image(self, size, led_state):
        images = self._session.typewriter_button_images[size]
        return images.leds.get(led_state, None)

    def set_led_image(self, size, led_state, image):
        images = self._session.typewriter_button_images[size]
        images.leds[led_state] = image

    def flush_images(self, size):
        images = self._session.typewriter_button_images[size]
        images.backgrounds = {}
        images.leds = {}

    def _get_pitch(self):
        return self._typewriter_mgr.get_button_pitch((self._row, self._index))

    def _get_hit(self):
        return self._typewriter_mgr.get_button_hit((self._row, self._index))

    def _get_event_type_and_param(self):
        pitch = self._get_pitch()
        if pitch != None:
            return ('n+', pitch)

        hit = self._get_hit()
        if hit != None:
            return ('h', hit)

        return None, None

    def _get_key_id(self):
        return self._typewriter_mgr.get_key_id((self._row, self._index))

    def start_tracked_note(self):
        event_type, param = self._get_event_type_and_param()
        if param == None:
            return

        if self._session.is_key_active(self._row, self._index):
            return

        selected_control = self._control_mgr.get_selected_control()
        if selected_control:
            selection = self._ui_model.get_selection()
            location = selection.get_location()
            ch_num = location.get_col_num()
            note = selected_control.start_tracked_note(ch_num, event_type, param)
            self._session.activate_key_with_note(self._row, self._index, note)

        if self._sheet_mgr.is_editing_enabled():
            allow_chord_mode = config.get_config().get_value('chord_mode')
            if allow_chord_mode:
                self._sheet_mgr.set_chord_mode(True)
                self._session.set_chord_note(event_type, param, True)

            if (self._sheet_mgr.get_replace_mode() and self._sheet_mgr.is_at_trigger()):
                trigger = self._sheet_mgr.get_selected_trigger()
                if ((event_type == 'n+' and
                        trigger.get_argument_type() == events.EVENT_ARG_PITCH) or
                        (event_type == 'h' and trigger.get_type() == 'h')):
                    new_trigger = Trigger(trigger.get_type(), str(param))
                    self._sheet_mgr.add_trigger(new_trigger)
                elif ((trigger.get_type() == 'n-') or
                        all(t in ('n+', 'h') for t in (event_type, trigger.get_type()))):
                    new_trigger = Trigger(event_type, str(param))
                    self._sheet_mgr.add_trigger(new_trigger)
            else:
                trigger = Trigger(event_type, str(param))
                self._sheet_mgr.add_trigger(trigger)

    def stop_tracked_note(self):
        if not self._session.is_key_active(self._row, self._index):
            return

        note = self._session.get_active_note(self._row, self._index)
        note.set_rest()
        self._session.deactivate_key(self._row, self._index)

        event_type, param = self._get_event_type_and_param()
        self._session.set_chord_note(event_type, param, False)
        if not self._session.are_chord_notes_down():
            self._sheet_mgr.set_chord_mode(False)


