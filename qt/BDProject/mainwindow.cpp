#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    path = "/home/juan/Documentos/2020-1/project1_bd2/qt/BDProject/";
    ui->setupUi(this);
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels({"Id", "Description", "City", "State", "Weather"});
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QString query;
    QStringList queries = ui->textEdit->toPlainText().toLower().split(";");
    for (int var = 0; var < queries.size(); ++var) {
        query = queries.at(var);
        //create table
        if (query.contains("create table ", Qt::CaseInsensitive)){
            int ind = query.indexOf("create table ");
            int pos = ind + QString("create table ").size();
            QString res = query.mid(pos , query.size() - pos);

            QFileInfo check_file(res + ".dat");
            // check if file exists and if yes: Is it really a file and nodirectory?
            if (check_file.exists() && check_file.isFile()) {
                ui->donebutton->setText("La tabla ya existe");
            } else {
                std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>((res + ".dat").toUtf8().constData(), false);
                std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>((res + ".index").toUtf8().constData(), false);
                bd2::DataBase<Default, int> newdb = bd2::DataBase<Default, int>(index, data, 0);
                ui->donebutton->setText("Tabla creada");
            }
            databases.push_back(res);
        }
        else if (query.contains("insert into ")) {
            int ind = query.indexOf("insert into ");
            int pos = ind + QString("insert into ").size();
            QString subquery = query.mid(pos , query.size() - pos);
            QStringList parts = subquery.split(" ");
            QString namedb = parts.at(0);

            QFileInfo check_file(namedb + ".dat");
            if (check_file.exists() && check_file.isFile()) {
                std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>((namedb + ".dat").toUtf8().constData(), true);
                std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>((namedb + ".index").toUtf8().constData(), true);
                bd2::DataBase<Default, int> dbconsult = bd2::DataBase<Default, int>(index, data, 0);
                if (query.contains("from ")){
                    ind = query.indexOf("from ");
                    pos = ind + QString("from ").size();
                    QString res = query.mid(pos , query.size() - pos);
                    QFileInfo fi (path + res);
                    if (fi.exists() && fi.isFile()){
                        dbconsult.loadFromExternalFile((path + res).toUtf8().constData());
                        ui->donebutton->setText("Inserted from file");
                    }
                    else
                        ui->donebutton->setText("El archivo mencionado no existe");

                }
                else if (query.contains("values ")){
                    ind = query.indexOf("values ");
                    pos = ind + QString("values ").size();
                    QString res = query.mid(pos , query.size() - pos);
                    res.remove(QChar('('),Qt::CaseSensitive);
                    res.remove(QChar(')'),Qt::CaseSensitive);
                    res.remove(QChar(' '),Qt::CaseSensitive);
                    QStringList entry = res.split(",");
                    if (entry.size()!=5)
                        ui->donebutton->setText("Entradas no validas");
                    else{
                        Default new_elem;
                        new_elem.id = atoi(entry.at(0).toUtf8().constData());
                        strcpy(new_elem.description, entry.at(1).toUtf8().constData());
                        strcpy(new_elem.city, entry.at(2).toUtf8().constData());
                        strcpy(new_elem.state, entry.at(3).toUtf8().constData());
                        strcpy(new_elem.weather, entry.at(4).toUtf8().constData());
                        dbconsult.insertWithBPlusTreeIndex(new_elem, new_elem.id, false);
                        ui->donebutton->setText("Insertado nuevo elemento");
                    }
                }
            } else {
                ui->donebutton->setText("No se encontró la tabla");
            }
        }
        else if (query.contains("select * from ")){
            int ind = query.indexOf("select * from ");
            int pos = ind + QString("select * from ").size();
            QString subquery = query.mid(pos , query.size() - pos);
            QStringList parts = subquery.split(" ");
            QString namedb = parts.at(0);

            QFileInfo check_file(namedb + ".dat");
            if (check_file.exists() && check_file.isFile()){
                if (parts.size() == 1){
                    ui->tableWidget->setRowCount(0);
                    std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>((namedb + ".dat").toUtf8().constData(), false);
                    std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>((namedb + ".index").toUtf8().constData(), false);
                    bd2::DataBase<Default, int> dbconsult = bd2::DataBase<Default, int>(index, data, 0);
                    Default d;
                    int i = 1;
                    while (dbconsult.readRecord(d, i)) {
                        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
                        ui->tableWidget->setItem(i - 1, 0, new QTableWidgetItem(QString::number(d.id)) );
                        ui->tableWidget->setItem(i - 1, 1, new QTableWidgetItem(d.description) );
                        ui->tableWidget->setItem(i - 1, 2, new QTableWidgetItem(d.city) );
                        ui->tableWidget->setItem(i - 1, 3, new QTableWidgetItem(d.state) );
                        ui->tableWidget->setItem(i - 1, 4, new QTableWidgetItem(d.weather) );
                        i++;
                    }
                    ui->tableWidget->resizeColumnsToContents();
                    ui->donebutton->setText("Select Done!");
                }
                else if (subquery.contains("where ")) {
                    if (subquery.contains("where key")){
                        subquery.remove(QChar('='), Qt::CaseSensitive);
                        subquery.remove(QChar(' '), Qt::CaseSensitive);
                        ind = subquery.indexOf("wherekey");
                        pos = ind + QString("wherekey").size();
                        subquery = subquery.mid(pos , subquery.size() - pos);
                        std::cout << subquery.toUtf8().constData();
                        ui->tableWidget->setRowCount(0);
                        std::shared_ptr<bd2::DiskManager> data = std::make_shared<bd2::DiskManager>((namedb + ".dat").toUtf8().constData(), false);
                        std::shared_ptr<bd2::DiskManager> index = std::make_shared<bd2::DiskManager>((namedb + ".index").toUtf8().constData(), false);
                        bd2::DataBase<Default, int> dbconsult = bd2::DataBase<Default, int>(index, data, 0);
                        Default d;
                        dbconsult.readRecord(d, atoi(subquery.toUtf8().constData()));
                        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
                        ui->tableWidget->setItem(0, 0, new QTableWidgetItem(QString::number(d.id)) );
                        ui->tableWidget->setItem(0, 1, new QTableWidgetItem(d.description) );
                        ui->tableWidget->setItem(0, 2, new QTableWidgetItem(d.city) );
                        ui->tableWidget->setItem(0, 3, new QTableWidgetItem(d.state) );
                        ui->tableWidget->setItem(0, 4, new QTableWidgetItem(d.weather) );
                        ui->donebutton->setText("Select with key correct");
                    }
                    else{
                        ui->donebutton->setText("Debe hacer una consulta con la primary key");
                    }
                }
                else {
                    ui->donebutton->setText("Incorrect select");
                }
            }
            else{
                ui->donebutton->setText("No se encontró la tabla " + namedb);
            }

        }
        else{
            ui->donebutton->setText("Incorrect Query");
        }
    }
}
