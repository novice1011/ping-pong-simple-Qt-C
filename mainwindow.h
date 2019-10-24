#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

//for bars
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsPathItem>

//HEADER FOR SERIAL COM
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <string>
#include <iostream>
#include <QtDebug>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:

    void timercallfunction();

    void on_lineEdit_returnPressed();

    void on_lineEdit_2_returnPressed();

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QTimer *timer1 ;      //object to start and stop timer
    QString port1, port2;
    int port_handler_left=0;
    int port_handler_right=0;
    int command=99;

    void checkSerial(int handler, int *command);

    void paintEvent(QPaintEvent *);

    void moveBar(int command, int *barLeft, int *barRight);

    QImage* canvas1;
    QImage* canvas2;
    QImage* canvas3;
    QPoint p1;
    QPoint p2;
    int barLeft, barRight;
    void disconnectSerial(int handler);
    void connectSerial(int *handler, QString port);
};

#endif // MAINWINDOW_H
