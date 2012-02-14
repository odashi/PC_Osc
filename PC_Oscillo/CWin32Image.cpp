// CWin32Image.cpp



#include "CWin32Image.h"



CWin32Image::CWin32Image()
: m_hBmpDC(NULL)
, m_hBmp(NULL)
, m_pBuf(NULL)
, m_iWidth(0)
, m_iHeight(0)
{
	/* Create�ō쐬����̂ŊO������͌Ăяo����Ȃ� */
}

CWin32Image::~CWin32Image()
{
	/* ���ꂼ��̃n���h�����폜 */

	if (m_hBmpDC)
		::DeleteDC(m_hBmpDC);

	if (m_hBmp)
		::DeleteObject(m_hBmp);
}

/*
	CWin32Image�I�u�W�F�N�g�̐���
*/
CWin32Image *CWin32Image::Create(HDC hdc, int iWidth, int iHeight)
{
	if (iWidth <= 0 || iHeight <= 0)
		return NULL;
	if (iWidth > MaxSize || iHeight > MaxSize)
		return NULL;
	if (!hdc)
		return NULL;

	CWin32Image *pImg = new CWin32Image();

	// �r�b�g�}�b�v�̐���
	
	BITMAPINFO bmi;
	::memset(&bmi, 0, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth			= iWidth;
	bmi.bmiHeader.biHeight			= iHeight;
	bmi.bmiHeader.biPlanes			= 1;
	bmi.bmiHeader.biBitCount		= 32;
	//bmi.bmiHeader.biCompression		= BI_RGB;
	//bmi.bmiHeader.biSizeImage		= 0;
	//bmi.bmiHeader.biXPelsPerMeter	= 0;
	//bmi.bmiHeader.biYPelsPerMeter	= 0;
	//bmi.bmiHeader.biClrUsed			= 0;
	//bmi.bmiHeader.biClrImportant	= 0;

	pImg->m_hBmp = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &(pImg->m_pBuf), NULL, 0);
	if (!pImg->m_hBmp)
	{
		delete pImg;
		return NULL;
	}

	// �������f�o�C�X�R���e�L�X�g�̐���
	pImg->m_hBmpDC = ::CreateCompatibleDC(hdc);
	if (!pImg->m_hBmpDC)
	{
		// m_hBmp�̓f�X�g���N�^�ō폜
		delete pImg;
		return NULL;
	}

	// �f�o�C�X�R���e�L�X�g�ƃr�b�g�}�b�v�̊֘A�t��
	::SelectObject(pImg->m_hBmpDC, pImg->m_hBmp);

	// ���ݒ�
	pImg->m_iWidth = iWidth;
	pImg->m_iHeight = iHeight;

	return pImg;
}
