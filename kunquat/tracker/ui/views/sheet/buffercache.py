# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections import MutableMapping
from itertools import count

from PySide.QtGui import *


class BufferCache(MutableMapping):

    def __init__(self):
        self.flush()
        self._memory_limit = float('inf')
        self._reduce_ahead = 0

    def flush(self):
        self._items = {}
        self._access_counter = count()
        self._item_mem = None

    def set_memory_limit(self, limit, reduce_ahead=0.5):
        self._memory_limit = limit
        self._reduce_ahead = min(max(0, reduce_ahead), 1)

    def get_memory_usage(self):
        if not self._item_mem:
            return 0
        return self._item_mem * len(self._items)

    def _limit_memory_usage(self):
        amount_to_free = self.get_memory_usage() - self._memory_limit
        if amount_to_free <= 0:
            return

        amount_to_free += self._memory_limit * self._reduce_ahead

        items_by_age = sorted(self._items.items(), key=lambda item: item[1][0])
        for key, _ in items_by_age:
            del self._items[key]
            amount_to_free -= self._item_mem
            if amount_to_free <= 0:
                break

    def _estimate_buf_size(self, buf):
        return (buf.depth() // 8) * buf.width() * buf.height()

    def __contains__(self, key):
        return key in self._items

    def __getitem__(self, key):
        (_, buf) = self._items[key]
        self._items[key] = (next(self._access_counter), buf)
        return buf

    def __setitem__(self, key, buf):
        if not self._item_mem:
            self._item_mem = self._estimate_buf_size(buf)
        self._items[key] = (next(self._access_counter), buf)
        self._limit_memory_usage()

    def __delitem__(self, key):
        del self._items[key]

    def __iter__(self):
        for key, item in self._items.items():
            (_, buf) = item
            yield (key, buf)

    def __len__(self):
        return len(self._items)


