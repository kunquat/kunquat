from PyQt4.QtGui import QToolButton, QVBoxLayout, QLabel, QPushButton
from PyQt4.QtCore import Qt

from kunquat.qt.twled import TWLed

class TWButton(QPushButton):

    def __init__(self, color_style = '100', parent = None):
        super(self.__class__, self).__init__(parent)
        self.setMinimumWidth(60)
        self.setMinimumHeight(60)
        layout = QVBoxLayout(self)
        led = TWLed(color_style)
        self._led = led
        layout.addWidget(led)
        notename = QLabel()
        self._notename = notename
        notename.setAlignment(Qt.AlignCenter)
        layout.addWidget(notename)
        layout.setAlignment(Qt.AlignCenter)
        self.setFocusPolicy(Qt.NoFocus)

    def set_name(self, name):
        self._notename.setText(name)

    def set_enabled(self, enabled):
        self._led.set_enabled(enabled)
        self.setEnabled(enabled)

    def set_led(self, light):
        self._led.light(light)

    def set_color(self, color):
        if color == 'dark':
            self.setStyleSheet("QLabel { background-color: #001; }")
            self._notename.setStyleSheet("QLabel { color: #fff; }")
        if color == 'light':
            self.setStyleSheet("QLabel { background-color: #ffe; }")
            self._notename.setStyleSheet("QLabel { color: #000; }")
        if color == None:
            self.setStyleSheet("")
            self._notename.setStyleSheet("")
            



