#include<opencv2/opencv.hpp>
#include<iostream>
#include<vector>
#include<cmath>
using namespace std;
using namespace cv;

void show(Mat img, string name)//创建窗口
{
    namedWindow(name, 0);
    imshow(name, img);
    return;
}

void drawrect(Mat &img, RotatedRect ob)
{
    Point2f v[4];
    ob.points(v);
    for (int i = 0; i < 4; i++)
        line(img, v[i], v[(i + 1) % 4], Scalar(0, 255, 0), 1);
    return;
}

void drawtarget(Mat &img, RotatedRect ob1, RotatedRect ob2)
{
    Point2f v1[4], v2[4];
    ob1.points(v1);
    ob2.points(v2);
    line(img, v1[0], v1[1], Scalar(0, 255, 0), 1);
    line(img, v1[0], v2[3], Scalar(0, 255, 0), 1);
    line(img, v2[2], v1[1], Scalar(0, 255, 0), 1);
    line(img, v2[2], v2[3], Scalar(0, 255, 0), 1);
    return;
}

Mat pretreat(Mat raw)
{
    Mat grey;
    cvtColor(raw, grey, COLOR_BGR2GRAY);//转化为灰度图
    Mat bigrey;
    threshold(grey, bigrey, 125, 255, THRESH_BINARY);//转化为二值图
    Mat channel[3];
    split(raw, channel);//将原图rgb分离
    Mat sub_br;
    subtract(channel[0], channel[2], sub_br);//将b通道减去r通道
    threshold(sub_br, sub_br, 115, 255, THRESH_BINARY);//将相减后的图片二值化
    Mat element1 = getStructuringElement(MORPH_RECT, Size(2, 2));
    dilate(sub_br, sub_br, element1);//膨胀
    Mat ret;
    ret = sub_br & bigrey;//用原图 灰度二值 之后的图片与（&）上原图 通道相减二值膨胀 之后的图片，提取出较为可信的候选区域
    Mat element2 = getStructuringElement(MORPH_RECT, Size(3,3));
    dilate(ret, ret, element2);//再次膨胀
    return ret;
}

void fixarmor(Mat &raw,Mat img)
{
    vector<vector<Point>> contour;
    vector<RotatedRect> rect,finall;
    vector<pair<RotatedRect, RotatedRect>> ret;
    findContours(img, contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);//找轮廓
    for (auto x : contour)//储存轮廓
    {
        RotatedRect temp = minAreaRect(x);
        rect.push_back(temp);
    }
    for (auto x : rect)   //利用角度，面积，长宽比筛选灯条
    {
        for (auto y : rect)
        {
            double xangle, yangle, xarea, yarea, xrate, yrate;
            xarea = x.size.area();
            xangle = x.angle;
            yarea = y.size.area();
            yangle = y.angle;
            xrate = x.size.height / x.size.width;
            yrate = y.size.height / y.size.width;
            if (xarea == yarea) continue;
            if (xangle > 5 || yangle > 5) continue;
            if (xarea < 35 || yarea < 35) continue;
            if (fabs(xarea - yarea) < 8 && fabs(xangle - yangle) < 35
                && fabs(xrate - yrate) < 0.1&&fabs(x.size.height - y.size.height) < 5
                && fabs(x.size.width - y.size.width) < 10)
                ret.push_back({ x,y });
        }
    }
    for (auto x : ret) { drawrect(raw, x.first); drawrect(raw, x.second);}
    for (auto x : ret) drawtarget(raw, x.first, x.second);
    return;
}

//主函数
int main()
{
    Mat image = imread("D:\\week3\\armour.jpg");
    Mat temp = pretreat(image);
    fixarmor(image,temp);
    show(image, "armour");
    waitKey(0);
    return 0;
}
