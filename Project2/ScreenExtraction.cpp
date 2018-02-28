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

	//����
	blur(m_MatSrcImage,cannyEdge,Size(3,3));
#ifdef DEBUG_SHOW_ScreenExtract
	//namedWindow("blur()",CV_WINDOW_AUTOSIZE);
	//imshow(Windows_Edge,g_dstImage);
	imshow("blur()",cannyEdge);	
#endif

	//����Canny����
	Canny(cannyEdge,cannyEdge,cannyThrel,cannyThrel*3,3);

	//�Ƚ�g_dstImage�ڵ�����Ԫ������Ϊ0
	dstImage = Scalar::all(0);

	//ʹ��Canny��������ı�Եͼg_cannyDetectedEdges��Ϊ���룬����ԭͼg_srcImage������Ŀ��ͼg_dstImage��
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
	////ѭ����������ÿһ���߶�  
	//  for( size_t i = 0; i < mylines.size(); i++ )  
	//  {  
	//      Vec4i lines = mylines[i];  
	//	//line�������� 
	//      line( dstImage, Point(lines[0], lines[1]), Point(lines[2], lines[3]), Scalar(55,100,195), 1,CV_AA);// CV_AA);  
	//  }  

	//houghlines
	vector<Vec2f> mylines;

	HoughLines(srcImage, mylines, 1, (CV_PI-0.2)/180,srcImage.cols/8, 0, 0 );  
	//HoughLines(g_cannyDetectedEdges, mylines, 1, CV_PI/180, valueA+1, valueB, valueC );  

	//������ͼ�л��Ƴ�ÿ���߶�  
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
	//��ʾͼƬ
	//imshow(Windows_Hough,dstImage); 
	//imshow(Windows_Hough,g_houghLine);
	imshow("HoughLine()",dstImage);
#endif

	return dstImage;
}


Mat ScreenExtract::CornerHarris( Mat srcImage ,vector<Point2f>  &vecPoint)  
{  
    //---------------------------��1������һЩ�ֲ�����-----------------------------  
	Mat g_srcImageClone = m_MatSrcImage.clone() ;
	Mat writeImage(m_MatSrcImage.rows,m_MatSrcImage.cols,m_MatSrcImage.type());
    Mat dstImage;//Ŀ��ͼ  
    Mat normImage;//��һ�����ͼ  
    Mat scaledImage;//���Ա任��İ�λ�޷������͵�ͼ  
	int iCornerThresh = 110;

	//g_CornerWithHoughWithSrc = g_houghWithSrc;
	writeImage = Scalar::all(255);
    //---------------------------��2����ʼ��---------------------------------------  
    //���㵱ǰ��Ҫ��ʾ������ͼ���������һ�ε��ô˺���ʱ���ǵ�ֵ  
    //dstImage = Mat::zeros( srcImage.size(), CV_32FC1 );  
    //g_srcImageClone=g_srcImage.clone( );  
  
    //---------------------------��3����ʽ���-------------------------------------  
    //���нǵ���  
    //cornerHarris( g_srcGrayImage, dstImage, 2, 3, 0.04, BORDER_DEFAULT );  //ԭͼ�ǵ���
	cornerHarris( srcImage, dstImage, 2, 3, 0.04, BORDER_DEFAULT );	//����任��Ľǵ���

  
    // ��һ����ת��  
    normalize( dstImage, normImage, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );  
    convertScaleAbs( normImage, scaledImage );//����һ�����ͼ���Ա任��8λ�޷�������   
  
    //---------------------------��4�����л���-------------------------------------  
    // ����⵽�ģ��ҷ�����ֵ�����Ľǵ���Ƴ���  
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
    //---------------------------��4����ʾ����Ч��---------------------------------  
	//imshow( "CornerHarris", g_CornerWithHoughWithSrc );  //��ԭͼ���ӻ���ͼ����ʾ
    //imshow( "CornerHarris", g_srcImageClone );  //��ԭͼ����ʾ
    //imshow( "CornerHarris", scaledImage );  //ʹ�ûҶ�ͼ��ʾ
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


	//ת�ɻҶ�ͼ
	cvtColor(srcImage, grayImage, COLOR_BGR2GRAY);

    //canny��Ե���
    Canny(grayImage, canny_output, 50, 50 * 2, 3);

    //������ȡ
    findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

    //����ͼ���
    vector<Moments>mu(contours.size());
    for (unsigned int i = 0; i < contours.size(); i++)
    {
        mu[i] = moments(contours[i], false);
    }
    //����ͼ�������
    vector<Point2f>mc(contours.size());
    for (unsigned int i = 0; i < contours.size(); i++)
    {
        mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
    }

    //��������
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
	//Mat����
	Mat dstMat(srcMat.rows,srcMat.cols,srcMat.type());
	Mat perspectiveMat( 2,3,CV_32FC1);

	//͸�ӱ任����
	Point2f perspectiveSrcBuff[4];
	Point2f perspectiveDesBuff[4];

	//͸�ӱ任��������
	perspectiveSrcBuff[0] = Point2f((srcBuff[0].x+srcBuff[1].x)/2,(srcBuff[0].y+srcBuff[1].y)/2) ;  
    perspectiveSrcBuff[1] = Point2f((srcBuff[2].x+srcBuff[3].x)/2,(srcBuff[2].y+srcBuff[3].y)/2) ;   
    perspectiveSrcBuff[2] = Point2f((srcBuff[4].x+srcBuff[5].x)/2,(srcBuff[4].y+srcBuff[5].y)/2) ;
	perspectiveSrcBuff[3] = Point2f((srcBuff[6].x+srcBuff[7].x)/2,(srcBuff[6].y+srcBuff[7].y)/2) ;

	//��任������
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
 
	//��͸�ӱ任
	perspectiveMat = getPerspectiveTransform( perspectiveSrcBuff, perspectiveDesBuff );  

	//��Դͼ��Ӧ�øոյ�͸�ӱ任
	warpPerspective(srcMat, dstMat, perspectiveMat, dstMat.size());

#ifdef DEBUG_SHOW_ScreenExtract
	//��ʾ
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

