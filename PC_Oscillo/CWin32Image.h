// CWin32Image.h



#ifndef INCLUDED_CWIN32IMAGE_H
#define INCLUDED_CWIN32IMAGE_H



#include <windows.h>



class CWin32Image
{
	HDC		m_hBmpDC;
	HBITMAP	m_hBmp;
	void	*m_pBuf;

	int m_iWidth;
	int m_iHeight;

	// コンストラクタは不可視
	CWin32Image();

public:
	static const unsigned int MaxSize = 32767;

	~CWin32Image();

	// 生成
	static CWin32Image *Create(HDC hdc, int iWidth, int iHeight);

	// 各種情報を取得
	int Width() const	{ return m_iWidth; }
	int Height() const	{ return m_iHeight; }
	HDC DC()			{ return m_hBmpDC; }
	void *Buffer()		{ return m_pBuf; }
};



#endif // INCLUDED_WIN32DDB_H
