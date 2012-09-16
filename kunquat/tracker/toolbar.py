# -*- coding: utf-8 -*-

from PyQt4 import QtCore, QtGui
import kqt_limits as lim

class Toolbar():
    def __init__(self, p):
        self.p = p
        top_control = QtGui.QWidget()
        self._songcount = QtGui.QLabel()
        layout = QtGui.QHBoxLayout(top_control)
        layout.setMargin(5)
        layout.setSpacing(5)

        new_project = QtGui.QToolButton()
        new_project.setText('Clear Project')
        new_project.setIcon(QtGui.QIcon.fromTheme('document-new'))
        new_project.setAutoRaise(True)
        new_project.setDisabled(True)
        #QtCore.QObject.connect(new_project,
        #                       QtCore.SIGNAL('clicked()'),
        #                       self.clear)

        open_project = QtGui.QToolButton()
        open_project.setText('Import Composition')
        open_project.setIcon(QtGui.QIcon.fromTheme('document-open'))
        open_project.setAutoRaise(True)
        open_project.setDisabled(True)
        #QtCore.QObject.connect(open_project,
        #                       QtCore.SIGNAL('clicked()'),
        #                       self.import_composition)

        save_project = QtGui.QToolButton()
        save_project.setText('Export Composition')
        save_project.setIcon(QtGui.QIcon.fromTheme('document-save'))
        save_project.setAutoRaise(True)
        QtCore.QObject.connect(save_project, QtCore.SIGNAL('clicked()'),
                               self.p.export_composition)

        env_label = QtGui.QLabel('in: 0 out: 0')

        env_ter = QtGui.QToolButton()
        env_ter.setText('Bindigs')
        env_ter.setIcon(QtGui.QIcon.fromTheme('modem'))
        env_ter.setAutoRaise(True)
        QtCore.QObject.connect(env_ter, QtCore.SIGNAL('clicked()'),
                               self.p.environment_window)

        sheet_but = QtGui.QToolButton()
        sheet_but.setText(u'♫')
        sheet_but.setAutoRaise(True)
        QtCore.QObject.connect(sheet_but, QtCore.SIGNAL('clicked()'),
                               self.p.show_sheet)

        instrument_ter = QtGui.QToolButton()
        instrument_ter.setText('Instrument Configuration')
        instrument_ter.setIcon(QtGui.QIcon.fromTheme('audio-card'))
        instrument_ter.setAutoRaise(True)
        QtCore.QObject.connect(instrument_ter, QtCore.SIGNAL('clicked()'),
                               self.p.instruments_window)

        self._instrument = QtGui.QComboBox()
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        self._instrument.setSizePolicy(sizePolicy)


        ''''
        self._instrument = QtGui.QSpinBox()
        self._instrument.setMinimum(0)
        self._instrument.setMaximum(lim.INSTRUMENTS_MAX - 1)
        self._instrument.setValue(0)
        self._instrument.setToolTip('Instrument')
        '''

        octave_label = QtGui.QLabel('octave:')
        self._octave = QtGui.QSpinBox()
        self._octave.setMinimum(lim.SCALE_OCTAVE_FIRST)
        self._octave.setMaximum(lim.SCALE_OCTAVE_LAST)
        self._octave.setValue(4)
        self._octave.setToolTip('Base octave')

        self._scale_selector = QtGui.QComboBox()
        self._scale_selector.setSizePolicy(sizePolicy)
        self._scale_selector.addItem(u'chromatic')

        scale_config = QtGui.QToolButton()
        scale_config.setText(u'Scale configuration')
        scale_config.setIcon(QtGui.QIcon.fromTheme('input-gaming'))
        scale_config.setAutoRaise(True)
        scale_config.setDisabled(True)
        #QtCore.QObject.connect(open_project,
        #                       QtCore.SIGNAL('clicked()'),
        #                       self.import_composition)


        #layout.addWidget(new_project)
        #layout.addWidget(open_project)
        layout.addWidget(save_project)
        layout.addWidget(self._create_separator())

        layout.addWidget(self._instrument)
        layout.addWidget(instrument_ter)
        layout.addWidget(self._create_separator())

        layout.addWidget(octave_label)
        layout.addWidget(self._octave)
        layout.addWidget(self._scale_selector)
        #layout.addWidget(scale_config)
        layout.addWidget(self._create_separator())

        layout.addWidget(self._songcount)
        layout.addWidget(sheet_but)
        layout.addWidget(self._create_separator())

        #layout.addWidget(env_label)
        layout.addWidget(env_ter)

        self._view = top_control

        QtCore.QObject.connect(self._scale_selector,
                               QtCore.SIGNAL('currentIndexChanged (const QString&)'),
                               self.scale_changed)

    def scale_changed(self, text):
        if text == '':
            return
        parts = text.split(':')
        number = int(parts[0] )
        self.p._scale = self.p._scales[number]

        # FIXME: Remove try-catch
        try:
            self.p._tw.update()
        except AttributeError:
            pass

    def _create_separator(self):
        separator = QtGui.QFrame()
        separator.setFrameShape(QtGui.QFrame.VLine)
        separator.setFrameShadow(QtGui.QFrame.Sunken)
        return separator

    def update_songs(self):
        songs = len(self.p.project._composition.song_ids())
        patterns = len(self.p.project._composition.pattern_ids())
        if songs > 0 or patterns > 0:
            info = u'%s song(s)' % songs
        else:
            info = u'no score'
        self._songcount.setText(info)

    def setEnabled(self, value):
        self._view.setEnabled(value)

    def instrument_string(self, number):
        ins = self.p.project._composition.get_instrument(number)
        name = ins.get_json('m_name.json') or '-'
        s = u'%s: %s' % (number, name)
        return s

    def scale_string(self, number):
        scale = self.p._scales[number]
        name = scale.get_name()
        s = u'%s: %s' % (number, name)
        return s

    def select_instrument(self, number):
        s = self.instrument_string(number)
        index = self._instrument.findText(s)
        self._instrument.setCurrentIndex(index)

    def select_scale(self, number):
        s = self.scale_string(number)
        index = self._scale_selector.findText(s)
        self._scale_selector.setCurrentIndex(index)

    def all_ints(self):
        i = 0
        while True:
            yield i
            i += 1

    def update_scales(self):
        current = 0

        for i, s in zip(self.all_ints(), self.p._scales):
            if s == self.p._scale:
                current = i

        while self._scale_selector.count() > 0:
            self._scale_selector.removeItem(0)
        i = 0
        for s in self.p._scales:
            s = self.scale_string(i)
            self._scale_selector.addItem(s)
            if i == current:
                self.select_scale(i)
            i += 1


    def update_instruments(self):
        inst_num = self.p._instruments._inst_num
        while self._instrument.count() > 0:
            self._instrument.removeItem(0)
        ids = self.p.project._composition.instrument_ids()
        numbers = [int(i.split('_')[1]) for i in ids]
        for i in sorted(numbers):
            s = self.instrument_string(i)
            self._instrument.addItem(s)
            if i == inst_num:
                self.select_instrument(i)

    def get_view(self):
        return self._view

