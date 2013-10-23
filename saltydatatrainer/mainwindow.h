#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QGraphicsPixmapItem>
#include <QLayout>
#include "boxlineedit.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void saveAndContinue();
    void loadNextFile();
    void deleteCurrentFile();

private:
    QString execShell(const QString &command);
    QString getBaseName(const QString &file);
    QString getBoxName(const QString &file);
    void clearLayout(QLayout* layout);
    void populateCharacters();
    void combineAndFinish();


    Ui::MainWindow *ui;
    long currentImage;
    QStringList _files;
    QGraphicsPixmapItem* _imageItem;
    QLayout* _inputLayout;
    QStringList::Iterator _currentFile;
    QStringList::Iterator _nextFile;
    QList<BoxLineEdit*> _boxLineEdits;

};

#endif // MAINWINDOW_H
