#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLayout>
#include <QDir>
#include <iostream>
#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <stdio.h>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRegExp>
#include "boxlineedit.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentImage(0),
    _files(),
    _imageItem(new QGraphicsPixmapItem),
    _boxLineEdits()
{
    ui->setupUi(this);
    setWindowTitle("Saltybot Data Trainer");
    QDir::setCurrent("../saltydatabot/training");
    QDir dir("./");
    QStringList nameFilters;
    nameFilters << "*.tif";
    QStringList list = dir.entryList(nameFilters);
    foreach(QString file, list){
        _files << file;
    }

    _nextFile = _files.begin();

    QGroupBox *firstBox = new QGroupBox;
    QGroupBox *secondBox = new QGroupBox;
    QGroupBox *thirdBox = new QGroupBox;

    QVBoxLayout *firstLayout = new QVBoxLayout();
    QGraphicsScene *scene = new QGraphicsScene;
    QGraphicsView *view = new QGraphicsView(scene);
    scene->addItem(_imageItem);
    firstBox->setLayout(firstLayout);
    firstLayout->addWidget(view);

     _inputLayout = new QHBoxLayout;
     _inputLayout->setAlignment(Qt::AlignLeft);
    secondBox->setLayout(_inputLayout);
    firstLayout->addWidget(secondBox);

    QHBoxLayout *thirdLayout = new QHBoxLayout();
    QPushButton *deleteButton = new QPushButton("Delete Image");
    connect(deleteButton,SIGNAL(clicked()),this,SLOT(deleteCurrentFile()));
    thirdLayout->addWidget(deleteButton);
    QPushButton *acceptButton = new QPushButton("Accept");
    connect(acceptButton,SIGNAL(clicked()),this,SLOT(saveAndContinue()));
    thirdLayout->addWidget(acceptButton);
    thirdBox->setLayout(thirdLayout);
    firstLayout->addWidget(thirdBox);

    setCentralWidget(firstBox);
    loadNextFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::execShell(const QString &cmd){
    qDebug() << "Executing " << cmd;
    FILE* pipe = popen(cmd.toLatin1().constData(),"r");
    if(!pipe) return "ERROR";
    char buffer[128];
    QString res("");
    while(!feof(pipe)) {
        if(fgets(buffer,128,pipe) != NULL) res += buffer;
    }
    pclose(pipe);
    qDebug() << res;
    return res;
}

void MainWindow::loadNextFile(){
    _currentFile = _nextFile;

    if(_nextFile == _files.end()) {
        combineAndFinish();
    }
    _imageItem->setPixmap(QPixmap(*_nextFile++));
    setMinimumWidth(_imageItem->boundingRect().width());
    if(!QFile(getBoxName(*_currentFile)).exists()){
        QString command = "tesseract " + *_currentFile + " " + getBaseName(*_currentFile) + " batch.nochop makebox";
        execShell(command);
    }

    clearLayout(_inputLayout);
    populateCharacters();

}

void MainWindow::clearLayout(QLayout* layout) {
    while(QLayoutItem* item = layout->takeAt(0)){
        if(QWidget* widget = item->widget()){
            widget->deleteLater();
        }

        if(QLayout* childLayout = item->layout()){
            clearLayout(childLayout);
        }
        delete item;
    }
}

QString MainWindow::getBaseName(const QString &file){
    QString ret = file;
    QRegExp rx("(.*eng.salt.exp[0-9]+).*");
    ret.replace(rx,"\\1");
    return ret;
}

QString MainWindow::getBoxName(const QString &file) {
    return getBaseName(file) + ".box";
}

void MainWindow::populateCharacters() {
    QFile f(getBoxName(*_currentFile));
    if(!f.open(QFile::ReadOnly)){
        qWarning() << "Could not open " << getBoxName(*_currentFile) << " attempting to proceed to next file.";
        loadNextFile();
        return;
    }

    _boxLineEdits.clear();

    bool firstLine = true;
    while(!f.atEnd()){
        QString line = f.readLine();
        QStringList vals = line.split(' ');
        BoxLineEdit *text = new BoxLineEdit(vals);
        _boxLineEdits << text;
        connect(text,SIGNAL(returnPressed()),this,SLOT(saveAndContinue()));
        text->setMaximumHeight(30);
        text->setMaximumWidth(30);
        _inputLayout->addWidget(text);
        if(firstLine){
            firstLine = false;
            text->show();
            text->setFocus();

        }
    }
    f.close();;
}

void MainWindow::saveAndContinue(){
    QFile f(getBoxName(*_currentFile));
    if(!f.open(QFile::WriteOnly)){
        qWarning() << "Could not save file " << getBoxName(*_currentFile);
    } else {
        QTextStream stream(&f);
        foreach(BoxLineEdit* line, _boxLineEdits){
            stream << *line;
        }
        if(stream.status() != QTextStream::Ok){
            qWarning() << "Error occurred writing file " << getBoxName(*_currentFile);
        }
        f.close();
    }
    loadNextFile();
}

void MainWindow::deleteCurrentFile(){
    QFile f1(*_currentFile);
    QFile f2(getBoxName(*_currentFile));
    if(f1.exists()){
        if(!f1.remove()){
            qWarning() << "Unable to delete file " << f1.fileName();
        }
    } else {
        qWarning() << "Could not find file: " << *_currentFile;
    }

    if(f2.exists()){
        if(!f2.remove()){
            qWarning() << "Unable to delete file " << f2.fileName();
        }
    } else {
        qWarning() << "No box file for " << *_currentFile;
    }

    _files.removeOne(*_currentFile);

    loadNextFile();
}

void MainWindow::combineAndFinish() {
    qDebug() << "Finished reading files. Combining training data and cleaning up";

    QStringList::Iterator it = _files.begin();
    while(it != _files.end()){
        QString command("convert stage/eng.salt.exp0.tif ");
        QTextStream commandStream(&command);
        for(int i = 0; i < 20 && it != _files.end(); i++){
            commandStream << *it++ << " ";
        }

        commandStream << "stage/eng.salt.exp0.tif";
        commandStream.flush();
        execShell(command);
    }

    QFile combinedBoxFile("stage/eng.salt.exp0.box");

    //find last page
    combinedBoxFile.open(QFile::ReadOnly);
    combinedBoxFile.seek(combinedBoxFile.size() - 2*sizeof(char));
    char lastChar;
    QString lastPage("");

    while(!combinedBoxFile.error()){
        combinedBoxFile.read(&lastChar,1);
        if(lastChar == ' ') break;
        combinedBoxFile.seek(combinedBoxFile.pos()-2*sizeof(char));
        lastPage.prepend(lastChar);
    }
    qDebug() << "Previous last page was " << lastPage;
    long lastPageNum = lastPage.toLong();


    combinedBoxFile.close();

    //append data
    combinedBoxFile.open(QFile::Append);
    QTextStream fileStream(&combinedBoxFile);
    foreach(QString fileName, _files){
        QFile boxFile(getBoxName(fileName));
        boxFile.open(QFile::ReadOnly);
        QTextStream singleFileStream(&boxFile);
        ++lastPageNum;
        while(!singleFileStream.atEnd()){
            QString line = singleFileStream.readLine();
            line.chop(1);
            line += QString::number(lastPageNum) + '\n';
            fileStream << line;


        }
    }

    fileStream.flush();
    combinedBoxFile.close();

    QDir::setCurrent("stage");
    execShell("tesseract eng.salt.exp0.tif eng.salt.exp0 box.train");
    execShell("unicharset_extractor eng.salt.exp0.box");
    execShell("spaheclustering -F font_properties -U unicharset eng.salt.exp0.tr");
    execShell("mftraining -F font_properties -U unicharset -O eng.unicharset eng.salt.exp0.tr");
    execShell("cntraining eng.salt.exp0.tr");
    execShell("mv inttemp eng.inttemp; mv pffmtable eng.pffmtable; cp font_properties eng.font_properties; mv normproto eng.normproto; mv shapetable eng.shapetable");
    execShell("combine_tessdata eng.");

    QDir::setCurrent("../");
    foreach(QString fileName, _files){
        QFile f0(QString("backup/") + fileName);
        if(f0.exists()) f0.remove();
        QFile f1(fileName);
        f1.rename(QString("backup/") + fileName);

        QFile f2(QString("backup/") + getBoxName(fileName));
        if(f2.exists()) f2.remove();
        QFile f3(getBoxName(fileName));
        f3.rename(QString("backup/") + getBoxName(fileName));
    }

    exit(0);
}

