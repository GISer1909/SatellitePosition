#pragma execution_character_set("utf-8")
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "QMainWindow"
#include "QFileDialog"
#include "QMessageBox"
#include "QDebug"
#include "QKeyEvent"
#include "QTableWidget"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <QVector>
#include <QDesktopServices>
#include <Eigen/Dense>
#include <rtklib.h>
#include <sat.h>
#include <MyOpenGLWidget.h>
#include <sstream>
#include <iomanip>
#include "GL/freeglut.h"
using namespace std;
#define bGM84 3.986004418e14//地球引力常数
#define bOMEGAE84 7.2921151467e-5
#define M_PI 3.14159265358979323846

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    long double str_to_double(std::string s) {
        size_t pos = s.find('D'); // 查找字符串中的D字符
        if (pos != std::string::npos) { // 如果找到了D字符
            s[pos] = 'E'; // 将D字符替换为E字符
        }
        //转换为long double类型,小数点后保留6位
        return std::stold(s);
    };
    long double str_to_long_double(std::string s) {
        //不要四舍五入，保留6位小数
        size_t pos = s.find('D'); // 查找字符串中的D字符
        if (pos != std::string::npos) { // 如果找到了D字符
            s[pos] = 'E'; // 将D字符替换为E字符
        }

        return std::stold(s)*1000;
    };
protected:

private slots:
    void on_openGLWidget_resized();
    void on_openRNX_clicked();

    void on_calculateBtn_clicked();

    void on_openglBtn_clicked();

    void on_openSP3_clicked();

    void on_satsDetail_clicked();

    void on_donwloadBtn_clicked();

    void on_satType_currentTextChanged(const QString &arg1);

    void on_calculateBtn2_clicked();


private:
    Ui::MainWindow *ui;
    struct  position{
        double x;
        double y;
        double z;
    };
    MyOpenGLWidget *glWidget;
    MyOpenGLWidget *glWidget2;
    //定义一个字典，包括卫星类型和该类型卫星的prn
    std::map<QString,std::vector<int>> type_prns;
    //将时分秒转换成以当日0时开始的时间戳
    double timeToTimestamp(int hour, int minute, double second) {
        return hour * 3600 + minute * 60 + second;
    }
    //将以当日0时开始的时间戳转换成时分秒
    vector<double> timestampToTime(double timestamp) {
        vector<double> time;
        time.push_back(timestamp / 3600);
        time.push_back((timestamp - time[0] * 3600) / 60);
        time.push_back(timestamp - time[0] * 3600 - time[1] * 60);
        return time;
    }
    // 计算切比雪夫多项式
    double chebyshevPolynomial(double tau, int n) {
        if (n == 0)
            return 1;
        else if (n == 1)
            return tau;
        else
            return 2 * tau * chebyshevPolynomial(tau, n - 1) - chebyshevPolynomial(tau, n - 2);
    }
    std::vector<Satellite> sats;//卫星数组
    std:: vector<Satellite_GLO> sat_GLOs;//卫星数组
    std::vector<Satellite> sats_showed;//用于显示的卫星1/2数组
    std::vector<Satellite> shows;//用于卫星数组2
    std::vector<Satellite_GLO> shows_GLO;//用于卫星数组2
    // 定义一个函数将Satellite_GLO转换为geph_t
    geph_t ConvertToGeph(const Satellite_GLO& satellite)
    {
        geph_t geph;

        // 设置geph_t结构体的成员变量
        geph.sat = satellite.prn;
        geph.svh = 0; // 这里假设没有卫星健康信息
        geph.frq = 0; // 这里假设使用默认的频率
        geph.age = 0; // 这里假设没有星历的年龄信息
        geph.sva = 0; // 这里假设没有卫星精度信息
        geph.svh = 0; // 这里假设没有卫星健康信息
        double epoch[6] = {double(satellite.year),
                           double(satellite.month),
                           double(satellite.day),
                           double(satellite.hour),
                           double(satellite.minute),
                           satellite.second};
        geph.toe = epoch2time(epoch);
        geph.tof = geph.toe;
        geph.pos[0] = satellite.x;//转换为m
        geph.pos[1] = satellite.y;
        geph.pos[2] = satellite.z;
        //转换为m/s
        geph.vel[0] = satellite.x_speed;
        geph.vel[1] = satellite.y_speed;
        geph.vel[2] = satellite.z_speed;
        //转换为m/s^2
        geph.acc[0] = satellite.x_acceleration;
        geph.acc[1] = satellite.y_acceleration;
        geph.acc[2] = satellite.z_acceleration;
        geph.taun = satellite.clock_bias;
        geph.gamn = satellite.clock_drift;

        return geph;
    }
    position calculate(Satellite satellite,long double t)
    //t是以当日0时开始的时间戳，默认为GPST时间
    {
        //将卫星参数存入rinex向量
        std::vector<double> rinex;
        rinex.push_back(satellite.prn);
        rinex.push_back(satellite.year);
        rinex.push_back(satellite.month);
        rinex.push_back(satellite.day);
        rinex.push_back(satellite.hour);
        rinex.push_back(satellite.minute);
        rinex.push_back(satellite.second);
        rinex.push_back(satellite.clock_bias);
        rinex.push_back(satellite.clock_drift);
        rinex.push_back(satellite.clock_drift_rate);
        rinex.push_back(satellite.iode);
        rinex.push_back(satellite.crs);
        rinex.push_back(satellite.delta_n);
        rinex.push_back(satellite.m0);
        rinex.push_back(satellite.cuc);
        rinex.push_back(satellite.e);
        rinex.push_back(satellite.cus);
        rinex.push_back(satellite.sqrt_a);
        rinex.push_back(satellite.toe);
        rinex.push_back(satellite.cic);
        rinex.push_back(satellite.omega0);
        rinex.push_back(satellite.cis);
        rinex.push_back(satellite.i0);
        rinex.push_back(satellite.crc);
        rinex.push_back(satellite.omega);
        rinex.push_back(satellite.omega_dot);
        rinex.push_back(satellite.idot);
        rinex.push_back(satellite.codes_l2_channel);
        rinex.push_back(satellite.gps_week_number);
        rinex.push_back(0);
        rinex.push_back(satellite.accuracy_index);
        rinex.push_back(satellite.health_status_bitmask);
        rinex.push_back(satellite.tgd);
        rinex.push_back(satellite.iodc);
        rinex.push_back(satellite.transmission_time_of_message_sow);
        rinex.push_back(0);


        // 从rinex向量中获取卫星参数

        long double clock_bias = rinex[7]; //卫星钟差
        long double clock_drift = rinex[8]; // 卫星钟漂
        long double clock_drift_rate = rinex[9]; // 卫星钟漂变化率
        long double crs = rinex[11]; // 卫星轨道半径的余弦调和项
        long double delta_n = rinex[12]; // 平均角速度改正项
        long double m0 = rinex[13]; // 参考时刻平近点角
        long double cuc = rinex[14]; // 升交距角的余弦调和项
        long double e = rinex[15]; // 轨道偏心率
        long double cus = rinex[16]; // 升交距角的正弦调和项
        long double roota = rinex[17]; // 轨道长半轴的平方根
        long double toe = rinex[18]; // 参考时间,代表卫星钟的时间，以周为单位，是gps周内的秒数，一般是0-604800秒，也就是0-7天。
        long double cic = rinex[19]; // 轨道倾角的余弦调和项
        long double omega0 = rinex[20]; // 升交距角的初始值
        long double cis = rinex[21]; // 轨道倾角的正弦调和项
        long double i0 = rinex[22]; // 轨道倾角
        long double crc = rinex[23]; // 卫星轨道半径的余弦调和项
        long double omega = rinex[24]; // 近地点角距
        long double omega_dot = rinex[25]; // 升交距角的变化率
        long double idot = rinex[26]; // 轨道倾角的变化率
        long double earthrate = bOMEGAE84; //地球自转角速度




        //计算
        // 计算轨道长半轴的平方
        long double A = roota * roota;
        // 计算卫星运行的平均角速度
        long double n0 = sqrt(bGM84 / (A * A * A));
        long double n = n0 + delta_n;


        //观测时刻，考虑GPST-UTC=18s,以GPST为准
        long double _t = toe +(t) - timeToTimestamp(rinex[4], rinex[5], rinex[6]);//t是以当日0时开始的时间戳，默认为GPST时间
        if(satellite.type == "BDS"){
            _t = toe +(t) - timeToTimestamp(rinex[4], rinex[5], rinex[6]) - 14;
        }
        //打印观测时间
        //        cout << "_t" << _t << endl;
        long double delt_t = clock_bias + clock_drift * (_t - toe) + clock_drift_rate * (_t - toe) * (_t - toe);
        long double dt = _t - delt_t - toe;


        // 计算t时刻的卫星平近角点
        long double  mk = m0 + n * dt;


        // 迭代计算偏近点角Ek,
        long double ek = mk;
        for (int iter = 0; iter < 7; iter++)
            ek = mk + e * sin(ek);



        // 计算真近点角
        long double fk = atan2(sqrt(1.0 - e * e) * sin(ek), cos(ek) - e);

        // 计算升交距角phik
        long double phik = fk + omega;



        // 计算摄动改正项
        long double corr_u = cus * sin(2.0 * phik) + cuc * cos(2.0 * phik);
        long double corr_r = crs * sin(2.0 * phik) + crc * cos(2.0 * phik);
        long double corr_i = cis * sin(2.0 * phik) + cic * cos(2.0 * phik);


        // 计算摄动改正后的升交距角，轨道半径，轨道倾角
        long double uk = phik + corr_u;
        long double rk = A * (1.0 - e * cos(ek)) + corr_r;
        long double ik = i0 + idot * dt + corr_i;

        // 计算在轨道平面坐标系中的位置
        long double  xpk = rk * cos(uk);
        long double  ypk = rk * sin(uk);

        //计算升交点经度
        long double  omegak = omega0 + (omega_dot - earthrate) * dt - earthrate * toe;

        // 计算在地心惯性系中的位置
        long double  xk = xpk * cos(omegak) - ypk * cos(ik) * sin(omegak);
        long double  yk = xpk * sin(omegak) + ypk * cos(ik) * cos(omegak);
        long double  zk = ypk * sin(ik);

        //判断是否是GEO卫星
        if(satellite.type == "BDS" && (satellite.prn == 1
                                        || satellite.prn == 2
                                        || satellite.prn == 3
                                        || satellite.prn == 4
                                        || satellite.prn == 5
                                        || satellite.prn == 59
                                        || satellite.prn == 60
                                        || satellite.prn == 61
                                        || satellite.prn == 62
                                        )){


            //计算GEO卫星位置
            omegak =  omega0 + (omega_dot) * dt - earthrate * toe;
            long double x_GK = xpk * cos(omegak) - ypk * cos(ik) * sin(omegak);
            long double y_GK = xpk * sin(omegak) + ypk * cos(ik) * cos(omegak);
            long double z_GK = ypk * sin(ik);
            Eigen::Matrix <double, 3, 3> R_x_f;//f为-5°
            R_x_f << 1, 0, 0,
                0, cos(-5 * M_PI / 180), sin(-5 * M_PI / 180),
                0, -sin(-5 * M_PI / 180), cos(-5 * M_PI / 180);
            Eigen::Matrix <double, 3, 3> R_z_f;//f = earthrate * dt
            R_z_f << cos(earthrate * dt), sin(earthrate * dt), 0,
                -sin(earthrate * dt), cos(earthrate * dt), 0,
                0, 0, 1;
            Eigen::Matrix <double, 3, 1> Rk;
            Rk = R_z_f * R_x_f * Eigen::Matrix <double, 3, 1>(x_GK, y_GK, z_GK);
            xk = Rk(0, 0);
            yk = Rk(1, 0);
            zk = Rk(2, 0);
        }



        position result;
        result.x = xk;
        result.y = yk;
        result.z = zk;
        return result;
    }
    //间接平差函数
    // t0是初始时间(从0时为起点的时间戳)，delt_t是时间间隔(s)，satellites卫星数组，n是切比雪夫多项式阶数,slice取时间间隔的秒数(s),target是预测时间的时间戳
    position GPS_Adjust(double t0, double delt_t, vector<Satellite> satellites, int n, double slice,double target) {
        vector<double> t;//从t0到delt_t的时间间隔点
        //计算t
        for (int i = 0; i <=delt_t; i += slice) {
            t.push_back(t0 + i);
        }

        Eigen::MatrixXd x1(t.size(), 1);//x1是卫星位置
        Eigen::MatrixXd y1(t.size(), 1);//y1是卫星位置
        Eigen::MatrixXd z1(t.size(), 1);//z1是卫星位置
        //satellites是0时，2时，4时到22时的卫星参数，需要先根据t的时间计算卫星位置，例如t=1时55分的时间戳时，需要计算1时55分的卫星位置，但是卫星参数是2时的，所以需要以计算2时的卫星位置，计算1时55分的卫星位置
        for(int i = 0; i < t.size(); i++){
            vector<double> time = timestampToTime(t[i]);
            //找到时间戳与卫星参数时间差最小的最接近的卫星，使t[i]-timeToTimestamp(satellites[j].hour, satellites[j].minute, satellites[j].second)最小
            int j = 0;
            for(int k = 0; k < satellites.size(); k++){
                if(abs(t[i]-timeToTimestamp(satellites[k].hour, satellites[k].minute, satellites[k].second)) < abs(t[i]-timeToTimestamp(satellites[j].hour, satellites[j].minute, satellites[j].second))){
                    j = k;
                }
            }
            //计算卫星位置
            position result = calculate(satellites[j],t[i]);
            x1(i, 0) = result.x;
            y1(i, 0) = result.y;
            z1(i, 0) = result.z;
        }
        //将t转换成tau
        for (int i = 0; i < t.size(); i++)
        {
            double tau = (2 / (delt_t)) * (t[i] - t0) - 1;
            t[i] = tau;

        }

        //切比雪夫多项式矩阵，t的个数*切比雪夫多项式阶数,使用Eigen库
        Eigen::MatrixXd T(t.size(), n);
        for (int i = 0; i < t.size(); i++)
        {
            for (int j = 0; j < n; j++)
            {
                T(i, j) = chebyshevPolynomial(t[i], j);
            }
        }




        //V=TC-X,C为切比雪夫多项式系数，X为卫星坐标，V为残差
        //C =(T^T*T)^-1*T^T*X
        Eigen::MatrixXd T1 = T.transpose() * T;
        Eigen::MatrixXd T2 = T1.inverse();
        Eigen::MatrixXd Cx = T2 * T.transpose() * x1;
        Eigen::MatrixXd Cy = T2 * T.transpose() * y1;
        Eigen::MatrixXd Cz = T2 * T.transpose() * z1;
        //计算预测位置
        double tau = (2 / (delt_t)) * (target - t0) - 1;
        double x = 0;
        double y = 0;
        double z = 0;
        for (int i = 0; i < n; i++)
        {
            x += Cx(i, 0) * chebyshevPolynomial(tau, i);
            y += Cy(i, 0) * chebyshevPolynomial(tau, i);
            z += Cz(i, 0) * chebyshevPolynomial(tau, i);
        }
        position result;
        result.x = x;
        result.y = y;
        result.z = z;
        return result;


    }
    position GLO_Adjust(double t0, double delt_t, vector<Satellite_GLO> satellites, int n, double slice,double target) {
        //查看satellites中时间差与target最小的卫星
        int j = 0;
        for(int k = 0; k < satellites.size(); k++){
            if(abs(target - timeToTimestamp(satellites[k].hour, satellites[k].minute, satellites[k].second)) < abs(target - timeToTimestamp(satellites[j].hour, satellites[j].minute, satellites[j].second))){
                j = k;
            }
        }
        //计算卫星位置
        const geph_t geph =  ConvertToGeph(satellites[j]);
        double rs[3];     // 卫星位置
        double dts[2];    // 卫星钟差
        double var;       // 位置和钟差的方差
        gtime_t _time;     // 时间
        //time赋值为GPST时间
        _time = timeadd(geph.toe,target-timeToTimestamp(satellites[j].hour, satellites[j].minute, satellites[j].second));
        geph2pos(_time, &geph, rs, dts, &var);
        position result;
        result.x = rs[0];
        result.y = rs[1];
        result.z = rs[2];
        return result;
    }

    //        vector<double> t;//从t0到delt_t的时间间隔点
    //        //计算t
    //        for (int i = 0; i <=delt_t; i += slice) {
    //            t.push_back(t0 + i);
    //        }

    //        Eigen::MatrixXd x1(t.size(), 1);//x1是卫星位置
    //        Eigen::MatrixXd y1(t.size(), 1);//y1是卫星位置
    //        Eigen::MatrixXd z1(t.size(), 1);//z1是卫星位置
    //        //satellites是0时，2时，4时到22时的卫星参数，需要先根据t的时间计算卫星位置，例如t=1时55分的时间戳时，需要计算1时55分的卫星位置，但是卫星参数是2时的，所以需要以计算2时的卫星位置，计算1时55分的卫星位置
    //        for(int i = 0; i < t.size(); i++){
    //            vector<double> time = timestampToTime(t[i]);
    //            //找到时间戳与卫星参数时间差最小的最接近的卫星，使t[i]-timeToTimestamp(satellites[j].hour, satellites[j].minute, satellites[j].second)最小
    //            int j = 0;
    //            for(int k = 0; k < satellites.size(); k++){
    //                if(abs(t[i]-timeToTimestamp(satellites[k].hour, satellites[k].minute, satellites[k].second)) < abs(t[i]-timeToTimestamp(satellites[j].hour, satellites[j].minute, satellites[j].second))){
    //                        j = k;
    //                }
    //            }
    //            //计算卫星位置
    //            const geph_t geph =  ConvertToGeph(satellites[j]);
    //            double rs[3];     // 卫星位置
    //            double dts[2];    // 卫星钟差
    //            double var;       // 位置和钟差的方差
    //            gtime_t _time;     // 时间
    //            //time赋值为GPST时间
    //            _time = timeadd(geph.toe,t[i]-timeToTimestamp(satellites[j].hour, satellites[j].minute, satellites[j].second));
    //            geph2pos(_time, &geph, rs, dts, &var);
    //            x1(i, 0) = rs[0];
    //            y1(i, 0) = rs[1];
    //            z1(i, 0) = rs[2];
    //        }
    //        //将t转换成tau
    //        for (int i = 0; i < t.size(); i++)
    //        {
    //            double tau = (2 / (delt_t)) * (t[i] - t0) - 1;
    //            t[i] = tau;

    //        }

    //        //切比雪夫多项式矩阵，t的个数*切比雪夫多项式阶数,使用Eigen库
    //        Eigen::MatrixXd T(t.size(), n);
    //        for (int i = 0; i < t.size(); i++)
    //        {
    //            for (int j = 0; j < n; j++)
    //            {
    //                T(i, j) = chebyshevPolynomial(t[i], j);
    //            }
    //        }




    //        //V=TC-X,C为切比雪夫多项式系数，X为卫星坐标，V为残差
    //        //C =(T^T*T)^-1*T^T*X
    //        Eigen::MatrixXd T1 = T.transpose() * T;
    //        Eigen::MatrixXd T2 = T1.inverse();
    //        Eigen::MatrixXd Cx = T2 * T.transpose() * x1;
    //        Eigen::MatrixXd Cy = T2 * T.transpose() * y1;
    //        Eigen::MatrixXd Cz = T2 * T.transpose() * z1;
    //        //计算预测位置
    //        double tau = (2 / (delt_t)) * (target - t0) - 1;
    //        double x = 0;
    //        double y = 0;
    //        double z = 0;
    //        for (int i = 0; i < n; i++)
    //        {
    //            x += Cx(i, 0) * chebyshevPolynomial(tau, i);
    //            y += Cy(i, 0) * chebyshevPolynomial(tau, i);
    //            z += Cz(i, 0) * chebyshevPolynomial(tau, i);
    //        }
    //        position result;
    //        result.x = x;
    //        result.y = y;
    //        result.z = z;
    //        return result;


    //    }

};



#endif // MAINWINDOW_H
