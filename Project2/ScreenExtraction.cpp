#include "ScreenExtraction.h"
#include "main.h"


ScreenExtract::ScreenExtract()
{

}

ScreenExtract::ScreenExtract(Mat srcMat)
{
	setSrc(srcMat);
}

ScreenExtract::~ScreenExtract()
{
}
Mat ScreenExtract::getDst()
{
	return m_MatDstImageStand;
}

bool ScreenExtract::setSrc(Mat srcMat)
{
	m_MatSrcImage = srcMat.clone();
	m_MatEdgeImage = srcMat.clone();
	m_MatHoughImage = srcMat.clone();
	m_MatCornerImage = srcMat.clone();
	Mat temp(1280,720,srcMat.type()); 
	m_MatDstImageLie = temp.clone();
	Mat temp2(720,1280,srcMat.type()); 
	m_MatDstImageStand = temp2.clone();
	return true;
}

Mat ScreenExtract::runExtract()
{
	int cannyThrel=100;
	int failCNT=0;
	while(1)
	{
		m_MatEdgeImage = EdgeDection(m_MatEdgeImage,cannyThrel);
		m_MatHoughImage = HoughLine(m_MatEdgeImage);
		m_MatCornerImage = CornerHarris(m_MatHoughImage,m_PointPerspectiveSrcBuff);
		if(failCNT>20)
		{
			throw("Extract fail");
			return m_MatSrcImage;
		}
		if(m_PointPerspectiveSrcBuff.size() == 8)
		{
			break;
		}
		else if(m_PointPerspectiveSrcBuff.size()>8)
		{
			if(cannyThrel<500)
			{
				cannyThrel+=20;
			}
			failCNT++;
		}
		else if(m_PointPerspectiveSrcBuff.size()<8)
		{
			if(cannyThrel>10)
			{
				cannyThrel-=5;
			}
			failCNT++;
		}
		waitKey(10);
	}

	m_MatDstImageLie = perspectiveChange(m_MatSrcImage,m_PointPerspectiveSrcBuff);
	m_MatDstImageStand = rotation(m_MatDstImageLie,90);
	return m_MatDstImageStand;
} 
Mat ScreenExtract::runFastExtract(Mat srcMat)
{
	m_MatSrcImage = srcMat.clone();
	m_MatDstImageLie = perspectiveChange(m_MatSrcImage,m_PointPerspectiveSrcBuff);
	m_MatDstImageStand = rotation(m_MatDstImageLie,90);
	return m_MatDstImageStand;

}


//private
Mat ScreenExtract::EdgeDection(Mat srcImage,int cannyThrel)
{
	Mat cannyEdge;
	Mat dstImage = m_MatSrcImage.clone(); 

	//降噪
	blur(m_MatSrcImage,cannyEdge,Size(3,3));
#ifdef DEBUG_SHOW_ScreenExtract
	//namedWindow("blur()",CV_WINDOW_AUTOSIZE);
	//imshow(Windows_Edge,g_dstImage);
	imshow("blur()",cannyEdge);	
#endif

	//运行Canny算子
	Canny(cannyEdge,cannyEdge,cannyThrel,cannyThrel*3,3);

	//先将g_dstImage内的所有元素设置为0
	dstImage = Scalar::all(0);

	//使用Canny算子输出的边缘图g_cannyDetectedEdges作为掩码，来将原图g_srcImage拷贝到目标图g_dstImage中
	srcImage.copyTo(dstImage,cannyEdge);

#ifdef DEBUG_SHOW_ScreenExtract
	//namedWindow("EdgeDection()",CV_WINDOW_AUTOSIZE);
	//imshow(Windows_Edge,g_dstImage);
	imshow("EdgeDection()",cannyEdge);	
#endif

	return cannyEdge;
}

Mat ScreenExtract::HoughLine(Mat srcImage)
{
	Mat dstImage = srcImage.clone();  
	//g_houghWithSrc = g_srcImage.clone(); 
	dstImage = Scalar::all(255);

	//houghlinesP
	//vector<Vec4i> mylines;
	//HoughLinesP(g_cannyDetectedEdges,mylines,1,CV_PI/180,valueA+1,valueB,valueC);
	////循环遍历绘制每一条线段  
	//  for( size_t i = 0; i < mylines.size(); i++ )  
	//  {  
	//      Vec4i lines = mylines[i];  
	//	//line（）划线 
	//      line( dstImage, Point(lines[0], lines[1]), Point(lines[2], lines[3]), Scalar(55,100,195), 1,CV_AA);// CV_AA);  
	//  }  

	//houghlines
	vector<Vec2f> mylines;

	HoughLines(srcImage, mylines, 1, (CV_PI-0.2)/180,srcImage.cols/8, 0, 0 );  
	//HoughLines(g_cannyDetectedEdges, mylines, 1, CV_PI/180, valueA+1, valueB, valueC );  

	//依次在图中绘制出每条线段  
    for( size_t i = 0; i < mylines.size(); i++ )  
    {  
        float rho = mylines[i][0], theta = mylines[i][1];  
        Point pt1, pt2;  
        double a = cos(theta), b = sin(theta);  
        double x0 = a*rho, y0 = b*rho;  
        pt1.x = cvRound(x0 + 3000*(-b));  
        pt1.y = cvRound(y0 + 3000*(a));  
        pt2.x = cvRound(x0 - 3000*(-b));  
        pt2.y = cvRound(y0 - 3000*(a));  
        line( dstImage, pt1, pt2, Scalar(0,0,0), 1, CV_AA);  
		//line( g_houghWithSrc, pt1, pt2, Scalar(0,255,0), 1, CV_AA);  
    }

#ifdef DEBUG_SHOW_ScreenExtract
	//显示图片
	//imshow(Windows_Hough,dstImage); 
	//imshow(Windows_Hough,g_houghLine);
	imshow("HoughLine()",dstImage);
#endif

	return dstImage;
}


Mat ScreenExtract::CornerHarris( Mat srcImage ,vector<Point2f>  &vecPoint)  
{  
    //---------------------------【1】定义一些局部变量-----------------------------  
	Mat g_srcImageClone = m_MatSrcImage.clone() ;
	Mat writeImage(m_MatSrcImage.rows,m_MatSrcImage.cols,m_MatSrcImage.type());
    Mat dstImage;//目标图  
    Mat normImage;//归一化后的图  
    Mat scaledImage;//线性变换后的八位无符号整型的图  
	int iCornerThresh = 110;

	//g_CornerWithHoughWithSrc = g_houghWithSrc;
	writeImage = Scalar::all(255);
    //---------------------------【2】初始化---------------------------------------  
    //置零当前需要显示的两幅图，即清除上一次调用此函数时他们的值  
    //dstImage = Mat::zeros( srcImage.size(), CV_32FC1 );  
    //g_srcImageClone=g_srcImage.clone( );  
  
    //---------------------------【3】正式检测-------------------------------------  
    //进行角点检测  
    //cornerHarris( g_srcGrayImage, dstImage, 2, 3, 0.04, BORDER_DEFAULT );  //原图角点检测
	cornerHarris( srcImage, dstImage, 2, 3, 0.04, BORDER_DEFAULT );	//霍夫变换后的角点检测

  
    // 归一化与转换  
    normalize( dstImage, normImage, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );  
    convertScaleAbs( normImage, scaledImage );//将归一化后的图线性变换成8位无符号整型   
  
    //---------------------------【4】进行绘制-------------------------------------  
    // 将检测到的，且符合阈值条件的角点绘制出来  
    for( int j = normImage.rows/100; j < normImage.rows-normImage.rows/100 ; j++ )  
    {
		for( int i = normImage.cols/100; i < normImage.cols-normImage.cols/100; i++ )  
		{  
			if( (int) normImage.at<float>(j,i) > iCornerThresh  )  
			{  
				circle( g_srcImageClone, Point( i, j ), 5,  Scalar(10,10,255), 2, 8, 0 );  
				//circle( g_CornerWithHoughWithSrc, Point( i, j ), 5,  Scalar(10,10,255), 2, 8, 0 );  
				circle( scaledImage, Point( i, j ), 10,  Scalar(0,10,255), -1, 8, 0 );  
				circle( writeImage, Point( i, j ), 10,  Scalar(0,10,255), -1, 8, 0 ); 
			}  
		}  
    }  
	
	vecPoint = GatherPoint( writeImage,iCornerThresh);
#ifdef DEBUG_SHOW_ScreenExtract
    //---------------------------【4】显示最终效果---------------------------------  
	//imshow( "CornerHarris", g_CornerWithHoughWithSrc );  //在原图叠加霍夫图上显示
    //imshow( "CornerHarris", g_srcImageClone );  //在原图上显示
    //imshow( "CornerHarris", scaledImage );  //使用灰度图显示
	imshow( "CornerHarris", writeImage );
#endif
	return scaledImage;
}  

vector<Point2f> ScreenExtract::GatherPoint( Mat srcImage, int CornerThresh ) 
{
	//imshow( "GatherPoint", srcImage );
	//Mat grayImage=srcImage.clone() ;
	Mat canny_output;
	Mat grayImage;
	vector<vector<Point>>contours;
	vector<Vec4i>hierarchy;
	//RNG rng(12345);


	//转成灰度图
	cvtColor(srcImage, grayImage, COLOR_BGR2GRAY);

    //canny边缘检测
    Canny(grayImage, canny_output, 50, 50 * 2, 3);

    //轮廓提取
    findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

    //计算图像矩
    vector<Moments>mu(contours.size());
    for (unsigned int i = 0; i < contours.size(); i++)
    {
        mu[i] = moments(contours[i], false);
    }
    //计算图像的质心
    vector<Point2f>mc(contours.size());
    for (unsigned int i = 0; i < contours.size(); i++)
    {
        mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
    }

    //绘制轮廓
    Mat drawing = Mat::zeros(srcImage.size(), CV_8UC3);
    for (unsigned int i = 0; i < contours.size(); i++)
    {
        Scalar color = Scalar(0, 255, 0);
        drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
        circle(drawing, mc[i], 4, color, -1, 8, 0);
    }
#ifdef DEBUG_SHOW_ScreenExtract
    //namedWindow("GatherPoint", WINDOW_AUTOSIZE);
    imshow("GatherPoint", drawing);
#endif
	
	return mc;
}


Mat ScreenExtract::perspectiveChange(Mat srcMat,vector<Point2f> srcBuff)
{
	//Mat变量
	Mat dstMat(srcMat.rows,srcMat.cols,srcMat.type());
	Mat perspectiveMat( 2,3,CV_32FC1);

	//透视变换参数
	Point2f perspectiveSrcBuff[4];
	Point2f perspectiveDesBuff[4];

	//透视变换坐标设置
	perspectiveSrcBuff[0] = Point2f((srcBuff[0].x+srcBuff[1].x)/2,(srcBuff[0].y+srcBuff[1].y)/2) ;  
    perspectiveSrcBuff[1] = Point2f((srcBuff[2].x+srcBuff[3].x)/2,(srcBuff[2].y+srcBuff[3].y)/2) ;   
    perspectiveSrcBuff[2] = Point2f((srcBuff[4].x+srcBuff[5].x)/2,(srcBuff[4].y+srcBuff[5].y)/2) ;
	perspectiveSrcBuff[3] = Point2f((srcBuff[6].x+srcBuff[7].x)/2,(srcBuff[6].y+srcBuff[7].y)/2) ;

	//求变换后坐标
	for(int i=0;i<3;i++)
	{
		if(0== getPointPlace(srcMat,perspectiveSrcBuff[i]) )
		{
			perspectiveDesBuff[i] = Point2f( 0, 0);  
		}
		else if(1 == getPointPlace(srcMat,perspectiveSrcBuff[i]) )
		{
			perspectiveDesBuff[i] = Point2f( 0, static_cast<float>(dstMat.rows-1)); 		
		}
		else if(2 == getPointPlace(srcMat,perspectiveSrcBuff[i]) )
		{
		    perspectiveDesBuff[i] = Point2f( static_cast<float>(dstMat.cols-1), 0); 
		}
		else if(3 == getPointPlace(srcMat,perspectiveSrcBuff[i]) )
		{
			perspectiveDesBuff[i] = Point2f( static_cast<float>(dstMat.cols-1), static_cast<float>(dstMat.rows-1));
		}
	}
 
	//求透视变换
	perspectiveMat = getPerspectiveTransform( perspectiveSrcBuff, perspectiveDesBuff );  

	//对源图像应用刚刚的透视变换
	warpPerspective(srcMat, dstMat, perspectiveMat, dstMat.size());

#ifdef DEBUG_SHOW_ScreenExtract
	//显示
	imshow( "perspectiveChange", dstMat );
#endif
	return dstMat;
}
int ScreenExtract::getPointPlace(Mat srcImage,Point2f point)
{
	if(point.x < srcImage.cols/2)
	{
		if(point.y < srcImage.rows/2)
		{
			return 0; 
		}
		else
		{
			return 1;
		}
	}
	else
	{
		if(point.y < srcImage.rows/2)
		{
			return 2; 
		}
		else
		{
			return 3;
		}	
	}
}


Mat ScreenExtract::rotation(Mat srcMat,float degree)
{
	Mat dstImage(srcMat.rows,srcMat.cols,srcMat.type());
	int len = max(srcMat.cols, srcMat.rows);
    Point2f pt(len/2.f,len/2.f);
    Mat r = getRotationMatrix2D(pt,degree,1.0);
    warpAffine(srcMat,dstImage,r,Size(srcMat.rows,srcMat.cols));

	return dstImage;
}

