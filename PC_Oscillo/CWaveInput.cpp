// CWaveInput.cpp



#include "CWaveInput.h"

#include <new>



CWaveInput::CWaveInput()
: m_hWaveIn(NULL)
, m_hWnd(NULL)
, m_uMessage(0)
, m_uFrameRate(0)
, m_pPrimaryBuffer(NULL)
, m_pTempBuffer(NULL)
, m_aryWaveHdr(NULL)
, m_bPrepared(false)
, m_bRecording(false)
, m_uLastBlock(0)
{
}

CWaveInput::~CWaveInput()
{
	if (m_bPrepared)
	{
		// �f�[�^�擾�̏I��
		m_bRecording = false;
		::waveInReset(m_hWaveIn);

		// �f�o�C�X�ƌ�������f�[�^��j��
		for (unsigned int i = 0; i < m_uFrameRate; i++)
			::waveInUnprepareHeader(m_hWaveIn, &m_aryWaveHdr[i], sizeof(WAVEHDR));
	}

	// �o�b�t�@��j��
	if (m_pTempBuffer)
		::HeapFree(::GetProcessHeap(), 0, m_pTempBuffer);
	if (m_pPrimaryBuffer)
		::HeapFree(::GetProcessHeap(), 0, m_pPrimaryBuffer);
	if (m_aryWaveHdr)
		delete[] m_aryWaveHdr;
}



/*
	�g�p�\�ȉ������̓f�o�C�X�̐�
*/
unsigned int CWaveInput::GetDeviceCount()
{
	return ::waveInGetNumDevs();
}



/*
	������
*/
CWaveInput::ErrorCode CWaveInput::Create(
	CWaveInput **	ppWaveIn,
	HWND			hWnd,
	unsigned int	uDeviceIndex,
	unsigned int	uFrameRate,
	unsigned int	uMessage,
	MMRESULT *		pMMRet
){
	// MMRESULT���󂯂�ϐ��̐ݒ�
	MMRESULT mmRetTemp;
	MMRESULT *pMMRetAlias = (pMMRet) ? pMMRet : &mmRetTemp;

	*pMMRetAlias = MMSYSERR_NOERROR;

	// �����̊m�F
#define CHECK(exp) if (!(exp)) return Err_InvalidParam

	CHECK(ppWaveIn);
	*ppWaveIn = NULL;

	CHECK(::IsWindow(hWnd));
	CHECK(uDeviceIndex < ::waveInGetNumDevs());
	CHECK(uFrameRate > 0 && uFrameRate <= MaxFrameRate && MaxFrameRate % uFrameRate == 0);
	CHECK(uMessage);

#undef CHECK

	// �����g�`�t�H�[�}�b�g
	WAVEFORMATEX fmt;
	fmt.wFormatTag			= WAVE_FORMAT_PCM;
	fmt.nChannels			= Channels;
	fmt.nSamplesPerSec		= SamplingFrequency;
	fmt.nAvgBytesPerSec		= (BitsPerSample >> 3) * SamplingFrequency * Channels;
	fmt.nBlockAlign			= BitsPerSample >> 3;
	fmt.wBitsPerSample		= BitsPerSample;
	fmt.cbSize				= sizeof(WAVEFORMATEX);

	// CWaveIn�I�u�W�F�N�g�̐���
	CWaveInput *pTempObj;
	try {
		pTempObj = new CWaveInput();
	} catch (std::bad_alloc e) {
		return Err_NoMemory;
	}

	// �����o�b�t�@�̐���
	try {
		pTempObj->m_aryWaveHdr = new WAVEHDR[uFrameRate];
	} catch (std::bad_alloc e) {
		delete pTempObj;
		return Err_NoMemory;
	}

	pTempObj->m_pPrimaryBuffer = (char *)::HeapAlloc(::GetProcessHeap(), 0, fmt.nAvgBytesPerSec);
	if (!(pTempObj->m_pPrimaryBuffer))
	{
		delete pTempObj;
		return Err_NoMemory;
	}

	pTempObj->m_pTempBuffer = (char *)::HeapAlloc(::GetProcessHeap(), 0, fmt.nAvgBytesPerSec);
	if (!(pTempObj->m_pTempBuffer))
	{
		delete pTempObj;
		return Err_NoMemory;
	}

	// �t�H�[�}�b�g���L�����ǂ����̊m�F
	*pMMRetAlias = ::waveInOpen(NULL, uDeviceIndex, &fmt, 0, 0, WAVE_FORMAT_QUERY);
	if (*pMMRetAlias != MMSYSERR_NOERROR)
	{
		delete pTempObj;
		return Err_MMSystemError;
	}

	// �������̓f�o�C�X�̎擾
	*pMMRetAlias = ::waveInOpen(&(pTempObj->m_hWaveIn), uDeviceIndex, &fmt, (DWORD_PTR)WaveInProc, 0, CALLBACK_FUNCTION);
	if (*pMMRetAlias != MMSYSERR_NOERROR)
	{
		delete pTempObj;

		if (*pMMRetAlias == MMSYSERR_NOMEM)
			return Err_NoMemory;
		else
			return Err_MMSystemError;

	}

	// �e����
	pTempObj->m_hWnd		= hWnd;
	pTempObj->m_uMessage	= uMessage;
	pTempObj->m_uFrameRate	= uFrameRate;
	pTempObj->m_bRecording	= false;
	pTempObj->m_uLastBlock	= uFrameRate-1;

	// �f�o�C�X�ƌ�������f�[�^�̐���
	unsigned int uBlockSize = fmt.nAvgBytesPerSec / uFrameRate;
	for (unsigned int i = 0; i < uFrameRate; i++)
	{
		pTempObj->m_aryWaveHdr[i].lpData			= pTempObj->m_pTempBuffer + uBlockSize * i;
		pTempObj->m_aryWaveHdr[i].dwBufferLength	= uBlockSize;
		pTempObj->m_aryWaveHdr[i].dwUser			= (DWORD_PTR)pTempObj;
		pTempObj->m_aryWaveHdr[i].dwFlags			= 0;

		::waveInPrepareHeader(pTempObj->m_hWaveIn, &pTempObj->m_aryWaveHdr[i], sizeof(WAVEHDR));
		::waveInAddBuffer(pTempObj->m_hWaveIn, &pTempObj->m_aryWaveHdr[i], sizeof(WAVEHDR));
	}
	pTempObj->m_bPrepared = true;

	*ppWaveIn = pTempObj;
	return Err_NoError;
}



/*
	�^���J�n
*/
CWaveInput::ErrorCode CWaveInput::Start()
{
	// �Đ������ǂ���
	if (m_bRecording)
		return Err_NoError;


	// �^���J�n

	m_bRecording = true;

	MMRESULT mmErr = ::waveInStart(m_hWaveIn);
	if (mmErr != MMSYSERR_NOERROR)
	{
		m_bRecording = false;

		if (mmErr == MMSYSERR_NOMEM)
			return Err_NoMemory;
		else
			return Err_MMSystemError;
	}

	return Err_NoError;
}

/*
	�^����~
*/
CWaveInput::ErrorCode CWaveInput::Stop()
{
	// ��~�����ǂ���
	if (!m_bRecording)
		return Err_NoError;


	// �^����~

	m_bRecording = false;

	MMRESULT mmErr = ::waveInStop(m_hWaveIn);
	if (mmErr != MMSYSERR_NOERROR)
	{
		m_bRecording = true;

		if (mmErr == MMSYSERR_NOMEM)
			return Err_NoMemory;
		else
			return Err_MMSystemError;
	}

	return Err_NoError;
}



/*
	�o�b�t�@���̎擾
*/
CWaveInput::ErrorCode CWaveInput::GetBuffer(char **ppBuffer, unsigned int *pSize)
{
	// �����̊m�F
	if (ppBuffer == NULL)
		return Err_InvalidParam;
	if (pSize == NULL)
		return Err_InvalidParam;

	// �l�̊i�[
	*pSize = m_aryWaveHdr[m_uLastBlock].dwBufferLength;
	*ppBuffer = m_aryWaveHdr[m_uLastBlock].lpData;

	return Err_NoError;
}



/*
	�������̓f�o�C�X�̃R�[���o�b�N����
*/
void CALLBACK CWaveInput::WaveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	switch (uMsg)
	{
	case WIM_DATA:
		{
			LPWAVEHDR lpwvhdr	= (LPWAVEHDR)dwParam1;
			CWaveInput *pWaveIn	= (CWaveInput *)(lpwvhdr->dwUser);

			if (pWaveIn->m_bRecording)
			{
				// ���̃u���b�N��
				if (++pWaveIn->m_uLastBlock >= pWaveIn->m_uFrameRate)
					pWaveIn->m_uLastBlock = 0;

				// �o�b�t�@���擾�����f�[�^�ŏ㏑��
				unsigned int uBlockSize = (BitsPerSample >> 3) * SamplingFrequency * Channels / pWaveIn->m_uFrameRate;
				::memcpy(pWaveIn->m_pPrimaryBuffer + uBlockSize * pWaveIn->m_uLastBlock, lpwvhdr->lpData, uBlockSize);

				// �\���̂��f�o�C�X�ɖ߂�
				::waveInUnprepareHeader(pWaveIn->m_hWaveIn, lpwvhdr, sizeof(WAVEHDR));
				::waveInPrepareHeader(pWaveIn->m_hWaveIn, lpwvhdr, sizeof(WAVEHDR));
				::waveInAddBuffer(pWaveIn->m_hWaveIn, lpwvhdr, sizeof(WAVEHDR));

				// ::SendMessage()���ƏI�����Ƀf�b�h���b�N����
				::SendNotifyMessage(pWaveIn->m_hWnd, pWaveIn->m_uMessage, 0, 0);
			}

		}
		break;
	}
}