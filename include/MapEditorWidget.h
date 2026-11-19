#ifndef MAPEDITORWIDGET_H
#define MAPEDITORWIDGET_H

#include "CWarehouseMap.h"
#include "WarehouseGridWidget.h"
#include <QComboBox>
#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QShortcut>
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
    QShortcut *undoShortcut;
    QShortcut *redoShortcut;
    QPushButton *exportButton;
    QPushButton *importButton;

    void refreshStatusUI();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onCreateNewGrid();
    void onToolChanged(int index);
    void onApproveClicked();
    void onCreateDraftCopyClicked();
    void onUndo();
    void onRedo();
    void onExportClicked();
    void onImportClicked();

public:
    explicit MapEditorWidget(QWidget *parent = nullptr);
    const CWarehouseMap &getCurrentMap() const;
};

#endif