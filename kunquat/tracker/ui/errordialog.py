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
import os
import sys
import traceback

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from kunquat.tracker.errorbase import *


MESSAGE_RICH = \
'''<p>Kunquat Tracker encountered an error.</p>
<p>Please submit an issue to Kunquat issue tracker at
<a href="https://github.com/kunquat/kunquat/issues">https://github.com/kunquat/kunquat/issues</a>
with the following information attached.</p>'''


class ErrorDetails(QTextEdit):

    def __init__(self):
        QTextEdit.__init__(self)
        self.setAcceptRichText(False)
        self.setReadOnly(True)
        self._details = ''
        self.setPlainText(
                'This window did not receive error details.'
                ' Please check terminal output if available.')

    def set_details(self, details):
        self._details = details
        self.setPlainText(details)

    def get_details(self):
        return self._details


class ErrorDialog(QDialog):

    exceptionReceived = pyqtSignal(str, name='exceptionReceived')

    def __init__(self):
        QDialog.__init__(self)

        self.setWindowTitle('I am error.')
        self._message = QLabel(MESSAGE_RICH)
        self._details = ErrorDetails()
        self._copybutton = QPushButton('Copy to clipboard')
        self._closebutton = QPushButton('Exit Kunquat')

        v = QVBoxLayout()
        v.addWidget(self._message)
        v.addWidget(self._details)
        h = QHBoxLayout()
        #h.addWidget(self._copybutton)
        h.addWidget(self._closebutton)
        v.addItem(h)
        self.setLayout(v)

        QObject.connect(
                self,
                SIGNAL('exceptionReceived(QString)'),
                self._show_dialog)
        QObject.connect(self._closebutton, SIGNAL('clicked()'), self.close)
        QObject.connect(self._copybutton, SIGNAL('clicked()'), self._copy_details)

        sys.excepthook = self._excepthook

    def _excepthook(self, eclass, einst, trace):
        print_error_msg(eclass, einst, trace)
        log_error(eclass, einst, trace)
        details = get_error_details(eclass, einst, trace)

        QObject.emit(
                self,
                SIGNAL('exceptionReceived(QString)'),
                details)

    def _show_dialog(self, details):
        details = str(details)
        self._details.set_details(details)
        self.exec_()
        os.abort()

    def _copy_details(self):
        details = self._details.get_details()
        clipboard = QApplication.clipboard()
        clipboard.setText(details)
        self._copybutton.setText('Copied!')


