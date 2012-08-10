from PyQt4.QtGui import QLabel, QFrame


class TWLed(QLabel):

    def __init__(self, parent = None):
        super(self.__class__, self).__init__(parent)
        self.setMinimumWidth(35)
        self.setMinimumHeight(15)
        self.setMaximumWidth(35)
        self.setMaximumHeight(15)
        self.setFrameStyle(QFrame.Panel | QFrame.Sunken)
        self.setLineWidth(2)
        self.light(8)

    def light(self, light):
        MINLIGHT = 6
        if light < 0 or light > 8:
            raise ValueError
        value = MINLIGHT + light
        self.setStyleSheet("QLabel { background-color: #%x00; }" % value)

