from PyQt6.QtWidgets import QApplication, QWidget, QLabel, QVBoxLayout, QHBoxLayout
from PyQt6.QtGui import QIcon, QFont, QPixmap, QMovie, QRegion
from PyQt6.QtCore import Qt, QSize
import sys

class Window(QWidget):
    def __init__(self):
        super().__init__()

        self.setGeometry(200, 200, 700, 400)
        self.setWindowTitle("Python QLabel")
        self.setWindowIcon(QIcon('qt.png'))

        self.layout = QHBoxLayout()

        editor_layout = QVBoxLayout()

        editor_layout.setStyleSheet("background-color: red")

        editor_layout.addWidget(QLabel('Text 1'))
        editor_layout.addWidget(QLabel('Text 2'))
        editor_layout.addWidget(QLabel('Text 3'))
        
        self.layout.addLayout(editor_layout)

        label = QLabel(self)
        pixmap = QPixmap('images/test1/IMG_0935.png')

        ratio = pixmap.width() / pixmap.height()

        new_width = 100
        new_hight = new_width / ratio

        high_rez = QSize(new_width, new_hight)

        pixmap = pixmap.scaled(high_rez)

        label.setStyleSheet("background-color: lightgreen")

        label.setPixmap(pixmap)

        self.layout.addWidget(label)

app = QApplication(sys.argv)
window = Window()
window.setLayout(window.layout)
window.show()
sys.exit(app.exec())