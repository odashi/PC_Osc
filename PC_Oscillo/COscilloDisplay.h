// COscilloDisplay.h



#ifndef INCLUDED_COSCILLODISPLAY_H
#define INCLUDED_COSCILLODISPLAY_H



#include <windows.h>



class CWin32Image;



// �I�V���X�R�[�v�̉��
class COscilloDisplay
{
	// �摜��ۑ����Ă����o�b�t�@
	CWin32Image *m_pImgPrimary;
	CWin32Image *m_pImgGrid;

	// �e��y��
	HPEN m_hpWave;

	unsigned int m_uVoltsPerDiv;	// �c�̔{��
	unsigned int m_uTimePerDiv;		// ���̔{��
	unsigned int m_uGridSize;		// �O���b�h�T�C�Y

	// �R���X�g���N�^�͕s���iCreate()�Ő�������j
	COscilloDisplay();

public:
	static const unsigned int HGridNum = 10;
	static const unsigned int VGridNum = 8;

	~COscilloDisplay();

	// ����
	static COscilloDisplay *Create(HDC hdc, unsigned int uVoltsPerDiv, unsigned int uTimePerDiv, unsigned int uGridSize);

	// �����摜�o�b�t�@�̍X�V
	void Update(short *pBuffer, unsigned int uBufferSize);

	// �`��
	void Draw(HDC hdc, int x, int y);

	// �e��ݒ�i����Update()���甽�f�j
	bool SetVoltsPerDiv(unsigned int value);
};



#endif // INCLUDED_COSCILLODISPLAY_H
