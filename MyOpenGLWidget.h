#ifndef MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QDebug>
#include "sat.h"
#include "GL/freeglut.h"
#define M_PI 3.14159265358979323846

class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:

    MyOpenGLWidget(QWidget  *parent) : QOpenGLWidget(parent) {}
    ~MyOpenGLWidget() {}
  //绘制卫星点
    void drawSat(Satellite sat) {
        //不渲染高度角小于0的卫星
        if (sat.elevation < 0)
        {
            return;
        }
        //绘制卫星点到画布上，画布中心点为高度角90度，画布边缘圆为高度角0度，方位角从北方向开始，逆时针增加，0度为北方向，90度为东方向，180度为南方向，270度为西方向，360度为北方向
        //计算卫星点在画布上的位置
        double ele = sat.elevation;
        double azi = sat.azimuth;
        //将ele归化到r,范围为0-9
        double r = (90.0 - ele) / 10.0;
        //打印r
        std::cout << "r:" << r << std::endl;
        double x = r * sin(azi * M_PI / 180.0);
        double y = r * cos(azi * M_PI / 180.0);

        //绘制卫星点,大小为5
        glPointSize(5.0f);
        glBegin(GL_POINTS);
        if (sat.type == "GPS")
        {
            //蓝色
            glColor3f(0.0f, 0.0f, 1.0f);
        }else if (sat.type == "BDS")
        {
            glColor3f(1.0f, 0.0f, 0.0f);
        }else if (sat.type == "GAL")
        {   //紫色,不是粉色
            glColor3f(0.5f, 0.0f, 0.5f);
        }else{
            //黑色
            glColor3f(0.0f, 0.0f, 0.0f);
        }
        glVertex2f(x, y);
        glEnd();
        //绘制卫星type和prn
        glRasterPos2f(x + 0.2f, y + 0.2f);

        //GPS用G表示，BDS用C表示，GAL用E表示，GLO用R表示，QZSS用J表示
        if (sat.type == "GPS")
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'G');
        }
        else if (sat.type == "BDS")
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'C');
        }
        else if (sat.type == "GAL")
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'E');
        }
        else if (sat.type == "GLO")
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'R');
        }
        else
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'J');
        }
        for (int i = 0; i < std::to_string(sat.prn).length(); i++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, std::to_string(sat.prn)[i]);
        }
        //打印卫星的方位角和高度角
       // std::cout << "卫星" << sat.prn << "的方位角为：" << sat.azimuth << "，高度角为：" << sat.elevation << std::endl;
//        qDebug() << "卫星" << sat.prn << "的方位角为：" << sat.azimuth << "，高度角为：" << sat.elevation;







    }
    //更新显示
    void updateOpenGLContent(std::vector<Satellite> sats1) {
        // 在这里更新OpenGL的显示内容
        qDebug() << "updateOpenGLContent";
        this->sats = sats1;
        //调用paintGL函数
        update();
    }

protected:
    std::vector<Satellite> sats;


    void initializeGL() override {


        // 初始化OpenGL上下文
        initializeOpenGLFunctions();
        qDebug() << "initializeGL";
    }

    void paintGL() override {
        qDebug() << "paintGL";

        //设置背景颜色为白色
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //清除颜色缓冲区
        glClear(GL_COLOR_BUFFER_BIT);
        drawCoordinateSystem();
        drawSatellites();



    }

    void resizeGL(int w, int h) override {
        // 设置OpenGL视口和投影矩阵
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-11.0, 11.0, -11.0, 11.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
    void drawCoordinateSystem()
    {
        // 绘制坐标系的代码
        //绘制东南西北坐标系
        glBegin(GL_LINES);
        //x轴
        //灰线
        glColor3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-9.0f, 0.0f, 0.0f);
        glVertex3f(9.0f, 0.0f, 0.0f);
        //y轴
        glColor3f(0.5f, 0.5f, 0.5f);
        glVertex3f(0.0f, -9.0f, 0.0f);
        glVertex3f(0.0f, 9.0f, 0.0f);
        glEnd();

        //绘制一个圆，圆心在原点，半径为9
        glBegin(GL_LINE_LOOP);
        glColor3f(0.5f, 0.5f, 0.5f);
        for (int i = 0; i < 360; i++)
        {
            float theta = i * M_PI / 180.0f;
            glVertex2f(9.0f * cos(theta), 9.0f * sin(theta));
        }
        glEnd();
            //绘制一个圆，圆心在原点，半径为3
        glBegin(GL_LINE_LOOP);
        glColor3f(0.5f, 0.5f, 0.5f);
        for (int i = 0; i < 360; i++)
        {
            float theta = i * M_PI / 180.0f;
            glVertex2f(3.0f * cos(theta), 3.0f * sin(theta));
        }
        glEnd();
        //绘制一个圆，圆心在原点，半径为6
        glBegin(GL_LINE_LOOP);
        glColor3f(0.5f, 0.5f, 0.5f);
        for (int i = 0; i < 360; i++)
        {
            float theta = i * M_PI / 180.0f;
            glVertex2f(6.0f * cos(theta), 6.0f * sin(theta));
        }
        glEnd();

        //绘制一个点，坐标为(9,0)
        glBegin(GL_POINTS);
        glColor3f(0.5f, 0.5f, 0.5f);
        glVertex2f(9.0f, 0.0f);
        glEnd();
        //绘制东南西北文字
        glRasterPos2f(9.5f, -0.5f);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'E');
        glRasterPos2f(-10.5f, -0.5f);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'W');
        glRasterPos2f(-0.5f, 9.5f);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'N');
        glRasterPos2f(-0.5f, -10.5f);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'S');
    }

    void drawSatellites()
    {
        // 绘制卫星的代码
        std::cout<<this->sats.size()<<std::endl;
        //绘制卫星点
        for (int i = 0; i < this->sats.size(); i++)
        {

            //绘制卫星点
            Satellite sat = this->sats[i];
            drawSat(sat);
        }
    }
};




#endif // MYOPENGLWIDGET_H
