# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function
from collections import MutableMapping
from itertools import count

from PyQt4.QtGui import *


class BufferCache(MutableMapping):

    def __init__(self):
        self.flush()
        self._memory_bound = float('inf')

    def flush(self):
        self._items = {}
        self._access_counter = count()
        self._item_mem = None

    def set_memory_bound(self, bound):
        self._memory_bound = bound

    def get_memory_usage(self):
        if not self._item_mem:
            return 0
        return self._item_mem * len(self._items)

    def _limit_memory_usage(self):
        amount_to_free = self.get_memory_usage() - self._memory_bound
        if amount_to_free <= 0:
            return

        items_by_age = sorted(self._items.iteritems(), key=lambda item: item[1][0])
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
        self._items[key] = (self._access_counter.next(), buf)
        return buf

    def __setitem__(self, key, buf):
        if not self._item_mem:
            self._item_mem = self._estimate_buf_size(buf)
        self._items[key] = (self._access_counter.next(), buf)
        self._limit_memory_usage()

    def __delitem__(self, key):
        del self._items[key]

    def __iter__(self):
        for key, item in self._items.iteritems():
            (_, buf) = item
            yield (key, buf)

    def __len__(self):
        return len(self._items)


