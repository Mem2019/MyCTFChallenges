
// kxctf2017OctMineDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "kxctf2017OctMine.h"
#include "kxctf2017OctMineDlg.h"
#include "afxdialogex.h"
#include "TrieTree.h"
#include "TrieTreeStatic.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FLAG_LEN 28
#define RESULT_MSG_LEN 16

#define ANSWER_LEN 22
#define ANSWER_BUF_LEN (ANSWER_LEN+1)

TrieTreeStatic answerTree(nullptr);

TrieTreeNodeStatic node_f;
TrieTreeNodeStatic node_t;
TrieTreeNodeStatic node_M;
TrieTreeNodeStatic node_7;
TrieTreeNodeStatic node_9;
TrieTreeNodeStatic node_k;
TrieTreeNodeStatic node_c;
TrieTreeNodeStatic node_kx;
TrieTreeNodeStatic node_root;

// Ckxctf2017OctMineDlg 对话框

void* JmpToMessageBoxTimeOutW;
//which is going to be the address of 
//middle of MessageBoxTimeOutW
#define EBP_ADJUST_LEN 5
//at the beginning of MessageBoxTimeOutW
//mov edi,edi 8B FF
//push ebp    55
//mov ebp,esp 8B EC

//int MessageBoxTimeoutW(HWND hWnd, 
//LPCWSTR lpText, LPCWSTR lpCaption, 
//UINT uType, WORD wLanguageId, DWORD dwMilliseconds)
void __declspec(naked) ToMessageBoxTimeOutW(wchar_t* msg)
{
	_asm
	{
		mov eax,[esp+4]//get message
		push -1 //DWORD dwMilliseconds
		push 0 //WORD wLanguageId
		push 0 //UINT uType
		push eax //LPCWSTR lpCaption
		push eax //LPCWSTR lpText
		push 0 //HWND hWnd
		mov eax, [esp+24]//load retn address into eax
		push eax //push the return address, mock the call
		push ebp
		mov ebp,esp
		jmp JmpToMessageBoxTimeOutW 
		//jmp to the middle of MessageBoxTimeOutW
	}
	//in this way, the reverse engineer 
	//cannot use things like bp MessageBoxA
	//to locate out critical examine code
}

volatile void toUnicode(const char* str, wchar_t* buf);
void MyMessageBox(const char* msgA)
{
	//pre: msg is correct message or wrong message
	//which has strlen 15, buflen 16
	wchar_t msg[RESULT_MSG_LEN];
	toUnicode(msgA, msg);
	ToMessageBoxTimeOutW(msg);
	//however, here the stack is not completely balanced
	//but it's okay, mov esp,ebp will solve the problem
}

Ckxctf2017OctMineDlg::Ckxctf2017OctMineDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_KXCTF2017OCTMINE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Ckxctf2017OctMineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Ckxctf2017OctMineDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &Ckxctf2017OctMineDlg::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_EDIT1, &Ckxctf2017OctMineDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDOK, &Ckxctf2017OctMineDlg::OnBnClickedOk)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// Ckxctf2017OctMineDlg 消息处理程序

#define MSG_BOX_TIMEOUT_LEN 20
//so MessageBoxTimeoutW will not appear in the .rodata
void getStrMessageBoxTimeoutW(char* szDesP)
//pre: the szDes must have size 19
{
	unsigned char* szDes = (unsigned char*)szDesP;
	szDes[0] = 220;
	szDes[1] = 230;
	szDes[2] = 156;
	szDes[3] = 224;
	szDes[4] = 76;
	szDes[5] = 28;
	szDes[6] = 94;
	szDes[7] = 105;
	szDes[8] = 102;
	szDes[9] = 235;
	szDes[10] = 211;
	szDes[11] = 202;
	szDes[12] = 200;
	szDes[13] = 46;
	szDes[14] = 60;
	szDes[15] = 14;
	szDes[16] = 245;
	szDes[17] = 116;
	szDes[18] = 0;
	for (unsigned int i = 0; i < 18; i++)
	{
		szDes[i] ^= (unsigned char)(0xf5 * i*i*i + 0x5e * i*i + 0x9f * i + 0x91);
	}
}

BOOL Ckxctf2017OctMineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	char messageBoxTimeoutW[MSG_BOX_TIMEOUT_LEN];
	getStrMessageBoxTimeoutW(messageBoxTimeoutW);
	HINSTANCE dllIns = LoadLibrary(_T("user32.dll"));
	JmpToMessageBoxTimeOutW = GetProcAddress(dllIns, messageBoxTimeoutW);
	JmpToMessageBoxTimeOutW = (void*)((DWORD)JmpToMessageBoxTimeOutW + EBP_ADJUST_LEN);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Ckxctf2017OctMineDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR Ckxctf2017OctMineDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void Ckxctf2017OctMineDlg::OnBnClickedCancel()
{
	::ExitProcess(0);
	CDialogEx::OnCancel();
}


void Ckxctf2017OctMineDlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

void getAnswerIsWrong(char* szDesP)
//pre: the szDes must have size 16
{
	unsigned char* szDes = (unsigned char*)szDesP;
	szDes[0] = 141;
	szDes[1] = 180;
	szDes[2] = 251;
	szDes[3] = 69;
	szDes[4] = 81;
	szDes[5] = 152;
	szDes[6] = 144;
	szDes[7] = 139;
	szDes[8] = 175;
	szDes[9] = 218;
	szDes[10] = 207;
	szDes[11] = 96;
	szDes[12] = 171;
	szDes[13] = 100;
	szDes[14] = 39;
	szDes[15] = 0;
	for (unsigned int i = 0; i < 15; i++)
	{
		szDes[i] ^= (unsigned char)(0x3a * i*i*i + 0x22 * i*i + 0xb2 * i + 0xcc);
	}
}

void getAnswerCorrect(char* szDesP)
//pre: the szDes must have size 16
{
	unsigned char* szDes = (unsigned char*)szDesP;
	szDes[0] = 192;
	szDes[1] = 187;
	szDes[2] = 248;
	szDes[3] = 148;
	szDes[4] = 120;
	szDes[5] = 11;
	szDes[6] = 23;
	szDes[7] = 244;
	szDes[8] = 182;
	szDes[9] = 79;
	szDes[10] = 113;
	szDes[11] = 14;
	szDes[12] = 214;
	szDes[13] = 85;
	szDes[14] = 206;
	szDes[15] = 0;
	for (unsigned int i = 0; i < 15; i++)
	{
		szDes[i] ^= (unsigned char)(0x60 * i*i*i + 0x11 * i*i + 0xe3 * i + 0x81);
	}
}

void getStr_c(char* szDesP)
//pre: the szDes must have size 2
{
	unsigned char* szDes = (unsigned char*)szDesP;
	szDes[0] = 114;
	szDes[1] = 0;
	for (unsigned int i = 0; i < 1; i++)
	{
		szDes[i] ^= (unsigned char)(0x6d * i*i*i + 0x64 * i*i + 0xaf * i + 0x11);
	}
}

void getStr_t(char* szDesP)
//pre: the szDes must have size 2
{
	unsigned char* szDes = (unsigned char*)szDesP;
	szDes[0] = 195;
	szDes[1] = 0;
	for (unsigned int i = 0; i < 1; i++)
	{
		szDes[i] ^= (unsigned char)(0x68 * i*i*i + 0x9e * i*i + 0x56 * i + 0xb7);
	}
}

void getStr_f(char* szDesP)
//pre: the szDes must have size 2
{
	unsigned char* szDes = (unsigned char*)szDesP;
	szDes[0] = 104;
	szDes[1] = 0;
	for (unsigned int i = 0; i < 1; i++)
	{
		szDes[i] ^= (unsigned char)(0x95 * i*i*i + 0x92 * i*i + 0x16 * i + 0xe);
	}
}

void getStr_7(char* szDesP)
//pre: the szDes must have size 2
{
	unsigned char* szDes = (unsigned char*)szDesP;
	szDes[0] = 81;
	szDes[1] = 0;
	for (unsigned int i = 0; i < 1; i++)
	{
		szDes[i] ^= (unsigned char)(0x68 * i*i*i + 0xf5 * i*i + 0x3b * i + 0x66);
	}
}

void getStr_M(char* szDesP)
//pre: the szDes must have size 2
{
	unsigned char* szDes = (unsigned char*)szDesP;
	szDes[0] = 171;
	szDes[1] = 0;
	for (unsigned int i = 0; i < 1; i++)
	{
		szDes[i] ^= (unsigned char)(0xe8 * i*i*i + 0x51 * i*i + 0x84 * i + 0xe6);
	}
}

void getStr_k(char* szDesP)
//pre: the szDes must have size 2
{
	unsigned char* szDes = (unsigned char*)szDesP;
	szDes[0] = 25;
	szDes[1] = 0;
	for (unsigned int i = 0; i < 1; i++)
	{
		szDes[i] ^= (unsigned char)(0x26 * i*i*i + 0x3 * i*i + 0x77 * i + 0x72);
	}
}

void getStr_kx(char* szDesP)
//pre: the szDes must have size 3
{
	unsigned char* szDes = (unsigned char*)szDesP;
	szDes[0] = 70;
	szDes[1] = 214;
	szDes[2] = 0;
	for (unsigned int i = 0; i < 2; i++)
	{
		szDes[i] ^= (unsigned char)(0x88 * i*i*i + 0x2d * i*i + 0xcc * i + 0x2d);
	}
}

void getStr_9(char* szDesP)
//pre: the szDes must have size 2
{
	unsigned char* szDes = (unsigned char*)szDesP;
	szDes[0] = 49;
	szDes[1] = 0;
	for (unsigned int i = 0; i < 1; i++)
	{
		szDes[i] ^= (unsigned char)(0x74 * i*i*i + 0x5e * i*i + 0xef * i + 0x8);
	}
}
//these function is auto generated by a python script
//so the important string will be generated dynamically
//instead of in the .rodata section

void thirdCheck(char* c7, char* kx, char* ctf,
	char* ct9, const char* szCor, const char* szWrong)
{
	if ((c7[0] ^ c7[1]) == ('c'^'7') &&
		(kx[0] ^ kx[1]) == ('x'^'k') &&
		(ctf[2] ^ ctf[1]) == ('f'^'t') &&
		(ct9[1] ^ ct9[2]) == ('t'^'9'))
		MyMessageBox(szCor);
	else
		MyMessageBox(szWrong);
	/*
	>>> ord('c')^ord('7')
	84
	>>> ord('k')^ord('x')
	19
	>>> ord('c')^ord('t')
	23
	>>> ord('M')^ord('7')
	122
	>>> ord('f')^ord('t')
	18
	>>> ord('t')^ord('9')
	77
	*/
}

void secondCheck(char* input, const char* szCor, const char* szWrong)
{
	char c7M[4];
	char c7[3];
	char ctf[4];
	char c7M2[4];
	char c7Mk[5];
	char ct[3];
	char ct9[4];
	char kx[3];

	c7[2] = 0;
	ct[0] = input[2];
	c7M2[3] = 0;
	c7[1] = input[1];
	c7Mk[3] = input[12];
	ct9[0] = input[16];
	c7Mk[0] = input[9];
	kx[0] = input[7];
	c7Mk[1] = input[10];
	kx[1] = input[8];
	c7M[1] = input[5];
	c7Mk[4] = 0;
	ctf[2] = input[15];
	ct[2] = 0;
	kx[2] = 0;
	c7M2[2] = input[6];
	ctf[1] = input[14];
	c7M2[0] = input[4];
	ct[1] = input[3];
	ctf[0] = input[13];
	ct9[3] = 0;
	c7M2[1] = input[20];
	ct9[1] = input[17];
	c7[0] = input[0];
	c7M[2] = input[21];
	ctf[3] = 0;
	c7M[0] = input[19];
	c7Mk[2] = input[11];
	ct9[2] = input[18];
	c7M[3] = 0;
	//seperate the input into serveral sections
	//shuffle the order, so more difficult to reverse
	//I don't use things like substring, that will make it easy to reverse
	TrieTree rt;
	rt.addString(ctf);
	rt.addString(c7);
	rt.addString(c7Mk);
	rt.addString(c7M2);
	rt.addString(ct);
	rt.addString(kx);
	rt.addString(ct9);
	rt.addString(c7M);
	//add the sections into RadixTree
	if (rt == answerTree)
	{//if tree is equal, do the third check
		thirdCheck(c7, kx, ctf, ct9, szCor, szWrong);
	}
	else
	{
		MyMessageBox(szWrong);
	}
}

bool isDigit(TCHAR c)
{
	return c >= _T('0') && c <= _T('9');
}

bool isLetter(TCHAR c)
{
	return (c | 32) >= _T('a') && (c | 32) <= _T('z');
}

bool checkCharRange(const CString& str, const char* szWrong)
{
	int len = str.GetLength();
	for (int i = 0; i < len; i++)
	{
		if (!isLetter(str[i]) && !isDigit(str[i]))
		{
			MyMessageBox(szWrong);
			return false;
		}
	}
	return true;
}

volatile void toUnicode(const char* str, wchar_t* buf)
{
	//pre: buf must be (strlen(str)+1)*2 in length
	//str must be number or digit
	size_t len = strlen(str);
	for (size_t i = 0; i < len; i++)
	{
		buf[i] = str[i];
	}
	buf[len] = 0;
}

void Ckxctf2017OctMineDlg::OnBnClickedOk()
{
	CString str;
	GetDlgItem(IDC_EDIT1)->GetWindowText(str);
	
	char szCorrectA[RESULT_MSG_LEN];
	char szWrongA[RESULT_MSG_LEN];
	getAnswerCorrect(szCorrectA);
	getAnswerIsWrong(szWrongA);

	if (!checkCharRange(str, szWrongA))
	{
		memset(szCorrectA, 0, RESULT_MSG_LEN);
		memset(szWrongA, 0, RESULT_MSG_LEN);
		return;
	}

	if (str.GetLength() != ANSWER_LEN)
	{
		MyMessageBox(szWrongA);
	}
	else
	{
		char input[ANSWER_BUF_LEN];
		WideCharToMultiByte(CP_ACP, 0, str.GetBuffer(), -1,
			input, ANSWER_BUF_LEN, 0, 0);
		secondCheck(input, szCorrectA, szWrongA);
	}

	memset(szWrongA, 0, RESULT_MSG_LEN);
	memset(szCorrectA, 0, RESULT_MSG_LEN);
}


int Ckxctf2017OctMineDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	char str_c[2];
	char str_k[2];
	char str_7[2];
	char str_M[2];
	char str_f[2];
	char str_9[2];
	char str_kx[3];
	char str_t[2];

	getStr_f(str_f);
	getStr_t(str_t);
	getStr_M(str_M);
	getStr_7(str_7);
	getStr_9(str_9);
	getStr_k(str_k);
	getStr_c(str_c);
	getStr_kx(str_kx);
	//get the string dynamically
	//so that these string will not appear in the .rodata

	node_9.setContent(str_9);
	node_M.setContent(str_M);
	node_k.setContent(str_k);
	node_c.setContent(str_c);
	node_7.setContent(str_7);
	node_t.setContent(str_t);
	node_kx.setContent(str_kx);
	node_f.setContent(str_f);
	//set the content of the Trie Tree Node

	node_M.addChild(&node_k);
	node_t.addChild(&node_9);
	node_7.addChild(&node_M);
	node_t.addChild(&node_f);
	node_c.addChild(&node_7);
	node_root.addChild(&node_kx);
	node_c.addChild(&node_t);
	node_root.addChild(&node_c);
	//build the trie tree

	node_c.setNumber(0);
	node_k.setNumber(1);
	node_9.setNumber(1);
	node_t.setNumber(1);
	node_7.setNumber(1);
	node_M.setNumber(2);
	node_root.setNumber(0);
	node_f.setNumber(1);
	node_kx.setNumber(1);
	//set the frequency

	answerTree.setRoot(&node_root);
	return 0;
}
