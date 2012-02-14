//CWaveInput.h

#ifndef INCLUDED_CWAVEINPUT_H
#define INCLUDED_CWAVEINPUT_H



#include <windows.h>
#include <mmsystem.h>



// �������̓f�o�C�X�i���m�����j������͂��󂯎��C�E�B���h�E�ɒ���񍐂���N���X
class CWaveInput
{
	HWAVEIN			m_hWaveIn;			// �������̓f�o�C�X

	HWND			m_hWnd;				// �E�B���h�E�n���h��
	unsigned int	m_uMessage;			// �f�[�^�擾���ɃE�B���h�E�ɑ��M���郁�b�Z�[�W

	unsigned int	m_uFrameRate;		// ��b������̃T���v�������X�V��

	char *			m_pPrimaryBuffer;	// �����f�[�^��ۑ����Ă����z�o�b�t�@
	char *			m_pTempBuffer;		// �f�o�C�X����f�[�^���󂯎��o�b�t�@
	WAVEHDR *		m_aryWaveHdr;		// �f�o�C�X����f�[�^���󂯎��\���̂̔z��
	bool			m_bPrepared;		// m_aryWaveHdr��waveInPrepareHeader()�ŏ������������ǂ���

	bool			m_bRecording;		// �f�[�^�擾�����ǂ���
	unsigned int	m_uLastBlock;		// �^�����I�����o�b�t�@�̃u���b�N


	// �f�o�C�X����f�[�^���󂯎�����Ƃ��ɋN������֐�
	static void CALLBACK WaveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

	// �R���X�g���N�^�͕s���iCreate()�Ő�������j
	CWaveInput();

public:
	// �萔
	static const unsigned int SamplingFrequency	= 44100;
	static const unsigned int Channels			= 1;
	static const unsigned int BitsPerSample		= 16;
	static const unsigned int MaxFrameRate		= 60;

	// �G���[�R�[�h
	typedef unsigned int ErrorCode;
	static const ErrorCode Err_NoError			= 0;
	static const ErrorCode Err_InvalidParam		= 1;
	static const ErrorCode Err_NoMemory			= 2;
	static const ErrorCode Err_MMSystemError	= 3;

	~CWaveInput();

	// �g�p�\�ȉ������̓f�o�C�X�̐�
	static unsigned int GetDeviceCount();

	// ����
	static ErrorCode Create(
		CWaveInput **	ppWaveIn,
		HWND			hWnd,
		unsigned int	uDeviceIndex,
		unsigned int	uFrameRate,
		unsigned int	uMessage,
		MMRESULT *		pMMRet
	);

	// �^���J�n
	ErrorCode Start();

	// �^����~
	ErrorCode Stop();

	// �o�b�t�@���̎擾
	ErrorCode GetBuffer(char **ppBuffer, unsigned int *pSize);
};



#endif // INCLUDED_CWAVEINPUT_H




/*
	�킩���Ă���G���[
		1. �f�o�C�X�擾��ɂ��̃f�o�C�X��OFF��ON����ƃt���[�Y����B
*/