// COscilloDsiplay.cpp



#include "COscilloDisplay.h"
#include "CWin32Image.h"

#include <new>



COscilloDisplay::COscilloDisplay()
: m_pImgPrimary(NULL)
, m_pImgGrid(NULL)
, m_hpWave(NULL)
, m_uGridSize(0)
{
}



COscilloDisplay::~COscilloDisplay()
{
	if (m_pImgPrimary)
		delete m_pImgPrimary;
	if (m_pImgGrid)
		delete m_pImgGrid;
	if (m_hpWave)
		::DeleteObject(m_hpWave);
}



// ����
COscilloDisplay *COscilloDisplay::Create(HDC hdc, unsigned int uVoltsPerDiv, unsigned int uTimePerDiv, unsigned int uGridSize)
{
	// �����̊m�F
	if (!hdc)
		return NULL;
	if (!uGridSize)
		return NULL;


	COscilloDisplay *pDisp;

	// �I�u�W�F�N�g����
	try {
		pDisp = new COscilloDisplay();
	} catch (std::bad_alloc e) {
		return NULL;
	}

	int w = uGridSize * HGridNum;
	int h = uGridSize * VGridNum;

	// �摜�o�b�t�@����
	pDisp->m_pImgPrimary = CWin32Image::Create(hdc, w, h);
	if (!pDisp->m_pImgPrimary)
	{
		delete pDisp;
		return NULL;
	}

	pDisp->m_pImgGrid = CWin32Image::Create(hdc, w, h);
	if (!pDisp->m_pImgGrid)
	{
		delete pDisp;
		return NULL;
	}

	// �y���̐���
	pDisp->m_hpWave = ::CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	if (!pDisp->m_hpWave)
	{
		delete pDisp;
		return NULL;
	}

	// �摜�o�b�t�@�̏������i�v���C�}���j
	HDC hdcTemp = pDisp->m_pImgPrimary->DC();
	::PatBlt(hdcTemp, 0, 0, w, h, BLACKNESS);
	::SetBkMode(hdcTemp, TRANSPARENT);

	// �摜�o�b�t�@�̏������i�O���b�h�ۑ��p�j
	hdcTemp = pDisp->m_pImgGrid->DC();
	::PatBlt(hdcTemp, 0, 0, w, h, BLACKNESS);
	::SetBkMode(hdcTemp, TRANSPARENT);

	HPEN hpGrid = ::CreatePen(PS_DOT, 1, RGB(63, 63, 63));
	HPEN hpOld = (HPEN)::SelectObject(hdcTemp, hpGrid);

	for (int x = 0; x < w; x += uGridSize)
	{
		::MoveToEx(hdcTemp, x, 0, NULL);
		::LineTo(hdcTemp, x, h);
	}
	for (int y = 0; y < h; y += uGridSize)
	{
		::MoveToEx(hdcTemp, 0, y, NULL);
		::LineTo(hdcTemp, w, y);
	}

	::SelectObject(hdcTemp, hpOld);
	::DeleteObject(hpGrid);

	// �e����
	pDisp->m_uVoltsPerDiv = uVoltsPerDiv;
	pDisp->m_uTimePerDiv = uTimePerDiv;
	pDisp->m_uGridSize = uGridSize;

	return pDisp;
}



/*
	�����摜�o�b�t�@�̍X�V
*/
void COscilloDisplay::Update(short *pBuffer, unsigned int uBufferSize)
{
	int w = m_uGridSize * HGridNum;
	int h = m_uGridSize * VGridNum;
	HDC hdcTemp = m_pImgPrimary->DC();

	// �v���C�}���o�b�t�@���O���b�h�݂̂̉摜�ŏ�����
	::BitBlt(m_pImgPrimary->DC(), 0, 0, w, h, m_pImgGrid->DC(), 0, 0, SRCCOPY);

	// �g�`�̕`��
	HPEN hpOld = (HPEN)::SelectObject(hdcTemp, m_hpWave);

	::MoveToEx(hdcTemp, 0, (h >> 1) + (((int)pBuffer[0] * h) >> (16 - m_uVoltsPerDiv)), NULL);

	for (int x = 1; x < w; x++)
	{
		if (x < (int)uBufferSize)
			::LineTo(hdcTemp, x, (h >> 1) + (((int)pBuffer[x] * h) >> (16 - m_uVoltsPerDiv)));
		else
			::LineTo(hdcTemp, x, (h >> 1));
	}

	::SelectObject(hdcTemp, hpOld);
}

/*
	�w�肵���f�o�C�X�R���e�L�X�g�Ƀv���C�}���o�b�t�@��`��
*/
void COscilloDisplay::Draw(HDC hdc, int x, int y)
{
	int w = m_uGridSize * HGridNum;
	int h = m_uGridSize * VGridNum;

	::BitBlt(hdc, x, y, w, h, m_pImgPrimary->DC(), 0, 0, SRCCOPY);
}



/*
	Volts/Div�ݒ�
*/
bool COscilloDisplay::SetVoltsPerDiv(unsigned int value)
{
	m_uVoltsPerDiv = value;
	return true;
}