#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QDesktopWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QtWidgets>
#include <QtNetwork>
#include <QUrl>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void get(QString path);

private slots:
    void on_pushButton_clicked();
    void readyRead();
    void on_checkBox_stateChanged(int arg1);

private:
    QNetworkAccessManager manager;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
