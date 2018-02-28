#ifndef __SCREENEXTRACTION_H
#define __SCREENEXTRACTION_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
/* 
#include <opencv2/nonfree/nonfree.hpp>  
#include<opencv2/legacy/legacy.hpp> */ 
#include <iostream>  
using namespace cv;
using namespace std;

#define DEBUG_SHOW_ScreenExtract

class ScreenExtract
{
public:
	ScreenExtract();
	ScreenExtract(Mat srcMat);
	~ScreenExtract();
	Mat getDst();

	bool setSrc(Mat srcMat);
	Mat runExtract();
	Mat runFastExtract(Mat srcMat);
private:
	Mat EdgeDection(Mat srcImage,int cannyThrel);
	Mat HoughLine(Mat srcImage);
	Mat CornerHarris( Mat srcImage,vector<Point2f> &vecPoint) ;
	vector<Point2f> GatherPoint( Mat srcImage, int CornerThresh ) ;
	Mat perspectiveChange(Mat srcMat,vector<Point2f> srcBuff);
	int ScreenExtract::getPointPlace(Mat srcImage,Point2f point);//获取点在屏幕的位置
	Mat rotation(Mat srcImage,float degree);
private:
	Mat m_MatSrcImage;
	Mat m_MatEdgeImage;
	Mat m_MatHoughImage;
	Mat m_MatCornerImage;
	Mat m_MatDstImageLie;
	Mat m_MatDstImageStand;
	vector<Point2f> m_PointPerspectiveSrcBuff;
};


#endif

