#ifndef MAPEDITORWIDGET_H
#define MAPEDITORWIDGET_H

#include "CWarehouseMap.h"
#include "WarehouseGridWidget.h"
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

class MapEditorWidget : public QWidget
{
    Q_OBJECT

private:
    CWarehouseMap currentMap;
    WarehouseGridWidget *gridWidget;

    QLineEdit *titleEdit;
    QSpinBox *widthSpin;
    QSpinBox *heightSpin;
    QComboBox *toolCombo;
    QLabel *statusLabel;
    QPushButton *approveButton;
    QPushButton *newDraftButton;

    void refreshStatusUI();

private slots:
    void onCreateNewGrid();
    void onToolChanged(int index);
    void onApproveClicked();
    void onCreateDraftCopyClicked();

public:
    explicit MapEditorWidget(QWidget *parent = nullptr);
    const CWarehouseMap &getCurrentMap() const;
};

#endif