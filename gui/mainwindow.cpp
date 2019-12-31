#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDebug>
#include <QDir>

#if defined(Q_OS_WIN)
#define MQABIN "./MQA_identifier.exe"
#else
#define MQABIN ".//MQA_identifier"
#endif


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    QObject::connect(&process, SIGNAL(readyReadStandardOutput()), this, SLOT(got_output()));

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_folderButton_clicked() {
    QString folder = QFileDialog::getExistingDirectory(
        this,
        tr("Select Folder"),
        "."
    );
    ui->outputTextBox->clear();
    ui->outputTextBox->moveCursor(QTextCursor::End);

    process.start(MQABIN, QStringList(folder));
    process.waitForFinished();
}

void MainWindow::on_FileButton_clicked() {
    QStringList filenames = QFileDialog::getOpenFileNames(
        this,
        tr("Selct File(s)"),
        ".",
        "flac Files (*.flac)"
    );
    ui->outputTextBox->clear();
    ui->outputTextBox->moveCursor(QTextCursor::End);

    process.start(MQABIN, filenames);
    process.waitForFinished();

}


void MainWindow::got_output() {
    QProcess *p = reinterpret_cast<QProcess *>(sender());
    ui->outputTextBox->insertPlainText(p->readAllStandardOutput());
    ui->outputTextBox->moveCursor(QTextCursor::End);
}
