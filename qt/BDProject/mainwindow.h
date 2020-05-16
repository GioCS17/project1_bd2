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
        char state [4];
        char weather [35];

        void assign(int ids, std::string d, std::string c, std::string s, std::string w){
              id = ids;
              for (int i = 0; i < 249; i++){
                description[i] = d[i];
              }
              for (int i = 0; i < 30; i++){
                city[i] = c[i];
              }
              for (int i = 0; i < 2; i++){
                state[i] = s[i];
              }
              for (int i = 0; i < 34; i++){
                weather[i] = w[i];
              }
            }


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
