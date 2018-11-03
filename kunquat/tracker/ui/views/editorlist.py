# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *


class EditorList(QWidget):

    def __init__(self):
        super().__init__()
        self._area = EditorListArea()
        self._has_adder = False

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._area)
        self.setLayout(v)

    def update_list(self, force_rebuild=False):
        if not self._area.widget():
            self._init_container()

        layout = self._area.widget().layout()
        cur_editor_count = layout.count()

        new_editor_count = self._get_updated_editor_count()
        new_widget_count = new_editor_count

        if self._has_adder:
            cur_editor_count -= 1
            assert cur_editor_count >= 0
            new_widget_count += 1

        if (new_widget_count < layout.count()) or force_rebuild:
            # Create contents from scratch because
            # Qt doesn't update visuals properly on single item removal
            self.disconnect_widgets()
            self._init_container()
            layout = self._area.widget().layout()

            cur_editor_count = layout.count()
            if self._has_adder:
                cur_editor_count -= 1
                assert cur_editor_count >= 0

        # Create new editors (if needed)
        for i in range(cur_editor_count, new_editor_count):
            editor = self._make_editor_widget(i)
            layout.insertWidget(i, editor)

        # Update editor contents
        for i in range(new_editor_count):
            editor = layout.itemAt(i).widget()
            self._update_editor(i, editor)

        self._area.do_size_hack()

    def disconnect_widgets(self):
        layout = self._area.widget().layout()
        for i in range(layout.count()):
            widget = layout.itemAt(i).widget()
            self._disconnect_widget(widget)

    def _init_container(self):
        self._area.setWidget(EditorListContainer())
        adder = self._make_adder_widget()
        if adder:
            self._has_adder = True
            self._area.widget().layout().addWidget(adder)

    def resizeEvent(self, event):
        self._area.do_size_hack()

    # Callbacks

    def _make_adder_widget(self):
        return None

    def _get_updated_editor_count(self):
        raise NotImplementedError

    def _make_editor_widget(self, index):
        raise NotImplementedError

    def _update_editor(self, index, editor):
        raise NotImplementedError

    def _disconnect_widget(self, widget):
        raise NotImplementedError


class EditorListContainer(QWidget):

    def __init__(self):
        super().__init__()
        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.setSizeConstraint(QLayout.SetMinimumSize)
        self.setLayout(v)


class EditorListArea(QScrollArea):

    def __init__(self):
        super().__init__()

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)

    def do_size_hack(self):
        widget = self.widget()
        if widget:
            widget.setFixedWidth(self.width() - self.verticalScrollBar().width() - 5)
            widget.setMaximumHeight(2)

    def resizeEvent(self, event):
        self.do_size_hack()
        self.widget().updateGeometry()


