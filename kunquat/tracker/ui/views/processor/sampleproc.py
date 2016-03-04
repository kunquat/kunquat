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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import utils


class SampleProc(QTabWidget):

    @staticmethod
    def get_name():
        return u'Sample synthesis'

    def __init__(self):
        QTabWidget.__init__(self)

        self._samples = Samples()

        self.addTab(self._samples, 'Samples')

    def set_au_id(self, au_id):
        self._samples.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._samples.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._samples.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._samples.unregister_updaters()


class Samples(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._sample_list = SampleList()
        self._sample_editor = SampleEditor()

        h = QHBoxLayout()
        h.setMargin(2)
        h.setSpacing(4)
        h.addWidget(self._sample_list)
        h.addWidget(self._sample_editor)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._sample_list.set_au_id(au_id)
        self._sample_editor.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._sample_list.set_proc_id(proc_id)
        self._sample_editor.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._sample_list.set_ui_model(ui_model)
        self._sample_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._sample_editor.unregister_updaters()
        self._sample_list.unregister_updaters()


class SampleListToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._load_button = QToolButton()
        self._load_button.setText('Load sample')
        self._load_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove sample')
        self._remove_button.setEnabled(False)

        self.addWidget(self._load_button)
        self.addWidget(self._remove_button)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return 'signal_proc_sample_list_{}'.format(self._proc_id)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_enabled()

    def _update_enabled(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        id_count = len(sample_params.get_sample_ids())
        self._load_button.setEnabled(id_count < sample_params.get_max_sample_count())


class SampleListModel(QAbstractListModel):

    def __init__(self):
        QAbstractListModel.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._items = []

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._make_items()

    def unregister_updaters(self):
        pass

    def get_item(self, index):
        row = index.row()
        if 0 <= row < len(self._items):
            item = self._items[row]
            return item
        return None

    def _make_items(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        for sample_id in sample_params.get_sample_ids():
            name = sample_params.get_sample_name(sample_id)
            self._items.append((sample_id, name))

    # Qt interface

    def rowCount(self, parent):
        return len(self._items)

    def data(self, index, role):
        if role == Qt.DisplayRole:
            row = index.row()
            if 0 <= row < len(self._items):
                _, vis_name = self._items[row]
                if vis_name == None:
                    vis_name = '-'
                return QVariant(vis_name)

        return QVariant()

    def headerData(self, section, orientation, role):
        return QVariant()


class SampleListView(QListView):

    def __init__(self):
        QListView.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self.setSelectionMode(QAbstractItemView.SingleSelection)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        for signal_type in ('clicked', 'activated'):
            signal = '{}(const QModelIndex&)'.format(signal_type)
            QObject.connect(self, SIGNAL(signal), self._select_sample)

    def unregister_updaters(self):
        pass

    def _get_update_signal_type(self):
        return 'signal_proc_select_sample_{}'.format(self._proc_id)

    def _select_sample(self, index):
        item = self.model().get_item(index)
        if item:
            sample_id, _ = item
            sample_params = utils.get_proc_params(
                    self._ui_model, self._au_id, self._proc_id)
            sample_params.set_selected_sample_id(sample_id)
            self._updater.signal_update(set([self._get_update_signal_type()]))


class SampleList(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._toolbar = SampleListToolBar()

        self._list_model = None
        self._list_view = SampleListView()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._list_view)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._toolbar.set_au_id(au_id)
        self._list_view.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._toolbar.set_proc_id(proc_id)
        self._list_view.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._toolbar.set_ui_model(ui_model)
        self._list_view.set_ui_model(ui_model)
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_model()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._list_view.unregister_updaters()
        self._toolbar.unregister_updaters()

    def _get_update_signal_type(self):
        return 'signal_proc_sample_list_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_model()

    def _update_model(self):
        self._list_model = SampleListModel()
        self._list_model.set_au_id(self._au_id)
        self._list_model.set_proc_id(self._proc_id)
        self._list_model.set_ui_model(self._ui_model)
        self._list_view.setModel(self._list_model)


class SampleEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._name_editor = QLineEdit()

        gl = QGridLayout()
        gl.setMargin(0)
        gl.setSpacing(2)
        gl.addWidget(QLabel('Name:'), 0, 0)
        gl.addWidget(self._name_editor, 0, 1)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addLayout(gl)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self._name_editor, SIGNAL('editingFinished()'), self._change_name)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_list_update_signal_type(self):
        return 'signal_proc_sample_list_{}'.format(self._proc_id)

    def _get_selection_update_signal_type(self):
        return 'signal_proc_select_sample_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        update_signals = set([
            self._get_list_update_signal_type(),
            self._get_selection_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        sample_id = sample_params.get_selected_sample_id()
        has_sample = sample_id in sample_params.get_sample_ids()
        self.setEnabled(has_sample)

        name = u''
        if has_sample:
            name = sample_params.get_sample_name(sample_id)

        old_block = self._name_editor.blockSignals(True)
        if self._name_editor.text() != name:
            self._name_editor.setText(name)
        self._name_editor.blockSignals(old_block)

    def _change_name(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        sample_id = sample_params.get_selected_sample_id()
        sample_params.set_sample_name(sample_id, unicode(self._name_editor.text()))
        self._updater.signal_update(set([self._get_list_update_signal_type()]))


