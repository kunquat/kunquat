# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
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

    def get_channel(self, channel_number):
        if not channel_number in self._channels:
            self._channels[channel_number] = Channel()
        return self._channels[channel_number]

