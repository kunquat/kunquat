from PyQt4.QtGui import QLabel, QFrame


class TWLed(QLabel):

    def __init__(self, color_style, parent = None):
        super(self.__class__, self).__init__(parent)
        self.setMinimumWidth(35)
        self.setMinimumHeight(15)
        self.setMaximumWidth(35)
        self.setMaximumHeight(15)
        self.setFrameStyle(QFrame.Panel | QFrame.Sunken)
        self.setLineWidth(2)
        self._color_style = color_style
        self._light = 8
        self._enabled = True
        self.update()

    def set_enabled(self, enabled):
        self._enabled = enabled
        self.update()

    def set_color_style(self, color_style):
        self._color_style = color_style
        self.update()

    def light(self, light):
        if light < 0 or light > 8:
            raise ValueError
        self._light = light
        self.update()

    def _set_color(self, color):
        self.setStyleSheet("QLabel { background-color: %s; }" % color)

    def update(self):
        if self._enabled:
            MINLIGHT = 6
            value = MINLIGHT + self._light
            hex_value = '%x' % value
            color = '#' + hex_value.join(self._color_style.split('1'))
        else:
            color = '#777'
        self._set_color(color)

