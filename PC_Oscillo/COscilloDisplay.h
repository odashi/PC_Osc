// COscilloDisplay.h



#ifndef INCLUDED_COSCILLODISPLAY_H
#define INCLUDED_COSCILLODISPLAY_H



#include <windows.h>



class CWin32Image;



// オシロスコープの画面
class COscilloDisplay
{
	// 画像を保存しておくバッファ
	CWin32Image *m_pImgPrimary;
	CWin32Image *m_pImgGrid;

	// 各種ペン
	HPEN m_hpWave;

	unsigned int m_uVoltsPerDiv;	// 縦の倍率
	unsigned int m_uTimePerDiv;		// 横の倍率
	unsigned int m_uGridSize;		// グリッドサイズ

	// コンストラクタは不可視（Create()で生成する）
	COscilloDisplay();

public:
	static const unsigned int HGridNum = 10;
	static const unsigned int VGridNum = 8;

	~COscilloDisplay();

	// 生成
	static COscilloDisplay *Create(HDC hdc, unsigned int uVoltsPerDiv, unsigned int uTimePerDiv, unsigned int uGridSize);

	// 内部画像バッファの更新
	void Update(short *pBuffer, unsigned int uBufferSize);

	// 描画
	void Draw(HDC hdc, int x, int y);

	// 各種設定（次回Update()から反映）
	bool SetVoltsPerDiv(unsigned int value);
};



#endif // INCLUDED_COSCILLODISPLAY_H
