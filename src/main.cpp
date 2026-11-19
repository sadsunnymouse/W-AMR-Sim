#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QLabel label("W-AMR Sim — базовое окружение готово");
    label.setWindowTitle("W-AMR Sim");
    label.resize(400, 150);
    label.show();

    return app.exec();
}