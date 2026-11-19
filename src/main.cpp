#include "MapEditorWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MapEditorWidget editor;
    editor.setWindowTitle("W-AMR Sim — АРМ Руководителя склада (Редактор топологии)");
    editor.resize(900, 650);
    editor.show();

    return app.exec();
}