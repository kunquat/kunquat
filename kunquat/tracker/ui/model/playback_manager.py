# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from channel import Channel

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

    def start_recording(self):
        self._session.set_record_mode(True)
        self._updater.signal_update(set(['signal_record_mode']))

    def stop_recording(self):
        self._session.set_record_mode(False)
        self._updater.signal_update(set(['signal_record_mode']))

    def is_recording(self):
        return self._session.get_record_mode()
