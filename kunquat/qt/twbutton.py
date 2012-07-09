from PyQt4.QtGui import QToolButton, QVBoxLayout, QLabel, QPushButton
from PyQt4.QtCore import Qt

from kunquat.qt.twled import TWLed

class TWButton(QPushButton):

    def __init__(self, parent = None):
        super(self.__class__, self).__init__(parent)
        self.setMinimumWidth(60)
        self.setMinimumHeight(60)
        layout = QVBoxLayout(self)
        led = TWLed()
        layout.addWidget(led)
        notename = QLabel()
        self._notename = notename
        notename.setAlignment(Qt.AlignCenter)
        layout.addWidget(notename)
        layout.setAlignment(Qt.AlignCenter)

    def set_name(self, name):
        self._notename.setText(name)

    def set_color(self, color):
        if color == 'dark':
            self.setStyleSheet("QPushButton { background-color: #001; }")
            self._notename.setStyleSheet("QLabel { color: #fff; }")
        if color == 'light':
            self.setStyleSheet("QPushButton { background-color: #ffe; }")
            self._notename.setStyleSheet("QLabel { color: #000; }")



