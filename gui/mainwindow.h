#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
 Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 private slots:
  void on_folderButton_clicked();

  void on_FileButton_clicked();

  void got_output();

 private:
  Ui::MainWindow *ui;
  QProcess process;
};


#endif // MAINWINDOW_H
