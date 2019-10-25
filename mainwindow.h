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

#include <QtMath>       //qCos qSin
#include <stdlib.h>     /* srand, rand */
#include <time.h>

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

    void animation_refresh();

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

    QImage* field_image;
    QImage* net_image;

    void disconnectSerial(int handler);
    void connectSerial(int *handler, QString port);

    int WIDTH = 640*2;
    int HEIGHT = 480;

    typedef struct Square{
        int width = 640*2;
        int height = 480;
        int pos_x = 0;
        int pos_y = 0;
        int center_x_pos() {return (width/2+pos_x);}
        int center_y_pos() {return (height/2+pos_y);}
    }Square;

    Square field;
    Square net;
    Square Bar_Left,Bar_Right;

    void moveBar(int command, Square *barleft, Square *barright, Square *field);

    const int barspeed = 10;

    typedef struct Ball{
        float x=640;
        float y=240;
        float speed = 1;
        float dir_deg = 45;
        float traj_x(){
            return speed*qSin(qDegreesToRadians(dir_deg));
        }
        float traj_y(){
            return speed*qCos(qDegreesToRadians(dir_deg))*-1;
        }
        /*  from left of BAR | from right of BAR    from left of BAR |
         *                -0 | 0                                  -0 |
         *                   |                                       |
         *                   |                                       |
         *       -90 ------- | ------- 90                -90 ------- |
         *                   |                                       |
         *                   |                                       |
         *              -180 | 180                              -180 |
         */
    }Ball;

    Ball ball;
    //flip speed when hit bars
    //v (speed) always 1
    //v_y = v cos position
    //v_x = v sin position
    //position of ball to bar == angle (top bar == 0, bottom == 180);
    //that's why we use v_y = -(v cos position)
    //qcos and qsin using radian as input

    //flip when hit right bar
    //v_y = -v cos position
    //v_x = -v sin position
    //sin will always positif so turn it negative to bouch the ball to the left
    int scoreCheck(Ball *b, Square rightbar, Square leftbar);
    void updateballposition(Ball *b, Square field);
    float mapbartodeg(Square bar, Ball ball);
    int scoreL,scoreR;
    void resetballposition(Ball *b, Square field);
    void resetscore();
};

#endif // MAINWINDOW_H
