# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *


class KqtComboBox(QComboBox):

    def __init__(self):
        super().__init__()
        self.setItemDelegate(QStyledItemDelegate())
        self.view().setResizeMode(QListView.Adjust)

    def set_items(self, new_items):
        """Set items of the KqtComboBox, replacing existing ones.

        Arguments:
        new_items -- The sequence of new items. If each item consists of a
                     single value, they are stored as the displayed value.
                     If each item is a 2-tuple, the first is stored as the
                     displayed value and the second as the userdata value.

        """
        old_block = self.blockSignals(True)
        model = QStandardItemModel(0, 1, self)
        for i, entry in enumerate(new_items):
            if isinstance(entry, tuple):
                vis, user = entry
                item = QStandardItem(vis)
                item.setData(user, Qt.UserRole)
            else:
                item = QStandardItem(entry)
            model.appendRow(item)
        self.setModel(model)
        self.blockSignals(old_block)


