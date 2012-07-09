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
        self.dark()

    def dark(self):
        self.setStyleSheet("QLabel { background-color: #500; }")

    def bright(self):
        self.setStyleSheet("QLabel { background-color: #f00; }")

    def dim(self):
        self.setStyleSheet("QLabel { background-color: #720; }")
