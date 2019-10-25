#include "mainwindow.h"
#include "ui_mainwindow.h"

#define P1_RIGHT_DOWN 0
#define P1_RIGHT_UP 1
#define P2_LEFT_DOWN 2
#define P2_LEFT_UP 3

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer1 = new QTimer(this);
    timer1->start(10);
    connect(timer1,SIGNAL(timeout()), this, SLOT(timercallfunction()));

    net.width = 5;
    net.height = field.width;

    Bar_Left.width = 5;
    Bar_Right.width = 5;

    Bar_Left.height = 180;
    Bar_Right.height = 180;

    Bar_Left.pos_x = net.width;
    Bar_Left.pos_y = field.center_y_pos()-Bar_Left.height/2;

    Bar_Right.pos_x = field.width - 2*net.width;
    Bar_Right.pos_y = field.center_y_pos()-Bar_Right.height/2;

//    p1=QPoint(net.width,field.center_y);
//    p2=QPoint(field.width-2*net.width , field.center_y);

    field_image = new QImage(field.width,field.height,QImage::Format_RGB32);
    field_image->fill(qRgb(0,128,0));

    net_image = new QImage(net.width,field.height,QImage::Format_RGB32);
    net_image->fill(qRgb(255,255,255));
}

MainWindow::~MainWindow()
{
    disconnectSerial(port_handler_left);
    disconnectSerial(port_handler_right);
    timer1->stop();

    delete ui;
}

void MainWindow::checkSerial(int handler, int *command){
    if (handler<=0)return;

    char charbuff[1];
    std::string words="";
    bool startwordfound = 0;
    while(read(handler, charbuff, 1)>0){

//        read(handler, charbuff, 1);
        if (charbuff[0] == '$'){
            startwordfound = 1;
            words="";
        }
        else if (startwordfound && charbuff[0] != '\n'){
            words = words + charbuff[0];
        }
        else if (charbuff[0] == '\n' && words!="") {

            int idx1 = words.find(",");
            std::string checksumsubstring = words.substr (idx1+1);
            std::string realword = words.substr(0,idx1);

            std::string debug = "recive:{" + realword + "}\n";
            qDebug()<<debug.c_str();

            startwordfound=0;

            if (realword=="l1") *command = P1_RIGHT_DOWN;
            else if (realword=="r1") *command = P1_RIGHT_UP;
            else if (realword=="l2") *command = P2_LEFT_DOWN;
            else if (realword=="r2") *command = P2_LEFT_UP;

            break;
        }
    }

}

void MainWindow::timercallfunction(){
    checkSerial(port_handler_left, &command);
//    qDebug()<<command;
//    checkSerial(port_handler_right, &command);//doesnt work somehow
    moveBar(command, &Bar_Left, &Bar_Right, &field);
    command = 99;
}

void MainWindow::on_lineEdit_returnPressed()
{
    port1 = ui->lineEdit->text();
    connectSerial(&port_handler_left, port1);

}

void MainWindow::on_lineEdit_2_returnPressed()
{
    port2 = ui->lineEdit->text();
    connectSerial(&port_handler_right, port2);
}

void MainWindow::connectSerial(int *handler, QString port){

    *handler = open(port.toLocal8Bit(), O_RDWR|O_NOCTTY);
    if (*handler < 0) {
        qDebug()<<"Serial Error "<<errno << '\n';
    }
    else {
        qDebug()<<"connected to"; qDebug()<<port.toLocal8Bit();
    }
    //========= Serial Port Configuration =========//
    //This will prevent from block reading (ICANON part is enough)
    //setting
    struct termios oldtio, newtio;
    tcgetattr(*handler, &oldtio);
    bzero(&newtio, sizeof(newtio));

    newtio.c_cflag = B9600|CS8|CLOCAL|CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag |= ~(ICANON|ECHO|ECHOE); //constrain to only to printable char, and enable non blokcing read by default
//    newtio.c_cc = VMIN(0)|VTIME(0);

    tcflush(*handler, TCIFLUSH);
    tcsetattr(*handler, TCSANOW, &newtio);
    //========= Serial Port Configuration =========//
//    while (tcflush(*handler,TCIOFLUSH)>0)sleep(2);
}

void MainWindow::disconnectSerial(int handler){
    ::close(handler); // :: mean global namespace, and close from unistd.h is in global namespace
//    tcsetattr(fd, TCSANOW, &oldtio);
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(QPoint(field.pos_x,field.pos_y), *field_image);
    painter.drawImage(QPoint(field.center_x_pos()-net.width/2,field.pos_y), *net_image);

    painter.setPen(QPen(Qt::blue,Bar_Left.width));

    painter.drawLine(Bar_Left.pos_x,Bar_Left.pos_y,
                     Bar_Left.pos_x,Bar_Left.pos_y+Bar_Left.height); //draw left bar

    painter.drawLine(field.width-2*Bar_Right.width,Bar_Right.pos_y,
                     field.width-2*Bar_Right.width,Bar_Right.pos_y+Bar_Right.height); //draw right bar
}

void MainWindow::moveBar(int command, Square *barleft, Square *barright, Square *field){

    Square templeft = *barleft;
    Square tempright = *barright;

    switch (command) {
    case P1_RIGHT_DOWN:
        barright->pos_y = barright->pos_y+barspeed;
        break;
    case P1_RIGHT_UP:
        barright->pos_y = barright->pos_y-barspeed;
        break;
    case P2_LEFT_DOWN:
        barleft->pos_y = barleft->pos_y+barspeed;
        break;
    case P2_LEFT_UP:
        barleft->pos_y = barleft->pos_y-barspeed;
        break;
    }

    if (barleft->pos_y >= (field->height -barleft->height )
            || barleft->pos_y <= field->pos_y){
        *barleft = templeft;
    }
    if (barright->pos_y >= (field->height -barright->height )
            || barright->pos_y <= field->pos_y){
        *barright = tempright;
    }

    update();
}
