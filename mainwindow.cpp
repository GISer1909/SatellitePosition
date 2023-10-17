#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    //glutinit
    int argc = 1;
    char *argv[1] = {(char*)"Something"};
    glutInit(&argc, argv);//初始化glut
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);//设置显示模式
    ui->setupUi(this);
    //禁用全屏
    this->setWindowFlags(Qt::WindowCloseButtonHint);
    //禁止放大缩小
    this->setFixedSize(this->width(), this->height());
    //标题：卫星位置计算软件
    this->setWindowTitle("卫星位置计算软件");
    ui->statusbar->showMessage("欢迎使用卫星位置计算软件。本软件可解析GPS，北斗，GLONASS，Galileo卫星系统的星历数据，计算卫星位置。");
    glWidget = new MyOpenGLWidget(ui->Sats);
    glWidget->resize(ui->Sats->width(), ui->Sats->height());
    glWidget->show();
    //rnxPath只读
    ui->rnxPath->setReadOnly(true);
    //设置everyTable的列
    ui->everyTable->setColumnCount(7);
    //设置everyTable的行高为3*default
    ui->everyTable->verticalHeader()->setDefaultSectionSize(3*ui->everyTable->verticalHeader()->defaultSectionSize());
    //设置everyTable的行
    ui->everyTable->setRowCount(0);
    //设置everyTable的表头
    ui->everyTable->setHorizontalHeaderLabels(QStringList()<<"卫星"<<"历元(GPST)"<<"解算坐标"<<"精密星历坐标"<<"偏差"<<"卫星方位角"<<"卫星高度角");
    //取消表前的行号
    ui->everyTable->verticalHeader()->setVisible(false);
    //表格禁止编辑
    ui->everyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  //设置BLH默认值
    ui->LInput->setText("117.14");
    ui->BInput->setText("34.22");
    ui->HInput->setText("10");


    //取消表前的行号
    ui->timeTable->verticalHeader()->setVisible(false);
    //表格禁止编辑
    ui->timeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //设置timeTable的列
    ui->timeTable->setColumnCount(6);
    //设置timeTable的表头
    ui->timeTable->setHorizontalHeaderLabels(QStringList()<<"卫星"<<"PRN"<<"历元(GPST)"<<"解算坐标"<<"卫星方位角"<<"卫星高度角");
    //设置timeTable的行高为3*default
    ui->timeTable->verticalHeader()->setDefaultSectionSize(3*ui->timeTable->verticalHeader()->defaultSectionSize());
    //设置timeTable的行
    ui->timeTable->setRowCount(0);
    //ui->startTime:9:00:00
    ui->startTime->setTime(QTime(9,0,0));
    //ui->endTime:10:00:00
    ui->endTime->setTime(QTime(10,0,0));










}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_openGLWidget_resized()
{

}


void MainWindow::on_openRNX_clicked()
{
    //选择文件,RINEX文件(*.rnx和*.*n)
    QString fileName = QFileDialog::getOpenFileName(this, "选择文件", "", "RINEX文件(*.rnx brdc*.*n brdc*.*g brdc*.*b)");
    if(fileName.isEmpty())
    {
        QMessageBox::warning(this, "警告", "未选择文件！");
        return;
    }
    //打开文件
    std::ifstream fin(fileName.toStdString());
    if(!fin.is_open())
    {
        QMessageBox::warning(this, "警告", "文件打开失败！");
        return;
    }
    ui->rnxPath->setText(fileName);
    //清空sats和sat_GLOs，表格
    sats.clear();
    sat_GLOs.clear();
    ui->everyTable->clearContents();

    //读取文件
    std::string line;
    getline(fin, line);
    double rnx_version;
        //判断rnx版本
        if(line.substr(60, 20) == "RINEX VERSION / TYPE"){
         cout<<line.substr(5, 4)<<endl;
      rnx_version = str_to_double(line.substr(5, 4));
         if(rnx_version>3.04){
                //alert
                QMessageBox::warning(this,"警告","只支持读取RINEX版本为2至3.04,读取失败",QMessageBox::Ok);
                return;
            }
        }else{
            //alert
            QMessageBox::warning(this,"警告","RINEX无效",QMessageBox::Ok);
            return;
        }
    for(int i=0;i<10e3;i++){
        getline(fin, line);
        if (line.find("END OF HEADER") != std::string::npos) {
            break;
        }
    }
    if(rnx_version>=3){

    //读取卫星数据
    while(getline(fin, line)){
        //read_rnx_GPS_BDS_GAL
        if(line.substr(0, 1) == "G" || line.substr(0, 1) == "C" || line.substr(0, 1) == "E"){
        Satellite sat;
        //判断该行数据是否完整,若不完整则退出
        if(line.length()<60){
            //alert
            QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
            break;
        }
        if(line.substr(0, 1) == "G" ){
            sat.type = "GPS";
        }
        else if(line.substr(0, 1) == "C" ){
            sat.type = "BDS";
        }
        else if(line.substr(0, 1) == "E" ){
            sat.type = "GAL";
        }
        sat.prn = stoi(line.substr(1, 2)); // 提取伪随机噪声码
        sat.year = stoi(line.substr(4, 4)); // 提取年份
        sat.month = stoi(line.substr(9, 2)); // 提取月份
        sat.day = stoi(line.substr(12, 2)); // 提取日期
        sat.hour = stoi(line.substr(15, 2)); // 提取小时
        sat.minute = stoi(line.substr(18, 2)); // 提取分钟
        sat.second = str_to_double(line.substr(21, 2)); // 提取秒
        sat.clock_bias = str_to_double(line.substr(23, 19)); // 提取钟差
        sat.clock_drift = str_to_double(line.substr(42, 19)); // 提取钟偏
        sat.clock_drift_rate = str_to_double(line.substr(61, 19)); // 提取钟偏变化率
        std::string line;
        getline(fin, line); // 读取下一行
        //判断该行数据是否完整,若不完整则退出
        if(line.length()<60){
            //alert
            QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
            break;
        }
        sat.iode = str_to_double(line.substr(4, 19)); // 提取数据龄期
        sat.crs = str_to_double(line.substr(23, 19)); // 提取轨道半径正弦调和项
        sat.delta_n = str_to_double(line.substr(42, 19)); // 提取平均角速度改正项
        sat.m0 = str_to_double(line.substr(61, 19)); // 提取参考时刻平近点角
        getline(fin, line); // 读取下一行
        //判断该行数据是否完整,若不完整则退出
        if(line.length()<60){
            //alert
            QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
            break;
        }
        sat.cuc = str_to_double(line.substr(4, 19)); // 提取升交距角余弦调和项
        sat.e = str_to_double(line.substr(23, 19)); // 提取偏心率
        sat.cus = str_to_double(line.substr(42, 19)); // 提取升交距角正弦调和项
        sat.sqrt_a = str_to_double(line.substr(61, 19)); // 提取轨道长半轴平方根
        getline(fin, line); // 读取下一行
        //判断该行数据是否完整,若不完整则退出
        if(line.length()<60){
            //alert
            QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
            break;
        }
        sat.toe = str_to_double(line.substr(4, 19)); // 提取参考时刻
        sat.cic = str_to_double(line.substr(23, 19)); // 提取轨道倾角余弦调和项
        sat.omega0 = str_to_double(line.substr(42, 19)); // 提取参考时刻升交点赤经
        sat.cis = str_to_double(line.substr(61, 19)); // 提取轨道倾角正弦调和项
        getline(fin, line); // 读取下一行
        //判断该行数据是否完整,若不完整则退出
        if(line.length()<60){
            //alert
            QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
            break;
        }
        sat.i0 = str_to_double(line.substr(4, 19)); // 提取参考时刻轨道倾角
        sat.crc = str_to_double(line.substr(23, 19)); // 提取轨道半径余弦调和项
        sat.omega = str_to_double(line.substr(42, 19)); // 提取近地点角距
        sat.omega_dot = str_to_double(line.substr(61, 19)); // 提取升交点赤经变化率
        getline(fin, line); // 读取下一行
        //判断该行数据是否完整,若不完整则退出
        if(line.length()<60){
            //alert
            QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
            break;
        }
        sat.idot = str_to_double(line.substr(4, 19)); // 提取轨道倾角变化率
        sat.codes_l2_channel = str_to_double(line.substr(23, 19)); // 提取L2频道C/A码标识
        sat.gps_week_number = str_to_double(line.substr(42, 19)); // 提取GPS周数
        getline(fin, line); // 读取下一行
        //判断该行数据是否完整,若不完整则退出
        if(line.length()<60){
            //alert
            QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
            break;
        }
        sat.accuracy_index = str_to_double(line.substr(4, 19)); // 提取精度指数
        sat.health_status_bitmask = str_to_double(line.substr(23, 19)); // 提取卫星健康状态掩码
        sat.tgd = str_to_double(line.substr(42, 19)); // 提取电离层延迟
        sat.iodc = str_to_double(line.substr(61, 19)); // 提取星钟数据质量
        getline(fin, line); // 读取下一行
        //判断该行数据是否完整,若不完整则退出
        if(line.length()<30){
            //alert
            QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
            break;
        }
        sat.transmission_time_of_message_sow = str_to_double(line.substr(4, 19)); // 提取信息发射时间
        sats.push_back(sat);

        }
        //read_rnx_GLONASS
        else if(line.substr(0, 1) == "R"){
        Satellite_GLO sat;
        sat.type = "GLO";
        sat.prn = stoi(line.substr(1, 2)); // 提取伪随机噪声码
        sat.year = stoi(line.substr(4, 4)); // 提取年份
        sat.month = stoi(line.substr(9, 2)); // 提取月份
        sat.day = stoi(line.substr(12, 2)); // 提取日期
        sat.hour = stoi(line.substr(15, 2)); // 提取小时
        sat.minute = stoi(line.substr(18, 2)); // 提取分钟
        sat.second = str_to_double(line.substr(21, 2)); // 提取秒
        sat.clock_bias = str_to_double(line.substr(23, 19)); // 提取钟差
        sat.clock_drift = str_to_double(line.substr(42, 19)); // 提取钟偏
        std::string line;
        getline(fin, line); // 读取下一行数据
        sat.x = str_to_double(line.substr(4, 19))*1000; // 提取x
        sat.x_speed = str_to_double(line.substr(23, 19))*1000; // 提取x速度
        sat.x_acceleration = str_to_double(line.substr(42, 19))*1000; // 提取x加速度
        getline(fin, line); // 读取下一行数据
        sat.y = str_to_double(line.substr(4, 19))*1000; // 提取y
        sat.y_speed = str_to_double(line.substr(23, 19))*1000; // 提取y速度
        sat.y_acceleration = str_to_double(line.substr(42, 19))*1000; // 提取y加速度
        getline(fin, line); // 读取下一行数据
        sat.z = str_to_double(line.substr(4, 19))*1000; // 提取z
        sat.z_speed = str_to_double(line.substr(23, 19))*1000; // 提取z速度
        sat.z_acceleration = str_to_double(line.substr(42, 19))*1000; // 提取z加速度
        const geph_t geph =  ConvertToGeph(sat);
        //调用geph2pos
        double rs[3];     // 卫星位置
        double dts[2];    // 卫星钟差
        double var;       // 位置和钟差的方差
        gtime_t time;     // 时间
        //time赋值为toe-18s，转换为GPST
        time = timeadd(geph.toe,-18.0);
        geph2pos(time, &geph, rs, dts, &var);
        sat.x = rs[0];
        sat.y = rs[1];
        sat.z = rs[2];
        sat_GLOs.push_back(sat);

        }
        //不读取SBAS
        else if(line.substr(0, 1) == "S" ){
        for(int i=0;i<3;i++){
            getline(fin, line);
        }
        continue;
        }
        //不读取QZSS和IRNSS
        else if(line.substr(0, 1) == "J" ||line.substr(0, 1) == "I"){
        for(int i=0;i<7;i++){
            getline(fin, line);
        }
        continue;
        }

    }
    }else{
    //如果文件的后缀是n，则是GPS
    if(fileName[fileName.length()-1] == 'n'){
    //GPS
            while (getline(fin, line)) {

                Satellite sat;

                //判断该行数据是否完整,若不完整则退出
                if(line.length()<60){
                    //alert
                    QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
                    break;
                }
                    //n文件
                    sat.type = "GPS";
                    sat.prn = stoi(line.substr(0, 2)); // 提取伪随机噪声码
                    sat.year = stoi("20"+line.substr(3, 2)); // 提取年份
                    sat.month = stoi(line.substr(6, 2)); // 提取月份
                    sat.day = stoi(line.substr(9, 2)); // 提取日期
                    sat.hour = stoi(line.substr(12, 2)); // 提取小时
                    sat.minute = stoi(line.substr(15, 2)); // 提取分钟
                    sat.second = str_to_double(line.substr(18, 4)); // 提取秒
                    sat.clock_bias = str_to_double(line.substr(22, 19)); // 提取钟差
                    sat.clock_drift = str_to_double(line.substr(41, 19)); // 提取钟偏
                    sat.clock_drift_rate = str_to_double(line.substr(60, 19)); // 提取钟偏变化率

                getline(fin, line); // 读取下一行数据
                 qDebug()<<QString::fromStdString(line);
                if(line.length()<60){
                    //alert
                    QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
                    break;
                }

                    sat.iode = str_to_double(line.substr(3, 19)); // 提取数据龄期
                    sat.crs = str_to_double(line.substr(22, 19)); // 提取轨道半径正弦调和项
                    sat.delta_n = str_to_double(line.substr(41, 19)); // 提取平均角速度改正项
                    sat.m0 = str_to_double(line.substr(60, 19)); // 提取参考时刻平近点角



                getline(fin, line); // 读取下一行数据
                 qDebug()<<QString::fromStdString(line);
                if(line.length()<60){
                    //alert
                    QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
                    break;
                }


                    sat.cuc = str_to_double(line.substr(3, 19)); // 提取升交距角余弦调和项
                    sat.e = str_to_double(line.substr(22, 19)); // 提取偏心率
                    sat.cus = str_to_double(line.substr(41, 19)); // 提取升交距角正弦调和项
                    sat.sqrt_a = str_to_double(line.substr(60, 19)); // 提取轨道长半轴平方根



                getline(fin, line); // 读取下一行数据
                 qDebug()<<QString::fromStdString(line);
                if(line.length()<60){
                    //alert
                    QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
                    break;
                }

                    sat.toe = str_to_double(line.substr(3, 19)); // 提取参考时刻
                    sat.cic = str_to_double(line.substr(22, 19)); // 提取轨道倾角余弦调和项
                    sat.omega0 = str_to_double(line.substr(41, 19)); // 提取参考时刻升交点赤经
                    sat.cis = str_to_double(line.substr(60, 19)); // 提取轨道倾角正弦调和项
                getline(fin, line); // 读取下一行数据
                 qDebug()<<QString::fromStdString(line);
                if(line.length()<60){
                    //alert
                    QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
                    break;
                }

                    sat.i0 = str_to_double(line.substr(3, 19)); // 提取参考时刻轨道倾角
                    sat.crc = str_to_double(line.substr(22, 19)); // 提取轨道半径余弦调和项
                    sat.omega = str_to_double(line.substr(41, 19)); // 提取近地点角距
                    sat.omega_dot = str_to_double(line.substr(60, 19)); // 提取升交点赤经变化率




                getline(fin, line); // 读取下一行数据
                 qDebug()<<QString::fromStdString(line);
                if(line.length()<60){
                    //alert
                    QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
                    break;
                }

                    sat.idot = str_to_double(line.substr(3, 19)); // 提取轨道倾角变化率
                    sat.codes_l2_channel = str_to_double(line.substr(22, 19)); // 提取L2频道C/A码标识
                    sat.gps_week_number = str_to_double(line.substr(41, 19)); // 提取GPS周数




                getline(fin, line); // 读取下一行数据
                 qDebug()<<QString::fromStdString(line);
                if(line.length()<60){
                    //alert
                    QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
                    break;
                }

                    sat.accuracy_index = str_to_double(line.substr(3, 19)); // 提取精度指数
                    sat.health_status_bitmask = str_to_double(line.substr(22, 19)); // 提取卫星健康状态掩码
                    sat.tgd = str_to_double(line.substr(41, 19)); // 提取电离层延迟
                    sat.iodc = str_to_double(line.substr(60, 19)); // 提取星钟数据质量




                getline(fin, line); // 读取下一行数据
                 qDebug()<<QString::fromStdString(line);
                if(line.length()<30){
                    //alert
                    QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
                    break;
                }

                    sat.transmission_time_of_message_sow = str_to_double(line.substr(3, 19)); // 提取信息发射时间
                   // sat.fit_interval = str_to_double(line.substr(22, 19)); // 提取拟合区间
                    sats.push_back(sat); // 将结构体对象添加到向量中




            }

    }else if(fileName[fileName.length()-1] == 'g'){
    //GLO
            while (getline(fin, line)) {
                    //如果line为空行，则跳过
                    if(line.length()==0){
                        break;
                    }



                Satellite_GLO sat;

                //判断该行数据是否完整,若不完整则退出
                if(line.length()<60){
                    //alert
                    QMessageBox::warning(this,"警告","文件数据不完整",QMessageBox::Ok);
                    break;
                }

                //g文件
                sat.type = "GLO";
                sat.prn = stoi(line.substr(0, 2)); // 提取伪随机噪声码
                sat.year = stoi("20"+line.substr(3, 4)); // 提取年份
                sat.month = stoi(line.substr(6, 2)); // 提取月份
                sat.day = stoi(line.substr(9, 2)); // 提取日期
                sat.hour = stoi(line.substr(12, 2)); // 提取小时
                sat.minute = stoi(line.substr(15, 2)); // 提取分钟
                sat.second = str_to_double(line.substr(18, 2)); // 提取秒
                sat.clock_bias = str_to_double(line.substr(22, 19)); // 提取钟差
                sat.clock_drift = str_to_double(line.substr(41, 19)); // 提取钟偏
                std::string line;
                getline(fin, line); // 读取下一行数据
                sat.x = str_to_double(line.substr(3, 19))*1000; // 提取x
                sat.x_speed = str_to_double(line.substr(22, 19))*1000; // 提取x速度
                sat.x_acceleration = str_to_double(line.substr(41, 19))*1000; // 提取x加速度
                getline(fin, line); // 读取下一行数据
                sat.y = str_to_double(line.substr(3, 19))*1000; // 提取y
                sat.y_speed = str_to_double(line.substr(22, 19))*1000; // 提取y速度
                sat.y_acceleration = str_to_double(line.substr(41, 19))*1000; // 提取y加速度
                getline(fin, line); // 读取下一行数据
                sat.z = str_to_double(line.substr(3, 19))*1000; // 提取z
                sat.z_speed = str_to_double(line.substr(22, 19))*1000; // 提取z速度
                sat.z_acceleration = str_to_double(line.substr(41, 19))*1000; // 提取z加速度
                const geph_t geph =  ConvertToGeph(sat);
                //调用geph2pos
                double rs[3];     // 卫星位置
                double dts[2];    // 卫星钟差
                double var;       // 位置和钟差的方差
                gtime_t time;     // 时间
                //time赋值为toe-18s，转换为GPST
                time = timeadd(geph.toe,-18.0);
                geph2pos(time, &geph, rs, dts, &var);
                sat.x = rs[0];
                sat.y = rs[1];
                sat.z = rs[2];
                sat_GLOs.push_back(sat);
                //打印sat的时间
                std::cout<<"sat.year:"<<sat.year<<std::endl;
                std::cout<<"sat.month:"<<sat.month<<std::endl;
                std::cout<<"sat.day:"<<sat.day<<std::endl;
                std::cout<<"sat.hour:"<<sat.hour<<std::endl;
                std::cout<<"sat.minute:"<<sat.minute<<std::endl;
                std::cout<<"sat.second:"<<sat.second<<std::endl;

            }

    }
    }

    //渲染到表格
    //GPS,BDS,GAL
    ui->everyTable->clearContents();
    //打印sat.size()和sat_GLOs.size()
    std::cout<<"sat.size():"<<sats.size()<<std::endl;
    //设置行数
    ui->everyTable->setRowCount(sats.size()+sat_GLOs.size());
    for(int i=0;i<sats.size();i++){
        //设置卫星：type+prn，历元:YYYY-MM-DD HH:mm:ss,只保留整数
        ui->everyTable->setItem(i,0,new QTableWidgetItem(QString::fromStdString(sats[i].type+" "+std::to_string(sats[i].prn))));
        ui->everyTable->setItem(i,1,new QTableWidgetItem(QString::fromStdString(std::to_string(sats[i].year)+"-"+std::to_string(sats[i].month)+"-"+std::to_string(sats[i].day)+" "+std::to_string(sats[i].hour)+":"+std::to_string(sats[i].minute)+":"+std::to_string(int(sats[i].second)))));
    }
    std::cout<<"sat_GLOs.size():"<<sat_GLOs.size()<<std::endl;
    //GLO,在GPS,BDS,GAL后面
    for(int i=0;i<sat_GLOs.size();i++){
        //设置卫星：type+prn，历元:YYYY-MM-DD HH:mm:ss,只保留整数
        ui->everyTable->setItem(i+sats.size(),0,new QTableWidgetItem(QString::fromStdString(sat_GLOs[i].type+" "+std::to_string(sat_GLOs[i].prn))));
        ui->everyTable->setItem(i+sats.size(),1,new QTableWidgetItem(QString::fromStdString(std::to_string(sat_GLOs[i].year)+"-"+std::to_string(sat_GLOs[i].month)+"-"+std::to_string(sat_GLOs[i].day)+" "+std::to_string(sat_GLOs[i].hour)+":"+std::to_string(sat_GLOs[i].minute)+":"+std::to_string(int(sat_GLOs[i].second)))));
    }
    //表格自适应
    ui->everyTable->resizeColumnsToContents();
    //第一列宽+10
    ui->everyTable->setColumnWidth(0,ui->everyTable->columnWidth(0)+10);
    if(sats.size()!=0)
    //ui->openglDate改成sats[0]的date
    ui->openglDate->setDate(QDate(sats[0].year,sats[0].month,sats[0].day));
    else if(sat_GLOs.size()!=0)
    //ui->openglDate改成sat_GLOs[0]的date
    ui->openglDate->setDate(QDate(sat_GLOs[0].year,sat_GLOs[0].month,sat_GLOs[0].day));
    //禁止编辑
    ui->openglDate->setReadOnly(true);


    //提示：读取卫星数据完成
    QMessageBox::information(this,"提示","读取卫星数据完成",QMessageBox::Ok);
    //定义一个字典，包括卫星类型和该类型卫星的prn
    //遍历sats
    for(int i=0;i<sats.size();i++){
    //如果type_prns中没有该类型，则添加
    if(type_prns.find(QString::fromStdString(sats[i].type))==type_prns.end()){
            std::vector<int> temp;
            temp.push_back(sats[i].prn);
            type_prns[QString::fromStdString(sats[i].type)] = temp;
    }else{
            //如果type_prns中有该类型，则添加prn
            type_prns[QString::fromStdString(sats[i].type)].push_back(sats[i].prn);
    }
    }
    //遍历sat_GLOs
    for(int i=0;i<sat_GLOs.size();i++){
    //如果type_prns中没有该类型，则添加
    if(type_prns.find(QString::fromStdString(sat_GLOs[i].type))==type_prns.end()){
            std::vector<int> temp;
            temp.push_back(sat_GLOs[i].prn);
            type_prns[QString::fromStdString(sat_GLOs[i].type)] = temp;
    }else{
            //如果type_prns中有该类型，则添加prn
            type_prns[QString::fromStdString(sat_GLOs[i].type)].push_back(sat_GLOs[i].prn);
    }
    }

    //去重
    for(auto it=type_prns.begin();it!=type_prns.end();it++){
    //对it->second去重
    std::sort(it->second.begin(),it->second.end());
    it->second.erase(std::unique(it->second.begin(),it->second.end()),it->second.end());
    }
    //comboBox:ui->satType设置为type_prns的key
    ui->satType->clear();
    for(auto it=type_prns.begin();it!=type_prns.end();it++){
        ui->satType->addItem(it->first);
    }
  //选择satType后更新satPrn

}


void MainWindow::on_calculateBtn_clicked()
{
  //判断sats和sat_GLOs是否为空
  if(sats.empty()&&sat_GLOs.empty()){
      QMessageBox::warning(this,"警告","请先读取卫星数据",QMessageBox::Ok);
       //extern gtime_t gpst2utc(gtime_t t);
      gtime_t time = timeget();
      std::cout<<"time:"<<time.time<<std::endl;
      time = utc2gpst(time);
      //打印结果
      std::cout<<"time:"<<time.time<<std::endl;
      //gpst2bdt
      time = gpst2bdt(time);
      //打印结果
      std::cout<<"time:"<<time.time<<std::endl;
      //time2epoch()
      double* epoch = new double[6];
      time2epoch(time,epoch);
      //打印结果，用逗号隔开
      std::cout<<"epoch:"<<epoch[0]<<","<<epoch[1]<<","<<epoch[2]<<","<<epoch[3]<<","<<epoch[4]<<","<<epoch[5]<<std::endl;

      return;
  }
  //遍历每一个sats
  for(int i=0;i<sats.size();i++){
      position rs;
      rs = calculate(sats[i],timeToTimestamp(sats[i].hour,sats[i].minute,sats[i].second));
      //设置解算坐标
      sats[i].calculate_x = rs.x;
      sats[i].calculate_y = rs.y;
      sats[i].calculate_z = rs.z;
      //获取BLH
      double b = ui->BInput->text().toDouble();
      double l = ui->LInput->text().toDouble();
      double h = ui->HInput->text().toDouble();
      //将BLH转换为XYZ，extern void pos2ecef(const double *pos, double *r)
      double* ecef = new double[3];
      double* pos = new double[3];
      pos[0] = b*M_PI/180;
      pos[1] = l*M_PI/180;
      pos[2] = h;
      pos2ecef(pos,ecef);
      //GTRF转换为WGS-84
      if(sats[i].type == "GAL"){
        rs.x += 0.02;
        rs.y -= 0.01;
        rs.z += 0.01;
      }
     double* enu = new double[3];
     double* r = new double[3];
     r[0] = rs.x - ecef[0];
     r[1] = rs.y - ecef[1];
     r[2] = rs.z - ecef[2];
     ecef2enu(pos,r,enu);
     //计算方位角,方位角A范围在0~360°；若A＜0，A=A+2pi；若A＞2pi，A=A-2pi
     double azimuth = atan2(enu[0],enu[1])*180/M_PI;
     if(azimuth<0){
         azimuth = azimuth + 360;
     }else if(azimuth>360){
         azimuth = azimuth - 360;
     }

     //计算高度角
     double elevation = atan2(enu[2],sqrt(enu[0]*enu[0]+enu[1]*enu[1]))*180/M_PI;
     //设置方位角
     sats[i].azimuth = azimuth;
     //设置高度角
     sats[i].elevation = elevation;
  }
  //遍历每一个sat_GLOs
  for(int i=0;i<sat_GLOs.size();i++){

      //设置解算坐标
     const geph_t geph =  ConvertToGeph(sat_GLOs[i]);
     double rs[3];     // 卫星位置
     double dts[2];    // 卫星钟差
     double var;       // 位置和钟差的方差
     gtime_t _time;     // 时间
     //time赋值为GPST时间
     _time = timeadd(geph.toe,0.0);
     geph2pos(_time, &geph, rs, dts, &var);
     position result;
     result.x = rs[0];
     result.y = rs[1];
     result.z = rs[2];

      sat_GLOs[i].calculate_x = result.x;
      sat_GLOs[i].calculate_y = result.y;
      sat_GLOs[i].calculate_z = result.z;

      //获取BLH
      double b = ui->BInput->text().toDouble();
      double l = ui->LInput->text().toDouble();
      double h = ui->HInput->text().toDouble();
      //将BLH转换为XYZ，extern void pos2ecef(const double *pos, double *r)
      double* ecef = new double[3];
      double* pos = new double[3];
      pos[0] = b*M_PI/180;
      pos[1] = l*M_PI/180;
      pos[2] = h;
      pos2ecef(pos,ecef);
   //将XYZ转换为ENU
      double* enu = new double[3];
      double* r = new double[3];
     //PZ-90转换为WGS-84
      if(sat_GLOs[i].type == "GLO"){
         Eigen::Matrix <double, 3, 3> b;
         b << 1, 1.728e-6, -0.017e-6,
             1.728e-6, 1, 0.076e-6,
             0.0178e-6, -0.076e-6, 1;
         Eigen::Matrix <double, 3, 1> c;
         c << sat_GLOs[i].calculate_x, sat_GLOs[i].calculate_y, sat_GLOs[i].calculate_z;
         Eigen::Matrix <double, 3, 1> a;
         a << -0.47, -0.51, -1.56;
         Eigen::Matrix <double, 3, 1> Rk;
         Rk = a + (1+22e-9) * b * c;
         sat_GLOs[i].calculate_x = Rk(0);
         sat_GLOs[i].calculate_y = Rk(1);
         sat_GLOs[i].calculate_z = Rk(2);
      }
      r[0] = sat_GLOs[i].calculate_x - ecef[0];
      r[1] = sat_GLOs[i].calculate_y - ecef[1];
      r[2] = sat_GLOs[i].calculate_z - ecef[2];
      ecef2enu(pos,r,enu);
      //计算方位角,方位角A范围在0~360°；若A＜0，A=A+2pi；若A＞2pi，A=A-2pi
      double azimuth = atan2(enu[0],enu[1])*180/M_PI;
      if(azimuth<0){
         azimuth = azimuth + 360;
      }else if(azimuth>360){
         azimuth = azimuth - 360;
      }
      //计算高度角
      double elevation = atan2(enu[2],sqrt(enu[0]*enu[0]+enu[1]*enu[1]))*180/M_PI;
      //设置方位角
      sat_GLOs[i].azimuth = azimuth;
      //设置高度角
      sat_GLOs[i].elevation = elevation;

      //设置解算坐标
      sat_GLOs[i].calculate_x = result.x;
      sat_GLOs[i].calculate_y = result.y;
      sat_GLOs[i].calculate_z = result.z;

}

  //渲染到表格
  //GPS,BDS,GAL
  for(int i=0;i<sats.size();i++){
      //设置解算坐标,xyz的逗号换成换行，占三行
      ui->everyTable->setItem(i,2,new QTableWidgetItem(QString::fromStdString(std::to_string(sats[i].calculate_x)+"\n"+std::to_string(sats[i].calculate_y)+"\n"+std::to_string(sats[i].calculate_z))));
      //设置方位角
      ui->everyTable->setItem(i,5,new QTableWidgetItem(QString::fromStdString(std::to_string(sats[i].azimuth))));
      //设置高度角
      ui->everyTable->setItem(i,6,new QTableWidgetItem(QString::fromStdString(std::to_string(sats[i].elevation))));
      //如果precise_x不为0,则设置precise_x
      if(sats[i].precise_x !=0){
         //设置解算坐标,xyz的逗号换成换行，占三行
         ui->everyTable->setItem(i,3,new QTableWidgetItem(QString::fromStdString(std::to_string(sats[i].precise_x)+"\n"+std::to_string(sats[i].precise_y)+"\n"+std::to_string(sats[i].precise_z))));
         //设置偏差,xyz的逗号换成换行，占三行
         ui->everyTable->setItem(i,4,new QTableWidgetItem(QString::fromStdString(std::to_string(sats[i].calculate_x-sats[i].precise_x)+"\n"+std::to_string(sats[i].calculate_y-sats[i].precise_y)+"\n"+std::to_string(sats[i].calculate_z-sats[i].precise_z))));

      }




  }
  //GLO,在GPS,BDS,GAL后面
  for(int i=0;i<sat_GLOs.size();i++){
      //设置解算坐标,xyz的逗号换成换行，占三行
      ui->everyTable->setItem(i+sats.size(),2,new QTableWidgetItem(QString::fromStdString(std::to_string(sat_GLOs[i].calculate_x)+"\n"+std::to_string(sat_GLOs[i].calculate_y)+"\n"+std::to_string(sat_GLOs[i].calculate_z))));
      //设置方位角
      ui->everyTable->setItem(i+sats.size(),5,new QTableWidgetItem(QString::fromStdString(std::to_string(sat_GLOs[i].azimuth))));
      //设置高度角
      ui->everyTable->setItem(i+sats.size(),6,new QTableWidgetItem(QString::fromStdString(std::to_string(sat_GLOs[i].elevation))));
      //如果precise_x不为0,则设置precise_x
      if(sat_GLOs[i].precise_x!=0){

         ui->everyTable->setItem(i+sats.size(),3,new QTableWidgetItem(QString::fromStdString(std::to_string(sat_GLOs[i].precise_x)+"\n"+std::to_string(sat_GLOs[i].precise_y)+"\n"+std::to_string(sat_GLOs[i].precise_z))));
         //设置偏差,xyz的逗号换成换行，占三行
         ui->everyTable->setItem(i+sats.size(),4,new QTableWidgetItem(QString::fromStdString(std::to_string(sat_GLOs[i].calculate_x-sat_GLOs[i].precise_x)+"\n"+std::to_string(sat_GLOs[i].calculate_y-sat_GLOs[i].precise_y)+"\n"+std::to_string(sat_GLOs[i].calculate_z-sat_GLOs[i].precise_z))));

      }

  }
  //表格自适应
  ui->everyTable->resizeColumnsToContents();
  //第一列宽+10
  ui->everyTable->setColumnWidth(0,ui->everyTable->columnWidth(0)+10);
  //第3列宽+10
  ui->everyTable->setColumnWidth(2,ui->everyTable->columnWidth(2)+10);
  if(sats.size()!=0)
  //ui->openglTime设置成sat[0]的time
  ui->openglTime->setTime(QTime(sats[0].hour,sats[0].minute,sats[0].second));
  else if(sat_GLOs.size()!=0)
  //ui->openglTime设置成sat_GLOs[0]的time
  ui->openglTime->setTime(QTime(sat_GLOs[0].hour,sat_GLOs[0].minute,sat_GLOs[0].second));
  //提示：计算完成
  QMessageBox::information(this,"提示","计算完成",QMessageBox::Ok);



}


void MainWindow::on_openglBtn_clicked()
{
  //判断tabWidget当前页是否是第一页
  {
      //判断everytable是否为空
      if(ui->everyTable->rowCount()==0){
          QMessageBox::warning(this,"警告","请先读取卫星数据",QMessageBox::Ok);
          return;
      }
         vector<Satellite> all_sats;
 //GPS
      {
          std::vector<Satellite> gps;
          //筛选出GPS
          for(int i=0;i<sats.size();i++){
                    if(sats[i].type == "GPS"){
            gps.push_back(sats[i]);
                    }
          }
          //求出gps2h中的prn的种类
          std::vector<int> prns;
          for(int i=0;i<gps.size();i++){
                    if(std::find(prns.begin(),prns.end(),gps[i].prn)==prns.end()){
            prns.push_back(gps[i].prn);
                    }
          }
          //定义一个字典，将prn号相同的卫星放在一起
          std::map<int,std::vector<Satellite>> gps2h_map;
          for(int i=0;i<prns.size();i++){
                    std::vector<Satellite> temp;
                    for(int j=0;j<gps.size();j++){
            if(gps[j].prn==prns[i]){
                temp.push_back(gps[j]);
            }
                    }
                    gps2h_map[prns[i]] = temp;
          }
          //获取时间
          int hour = ui->openglTime->time().hour();
          int minute = ui->openglTime->time().minute();
          double second = ui->openglTime->time().second();
          double target_time = timeToTimestamp(hour,minute,second);
          //间接平差函数
          // t0是初始时间(从0时为起点的时间戳)，delt_t是时间间隔(s)，satellites卫星数组，n是切比雪夫多项式阶数,slice取时间间隔的秒数(s),target是预测时间的时间戳
          // position GPS_Adjust(double t0, double delt_t, vector<Satellite> satellites, int n, double slice,double target)
          double t0;//起始时间
          if(target_time<timeToTimestamp(1,0,0.0)){
                    t0 = 0;
          }else{
                    t0 = target_time - timeToTimestamp(1,0,0.0);
          }
          double delt_t = 60*60*2;//时间间隔
          int n = 10;//阶数
          double slice = 60;//取时间间隔的秒数(s)
          double target = target_time;//预测时间的时间戳
          //遍历gps2h_map，调用GPS_Adjust函数
          std::map<int,position> result_sats;
          for(auto it=gps2h_map.begin();it!=gps2h_map.end();it++){
                    position rs;
                    rs = GPS_Adjust(t0,delt_t,it->second,n,slice,target);
                    result_sats[it->first] = rs;
          }
          //坐标转换及导入
          for(auto it=result_sats.begin();it!=result_sats.end();it++){
                    std::cout<<it->first<<":"<<it->second.x<<","<<it->second.y<<","<<it->second.z<<std::endl;
                    //获取BLH
                    double b = ui->BInput->text().toDouble();
                    double l = ui->LInput->text().toDouble();
                    double h = ui->HInput->text().toDouble();
                    //将BLH转换为XYZ，extern void pos2ecef(const double *pos, double *r)
                    double* ecef = new double[3];
                    double* pos = new double[3];
                    pos[0] = b*M_PI/180;
                    pos[1] = l*M_PI/180;
                    pos[2] = h;
                    pos2ecef(pos,ecef);

                    double* enu = new double[3];
                    double* r = new double[3];
                    r[0] = it->second.x - ecef[0];
                    r[1] = it->second.y - ecef[1];
                    r[2] = it->second.z - ecef[2];

                    ecef2enu(pos,r,enu);
                    //计算方位角,方位角A范围在0~360°；若A＜0，A=A+2pi；若A＞2pi，A=A-2pi
                    double azimuth = atan2(enu[0],enu[1])*180/M_PI;
                    if(azimuth<0){
            azimuth = azimuth + 360;
                    }else if(azimuth>360){
            azimuth = azimuth - 360;
                    }
                    //计算高度角
                    double elevation = atan2(enu[2],sqrt(enu[0]*enu[0]+enu[1]*enu[1]))*180/M_PI;
                    Satellite s;
                    s.type = "GPS";
                    s.prn = it->first;
                    s.calculate_x = it->second.x;
                    s.calculate_y = it->second.y;
                    s.calculate_z = it->second.z;
                    s.azimuth = azimuth;
                    s.elevation = elevation;
                    all_sats.push_back(s);

          }

      }

  //GAL
      {
          std::vector<Satellite> gal;
          //筛选出GAL
          for(int i=0;i<sats.size();i++){
                    if(sats[i].type == "GAL"){
            gal.push_back(sats[i]);
                    }
          }
          //求出gal2h中的prn的种类
          std::vector<int> prns2;
          for(int i=0;i<gal.size();i++){
                    if(std::find(prns2.begin(),prns2.end(),gal[i].prn)==prns2.end()){
            prns2.push_back(gal[i].prn);
                    }
          }
          //定义一个字典，将prn号相同的卫星放在一起
          std::map<int,std::vector<Satellite>> gal2h_map;
          for(int i=0;i<prns2.size();i++){
                    std::vector<Satellite> temp;
                    for(int j=0;j<gal.size();j++){
            if(gal[j].prn==prns2[i]){
                temp.push_back(gal[j]);
            }
                    }
                    gal2h_map[prns2[i]] = temp;
          }
          //获取时间
          int hour2 = ui->openglTime->time().hour();
          int minute2 = ui->openglTime->time().minute();
          double second2 = ui->openglTime->time().second();
          double target_time2 = timeToTimestamp(hour2,minute2,second2);
          //间接平差函数
          // t0是初始时间(从0时为起点的时间戳)，delt_t是时间间隔(s)，satellites卫星数组，n是切比雪夫多项式阶数,slice取时间间隔的秒数(s),target是预测时间的时间戳
          // position GPS_Adjust(double t0, double delt_t, vector<Satellite> satellites, int n, double slice,double target)
          double t02;//起始时间
          if(target_time2<timeToTimestamp(1,0,0.0)){
                    t02 = 0;
          }else{
                    t02 = target_time2 - timeToTimestamp(1,0,0.0);
          }
          double delt_t2 = 60*60*2;//时间间隔
          int n2 = 10;//阶数
          double slice2 = 60;//取时间间隔的秒数(s)
          double target2 = target_time2;//预测时间的时间戳
          //遍历gal2h_map，调用GPS_Adjust函数
          std::map<int,position> result_sats2;
          for(auto it=gal2h_map.begin();it!=gal2h_map.end();it++){
                    position rs;
                    rs = GPS_Adjust(t02,delt_t2,it->second,n2,slice2,target2);
                    result_sats2[it->first] = rs;
          }
      //坐标转换及导入
          for(auto it=result_sats2.begin();it!=result_sats2.end();it++){
                    std::cout<<it->first<<":"<<it->second.x<<","<<it->second.y<<","<<it->second.z<<std::endl;
                    //获取BLH
                    double b = ui->BInput->text().toDouble();
                    double l = ui->LInput->text().toDouble();
                    double h = ui->HInput->text().toDouble();
                    //将BLH转换为XYZ，extern void pos2ecef(const double *pos, double *r)
                    double* ecef = new double[3];
                    double* pos = new double[3];
                    pos[0] = b*M_PI/180;
                    pos[1] = l*M_PI/180;
                    pos[2] = h;
                    pos2ecef(pos,ecef);
                    //将second.x,second.y,second.z转换为wgs84的XYZ
                    it->second.x +=0.02;
                    it->second.y -=0.01;
                    it->second.z +=0.01;
                    double* enu = new double[3];
                    double* r = new double[3];
                    r[0] = it->second.x - ecef[0];
                    r[1] = it->second.y - ecef[1];
                    r[2] = it->second.z - ecef[2];
                    //计算方位角,方位角A范围在0~360°；若A＜0，A=A+2pi；若A＞2pi，A=A-2pi
                    ecef2enu(pos,r,enu);
                    double azimuth = atan2(enu[0],enu[1])*180/M_PI;
                    if(azimuth<0){
            azimuth = azimuth + 360;
                    }else if(azimuth>360){
            azimuth = azimuth - 360;
                    }
                    //计算高度角
                    double elevation = atan2(enu[2],sqrt(enu[0]*enu[0]+enu[1]*enu[1]))*180/M_PI;
                    Satellite s;
                    s.type = "GAL";
                    s.prn = it->first;
                    s.calculate_x = it->second.x;
                    s.calculate_y = it->second.y;
                    s.calculate_z = it->second.z;
                    s.azimuth = azimuth;
                    s.elevation = elevation;
                    all_sats.push_back(s);
          }

      }

//BDS
      {
          std::vector<Satellite> bds;
          //筛选出BDS
          for(int i=0;i<sats.size();i++){
                    if(sats[i].type == "BDS"){
            bds.push_back(sats[i]);
                    }
          }
          //求出bds2h中的prn的种类
          std::vector<int> prns3;
          for(int i=0;i<bds.size();i++){
                    if(std::find(prns3.begin(),prns3.end(),bds[i].prn)==prns3.end()){
            prns3.push_back(bds[i].prn);
                    }
          }
          //定义一个字典，将prn号相同的卫星放在一起
          std::map<int,std::vector<Satellite>> bds2h_map;
          for(int i=0;i<prns3.size();i++){
                    std::vector<Satellite> temp;
                    for(int j=0;j<bds.size();j++){
            if(bds[j].prn==prns3[i]){
                temp.push_back(bds[j]);
            }
                    }
                    bds2h_map[prns3[i]] = temp;
          }
          //获取时间
          int hour3 = ui->openglTime->time().hour();
          int minute3 = ui->openglTime->time().minute();
          double second3 = ui->openglTime->time().second();
          double target_time3 = timeToTimestamp(hour3,minute3,second3);
          //间接平差函数
          // t0是初始时间(从0时为起点的时间戳)，delt_t是时间间隔(s)，satellites卫星数组，n是切比雪夫多项式阶数,slice取时间间隔的秒数(s),target是预测时间的时间戳
          // position GPS_Adjust(double t0, double delt_t, vector<Satellite> satellites, int n, double slice,double target)
          double t03;//起始时间
          if(target_time3<timeToTimestamp(1,0,0.0)){
                    t03 = 0;
          }else{
                    t03 = target_time3 - timeToTimestamp(1,0,0.0);
          }
          double delt_t3 = 60*60*2;//时间间隔
          int n3 = 10;//阶数
          double slice3 = 60;//取时间间隔的秒数(s)
          double target3 = target_time3;//预测时间的时间戳
          //遍历bds2h_map，调用GPS_Adjust函数
          std::map<int,position> result_sats3;
          for(auto it=bds2h_map.begin();it!=bds2h_map.end();it++){
                    position rs;
                    rs = GPS_Adjust(t03,delt_t3,it->second,n3,slice3,target3);
                    result_sats3[it->first] = rs;
          }
      //坐标转换及导入
          for(auto it=result_sats3.begin();it!=result_sats3.end();it++){
                    std::cout<<it->first<<":"<<it->second.x<<","<<it->second.y<<","<<it->second.z<<std::endl;
                    //获取BLH
                    double b = ui->BInput->text().toDouble();
                    double l = ui->LInput->text().toDouble();
                    double h = ui->HInput->text().toDouble();
                    //将BLH转换为XYZ，extern void pos2ecef(const double *pos, double *r)
                    double* ecef = new double[3];
                    double* pos = new double[3];
                    pos[0] = b*M_PI/180;
                    pos[1] = l*M_PI/180;
                    pos[2] = h;
                    pos2ecef(pos,ecef);
                    //将second.x,second.y,second.z转换为wgs84的XYZ
                    double* enu = new double[3];
                    double* r = new double[3];
                    r[0] = it->second.x - ecef[0];
                    r[1] = it->second.y - ecef[1];
                    r[2] = it->second.z - ecef[2];
                    //计算方位角,方位角A范围在0~360°；若A＜0，A=A+2pi；若A＞2pi，A=A-2pi
                    ecef2enu(pos,r,enu);
                    double azimuth = atan2(enu[0],enu[1])*180/M_PI;
                    if(azimuth<0){
            azimuth = azimuth + 360;
                    }else if(azimuth>360){
            azimuth = azimuth - 360;
                    }
                    //计算高度角
                    double elevation = atan2(enu[2],sqrt(enu[0]*enu[0]+enu[1]*enu[1]))*180/M_PI;
                    Satellite s;
                    s.type = "BDS";
                    s.prn = it->first;
                    s.calculate_x = it->second.x;
                    s.calculate_y = it->second.y;
                    s.calculate_z = it->second.z;
                    s.azimuth = azimuth;
                    s.elevation = elevation;
                    all_sats.push_back(s);
          }


      }
      //GLO
      {

          //求出sat_GLOsh中的prn的种类
          std::vector<int> prns3;
          for(int i=0;i<sat_GLOs.size();i++){
                    if(std::find(prns3.begin(),prns3.end(),sat_GLOs[i].prn)==prns3.end()){
            prns3.push_back(sat_GLOs[i].prn);
                    }
          }
          //定义一个字典，将prn号相同的卫星放在一起
          std::map<int,std::vector<Satellite_GLO>> sat_GLOs2h_map;
          for(int i=0;i<prns3.size();i++){
                    std::vector<Satellite_GLO> temp;
                    for(int j=0;j<sat_GLOs.size();j++){
            if(sat_GLOs[j].prn==prns3[i]){
                temp.push_back(sat_GLOs[j]);
            }
                    }
                    sat_GLOs2h_map[prns3[i]] = temp;
          }
          //获取时间
          int hour3 = ui->openglTime->time().hour();
          int minute3 = ui->openglTime->time().minute();
          double second3 = ui->openglTime->time().second();
          double target_time3 = timeToTimestamp(hour3,minute3,second3);
          //间接平差函数
          // t0是初始时间(从0时为起点的时间戳)，delt_t是时间间隔(s)，satellites卫星数组，n是切比雪夫多项式阶数,slice取时间间隔的秒数(s),target是预测时间的时间戳
          // position GPS_Adjust(double t0, double delt_t, vector<Satellite> satellites, int n, double slice,double target)
          double t03;//起始时间
          if(target_time3<timeToTimestamp(1,0,0.0)){
                    t03 = 0;
          }else{
                    t03 = target_time3 - timeToTimestamp(1,0,0.0);
          }
          double delt_t3 = 60*60*2;//时间间隔
          int n3 = 10;//阶数
          double slice3 = 60;//取时间间隔的秒数(s)
          double target3 = target_time3;//预测时间的时间戳
          //遍历bds2h_map，调用GLO_Adjust函数
          std::map<int,position> result_sats3;
          for(auto it=sat_GLOs2h_map.begin();it!=sat_GLOs2h_map.end();it++){
                    position rs;
                    rs = GLO_Adjust(t03,delt_t3,it->second,n3,slice3,target3);
                    result_sats3[it->first] = rs;
          }
          //坐标转换及导入
          for(auto it=result_sats3.begin();it!=result_sats3.end();it++){
                    std::cout<<it->first<<":"<<it->second.x<<","<<it->second.y<<","<<it->second.z<<std::endl;
                    double temp_x = it->second.x;
                    double temp_y = it->second.y;
                    double temp_z = it->second.z;

                    //获取BLH
                    double b = ui->BInput->text().toDouble();
                    double l = ui->LInput->text().toDouble();
                    double h = ui->HInput->text().toDouble();
                    //将BLH转换为XYZ，extern void pos2ecef(const double *pos, double *r)
                    double* ecef = new double[3];
                    double* pos = new double[3];
                    pos[0] = b*M_PI/180;
                    pos[1] = l*M_PI/180;
                    pos[2] = h;
                    pos2ecef(pos,ecef);
                    //将calculate_x,y,z转换为wgs84的XYZ
                    Eigen::Matrix <double, 3, 3> B;
                    B << 1, 1.728e-6, -0.017e-6,
                        1.728e-6, 1, 0.076e-6,
                        0.0178e-6, -0.076e-6, 1;
                    Eigen::Matrix <double, 3, 1> c;
                    c << it->second.x,it->second.y,it->second.z;
                    Eigen::Matrix <double, 3, 1> a;
                    a << -0.47, -0.51, -1.56;
                    Eigen::Matrix <double, 3, 1> Rk;
                    Rk = a + (1+22e-9) * B * c;
                    it->second.x = Rk(0,0);
                    it->second.y = Rk(1,0);
                    it->second.z = Rk(2,0);
                    double* enu = new double[3];
                    double* r = new double[3];
                    r[0] = it->second.x - ecef[0];
                    r[1] = it->second.y - ecef[1];
                    r[2] = it->second.z - ecef[2];
                    //计算方位角,方位角A范围在0~360°；若A＜0，A=A+2pi；若A＞2pi，A=A-2pi
                    ecef2enu(pos,r,enu);
                    double azimuth = atan2(enu[0],enu[1])*180/M_PI;
                    if(azimuth<0){
            azimuth = azimuth + 360;
                    }else if(azimuth>360){
            azimuth = azimuth - 360;
                    }
                    //计算高度角
                    double elevation = atan2(enu[2],sqrt(enu[0]*enu[0]+enu[1]*enu[1]))*180/M_PI;
                    Satellite s;
                    s.type = "GLO";
                    s.prn = it->first;
                    s.calculate_x = temp_x;
                    s.calculate_y = temp_y;
                    s.calculate_z = temp_z;

                    s.azimuth = azimuth;
                    s.elevation = elevation;

                    all_sats.push_back(s);
          }
          glWidget->updateOpenGLContent(all_sats);
          sats_showed = all_sats;


      }
}
}

void MainWindow::on_openSP3_clicked()
{
    //选择文件
    QString fileName = QFileDialog::getOpenFileName(this,tr("选择文件"),"",tr("sp3文件(*.sp3)"));
    //判断文件是否为空
    if(fileName.isEmpty()){
        QMessageBox::warning(this,"警告","未选择sp3文件",QMessageBox::Ok);
        return;
    }
    //打开文件
    QFile file(fileName);
    //判断文件是否打开成功
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        QMessageBox::warning(this,"警告","打开文件失败",QMessageBox::Ok);
        return;
    }
    ui->sp3Path->setText(fileName);
    //读取文件
   std::ifstream fin(fileName.toStdString());
   std::string line;
   string pre_line;
   //跳过若干行直到该行第一个符号是*
   while(getline(fin,line)){
       if(line[0]=='*'){
          pre_line = line;
           break;
       }
   }
   //读取数据
   //定义一个vector
   std::vector<precise_Satellite> precise_sats;
   //读取数据,while循环读取第一个同一时间的代码块
   while(getline(fin,line)){
       //打印line
       //line[0]=='*'，说明开始同一时间的代码块
       if(pre_line[0]=='*'){
           //*  2023  9 24 13 35  0.00000000
           int year = std::stoi(pre_line.substr(3,4));

           int month = std::stoi(pre_line.substr(8,2));
           int day = std::stoi(pre_line.substr(11,2));
           int hour = std::stoi(pre_line.substr(14,2));
           int minute = std::stoi(pre_line.substr(17,2));
           double second = str_to_double(pre_line.substr(20,20));

           //读取数据
           while(getline(fin,line)){
                    if(line[0]=='*'){
                      pre_line = line;
                        break;
                    }
                    precise_Satellite ps;
                    ps.year = year;
                    ps.month = month;
                    ps.day = day;
                    ps.hour = hour;
                    ps.minute = minute;
                    ps.second = second;
                    if(line.substr(0,2) == "PG"){
                        ps.type = "GPS";
                    }else if(line.substr(0,2) == "PE"){
                         ps.type = "GAL";
                    }else if(line.substr(0,2) == "PR"){
                         ps.type = "GLO";
                    }else if(line.substr(0,2) == "PC"){
                         ps.type = "BDS";
                    }else{
                        continue;
                    }
                    ps.prn = std::stoi(line.substr(2,2));
                    ps.precise_x =str_to_long_double(line.substr(5,13));
                    ps.precise_y =str_to_long_double(line.substr(19,13));
                    ps.precise_z =str_to_long_double(line.substr(33,13));
                    precise_sats.push_back(ps);

           }

       }




   }
   //循环遍历sats和sat_GLOs，填充时间符合的精密星历坐标
   for(int i=0;i<sats.size();i++){
       for(int j=0;j<precise_sats.size();j++){
           if(sats[i].type == precise_sats[j].type && sats[i].prn == precise_sats[j].prn){
                    if(sats[i].year == precise_sats[j].year && sats[i].month == precise_sats[j].month && sats[i].day == precise_sats[j].day && sats[i].hour == precise_sats[j].hour && sats[i].minute == precise_sats[j].minute && sats[i].second == precise_sats[j].second){
                        sats[i].precise_x = precise_sats[j].precise_x;
                        sats[i].precise_y = precise_sats[j].precise_y;
                        sats[i].precise_z = precise_sats[j].precise_z;
                    }
           }
       }
   }
   for(int i=0;i<sat_GLOs.size();i++){
       for(int j=0;j<precise_sats.size();j++){
           if(sat_GLOs[i].type == precise_sats[j].type && sat_GLOs[i].prn == precise_sats[j].prn){
                    if(sat_GLOs[i].year == precise_sats[j].year && sat_GLOs[i].month == precise_sats[j].month && sat_GLOs[i].day == precise_sats[j].day && sat_GLOs[i].hour == precise_sats[j].hour && sat_GLOs[i].minute == precise_sats[j].minute && sat_GLOs[i].second == precise_sats[j].second){
                        sat_GLOs[i].precise_x = precise_sats[j].precise_x;
                        sat_GLOs[i].precise_y = precise_sats[j].precise_y;
                        sat_GLOs[i].precise_z = precise_sats[j].precise_z;
                    }
           }
       }
   }

     //更新表格
     //GPS,BDS,GAL
     for(int i=0;i<sats.size();i++){
       //若precise_x等于0，则不显示
       if(sats[i].precise_x==0){
           continue;
       }
       //设置解算坐标,xyz的逗号换成换行，占三行
       ui->everyTable->setItem(i,3,new QTableWidgetItem(QString::fromStdString(std::to_string(sats[i].precise_x)+"\n"+std::to_string(sats[i].precise_y)+"\n"+std::to_string(sats[i].precise_z))));
       //设置偏差,xyz的逗号换成换行，占三行
       ui->everyTable->setItem(i,4,new QTableWidgetItem(QString::fromStdString(std::to_string(sats[i].calculate_x-sats[i].precise_x)+"\n"+std::to_string(sats[i].calculate_y-sats[i].precise_y)+"\n"+std::to_string(sats[i].calculate_z-sats[i].precise_z))));
     }
     //GLO,在GPS,BDS,GAL后面
     for(int i=0;i<sat_GLOs.size();i++){
       //若precise_x等于0，则不显示
       if(sat_GLOs[i].precise_x==0){
           continue;
       }
       //设置解算坐标,xyz的逗号换成换行，占三行
       ui->everyTable->setItem(i+sats.size(),3,new QTableWidgetItem(QString::fromStdString(std::to_string(sat_GLOs[i].precise_x)+"\n"+std::to_string(sat_GLOs[i].precise_y)+"\n"+std::to_string(sat_GLOs[i].precise_z))));
       //设置偏差,xyz的逗号换成换行，占三行
       ui->everyTable->setItem(i+sats.size(),4,new QTableWidgetItem(QString::fromStdString(std::to_string(sat_GLOs[i].calculate_x-sat_GLOs[i].precise_x)+"\n"+std::to_string(sat_GLOs[i].calculate_y-sat_GLOs[i].precise_y)+"\n"+std::to_string(sat_GLOs[i].calculate_z-sat_GLOs[i].precise_z))));
     }

     //表格自适应
     ui->everyTable->resizeColumnsToContents();
     //第一列宽+10
     ui->everyTable->setColumnWidth(0,ui->everyTable->columnWidth(0)+10);
     //第3列宽+10
     ui->everyTable->setColumnWidth(3,ui->everyTable->columnWidth(3)+10);
     //第4列宽+10
     ui->everyTable->setColumnWidth(4,ui->everyTable->columnWidth(4)+10);
     //第5列宽+10
     ui->everyTable->setColumnWidth(5,ui->everyTable->columnWidth(5)+10);
     //提示：读取完成
     QMessageBox::information(this,"提示","读取完成",QMessageBox::Ok);
}

void MainWindow::on_satsDetail_clicked()
{
     if(sats_showed.size()==0){
         QMessageBox::warning(this,"警告","请先点击可视化显示按钮",QMessageBox::Ok);
         return;
     }
    //定义一个全屏窗口
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("卫星详情");
    dialog->resize(1200,800);
    QTableWidget *table = new QTableWidget;
    //设置表格行数
    table->setRowCount(sats_showed.size());
    //设置表格列数
    table->setColumnCount(5);
    //设置表格行宽是默认的3倍
    table->verticalHeader()->setDefaultSectionSize(table->verticalHeader()->defaultSectionSize()*3);

    //设置表格标题
    QStringList header;
    header<<"类型"<<"PRN"<<"方位角"<<"高度角"<<"解算坐标";
    table->setHorizontalHeaderLabels(header);
    //设置表格内容
    for(int i=0;i<sats_showed.size();i++){
        //设置类型
        table->setItem(i,0,new QTableWidgetItem(QString::fromStdString(sats_showed[i].type)));
        //设置PRN
        table->setItem(i,1,new QTableWidgetItem(QString::fromStdString(std::to_string(sats_showed[i].prn))));
        //设置方位角
        table->setItem(i,2,new QTableWidgetItem(QString::fromStdString(std::to_string(sats_showed[i].azimuth))));
        //设置高度角
        table->setItem(i,3,new QTableWidgetItem(QString::fromStdString(std::to_string(sats_showed[i].elevation))));
        //设置解算坐标,xyz的逗号换成换行，占三行
        table->setItem(i,4,new QTableWidgetItem(QString::fromStdString(std::to_string(sats_showed[i].calculate_x)+"\n"+std::to_string(sats_showed[i].calculate_y)+"\n"+std::to_string(sats_showed[i].calculate_z))));
    }
    //表格自适应
    table->resizeColumnsToContents();
    //第3列宽+10
    table->setColumnWidth(2,table->columnWidth(2)+10);
    //第4列宽+10
    table->setColumnWidth(3,table->columnWidth(3)+10);
    //第5列宽+10
    table->setColumnWidth(4,table->columnWidth(4)+10);
    //左侧是未定义的widget，右侧是表格，水平布局，左侧占1/3，右侧占2/3
    QHBoxLayout *layout = new QHBoxLayout;
    //定义一个widget
    QWidget *widget = new QWidget;
    layout->addWidget(widget,2);
    layout->addWidget(table,1);
    dialog->setLayout(layout);

    glWidget2 = new MyOpenGLWidget(widget);
    glWidget2->resize(790, 790);
    glWidget2->show();
    glWidget2->updateOpenGLContent(sats_showed);
    dialog->show();





}

void MainWindow::on_donwloadBtn_clicked()
{
    //每日 RINEX 广播星历文件：https://cddis.nasa.gov/archive/gnss/data/daily/
    //精密星历文件：https://cddis.nasa.gov/archive/gnss/products/
    //打开一个界面，显示信息,两个链接，一个是每日RINEX广播星历文件，一个是精密星历文件，点击链接跳转到网页
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("下载星历");
    dialog->resize(300,100);
    //定义一个label
    QLabel *label = new QLabel;
    label->setText("请选择下载星历的类型");
    //定义一个按钮
    QPushButton *button1 = new QPushButton;
    button1->setText("每日RINEX广播星历文件");
    //定义一个按钮
    QPushButton *button2 = new QPushButton;
    button2->setText("精密星历文件");
    //连接信号和槽
    connect(button1,&QPushButton::clicked,[=](){
        QDesktopServices::openUrl(QUrl(QLatin1String("https://cddis.nasa.gov/archive/gnss/data/daily/")));//打开网页
        dialog->close();//关闭窗口
    });
    connect(button2,&QPushButton::clicked,[=](){
        QDesktopServices::openUrl(QUrl(QLatin1String("https://cddis.nasa.gov/archive/gnss/products/")));//打开网页
        dialog->close();//关闭窗口
    });
    //添加label和按钮
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(button1);
    layout->addWidget(button2);
    dialog->setLayout(layout);
    dialog->show();








}


void MainWindow::on_satType_currentTextChanged(const QString &arg1)
{
    //comboBox:ui->satType设置为type_prns的key
    //选择satType后更新satPrn
    ui->satPrn->clear();
    //获取satType的value
    std::vector<int> prns = type_prns[arg1];
    //添加到satPrn
    for(int i=0;i<prns.size();i++){
        ui->satPrn->addItem(QString::fromStdString(std::to_string(prns[i])));
    }

}


void MainWindow::on_calculateBtn2_clicked()
{
    //判断type_prns是否为空
    if(type_prns.size()==0){
        QMessageBox::warning(this,"警告","请先读取星历",QMessageBox::Ok);
        return;
    }
    //获取ui->satType的值
    string type = ui->satType->currentText().toStdString();
    //获取ui->satPrn的值
    int prn = ui->satPrn->currentText().toInt();
    //清空shows和shows_GLO
    shows.clear();
    shows_GLO.clear();
    if(type != "GLO"){
        //定义一个vector
        std::vector<Satellite> ss;
        //遍历sats，找到type和prn相同的卫星
        for(int i=0;i<sats.size();i++){
            if(sats[i].type == type && sats[i].prn == prn){
                ss.push_back(sats[i]);
            }
        }
        //打印ss
        for(int i=0;i<ss.size();i++){
            std::cout<<ss[i].type<<","<<ss[i].prn<<","<<ss[i].year<<","<<ss[i].month<<","<<ss[i].day<<","<<ss[i].hour<<","<<ss[i].minute<<","<<ss[i].second<<","<<std::endl;
        }
        //获取时间段,ui->startTime,ui->endTime,小时和分钟
        int startHour = ui->startTime->time().hour();
        int startMinute = ui->startTime->time().minute();
        int endHour = ui->endTime->time().hour();
        int endMinute = ui->endTime->time().minute();

         //在时间段内，每隔一分钟获取该卫星的xyz，用GPS_Adjust函数
        //遍历shows赋值从开始时间开始，每隔一分钟，到结束时间
        for(int i=0;i<(endHour-startHour)*60+endMinute-startMinute+1;i++){
            Satellite s;
            s.type = ss[0].type;
            s.prn = ss[0].prn;
            s.year = ss[0].year;
            s.month = ss[0].month;
            s.day = ss[0].day;
            s.hour = startHour+i/60;
            s.minute = startMinute+i%60;
            s.second = 0;
            shows.push_back(s);
        }
        //遍历shows，调用GPS_Adjust函数
        for(int i=0;i<shows.size();i++){
            //获取时间
            int hour3 = shows[i].hour;
            int minute3 = shows[i].minute;
            double second3 = shows[i].second;
            double target_time3 = timeToTimestamp(hour3,minute3,second3);
            //间接平差函数
            // t0是初始时间(从0时为起点的时间戳)，delt_t是时间间隔(s)，satellites卫星数组，n是切比雪夫多项式阶数,slice取时间间隔的秒数(s),target是预测时间的时间戳
            // position GPS_Adjust(double t0, double delt_t, vector<Satellite> satellites, int n, double slice,double target)
            double t03;//起始时间
            if(target_time3<timeToTimestamp(1,0,0.0)){
                t03 = 0;
            }else{
                t03 = target_time3 - timeToTimestamp(1,0,0.0);
            }
            double delt_t3 = 60*60*2;//时间间隔
            int n3 = 10;//阶数
            double slice3 = 60;//取时间间隔的秒数(s)
            double target3 = target_time3;//预测时间的时间戳
            position rs;
            rs = GPS_Adjust(t03,delt_t3,ss,n3,slice3,target3);
            shows[i].calculate_x = rs.x;
            shows[i].calculate_y = rs.y;
            shows[i].calculate_z = rs.z;
            //计算方位角,方位角A范围在0~360°；若A＜0，A=A+2pi；若A＞2pi，A=A-2pi
            //获取BLH
            double b = ui->BInput->text().toDouble();
            double l = ui->LInput->text().toDouble();
            double h = ui->HInput->text().toDouble();
            //将BLH转换为XYZ，extern void pos2ecef(const double *pos, double *r)
            double* ecef = new double[3];
            double* pos = new double[3];
            pos[0] = b*M_PI/180;
            pos[1] = l*M_PI/180;
            pos[2] = h;
            pos2ecef(pos,ecef);
            //将shows[i].calculate_x ,Y,Z转换为wgs84的XYZ
            //GTRF转换为WGS-84
            if(shows[i].type == "GAL"){
               shows[i].calculate_x  += 0.02;
               shows[i].calculate_y  -= 0.01;
               shows[i].calculate_z  += 0.01;
            }
            double* enu = new double[3];
            double* r = new double[3];
            r[0] = shows[i].calculate_x - ecef[0];
            r[1] = shows[i].calculate_y - ecef[1];
            r[2] = shows[i].calculate_z - ecef[2];
            //计算方位角,方位角A范围在0~360°；若A＜0，A=A+2pi；若A＞2pi，A=A-2pi
            ecef2enu(pos,r,enu);
            double azimuth = atan2(enu[0],enu[1])*180/M_PI;
            if(azimuth<0){
                azimuth = azimuth + 360;
            }else if(azimuth>360){
                azimuth = azimuth - 360;
            }
            //计算高度角
            double elevation = atan2(enu[2],sqrt(enu[0]*enu[0]+enu[1]*enu[1]))*180/M_PI;
            shows[i].azimuth = azimuth;
            shows[i].elevation = elevation;

        }
        //渲染到timeTable
        //清空timeTable的内容
        ui->timeTable->clearContents();
        //设置表格行数
        ui->timeTable->setRowCount(shows.size());
        //设置内容
        for(int i=0;i<shows.size();i++){
            //设置时间
            ui->timeTable->setItem(i,2,new QTableWidgetItem(QString::fromStdString(std::to_string(shows[i].year)+"-"+std::to_string(shows[i].month)+"-"+std::to_string(shows[i].day)+" "+std::to_string(shows[i].hour)+":"+std::to_string(shows[i].minute)+":00")));
            //设置类型
            ui->timeTable->setItem(i,0,new QTableWidgetItem(QString::fromStdString(shows[i].type)));
            //设置PRN
            ui->timeTable->setItem(i,1,new QTableWidgetItem(QString::fromStdString(std::to_string(shows[i].prn))));
            //设置坐标
            ui->timeTable->setItem(i,3,new QTableWidgetItem(QString::fromStdString(std::to_string(shows[i].calculate_x)+"\n"+std::to_string(shows[i].calculate_y)+"\n"+std::to_string(shows[i].calculate_z))));
            //设置方位角
            ui->timeTable->setItem(i,4,new QTableWidgetItem(QString::fromStdString(std::to_string(shows[i].azimuth))));
            //设置高度角
            ui->timeTable->setItem(i,5,new QTableWidgetItem(QString::fromStdString(std::to_string(shows[i].elevation))));


        }
        //表格自适应
        ui->timeTable->resizeColumnsToContents();
        //提示：计算完成
        QMessageBox::information(this,"提示","计算完成",QMessageBox::Ok);






    }
    else{
        //定义一个vector
        std::vector<Satellite_GLO> ss;
        //遍历sats，找到type和prn相同的卫星
        for(int i=0;i<sat_GLOs.size();i++){
            if(sat_GLOs[i].type == type && sat_GLOs[i].prn == prn){
                ss.push_back(sat_GLOs[i]);
            }
        }
        //打印ss
        for(int i=0;i<ss.size();i++){
            std::cout<<ss[i].type<<","<<ss[i].prn<<","<<ss[i].year<<","<<ss[i].month<<","<<ss[i].day<<","<<ss[i].hour<<","<<ss[i].minute<<","<<ss[i].second<<","<<std::endl;
        }
        //获取时间段,ui->startTime,ui->endTime,小时和分钟
        int startHour = ui->startTime->time().hour();
        int startMinute = ui->startTime->time().minute();
        int endHour = ui->endTime->time().hour();
        int endMinute = ui->endTime->time().minute();

         //在时间段内，每隔一分钟获取该卫星的xyz，用GPS_Adjust函数
        //遍历shows赋值从开始时间开始，每隔一分钟，到结束时间
        for(int i=0;i<(endHour-startHour)*60+endMinute-startMinute+1;i++){
            Satellite_GLO s;
            s.type = ss[0].type;
            s.prn = ss[0].prn;
            s.year = ss[0].year;
            s.month = ss[0].month;
            s.day = ss[0].day;
            s.hour = startHour+i/60;
            s.minute = startMinute+i%60;
            s.second = 0.0;
            s.clock_bias = ss[0].clock_bias;
            s.clock_drift = ss[0].clock_drift;
            shows_GLO.push_back(s);
        }
        //遍历shows，调用GPS_Adjust函数
        for(int i=0;i<shows_GLO.size();i++){

            //获取时间
            double target_time3 = timeToTimestamp(shows_GLO[i].hour,
                                                  shows_GLO[i].minute,
                                                  shows_GLO[i].second);
            //间接平差函数
            // t0是初始时间(从0时为起点的时间戳)，delt_t是时间间隔(s)，satellites卫星数组，n是切比雪夫多项式阶数,slice取时间间隔的秒数(s),target是预测时间的时间戳
            // position GPS_Adjust(double t0, double delt_t, vector<Satellite> satellites, int n, double slice,double target)
            double t03;//起始时间
            if(target_time3<timeToTimestamp(1,0,0.0)){
                t03 = 0;
            }else{
                t03 = target_time3 - timeToTimestamp(1,0,0.0);
            }
            double delt_t3 = 60*60*2;//时间间隔
            int n3 = 10;//阶数
            double slice3 = 60;//取时间间隔的秒数(s)
            double target3 = target_time3;//预测时间的时间戳
            position rs;
            rs = GLO_Adjust(t03,delt_t3,ss,n3,slice3,target3);
            shows_GLO[i].calculate_x = rs.x;
            shows_GLO[i].calculate_y = rs.y;
            shows_GLO[i].calculate_z = rs.z;
            //计算方位角,方位角A范围在0~360°；若A＜0，A=A+2pi；若A＞2pi，A=A-2pi
            //获取BLH
            double b = ui->BInput->text().toDouble();
            double l = ui->LInput->text().toDouble();
            double h = ui->HInput->text().toDouble();
            //将BLH转换为XYZ，extern void pos2ecef(const double *pos, double *r)
            double* ecef = new double[3];
            double* pos = new double[3];
            pos[0] = b*M_PI/180;
            pos[1] = l*M_PI/180;
            pos[2] = h;
            pos2ecef(pos,ecef);
            //将calculate_x,y,z转换为wgs84的XYZ
            Eigen::Matrix <double, 3, 3> B;
            B << 1, 1.728e-6, -0.017e-6,
                1.728e-6, 1, 0.076e-6,
                0.0178e-6, -0.076e-6, 1;
            Eigen::Matrix <double, 3, 1> c;
            c << shows_GLO[i].calculate_x, shows_GLO[i].calculate_y, shows_GLO[i].calculate_z;
            Eigen::Matrix <double, 3, 1> a;
            a << -0.47, -0.51, -1.56;
            Eigen::Matrix <double, 3, 1> Rk;
            Rk = a + (1+22e-9) * B * c;
            shows_GLO[i].calculate_x = Rk(0);
            shows_GLO[i].calculate_y = Rk(1);
            shows_GLO[i].calculate_z = Rk(2);


            double* enu = new double[3];
            double* r = new double[3];
            r[0] = shows_GLO[i].calculate_x - ecef[0];
            r[1] = shows_GLO[i].calculate_y - ecef[1];
            r[2] = shows_GLO[i].calculate_z - ecef[2];
            //计算方位角,方位角A范围在0~360°；若A＜0，A=A+2pi；若A＞2pi，A=A-2pi
            ecef2enu(pos,r,enu);
            double azimuth = atan2(enu[0],enu[1])*180/M_PI;
            if(azimuth<0){
                azimuth = azimuth + 360;
            }else if(azimuth>360){
                azimuth = azimuth - 360;
            }
            //计算高度角
            double elevation = atan2(enu[2],sqrt(enu[0]*enu[0]+enu[1]*enu[1]))*180/M_PI;
            shows_GLO[i].azimuth = azimuth;
            shows_GLO[i].elevation = elevation;
            //设置
            shows_GLO[i].calculate_x = rs.x;
            shows_GLO[i].calculate_y = rs.y;
            shows_GLO[i].calculate_z = rs.z;

        }
        //渲染到timeTable
        //清空timeTable的内容
        ui->timeTable->clearContents();
        //设置表格行数
        ui->timeTable->setRowCount(shows_GLO.size());
        //设置内容
        for(int i=0;i<shows_GLO.size();i++){
            //设置时间
            ui->timeTable->setItem(i,2,new QTableWidgetItem(QString::fromStdString(std::to_string(shows_GLO[i].year)+"-"+std::to_string(shows_GLO[i].month)+"-"+std::to_string(shows_GLO[i].day)+" "+std::to_string(shows_GLO[i].hour)+":"+std::to_string(shows_GLO[i].minute)+":00")));
            //设置类型
            ui->timeTable->setItem(i,0,new QTableWidgetItem(QString::fromStdString(shows_GLO[i].type)));
            //设置PRN
            ui->timeTable->setItem(i,1,new QTableWidgetItem(QString::fromStdString(std::to_string(shows_GLO[i].prn))));
            //设置坐标
            ui->timeTable->setItem(i,3,new QTableWidgetItem(QString::fromStdString(std::to_string(shows_GLO[i].calculate_x)+"\n"+std::to_string(shows_GLO[i].calculate_y)+"\n"+std::to_string(shows_GLO[i].calculate_z))));
            //设置方位角
            ui->timeTable->setItem(i,4,new QTableWidgetItem(QString::fromStdString(std::to_string(shows_GLO[i].azimuth))));
            //设置高度角
            ui->timeTable->setItem(i,5,new QTableWidgetItem(QString::fromStdString(std::to_string(shows_GLO[i].elevation))));
        }
        //表格自适应
        ui->timeTable->resizeColumnsToContents();
        //提示：计算完成
        QMessageBox::information(this,"提示","计算完成",QMessageBox::Ok);



    }

}



