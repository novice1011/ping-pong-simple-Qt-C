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
    timer2 = new QTimer(this);
//    timer1->start(10);
    connect(timer1,SIGNAL(timeout()), this, SLOT(timercallfunction()));
    connect(timer1,SIGNAL(timeout()),this,SLOT(animation_refresh()));

    connect(timer2,SIGNAL(timeout()),this,SLOT(animation_pause()));

    net.width = 5;
    net.height = field.width;

    Bar_Left.width = 5;
    Bar_Right.width = 5;

    Bar_Left.height = 180;
    Bar_Right.height = 180;

    Bar_Left.pos_x = net.width;
    Bar_Left.pos_y = field.center_y_pos()-Bar_Left.height/2;

    Bar_Right.pos_x = field.width - 2*Bar_Right.width;
    Bar_Right.pos_y = field.center_y_pos()-Bar_Right.height/2;

    field_image = new QImage(field.width,field.height,QImage::Format_RGB32);
    field_image->fill(qRgb(0,128,0));

    net_image = new QImage(net.width,field.height,QImage::Format_RGB32);
    net_image->fill(qRgb(255,255,255));

    ui->label_ball->setGeometry((int)ball.x,(int)ball.y,30,30);

    ball.dir_deg=90;

    scoreL = 0; scoreR = 0;

    resetballposition(&ball,field);

    resetscore();

    ui->countdown->setGeometry(field.center_x_pos(), field.pos_y, 80,80);
    ui->label_scoreL->setGeometry(field.pos_x+field.width/4, field.pos_y, 80,80);
    ui->label_scoreR->setGeometry(field.pos_x+field.width*3/4, field.pos_y, 80,80);
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

    if (barleft->pos_y >= (field->height -barleft->height )){
        barleft->pos_y = (field->height -barleft->height );
    }
    else if (barleft->pos_y <= field->pos_y) {
        barleft->pos_y = field->pos_y;
    }

    if (barright->pos_y >= (field->height -barright->height )){
        barright->pos_y = field->height -barright->height;
    }
    else if (barright->pos_y <= field->pos_y) {
        barright->pos_y = field->pos_y;
    }

    update();
}

void MainWindow::updateballposition(Ball *b, Square field){

    b->x = b->x + b->traj_x();
    b->y = b->y + b->traj_y();

    //bounce to ceiling from right
    if (b->y<=field.pos_y && b->dir_deg<0){
        b->dir_deg = -(180 + b->dir_deg);
        b->y = field.pos_y;
        //pos x need to calculate related to sampling
    }
    //bounce to floor from right
    else if (b->y >= field.pos_y+field.height && b->dir_deg<0) {
        b->dir_deg = -(180 + b->dir_deg);
        b->y = field.height;
        //pos x need to calculate related to sampling
    }
    //bounce to ceiling from left
    if (b->y<=field.pos_y && b->dir_deg>0){
        b->dir_deg = (180 - b->dir_deg);
        b->y = field.pos_y+field.pos_y;
        //pos x need to calculate related to sampling
    }
    //bounce to floor from left
    else if (b->y >= field.pos_y+field.height && b->dir_deg>0) {
        b->dir_deg = (180 - b->dir_deg);
        b->y = field.pos_y+field.height;
        //pos x need to calculate related to sampling
    }
}

int MainWindow::scoreCheck(Ball *b, Square rightbar, Square leftbar){
    //touch right border
    if (b->x > rightbar.pos_x){
        //on the bar
        if (b->y > rightbar.pos_y && b->y < rightbar.pos_y+rightbar.height){
            //change direction
            float angle = mapbartodeg(rightbar,*b);
            b->dir_deg = -angle;
            b->speed = b->speed + 0.5;
            return 0;
        }
        //not on the bar
        else {
            return 2; //score on the left
        }
    }
    //touch left border
    else if (b->x < leftbar.center_x_pos()) {
        //on the bar
        if (b->y > leftbar.pos_y && b->y < leftbar.pos_y+leftbar.height){
            //change direction
            float angle = mapbartodeg(leftbar,*b);
            b->dir_deg = angle;
            b->speed = b->speed + 0.5;
            return 0;
        }
        else {
            return 1;
        }
    }
}

void MainWindow::animation_refresh(){
    updateballposition(&ball,field);
    int score = scoreCheck(&ball,Bar_Right,Bar_Left);
    ui->label_ball->setGeometry((int)ball.x,(int)ball.y,10,10);
    if (score==1){
        scoreL++;
        ui->label_scoreR->setText(QString::number(scoreL));
        resetballposition(&ball,field);
    }
    else if (score==2) {
        scoreR++;
        ui->label_scoreL->setText(QString::number(scoreR));
        resetballposition(&ball,field);
    }
}

float MainWindow::mapbartodeg(Square bar, Ball ball){
    float touch_pos = ball.y-bar.pos_y;
    return  (touch_pos) * (180) / (bar.height);
}

void MainWindow::resetballposition(Ball *b, Square field){
    b->x = field.center_x_pos();
    b->y = field.center_y_pos();
    srand (time(NULL));
    b->dir_deg =  (float)((rand()%360)-180);
    b->speed = 1;
}

void MainWindow::resetscore(){
    scoreR = 0;
    scoreL = 0;
    ui->label_scoreL->setText(QString::number(0));
    ui->label_scoreR->setText(QString::number(0));
}

void MainWindow::on_pushButton_connect_clicked()
{
    port1 = ui->lineEdit->text();
    connectSerial(&port_handler_left, port1);

    if (port_handler_left>0){
        ui->pushButton_connect->setText("connected");
    }
}

void MainWindow::animation_pause(){
    if (coundownvalue>0){
        coundownvalue=coundownvalue-1;

        ui->countdown->setText(QString::number(coundownvalue));
        timer1->stop();
    }

    else {
       timer2->stop();
       timer1->start(10);
       ui->countdown->setText("");
    }
}

void MainWindow::on_pushButton_startstop_clicked()
{
        resetscore();
        resetballposition(&ball,field);
        ui->label_ball->setGeometry((int)ball.x,(int)ball.y,30,30);

        checked=0;
        timer1->stop();
        coundownvalue = 3;
        ui->countdown->setText(QString::number(coundownvalue));
        timer2->start(1000);
}
