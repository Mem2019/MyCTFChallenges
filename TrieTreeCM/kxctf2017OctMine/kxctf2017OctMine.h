
// kxctf2017OctMine.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// Ckxctf2017OctMineApp: 
// �йش����ʵ�֣������ kxctf2017OctMine.cpp
//

class Ckxctf2017OctMineApp : public CWinApp
{
public:
	Ckxctf2017OctMineApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern Ckxctf2017OctMineApp theApp;