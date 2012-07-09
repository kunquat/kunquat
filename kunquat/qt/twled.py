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
        from random import choice
        self.light(choice(['dark', 'bright', 'dim']))

    def light(self, light):
        if light == 'dark':
            self.setStyleSheet("QLabel { background-color: #600; }")
        if light == 'bright':
            self.setStyleSheet("QLabel { background-color: #e00; }")
        if light == 'dim':
            self.setStyleSheet("QLabel { background-color: #a00; }")
