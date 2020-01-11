#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//
void MainWindow::on_checkBox_stateChanged(int arg1)
{
    arg1++;
    arg1--;
}

//
void MainWindow::on_pushButton_clicked()
{
    QString ip = ui->line_Edit_IP->text();
    QString port = ui->line_Edit_Port->text();
    QString file = ui->line_Edit_File->text();
    if(ip == "192.168.56.101" && port == "12345")
    {
        this->get("http://" + ip + ":" + port + "/" + file);
    }
    else
    {
        ui->line_Edit_IP->setText("");
        ui->line_Edit_Port->setText("");
        ui->line_Edit_File->setText("");
        ui->status->setText("IP and/or Port# incorrect!");
        ui->textBrowser->setText("");
    }
}

void MainWindow::get(QString path)
{
    QNetworkReply* response = manager.get(QNetworkRequest(QUrl(path)));
    connect(response,&QNetworkReply::readyRead,this,&MainWindow::readyRead);
}

void MainWindow::readyRead()
{
    QNetworkReply* response = qobject_cast<QNetworkReply*>(sender());
    int status_code = response->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(response->error() == QNetworkReply::NoError)
    {
        if(status_code == 200)
        {
        QString response_text = response->readAll();
        QByteArray bytes = response_text.toUtf8();
        QString response_size = QString::number(bytes.size());
        ui->status->setText("File size is: " + response_size + " bytes");
        if(ui->checkBox->isChecked())
        {
            ui->textBrowser->setText(response_text);
        }
        else
        {
            ui->textBrowser->setText("");
        }
        QString file_name = ui->line_Edit_File->text();
        QString path = ui->line_Edit_Dir->text();
        QString save_as = ui->line_Edit_SaveAs->text();
        QString full_path = "";
        if(path.isEmpty())
        {
            if(save_as.isEmpty())
            {
              full_path = "index.html";
            }
            else
            {
              full_path = save_as;
            }
        }
        else
        {
            if(save_as.isEmpty())
            {
                full_path = path + "index.html";
            }
            else
            {
                full_path = path + save_as;
            }
        }
        {
            QMessageBox::information(this,"Saved Succesfully",full_path + "was saved succesfully");
        }
        QFile file(full_path);
        if(!file.open(QFile::WriteOnly | QFile::Text))
        {
            QMessageBox::critical(this,"File Error","File could not be opened!");
        }
        QTextStream out(&file);
        out << response_text;
        file.flush();
        file.close();
        }
        else if(status_code == 400)
        {
            QMessageBox::warning(this,"Bad Request","Request could not be handled by the server!");
        }
        else if(status_code == 404)
        {
            QMessageBox::warning(this,"File Not Found","File not found on the server!");
        }
    }
    else
    {
        QMessageBox::critical(this,"Network Error","Network error occured!");
    }
}

