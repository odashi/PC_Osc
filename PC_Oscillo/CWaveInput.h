//CWaveInput.h

#ifndef INCLUDED_CWAVEINPUT_H
#define INCLUDED_CWAVEINPUT_H



#include <windows.h>
#include <mmsystem.h>



// 音声入力デバイス（モノラル）から入力を受け取り，ウィンドウに逐一報告するクラス
class CWaveInput
{
	HWAVEIN			m_hWaveIn;			// 音声入力デバイス

	HWND			m_hWnd;				// ウィンドウハンドル
	unsigned int	m_uMessage;			// データ取得時にウィンドウに送信するメッセージ

	unsigned int	m_uFrameRate;		// 一秒あたりのサンプル分割更新数

	char *			m_pPrimaryBuffer;	// 音声データを保存しておく循環バッファ
	char *			m_pTempBuffer;		// デバイスからデータを受け取るバッファ
	WAVEHDR *		m_aryWaveHdr;		// デバイスからデータを受け取る構造体の配列
	bool			m_bPrepared;		// m_aryWaveHdrをwaveInPrepareHeader()で初期化したかどうか

	bool			m_bRecording;		// データ取得中かどうか
	unsigned int	m_uLastBlock;		// 録音を終えたバッファのブロック


	// デバイスからデータを受け取ったときに起動する関数
	static void CALLBACK WaveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

	// コンストラクタは不可視（Create()で生成する）
	CWaveInput();

public:
	// 定数
	static const unsigned int SamplingFrequency	= 44100;
	static const unsigned int Channels			= 1;
	static const unsigned int BitsPerSample		= 16;
	static const unsigned int MaxFrameRate		= 60;

	// エラーコード
	typedef unsigned int ErrorCode;
	static const ErrorCode Err_NoError			= 0;
	static const ErrorCode Err_InvalidParam		= 1;
	static const ErrorCode Err_NoMemory			= 2;
	static const ErrorCode Err_MMSystemError	= 3;

	~CWaveInput();

	// 使用可能な音声入力デバイスの数
	static unsigned int GetDeviceCount();

	// 生成
	static ErrorCode Create(
		CWaveInput **	ppWaveIn,
		HWND			hWnd,
		unsigned int	uDeviceIndex,
		unsigned int	uFrameRate,
		unsigned int	uMessage,
		MMRESULT *		pMMRet
	);

	// 録音開始
	ErrorCode Start();

	// 録音停止
	ErrorCode Stop();

	// バッファ情報の取得
	ErrorCode GetBuffer(char **ppBuffer, unsigned int *pSize);
};



#endif // INCLUDED_CWAVEINPUT_H




/*
	わかっているエラー
		1. デバイス取得後にそのデバイスをOFF→ONするとフリーズする。
*/