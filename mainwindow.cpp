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

    p1=QPoint(10,300);
    p2=QPoint(1485,300);
    canvas1 = new QImage(10,800,QImage::Format_RGB32);
    canvas2 = new QImage(1501,800,QImage::Format_RGB32);
    canvas3 = new QImage(1501,15,QImage::Format_RGB32);
    canvas1->fill(qRgb(255,255,255));
    canvas2->fill(qRgb(70,160,126));
    canvas3->fill(qRgb(255,255,255));

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
        else if (charbuff[0]>=32 && startwordfound && charbuff[0] != '\n'){
            //constrain only to printable char
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
    qDebug()<<command;
//    checkSerial(port_handler_right, &command);//doesnt work somehow
    moveBar(command, &barLeft, &barRight);
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
    newtio.c_lflag |= ~(ICANON|ECHO|ECHOE);
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
    painter.setPen(QPen(Qt::blue,5));
    painter.drawImage(QPoint(0,0), *canvas2);
    painter.drawImage(QPoint(755,0), *canvas1);
    painter.drawImage(QPoint(0,400), *canvas3);
    painter.drawLine(p1.x(),p1.y(),p1.x(),p1.y()+180);

    QPainter painter2(this);
    painter2.setPen(QPen(Qt::blue,5));
    painter2.drawLine(p2.x(),p2.y(),p2.x(),p2.y()+180);
}

void MainWindow::moveBar(int command, int *barLeft, int *barRight){
    switch (command) {
    case P1_RIGHT_DOWN:
        *barRight = p1.y()-30;
        p1.setY(*barRight);
        break;
    case P1_RIGHT_UP:
        *barRight = p1.y()+30;
        p1.setY(*barRight);
        break;
    case P2_LEFT_DOWN:
        *barLeft = p2.y()-30;
        p2.setY(*barLeft);
        break;
    case P2_LEFT_UP:
        *barLeft = p2.y()+30;
        p2.setY(*barLeft);
        break;
    }
    update();
}
