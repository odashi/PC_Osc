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



// 生成
COscilloDisplay *COscilloDisplay::Create(HDC hdc, unsigned int uVoltsPerDiv, unsigned int uTimePerDiv, unsigned int uGridSize)
{
	// 引数の確認
	if (!hdc)
		return NULL;
	if (!uGridSize)
		return NULL;


	COscilloDisplay *pDisp;

	// オブジェクト生成
	try {
		pDisp = new COscilloDisplay();
	} catch (std::bad_alloc e) {
		return NULL;
	}

	int w = uGridSize * HGridNum;
	int h = uGridSize * VGridNum;

	// 画像バッファ生成
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

	// ペンの生成
	pDisp->m_hpWave = ::CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	if (!pDisp->m_hpWave)
	{
		delete pDisp;
		return NULL;
	}

	// 画像バッファの初期化（プライマリ）
	HDC hdcTemp = pDisp->m_pImgPrimary->DC();
	::PatBlt(hdcTemp, 0, 0, w, h, BLACKNESS);
	::SetBkMode(hdcTemp, TRANSPARENT);

	// 画像バッファの初期化（グリッド保存用）
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

	// 各種情報
	pDisp->m_uVoltsPerDiv = uVoltsPerDiv;
	pDisp->m_uTimePerDiv = uTimePerDiv;
	pDisp->m_uGridSize = uGridSize;

	return pDisp;
}



/*
	内部画像バッファの更新
*/
void COscilloDisplay::Update(short *pBuffer, unsigned int uBufferSize)
{
	int w = m_uGridSize * HGridNum;
	int h = m_uGridSize * VGridNum;
	HDC hdcTemp = m_pImgPrimary->DC();

	// プライマリバッファをグリッドのみの画像で初期化
	::BitBlt(m_pImgPrimary->DC(), 0, 0, w, h, m_pImgGrid->DC(), 0, 0, SRCCOPY);

	// 波形の描画
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
	指定したデバイスコンテキストにプライマリバッファを描画
*/
void COscilloDisplay::Draw(HDC hdc, int x, int y)
{
	int w = m_uGridSize * HGridNum;
	int h = m_uGridSize * VGridNum;

	::BitBlt(hdc, x, y, w, h, m_pImgPrimary->DC(), 0, 0, SRCCOPY);
}



/*
	Volts/Div設定
*/
bool COscilloDisplay::SetVoltsPerDiv(unsigned int value)
{
	m_uVoltsPerDiv = value;
	return true;
}