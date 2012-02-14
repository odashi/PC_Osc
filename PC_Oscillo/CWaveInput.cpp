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
		// データ取得の終了
		m_bRecording = false;
		::waveInReset(m_hWaveIn);

		// デバイスと交換するデータを破棄
		for (unsigned int i = 0; i < m_uFrameRate; i++)
			::waveInUnprepareHeader(m_hWaveIn, &m_aryWaveHdr[i], sizeof(WAVEHDR));
	}

	// バッファを破棄
	if (m_pTempBuffer)
		::HeapFree(::GetProcessHeap(), 0, m_pTempBuffer);
	if (m_pPrimaryBuffer)
		::HeapFree(::GetProcessHeap(), 0, m_pPrimaryBuffer);
	if (m_aryWaveHdr)
		delete[] m_aryWaveHdr;
}



/*
	使用可能な音声入力デバイスの数
*/
unsigned int CWaveInput::GetDeviceCount()
{
	return ::waveInGetNumDevs();
}



/*
	初期化
*/
CWaveInput::ErrorCode CWaveInput::Create(
	CWaveInput **	ppWaveIn,
	HWND			hWnd,
	unsigned int	uDeviceIndex,
	unsigned int	uFrameRate,
	unsigned int	uMessage,
	MMRESULT *		pMMRet
){
	// MMRESULTを受ける変数の設定
	MMRESULT mmRetTemp;
	MMRESULT *pMMRetAlias = (pMMRet) ? pMMRet : &mmRetTemp;

	*pMMRetAlias = MMSYSERR_NOERROR;

	// 引数の確認
#define CHECK(exp) if (!(exp)) return Err_InvalidParam

	CHECK(ppWaveIn);
	*ppWaveIn = NULL;

	CHECK(::IsWindow(hWnd));
	CHECK(uDeviceIndex < ::waveInGetNumDevs());
	CHECK(uFrameRate > 0 && uFrameRate <= MaxFrameRate && MaxFrameRate % uFrameRate == 0);
	CHECK(uMessage);

#undef CHECK

	// 音声波形フォーマット
	WAVEFORMATEX fmt;
	fmt.wFormatTag			= WAVE_FORMAT_PCM;
	fmt.nChannels			= Channels;
	fmt.nSamplesPerSec		= SamplingFrequency;
	fmt.nAvgBytesPerSec		= (BitsPerSample >> 3) * SamplingFrequency * Channels;
	fmt.nBlockAlign			= BitsPerSample >> 3;
	fmt.wBitsPerSample		= BitsPerSample;
	fmt.cbSize				= sizeof(WAVEFORMATEX);

	// CWaveInオブジェクトの生成
	CWaveInput *pTempObj;
	try {
		pTempObj = new CWaveInput();
	} catch (std::bad_alloc e) {
		return Err_NoMemory;
	}

	// 内部バッファの生成
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

	// フォーマットが有効かどうかの確認
	*pMMRetAlias = ::waveInOpen(NULL, uDeviceIndex, &fmt, 0, 0, WAVE_FORMAT_QUERY);
	if (*pMMRetAlias != MMSYSERR_NOERROR)
	{
		delete pTempObj;
		return Err_MMSystemError;
	}

	// 音声入力デバイスの取得
	*pMMRetAlias = ::waveInOpen(&(pTempObj->m_hWaveIn), uDeviceIndex, &fmt, (DWORD_PTR)WaveInProc, 0, CALLBACK_FUNCTION);
	if (*pMMRetAlias != MMSYSERR_NOERROR)
	{
		delete pTempObj;

		if (*pMMRetAlias == MMSYSERR_NOMEM)
			return Err_NoMemory;
		else
			return Err_MMSystemError;

	}

	// 各種情報
	pTempObj->m_hWnd		= hWnd;
	pTempObj->m_uMessage	= uMessage;
	pTempObj->m_uFrameRate	= uFrameRate;
	pTempObj->m_bRecording	= false;
	pTempObj->m_uLastBlock	= uFrameRate-1;

	// デバイスと交換するデータの生成
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
	録音開始
*/
CWaveInput::ErrorCode CWaveInput::Start()
{
	// 再生中かどうか
	if (m_bRecording)
		return Err_NoError;


	// 録音開始

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
	録音停止
*/
CWaveInput::ErrorCode CWaveInput::Stop()
{
	// 停止中かどうか
	if (!m_bRecording)
		return Err_NoError;


	// 録音停止

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
	バッファ情報の取得
*/
CWaveInput::ErrorCode CWaveInput::GetBuffer(char **ppBuffer, unsigned int *pSize)
{
	// 引数の確認
	if (ppBuffer == NULL)
		return Err_InvalidParam;
	if (pSize == NULL)
		return Err_InvalidParam;

	// 値の格納
	*pSize = m_aryWaveHdr[m_uLastBlock].dwBufferLength;
	*ppBuffer = m_aryWaveHdr[m_uLastBlock].lpData;

	return Err_NoError;
}



/*
	音声入力デバイスのコールバック処理
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
				// 次のブロックへ
				if (++pWaveIn->m_uLastBlock >= pWaveIn->m_uFrameRate)
					pWaveIn->m_uLastBlock = 0;

				// バッファを取得したデータで上書き
				unsigned int uBlockSize = (BitsPerSample >> 3) * SamplingFrequency * Channels / pWaveIn->m_uFrameRate;
				::memcpy(pWaveIn->m_pPrimaryBuffer + uBlockSize * pWaveIn->m_uLastBlock, lpwvhdr->lpData, uBlockSize);

				// 構造体をデバイスに戻す
				::waveInUnprepareHeader(pWaveIn->m_hWaveIn, lpwvhdr, sizeof(WAVEHDR));
				::waveInPrepareHeader(pWaveIn->m_hWaveIn, lpwvhdr, sizeof(WAVEHDR));
				::waveInAddBuffer(pWaveIn->m_hWaveIn, lpwvhdr, sizeof(WAVEHDR));

				// ::SendMessage()だと終了時にデッドロックする
				::SendNotifyMessage(pWaveIn->m_hWnd, pWaveIn->m_uMessage, 0, 0);
			}

		}
		break;
	}
}