# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import os
import sys
import traceback

from kunquat.tracker.ui.qt import *

from kunquat.tracker.errorbase import *


MESSAGE_RICH_FMT = \
'''<h3>We are sorry, but Kunquat Tracker
encountered an error and needs to close.</h3>
<p>This is a programming error. If you would like to help us fix it,
please submit an issue to Kunquat issue tracker at
<a{} href="https://github.com/kunquat/kunquat/issues">https://github.com/kunquat/kunquat/issues</a>
with the following information attached.</p>'''


class ErrorDetails(QTextEdit):

    def __init__(self):
        super().__init__()
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

    exceptionReceived = Signal(str, name='exceptionReceived')

    def __init__(self):
        super().__init__()

        self.setWindowTitle('Oh no!')
        self._message = QLabel(MESSAGE_RICH_FMT.format(''))
        self._message.setWordWrap(True)
        self._details = ErrorDetails()
        self._closebutton = QPushButton('Exit Kunquat')

        v = QVBoxLayout()
        v.addWidget(self._message)
        v.addWidget(self._details)
        h = QHBoxLayout()
        h.addWidget(self._closebutton)
        v.addItem(h)
        self.setLayout(v)

        QObject.connect(self, SIGNAL('exceptionReceived(QString)'), self._show_dialog)
        QObject.connect(self._closebutton, SIGNAL('clicked()'), self.close)

        sys.excepthook = self._excepthook

    def update_link_colour(self, style_manager):
        style = ''

        if style_manager.is_custom_style_enabled():
            colour = style_manager.get_link_colour()
            style = ' style="color: {};"'.format(colour)

        self._message.setText(MESSAGE_RICH_FMT.format(style))

    def _excepthook(self, eclass, einst, trace):
        if eclass == KeyboardInterrupt:
            os.abort()
            return

        print_error_msg(eclass, einst, trace)
        log_error(eclass, einst, trace)
        details = get_error_details(eclass, einst, trace)

        QObject.emit(self, SIGNAL('exceptionReceived(QString)'), details)

    def _show_dialog(self, details):
        details = str(details)
        self._details.set_details(details)
        self.exec_()
        os.abort()

    def sizeHint(self):
        return QSize(768, 512)


