#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QMainWindow window;
    window.setWindowTitle("My First Qt App");
    window.resize(400, 300);
    
    // Центральный виджет
    QWidget *centralWidget = new QWidget(&window);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    
    // Метка
    QLabel *label = new QLabel("Hello, Qt with MSYS2!");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    
    // Кнопка
    QPushButton *button = new QPushButton("Click me!");
    layout->addWidget(button);
    
    // Подключение сигнала кнопки
    QObject::connect(button, &QPushButton::clicked, [label]() {
        label->setText("Button clicked!");
    });
    
    window.setCentralWidget(centralWidget);
    window.show();
    
    return app.exec();
}