# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2015-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.kunquat.limits import *
from kunquat.tracker.config import get_config
from .channel import Channel
from . import tstamp


class PlaybackManager():

    def __init__(self):
        self._channels = {}

    def set_controller(self, controller):
        self._controller = controller
        self._updater = controller.get_updater()
        self._session = controller.get_session()

    def get_channel(self, channel_number):
        if not channel_number in self._channels:
            self._channels[channel_number] = Channel()
        return self._channels[channel_number]

    def is_playback_active(self):
        return self._session.is_playback_active()

    def get_playback_pattern(self):
        return self._session.get_playback_pattern()

    def get_playback_position(self):
        track_num, system_num, row_ts = self._session.get_playback_cursor_position()
        return track_num, system_num, tstamp.Tstamp(row_ts)

    def set_infinite_mode(self, enabled):
        self._controller.set_infinite_mode(enabled)

    def get_infinite_mode(self):
        return self._controller.get_infinite_mode()

    def _update_enabled_channels(self):
        if self._session.any_channel_solo():
            mute_channel = lambda ch: self._session.get_channel_state(ch) != 'solo'
        else:
            mute_channel = lambda ch: self._session.get_channel_state(ch) == 'mute'

        for ch in range(CHANNELS_MAX):
            self._controller.set_channel_mute(ch, mute_channel(ch))

    def set_channel_mute(self, channel, mute):
        self._session.set_channel_state(channel, 'mute' if mute else None)
        self._update_enabled_channels()

    def set_channel_solo(self, channel, solo):
        self._session.set_channel_state(channel, 'solo' if solo else None)
        self._update_enabled_channels()

    def get_channel_mute(self, channel):
        return self._session.get_channel_state(channel) == 'mute'

    def get_channel_solo(self, channel):
        return self._session.get_channel_state(channel) == 'solo'

    def is_channel_active(self, channel):
        if self._session.any_channel_solo():
            return self._session.get_channel_state(channel) == 'solo'
        return self._session.get_channel_state(channel) != 'mute'

    def set_playback_cursor_following(self, enabled):
        get_config().set_value('follow_playback_cursor', enabled)

    def get_playback_cursor_following(self):
        return get_config().get_value('follow_playback_cursor')

    def follow_playback_cursor(self):
        return self.is_playback_active() and self.get_playback_cursor_following()

    def start_recording(self):
        self._session.set_record_mode(True)
        self._updater.signal_update('signal_record_mode')

    def stop_recording(self):
        self._session.set_record_mode(False)
        self._updater.signal_update('signal_record_mode')

    def is_recording(self):
        return self._session.get_record_mode()

    def get_runtime_var_value(self, var_name):
        return self._session.get_runtime_var_value(var_name)

    def set_runtime_var_value(self, var_name, var_value):
        self._controller.set_runtime_var_value(var_name, var_value)


