#include "ImageMatch.h"


ImageMatch::ImageMatch(Mat srcImage)
{	
	m_MatSrcImage = srcImage.clone();
	m_MatMatchImage = srcImage.clone();
	m_MatDstImage = srcImage.clone();
	m_iFeatureMaxSize = 50;
}

ImageMatch::~ImageMatch()
{	

}
/*******************************************************************************
* Function Name  : addObjectFeatureImage
* Description    : add an objcet feature to feature vector
* Input          : None
* Output         : None
* Return         : succeed/failed
*******************************************************************************/
bool ImageMatch::setSrcImage(Mat srcImage)
{
	m_MatSrcImage = srcImage.clone();
	return true;	
}
/*******************************************************************************
* Function Name  : addObjectFeatureImage
* Description    : add an objcet feature to feature vector
* Input          : None
* Output         : None
* Return         : succeed/failed
*******************************************************************************/
bool ImageMatch::addCubeFeatureImage(Mat image)
{
	if(m_vecCubeFeatureImage.size()<m_iFeatureMaxSize)
	{
		m_vecCubeFeatureImage.push_back(image);
		return true;
	}

	return false;
}
bool ImageMatch::addCubeFeatureImage(vector<Mat> vec)
{

	if(m_vecCubeFeatureImage.size()+vec.size()<m_iFeatureMaxSize)
	{
		m_vecCubeFeatureImage.insert(m_vecCubeFeatureImage.end(),vec.begin(),vec.end());
		return true;
	}

	return false;	
}

bool ImageMatch::addConfusingFeatureImage(Mat image)
{
	if(m_vecConfusingFeatureImage.size()<m_iFeatureMaxSize)
	{
		m_vecConfusingFeatureImage.push_back(image);
		return true;
	}
	return false;	
}
bool ImageMatch::addConfusingFeatureImage(vector<Mat> vec)
{

	if(m_vecConfusingFeatureImage.size()+vec.size()<m_iFeatureMaxSize)
	{
		m_vecConfusingFeatureImage.insert(m_vecConfusingFeatureImage.end(),vec.begin(),vec.end());
		return true;
	}

	return false;	
}
/*******************************************************************************
* Function Name  : setPersonFeature
* Description    : set the person feature
* Input          : None
* Output         : None
* Return         : succeed/failed
*******************************************************************************/
bool ImageMatch::setPersonFeature(Mat image)
{
	m_MatPersonFeatureImage = image.clone();
	return true;
}

/*******************************************************************************
* Function Name  : runMatch
* Description    : Match main programm
* Input          : None
* Output         : None
* Return         : destination image
*******************************************************************************/
Mat ImageMatch::runMatch()
{
	//【0】预处理
	Mat dstImage = m_MatSrcImage.clone();
	Mat srcImage_Adjust= AdjectDefinition(m_MatSrcImage,100,0);


	//【1】匹配人物
	m_pLocPerson = SearchPerson( srcImage_Adjust, 10, (int)(srcImage_Adjust.rows*0.35), srcImage_Adjust.cols-20, (int)(srcImage_Adjust.rows*0.35) );
#ifdef DEBUG_SHOW_ImageMatch
	//rectangle( srcImage_Adjust,  Rect(loc,  Size(m_MatPersonFeatureImage.cols, m_MatPersonFeatureImage.rows) ), Scalar(0, 0, 255), 2, 8, 0 );
	circle( srcImage_Adjust, m_pLocPerson, 10,  Scalar(0,0,255), 5, 8, 0 );  
	//imshow("TempleMatch",srcImage_Adjust);
#endif

	//【2】匹配建筑
	if(m_pLocPerson.x < srcImage_Adjust.cols*0.5)//人物在左边，搜索右半区域，为了加速
		m_pLocBlock = SearchBuilding( srcImage_Adjust, (int)(srcImage_Adjust.cols*0.3)+10, (int)(srcImage_Adjust.rows*0.20), (int)(srcImage_Adjust.cols*0.7)-20, (int)(srcImage_Adjust.rows*0.5));
	else//搜索左半区
		m_pLocBlock = SearchBuilding( srcImage_Adjust , 10, (int)(srcImage_Adjust.rows*0.20), (int)(srcImage_Adjust.cols*0.7)-20, (int)(srcImage_Adjust.rows*0.5));
#ifdef DEBUG_SHOW_ImageMatch
	circle( srcImage_Adjust, m_pLocBlock, 20,  Scalar(0,255,0), 5, 8, 0 );  

	imshow("TempleMatch",srcImage_Adjust);
#endif

	m_fDistance	= calculateDistance(m_pLocPerson,m_pLocBlock);

	return dstImage; 
}
/*******************************************************************************
* Function Name  : changeSrcAndrunMatch
* Description    : Change src image and run Match
* Input          : None
* Output         : None
* Return         : destination image
*******************************************************************************/
Mat ImageMatch::changeSrcAndrunMatch(Mat srcImage)
{
	setSrcImage(srcImage);
	return runMatch();
}
float ImageMatch::getDistance()
{
	float result = m_fDistance*2.1f+0;
	if(result<0)
		result=0;
	return result;
}
//private
/*******************************************************************************
* Function Name  : vecFeatureImageToVecFeatureImage
* Description    : update definition
* Input          : None
* Output         : None
* Return         : destination image
*******************************************************************************/
bool ImageMatch::FeatureVectorChangeToEdgeVector()
{
	m_vecFeatureEdgeImage.clear();
	for(unsigned int i=0;i<m_vecCubeFeatureImage.size();i++)
	{
		//m_MatSrcImage = FeatureMatchAndMark(m_vecCubeFeatureImage[i],m_MatSrcImage);
		m_vecFeatureEdgeImage.push_back ( calEdge(m_vecCubeFeatureImage[i],10));
#ifdef DEBUG_SHOW_ImageMatch
		imshow("Edge",m_vecFeatureEdgeImage[i]);
#endif
	}	
	return true;
}
/*******************************************************************************
* Function Name  : definition
* Description    : update definition
* Input          : None
* Output         : None
* Return         : destination image
*******************************************************************************/
Mat ImageMatch::AdjectDefinition(Mat srcImage,int contrast,int brightness)
{
	Mat dstImage;
	//平滑
	blur(srcImage,dstImage,Size(3,3));

	//对比度亮度调节
	for(int y = 0; y < dstImage.rows; y++ ) //遍历图片的纵坐标 
	{  
		for(int x = 0; x < dstImage.cols; x++ )//遍历图片的横坐标  
		{  
			for(int c = 0; c < 3; c++ )  //分开图像的RGB
			{  
				//对比度在0-300之间，所以乘以0.01,并用saturate_cast把计算结果转换成uchar类型
				dstImage.at<Vec3b>(y,x)[c]= saturate_cast<uchar>( (contrast*0.01)*(dstImage.at<Vec3b>(y,x)[c] ) + brightness );  
			}  
		}  
	}  
	//	imshow("src",srcImage);
	//	imshow("dst",dstImage);

	//二次平滑
	//blur(dstImage,dstImage,Size(3,3)); 

	return dstImage;
}
/*******************************************************************************
* Function Name  : addObjectFeatureImage
* Description    : add an objcet feature to feature vector
* Input          : None
* Output         : None
* Return         : succeed/failed
*******************************************************************************/
Mat ImageMatch::calEdge(Mat srcImage,int threl)
{
	Mat dstImage;
	//canny
	Canny(srcImage,dstImage,threl,threl*3,3);

#ifdef DEBUG_SHOW_ImageMatch
	imshow("calEdge",dstImage);

#endif
	return dstImage;
}

/*******************************************************************************
* Function Name  : SearchPerson
* Description    : find block point
* Input          : None
* Output         : None
* Return         : destination image
*******************************************************************************/
Point ImageMatch::SearchPerson(Mat TargetImage,int ROI_Xstart,int ROI_Ystart,int ROI_length,int ROI_heigh )
{
	//【0】局部变量用于返回人物坐标
	Point dstLoc;

	//【1】设置ROI区域
	Mat imageROI=TargetImage( Rect(ROI_Xstart, ROI_Ystart, ROI_length, ROI_heigh ) );	

	//【2】模板匹配
	float matchaValue = (float)TempleMatch(m_MatPersonFeatureImage, imageROI, dstLoc, CV_TM_CCOEFF_NORMED) ;
	if( matchaValue < 0.64)
	{
		throw("Person match fail");
	}
	else
	{
#ifdef DEBUG_SHOW_ImageMatch
		//rectangle( imageROI,  Rect(dstLoc,  Size(m_MatPersonFeatureImage.cols, m_MatPersonFeatureImage.rows) ), Scalar(0, 0, 255), 2, 8, 0 );
		//imshow("SearchqPerson",imageROI);
#endif
		//根据输入的ROI设置纠正坐标
		dstLoc.x+=ROI_Xstart;
		dstLoc.y+=ROI_Ystart;
		//让坐标重新纠正道指向人物与地面的接触中点
		dstLoc.x += (int)(m_MatPersonFeatureImage.cols/2);
		dstLoc.y += (int)(m_MatPersonFeatureImage.rows*0.8);
	}
	return dstLoc;
}

/*******************************************************************************
* Function Name  : FindBuilding
* Description    : find block point
* Input          : None
* Output         : None
* Return         : destination image
*******************************************************************************/
Point ImageMatch::SearchBuilding(Mat TargetImage, int ROI_Xstart,int ROI_Ystart,int ROI_length,int ROI_heigh)
{
	//【0】定义两个容器用于存放每个矩形特征的最佳匹配点和匹配值
	vector<Point> vecCubePoint(m_vecCubeFeatureImage.size());
	vector<float> vecCubeValue(m_vecCubeFeatureImage.size());
	//【0】定义一些阈值
	float completelyMatchThrel = 0.70f;//认为一种图形完全匹配的阈值
	float LowestThrel = 40;//认为基本相近的阈值
	//【0】定义一些临时变量
	Point loc;
	float matchValue;
	//【0】最终配置相关变量
	bool matchSucceed=false;//是否找到完全匹配特征标记
	Point lastLoc(0,0);//最终位置
	float lastValue;//最终匹配值
	int lastIndex=-1;//最终位置对应的特征序号

	//【0】计时
	TimeOperation time;
	int ms=0;

	//【1】转为灰度图
	cvtColor(TargetImage,TargetImage,CV_BGR2GRAY);

	//【2】设置ROI区域
	Mat imageROI=TargetImage( Rect(ROI_Xstart, ROI_Ystart, ROI_length, ROI_heigh ) );	


	//【3】匹配建筑
	for(unsigned int i=0;i<m_vecCubeFeatureImage.size();i++)
	{
		matchValue = (float)TempleMatch(m_vecCubeFeatureImage[i],imageROI,loc,CV_TM_CCOEFF_NORMED );
		if(matchValue > completelyMatchThrel)
		{
			lastLoc = loc ;
			//纠正坐标
			lastLoc.x += ROI_Xstart;
			lastLoc.y += ROI_Ystart;
			lastLoc.x += (int)(m_vecCubeFeatureImage[i].cols*0.5);
			lastLoc.y += (int)(m_vecCubeFeatureImage[i].rows*0.3);
			//若是任务脚底的建筑，继续找
			if(calculateDistance(lastLoc,m_pLocPerson)<LowestThrel+30)
			{
				lastLoc.x=0;
				lastLoc.y=0;
				continue;
			}
			matchSucceed = true;
			lastValue = matchValue;
			lastIndex = i;
#ifdef DEBUG_SHOW_ImageMatch
			imshow("matchfeature",m_vecCubeFeatureImage[i]);
			rectangle( imageROI,  Rect(loc,  Size(m_vecCubeFeatureImage[i].cols, m_vecCubeFeatureImage[i].rows) ), Scalar(0, 0, 255), 2, 8, 0 );
			imshow("SearchPerson",imageROI);
#endif
			break;
		}
		else
		{
			vecCubePoint[i]=loc;
			vecCubeValue[i]=matchValue;
		}
	}

	////【4】没有完全匹配，寻找最佳匹配
	if(!matchSucceed)
	{
		for(unsigned int i=0;i<m_vecCubeFeatureImage.size();i++)
		{
			Mat featureROI;
			if(m_pLocPerson.x<TargetImage.cols/2)//取特征的右边
			{
				featureROI =m_vecCubeFeatureImage[i]( Rect(m_vecCubeFeatureImage[i].cols*0.4, 0, m_vecCubeFeatureImage[i].cols*0.6-1, m_vecCubeFeatureImage[i].rows ) );	
				matchValue = (float)TempleMatch(featureROI,imageROI,loc,CV_TM_CCOEFF_NORMED );
				if(matchValue > completelyMatchThrel)
				{
					lastLoc = loc ;
					//纠正坐标
					lastLoc.x += ROI_Xstart;
					lastLoc.y += ROI_Ystart;
					lastLoc.x += (int)(m_vecCubeFeatureImage[i].cols*(0.5-0.4));
					lastLoc.y += (int)(m_vecCubeFeatureImage[i].rows*0.3);
					//若是任务脚底的建筑，继续找
					if(calculateDistance(lastLoc,m_pLocPerson)<LowestThrel)
					{
						lastLoc.x=0;
						lastLoc.y=0;
						continue;
					}
					matchSucceed = true;
					lastValue = matchValue;
					lastIndex = i;
#ifdef DEBUG_SHOW_ImageMatch
					imshow("matchfeature",m_vecCubeFeatureImage[i]);
					rectangle( imageROI,  Rect(loc,  Size(m_vecCubeFeatureImage[i].cols, m_vecCubeFeatureImage[i].rows) ), Scalar(0, 0, 255), 2, 8, 0 );
					imshow("SearchPerson",imageROI);
#endif
					break;
				}
			}
			else
			{
				featureROI =m_vecCubeFeatureImage[i]( Rect(0, 0, m_vecCubeFeatureImage[i].cols*0.6-1, m_vecCubeFeatureImage[i].rows ) );
				matchValue = (float)TempleMatch(featureROI,imageROI,loc,CV_TM_CCOEFF_NORMED );
				if(matchValue > completelyMatchThrel)
				{
					lastLoc = loc ;
					//纠正坐标
					lastLoc.x += ROI_Xstart;
					lastLoc.y += ROI_Ystart;
					lastLoc.x += (int)(m_vecCubeFeatureImage[i].cols*0.5);
					lastLoc.y += (int)(m_vecCubeFeatureImage[i].rows*0.3);
					//若是任务脚底的建筑，继续找
					if(calculateDistance(lastLoc,m_pLocPerson)<LowestThrel)
					{
						lastLoc.x=0;
						lastLoc.y=0;
						continue;
					}
					matchSucceed = true;
					lastValue = matchValue;
					lastIndex = i;
#ifdef DEBUG_SHOW_ImageMatch
					imshow("matchfeature",m_vecCubeFeatureImage[i]);
					rectangle( imageROI,  Rect(loc,  Size(m_vecCubeFeatureImage[i].cols, m_vecCubeFeatureImage[i].rows) ), Scalar(0, 0, 255), 2, 8, 0 );
					imshow("SearchPerson",imageROI);
#endif
					break;
				}
			}

		}
	}
	//易混淆图片匹配
	for(unsigned int i=0;i<m_vecConfusingFeatureImage.size();i++)
	{
		matchValue = (float)TempleMatch(m_vecConfusingFeatureImage[i],imageROI,loc,CV_TM_CCOEFF_NORMED );
		if(matchValue > 0.95)
		{
			lastLoc = loc ;
			//纠正坐标
			lastLoc.x += ROI_Xstart;
			lastLoc.y += ROI_Ystart;
			lastLoc.x += (int)(m_vecConfusingFeatureImage[i].cols*0.5);
			lastLoc.y += (int)(m_vecConfusingFeatureImage[i].rows*0.3);
			//若是任务脚底的建筑，继续找
			if(calculateDistance(lastLoc,m_pLocPerson)<LowestThrel+30)
			{
				lastLoc.x=0;
				lastLoc.y=0;
				continue;
			}
			matchSucceed = true;
			lastValue = matchValue;
			lastIndex = i;
#ifdef DEBUG_SHOW_ImageMatch
			imshow("matchfeature",m_vecConfusingFeatureImage[i]);
			rectangle( imageROI,  Rect(loc,  Size(m_vecConfusingFeatureImage[i].cols, m_vecConfusingFeatureImage[i].rows) ), Scalar(0, 0, 255), 2, 8, 0 );
			imshow("SearchPerson",imageROI);
#endif
			break;
		}
		else
		{
			vecCubePoint[i]=loc;
			vecCubeValue[i]=matchValue;
		}
	}

	if(!matchSucceed)
	{
		float bestValue=0;
		int bestIndex = 0;
		unsigned int i=0;

		bestValue = vecCubeValue[0];
		for(i=0;i<vecCubeValue.size();i++)
		{
			if(vecCubeValue[i]>bestValue)
			{
				bestValue = vecCubeValue[i];
				bestIndex = i;
			}
		}

#ifdef DEBUG_SHOW_ImageMatch
		imshow("matchfeature",m_vecCubeFeatureImage[bestIndex]);
#endif
		lastLoc = vecCubePoint[bestIndex];
		lastIndex=bestIndex;
	}
	return lastLoc;
}
/*******************************************************************************
* Function Name  : calculateDistance
* Description    : Temple match ,Get the best matching point and return it
* Input          : None
* Output         : None
* Return         : Point:best matching point.
*******************************************************************************/
float ImageMatch::calculateDistance(Point A,Point B)
{
	float distance;    
	distance = powf((float)(B.x - A.x),2) + powf((float)(B.y - A.y),2);    
	distance = sqrtf(distance);
	return distance;
}
/*******************************************************************************
* Function Name  : TempleMatch
* Description    : Temple match ,Get the best matching point and return it
* Input          : None
* Output         : None
* Return         : Point:best matching point.
*******************************************************************************/
double ImageMatch::TempleMatch( Mat tepl ,Mat image, Point &point,int method)
{
	int result_cols =  image.cols - tepl.cols + 1;
	int result_rows = image.rows - tepl.rows + 1;

	//模板检测
	Mat result =  Mat( result_cols, result_rows, CV_32FC1 );
	matchTemplate( image, tepl, result, method );

	//寻找最佳值
	double minVal, maxVal;
	Point minLoc, maxLoc;
	minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

	//根据方法返回最佳值
	if(CV_TM_SQDIFF == method || CV_TM_SQDIFF_NORMED == method)
	{
		point =  minLoc;
		return minVal;
	}
	else
	{
		point = maxLoc;
		return maxVal;
	}
	return 0;
}