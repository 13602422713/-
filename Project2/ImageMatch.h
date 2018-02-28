#ifndef __IMAGEMATCH_H
#define __IMAGEMATCH_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>  
  
#include <iostream> 
#include <vector>  
#include "TimeOperation.h"
  
#define DEBUG_SHOW_ImageMatch

using namespace std;
using namespace cv;

class ImageMatch
{
public:
	ImageMatch(Mat srcImage);
	~ImageMatch();
	bool setSrcImage(Mat srcImage);
	bool addCubeFeatureImage(Mat image);
	bool addCubeFeatureImage(vector<Mat> vec);
	bool addConfusingFeatureImage(Mat image);
	bool addConfusingFeatureImage(vector<Mat> vec);
	bool setPersonFeature(Mat image);

	Mat runMatch();
	Mat changeSrcAndrunMatch(Mat srcImage);
	float getDistance();
private:
	bool FeatureVectorChangeToEdgeVector();
	Mat AdjectDefinition(Mat srcImage,int contrast=100,int brightness=0);
	Mat calEdge(Mat srcImage,int threl);
	Mat FeatureMatchAndMark(Mat FeatureImage,Mat TargetImage);
	Point SearchPerson(Mat TargetImage,int ROI_Xstart,int ROI_Ystart,int ROI_length,int ROI_heigh );
	Point SearchBuilding(Mat TargetImage, int ROI_Xstart,int ROI_Ystart,int ROI_length,int ROI_heigh);
	float calculateDistance(Point A,Point B);
private:
	double TempleMatch( Mat tepl ,Mat image, Point &point,int method);
private:
	vector<Mat> m_vecCubeFeatureImage;
	vector<Mat> m_vecConfusingFeatureImage;
	vector<Mat> m_vecFeatureEdgeImage;
	unsigned int m_iFeatureMaxSize;
	Mat m_MatPersonFeatureImage;
	Mat m_MatSrcImage;
	Mat m_MatMatchImage;
	Mat m_MatDstImage;
	float m_fDistance;
	Point m_pLocPerson;
	Point m_pLocBlock;
};


#endif


