#include "main.h"

using namespace cv;
using namespace std;

typedef enum
{
    Step_ReadCammer = 0,
    Step_CorrectScreen = 1,
    Step_MatchFeature = 2,
	Step_Communication  = 3,
    Step_FastReadCammer =4,
    Step_FastCorrectScreen =5,
}mainFunctionStep;



vector<Mat> vecFeature;
vector<Mat> vecConfusingFeature;

void preconditioning(void)
{
	string Suffix = ".png";

	for(int i=1;i<50;i++)
	{
		//int 2 str
		stringstream ss;
		string index;
		ss<<i;
		ss>>index;

		string name = index+Suffix;

		Mat Img=imread(name,CV_LOAD_IMAGE_GRAYSCALE);
		//Mat Img=imread(name);
		if( !Img.data )
		{
			break;
		}
		vecFeature.push_back(Img);//��ӻҶ�����
	}	

	for(int i=101;i<150;i++)
	{
		//int 2 str
		stringstream ss;
		string index;
		ss<<i;
		ss>>index;

		string name = index+Suffix;

		Mat Img=imread(name,CV_LOAD_IMAGE_GRAYSCALE);
		//Mat Img=imread(name);
		if( !Img.data )
		{
			break;
		}
		vecConfusingFeature.push_back(Img);//��ӻҶ�����
	}	
	
}

int main()
{
	Mat g_srcImage;
	Mat g_extractImage;
	float distance;
	SerialPort *c_COM3;
	mainFunctionStep step = Step_ReadCammer;
	//mainFunctionStep step  = Step_Communication;

	//����У������
	ScreenExtract Screen;

	//����ͼƬ
	preconditioning();

	//�򿪴���
	try
	{
		c_COM3 = new SerialPort(3,115200);
	}
	catch(char *s)
	{
		return -1; 
	}
		

	//������ͷ
	VideoCapture cap(0);  
	if(!cap.isOpened())  
	{  
		return -1;  
	}  
	cap.set(CV_CAP_PROP_FRAME_WIDTH, CammerWidth);  
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, CammerHeigh);  

	while(1)
	{
		if( Step_ReadCammer == step )
		{
			//��0����ȡ����ͼƬ
			//g_srcImage = imread("2.jpg");
			//if(!g_srcImage.data)
			//{
			//	printf("Err");
			//}
			////namedWindow("��ԭʼͼ��");
			////imshow("��ԭʼͼ��",g_srcImage);

			//��0����ȡ����ͷ
			while(1)
			{
				cap>>g_srcImage;
				imshow("vedio",g_srcImage);
				if((char(waitKey(1))=='q'))
				{
					break;
				}
			}
			step = Step_CorrectScreen;
		}
		else if( Step_CorrectScreen == step)
		{
			//��1����Ļ��ȡ����
			Screen.setSrc(g_srcImage);
			Screen.runExtract();
			try
			{
				g_extractImage = Screen.getDst();
			}
			catch(char *s)
			{
				if(s == "Extract fail")//��ȡʧ��
				{
					step = Step_ReadCammer;
					continue;
				}
			}

			imshow("���",g_extractImage);
			step = Step_MatchFeature;
		}
		else if( Step_MatchFeature == step )
		{
			//��2��������ȡƥ��
			Mat featureImage1 = imread("С���ޱ���.png");
			if(!featureImage1.data)
			{
				printf("featureImage1");
				return 0;
			}
			//imshow("�ز�",featureImage1);

			ImageMatch match(g_extractImage);
			//�����ز�����
			match.setPersonFeature( featureImage1 );
			match.addCubeFeatureImage(vecFeature);
			match.addConfusingFeatureImage(vecConfusingFeature);

			//����ƥ��
			try
			{
				match.runMatch();
			}
			catch(char *s)
			{
				if(s == "Person match fail")//С��ƥ��ʧ��
				{
					step = Step_ReadCammer;
					continue;
				}
				else if(s == "Block match fail")//����ƥ��ʧ��
				{
					step = Step_ReadCammer;
					continue;
				}
			}

			distance = match.getDistance();

			while(1)
			{
				waitKey(1000);
			}
			step = Step_Communication;
		}
		else if( Step_Communication == step)
		{
			//for(;;)
			//{
			//	int ms;
			//	TimeOperation time;
			//	time.singularStopwatchRestart();
			//	c_COM3->packAndsend(5000);
			//	waitKey(1000);
			//	time.singularStopwatchPause(ms);
			//	cout<<"ms"<<ms<<endl;
			//}
			c_COM3->packAndsend((unsigned short)distance);
			step = Step_FastReadCammer;
			waitKey(100);
		}
		else if( Step_FastReadCammer == step)
		{
			TimeOperation time;
			time.TimeOutDectectionSetClock(2*distance+000);
			//��0����ȡ����ͷ
			while(1)
			{
				cap>>g_srcImage;
				imshow("vedio",g_srcImage);
				if( time.singularTimeOutDectectionCheckClock())
				{
					break;
				}
				else if((char(waitKey(1))=='q'))
				{
					break;
				}
			}
			step = Step_FastCorrectScreen;
		}
		else if( Step_FastCorrectScreen == step)
		{
			//��1����Ļ��ȡ����
			Screen.runFastExtract(g_srcImage);
			try
			{
				g_extractImage = Screen.getDst();
			}
			catch(char *s)
			{
				if(s == "Extract fail")//��ȡʧ��
				{
					step = Step_ReadCammer;
					continue;
				}
			}

			imshow("���",g_extractImage);
			step = Step_MatchFeature;
		}
	}
	while((char(waitKey(1))!='q'))
	{}
	return 0;
}