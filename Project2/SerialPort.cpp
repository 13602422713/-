#include "SerialPort.h"


/*******************************************************************************
* Function Name  : SerialPort
* Description    : open serial port.
* Input          : None
* Output         : None
* Return         : succeed/failed
*******************************************************************************/
SerialPort::SerialPort(int CommNumber,int BaudRate,int Parity ,int ByteSize , int StopBits )
{
	//��0��Ԥ����
	if(CommNumber<0 || BaudRate<0 || Parity<0 || ByteSize<0 || StopBits<0)
	{
		throw("SerialPort parameter wrong");
		return;
	}
	m_iComNumber = CommNumber;

	//int 2 str
	std::stringstream ss;
	std::string str;
	ss<<CommNumber;
	ss>>str;

	string strComNumber = "COM" + str;
	const  char *pComNumber =  strComNumber.c_str();

	//��1���򿪴���
	m_cHCom = CreateFileA( pComNumber, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
	//��1.5����ʱ����
	COMMTIMEOUTS stTimeOuts;
	GetCommTimeouts(m_cHCom, &stTimeOuts);
	stTimeOuts.ReadIntervalTimeout = 0;
	stTimeOuts.ReadTotalTimeoutMultiplier = 0;
	stTimeOuts.ReadTotalTimeoutConstant = 1;
	stTimeOuts.WriteTotalTimeoutMultiplier = 0;
	stTimeOuts.WriteTotalTimeoutConstant = 0;
	if (!SetCommTimeouts(m_cHCom, &stTimeOuts))
	{
		throw("SerialPort openning time out");
		CloseHandle(m_cHCom);
		return;
	}

	//��2��ȡ�ò����ö˿�״̬
	GetCommState(m_cHCom, &m_cDcb);
	m_cDcb.DCBlength = sizeof(DCB);
	m_cDcb.BaudRate = BaudRate;
	m_cDcb.Parity = Parity;  
	m_cDcb.ByteSize = ByteSize;
	m_cDcb.StopBits = StopBits;   
	if (!SetCommState(m_cHCom, &m_cDcb))
	{
		throw("SerialPort openning fail");
		CloseHandle(m_cHCom);
		return;
	}
}

SerialPort::~SerialPort( )
{
	CloseHandle(m_cHCom);
}

/*******************************************************************************
* Function Name  : SerialPort::send
* Description    : serialPort send.
* Input          : None
* Output         : None
* Return         : succeed/failed
*******************************************************************************/
bool SerialPort::send(vector<unsigned char> vecSend)
{
	DWORD dwWrittenLen;
	//��0�� ��vector����char*
	char *cSendBuff = new char[vecSend.size()];
	{
		 memcpy(cSendBuff, &vecSend[0], vecSend.size()*sizeof(char));  
	}

	if(!WriteFile(m_cHCom,cSendBuff, vecSend.size(),&dwWrittenLen,NULL))  
	{  
		throw("Sending fail 1");
		return false;
	} 	
	if(vecSend.size() != dwWrittenLen)
	{
		throw("Sending fail 2");
		return false;
	}
	return true;
}

/*******************************************************************************
* Function Name  : SerialPort::send
* Description    : serialPort send.
* Input          : None
* Output         : None
* Return         : succeed/failed
*******************************************************************************/
bool SerialPort::packAndsend(unsigned short data1)
{
	const int PackageLength=15;
	const unsigned char PackHead1=0x55;
	const unsigned char PackHead2=0xaa;

	static unsigned char number = 0;
	
	unsigned short check=0;
	vector<unsigned char> vecForSend(PackageLength);

	//��ͷ
	vecForSend[0] = PackHead1;
	vecForSend[1] = PackHead2;
	//���
	vecForSend[2] = number++;
	//ʱ��
	vecForSend[3] = (char)(data1>>8);
	vecForSend[4] = (char)data1;

	//У��
	for(int i=0;i<PackageLength-2;i++)
	{
		check += vecForSend[i];
	}
	vecForSend[13] = (char)(check>>8);
	vecForSend[14] = (char)check;
	
	//����
	if(send(vecForSend))
	{
		return true;
	}

	return false;
}


