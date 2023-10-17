#ifndef SAT_H
#define SAT_H
#include <string>
#include <iostream>
#include <vector>
//定义一个卫星的结构体

struct Satellite {
    std::string type; // 卫星类型
    int prn; // 伪随机噪声码
    int year; // 年份
    int month; // 月份
    int day; // 日期
    int hour; // 小时
    int minute; // 分钟
    double second; // 秒
    double clock_bias; // 钟差
    double clock_drift; // 钟偏
    double clock_drift_rate; // 钟偏变化率
    double iode; // 数据龄期
    double crs; // 轨道半径正弦调和项
    double delta_n; // 平均角速度改正项
    double m0; // 参考时刻平近点角
    double cuc; // 升交距角余弦调和项
    double e; // 偏心率
    double cus; // 升交距角正弦调和项
    double sqrt_a; // 轨道长半轴平方根
    double toe; // 参考时刻
    double cic; // 轨道倾角余弦调和项
    double omega0; // 参考时刻升交点赤经
    double cis; // 轨道倾角正弦调和项
    double i0; // 参考时刻轨道倾角
    double crc; // 轨道半径余弦调和项
    double omega; // 近地点角距
    double omega_dot; // 升交点赤经变化率
    double idot; // 轨道倾角变化率
    double codes_l2_channel; // L2频道C/A码标识
    double gps_week_number; // GPS周数
    double accuracy_index; // 精度指数
    double health_status_bitmask; // 卫星健康状态掩码
    double tgd; // 电离层延迟
    double iodc; // 星钟数据质量
    double transmission_time_of_message_sow;// 信息发射时间
    double calculate_x = 0;//解算坐标x
    double calculate_y = 0;//解算坐标y
    double calculate_z = 0;//解算坐标z
    double precise_x =0;//sp3文件坐标x
    double precise_y =0;//sp3文件坐标y
    double precise_z =0;//sp3文件坐标z
    //方位角
    double azimuth = 0;
    //高度角
    double elevation = 0;

};
struct Satellite_GLO{
    std::string type; // 卫星类型
    int prn; // 伪随机噪声码
    int year; // 年份
    int month; // 月份
    int day; // 日期
    int hour; // 小时
    int minute; // 分钟
    double second; // 秒
    double clock_bias; // 钟差
    double clock_drift; // 钟偏
    double x;//卫星位置
    double x_speed;//卫星速度
    double x_acceleration;//卫星加速度
    double y;//卫星位置
    double y_speed;//卫星速度
    double y_acceleration;//卫星加速度
    double z;//卫星位置
    double z_speed;//卫星速度
    double z_acceleration;//卫星加速度
    double calculate_x = 0;//解算坐标x
    double calculate_y = 0;//解算坐标y
    double calculate_z = 0;//解算坐标z
    double precise_x =0;//sp3文件坐标x
    double precise_y =0;//sp3文件坐标y
    double precise_z =0;//sp3文件坐标z
    //方位角
    double azimuth = 0;
    //高度角
    double elevation = 0;

};
struct precise_Satellite{
    std::string type;
    int prn;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    double second;
    long double precise_x;
    long double precise_y;
    long double precise_z;
};

#endif // SAT_H
