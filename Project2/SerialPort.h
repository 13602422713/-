#ifndef __SERIALPORT_H
#define __SERIALPORT_H

#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

class SerialPort
{
public:
	SerialPort(int CommNumber,int BaudRate = 9600,int Parity = 0,int ByteSize = 8, int StopBits = ONESTOPBIT);
	~SerialPort();
public:
	bool send(vector<unsigned char> vecSend);
	bool packAndsend(unsigned short data1);
private:
	HANDLE m_cHCom;
	int m_iComNumber;
	DCB m_cDcb;
	vector<char> m_vecRecived;
};


#endif

