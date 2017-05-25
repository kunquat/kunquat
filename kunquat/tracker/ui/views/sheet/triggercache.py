# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class TriggerCache():

    def __init__(self):
        self._images = {}
        self._mem_usage = 0

    def set_trigger_image(self, key, image):
        self._images[key] = image
        self._mem_usage += (image.depth() // 8) * image.width() * image.height()

    def get_trigger_image(self, key):
        return self._images.get(key)

    def flush(self):
        self._images = {}
        self._mem_usage = 0

    def trigger_count(self):
        return len(self._images)

    def get_memory_usage(self):
        return self._mem_usage


