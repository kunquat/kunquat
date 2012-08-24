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
        env_ter.setText('io')
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
                               self.p.instrument_window)

        self._instrument = QtGui.QComboBox()
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

        scale_selector = QtGui.QComboBox()
        scale_selector.addItem(u'chromatic')
        scale_selector.setDisabled(True)

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
        #layout.addWidget(scale_selector)
        #layout.addWidget(scale_config)
        layout.addWidget(self._create_separator())

        layout.addWidget(self._songcount)
        layout.addWidget(sheet_but)
        layout.addWidget(self._create_separator())

        #layout.addWidget(env_label)
        layout.addWidget(env_ter)

        self._view = top_control

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

    def update_instruments(self):
        # FIXME: Remove try-catch
        try:
            inst_num = self.p._instruments._inst_num
        except AttributeError:
            inst_num = 0
        while self._instrument.count() > 0:
            self._instrument.removeItem(0)
        ids = self.p.project._composition.instrument_ids()
        numbers = [int(i.split('_')[1]) for i in ids]
        for i in sorted(numbers):
            ins = self.p.project._composition.get_instrument(i)
            name = ins.get_json('kqti00/m_name.json') or '-'
            self._instrument.addItem(u'%s: %s' % (i, name))
            if i == inst_num:
                self._instrument.setCurrentIndex(self._instrument.count() - 1)

    def get_view(self):
        return self._view

