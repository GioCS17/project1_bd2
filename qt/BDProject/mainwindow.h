#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include "./../../src/data_base_manager.h"
#include <QLabel>
#include <QMap>
#include <QDir>
#include <QFileInfo>

struct Default
    {
        int id;
        char description [249];
        char city [30];
        char state [2];
        char weather [35];

        void show(){
            std:: cout << "id: " << id << std::endl;
            std:: cout << "desc: " << description << std::endl;
            std:: cout << "city: " << city << std::endl;
            std:: cout << "state: " << state << std::endl;
            std:: cout << "weather: " << weather << std::endl;
            std:: cout << "-------------------------------" << std::endl;        }
    };

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QStringList databases;
    QString path;


private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
