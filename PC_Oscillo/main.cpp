// main.cpp
// エントリポイント

#include "wndmsg.h"
#include "resource.h"

#include "CWaveInput.h"
#include "COscilloDisplay.h"

#include <windows.h>
#include <crtdbg.h>



// グローバル変数
HINSTANCE hInst;			// 現在のインターフェイス

// 定数
#define SZ_WINDOWCLASS_NAME "WIN32_PC_OSCILLO"
#define SZ_TITLE "PC Oscilloscope Ver.0.1"



ATOM				MyRegisterClass();
BOOL				InitInstance(int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void				MenuProc(HWND hWnd, UINT uItem, HWND hwndCtrl);
bool				MyCheckMenuItem(HMENU hMenu, UINT uItem, bool bChecked);
void				SetVoltsPerDiv(HWND hWnd, UINT uItem);
void				SetTimePerDiv(HWND hWnd, UINT uItem);
void				SetGridSize(HWND hWnd, UINT uItem);



/*
	エントリポイント
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	hInst = hInstance;
	MyRegisterClass();

	if (!InitInstance(nCmdShow))
	{
		_CrtDumpMemoryLeaks();
		return FALSE;
	}

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		//TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	_CrtDumpMemoryLeaks();
	return (int)(msg.wParam);
}



/*
	ウィンドウクラスの登録
*/
ATOM MyRegisterClass()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDR_MAINMENU);
	wcex.lpszClassName	= SZ_WINDOWCLASS_NAME;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

/*
	ウィンドウの生成
*/
BOOL InitInstance(int nCmdShow)
{
	HWND hWnd = CreateWindow(
		SZ_WINDOWCLASS_NAME,
		SZ_TITLE,
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0,
		NULL,
		NULL,
		hInst,
		NULL);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// ウィンドウサイズの固定
	RECT rw, rc;
	GetWindowRect(hWnd, &rw);
	GetClientRect(hWnd, &rc);
	SetWindowPos(
		hWnd, HWND_TOP, 0, 0,
		(rw.right-rw.left) - (rc.right-rc.left) + 48 * COscilloDisplay::HGridNum,
		(rw.bottom-rw.top) - (rc.bottom-rc.top) + 48 * COscilloDisplay::VGridNum,
		SWP_NOMOVE);

	return TRUE;
}



static CWaveInput *pWaveIn = NULL;
static COscilloDisplay *pDisp = NULL;



/*
	ウィンドウプロシージャ
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case 0:
			::MenuProc(hWnd, LOWORD(wParam), (HWND)lParam);
			break;
		}
		break;

	case WM_CREATE:
		{
			// 音声入力デバイス生成
			CWaveInput::ErrorCode err;
			MMRESULT mmErr;

			err = CWaveInput::Create(&pWaveIn, hWnd, CWaveInput::GetDeviceCount() - 1, 30, WM_WAVE_UPDATED, &mmErr);
			if (err != CWaveInput::Err_NoError)
			{
				char text[64];
				::wsprintf(text, "音声入力デバイスを開けません。\nMMRESULT : %ld", mmErr);
				::MessageBox(hWnd, text, SZ_TITLE, MB_OK | MB_ICONERROR);
				return -1;
			}
			
			// 画像バッファ生成
			hdc = ::GetDC(hWnd);
			pDisp = COscilloDisplay::Create(hdc, 0, 0, 48);
			::ReleaseDC(hWnd, hdc);
			if (!pDisp)
			{
				::MessageBox(hWnd, "画像バッファを用意するだけのメモリがありません。", SZ_TITLE, MB_OK | MB_ICONERROR);
				return -1;
			}

			// メニューのチェック
			HMENU hMenu = ::GetMenu(hWnd);
			::MyCheckMenuItem(hMenu, IDM_VOLTS_1, true);
			::MyCheckMenuItem(hMenu, IDM_TIME_1, true);
			::MyCheckMenuItem(hMenu, IDM_GRID_48, true);

			// 音声情報を取得開始
			pWaveIn->Start();
		}
		break;

	case WM_PAINT:
		hdc = ::BeginPaint(hWnd, &ps);
		
		// ディスプレイの描画
		pDisp->Draw(hdc, 0, 0);
		
		::EndPaint(hWnd, &ps);
		break;

	case WM_WAVE_UPDATED:
		{
			// バッファ情報の取得
			unsigned int size;
			char *pBuffer;
			pWaveIn->GetBuffer(&pBuffer, &size);

			// ディスプレイの更新
			pDisp->Update((short *)pBuffer, size);

			// 画面の更新
			::InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	
	case WM_DESTROY:

		// オブジェクトの破棄
		if (pWaveIn)
			delete pWaveIn;
		if (pDisp)
			delete pDisp;
		
		::PostQuitMessage(0);
		
		break;
	
	default:
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}



/*
	メニュー処理
*/
void MenuProc(HWND hWnd, UINT uItem, HWND hwndCtrl)
{
	switch (uItem)
	{
	case IDM_QUIT:
		// 終了
		::PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;

	case IDM_VOLTS_1:
	case IDM_VOLTS_2:
	case IDM_VOLTS_4:
	case IDM_VOLTS_8:
	case IDM_VOLTS_16:
	case IDM_VOLTS_32:
	case IDM_VOLTS_64:
	case IDM_VOLTS_128:
	case IDM_VOLTS_256:
		::SetVoltsPerDiv(hWnd, uItem);
		break;

	case IDM_TIME_1:
		::SetTimePerDiv(hWnd, uItem);
		break;

	case IDM_GRID_16:
	case IDM_GRID_32:
	case IDM_GRID_48:
	case IDM_GRID_64:
	case IDM_GRID_96:
		::SetGridSize(hWnd, uItem);
		break;
	}
}

/*
	メニューにチェックを付ける／外す
*/
bool MyCheckMenuItem(HMENU hMenu, UINT uItem, bool bChecked)
{
	// 設定
	MENUITEMINFO mii;
	mii.cbSize	= sizeof(MENUITEMINFO);
	mii.fMask	= MIIM_STATE;
	mii.fState	= (bChecked) ? MFS_CHECKED : MFS_UNCHECKED;

	// 適用
	return ::SetMenuItemInfo(hMenu, uItem, FALSE, &mii);
}



/*
	Volts/Divの設定（メニュー）
*/
void SetVoltsPerDiv(HWND hWnd, UINT uItem)
{
	// メニューのチェックを変更
	HMENU hMenu = ::GetMenu(hWnd);
	::MyCheckMenuItem(hMenu, IDM_VOLTS_1, false);
	::MyCheckMenuItem(hMenu, IDM_VOLTS_2, false);
	::MyCheckMenuItem(hMenu, IDM_VOLTS_4, false);
	::MyCheckMenuItem(hMenu, IDM_VOLTS_8, false);
	::MyCheckMenuItem(hMenu, IDM_VOLTS_16, false);
	::MyCheckMenuItem(hMenu, IDM_VOLTS_32, false);
	::MyCheckMenuItem(hMenu, IDM_VOLTS_64, false);
	::MyCheckMenuItem(hMenu, IDM_VOLTS_128, false);
	::MyCheckMenuItem(hMenu, IDM_VOLTS_256, false);
	::MyCheckMenuItem(hMenu, uItem, true);

	// 設定
	switch (uItem)
	{
	case IDM_VOLTS_1:
		pDisp->SetVoltsPerDiv(0);
		break;
	case IDM_VOLTS_2:
		pDisp->SetVoltsPerDiv(1);
		break;
	case IDM_VOLTS_4:
		pDisp->SetVoltsPerDiv(2);
		break;
	case IDM_VOLTS_8:
		pDisp->SetVoltsPerDiv(3);
		break;
	case IDM_VOLTS_16:
		pDisp->SetVoltsPerDiv(4);
		break;
	case IDM_VOLTS_32:
		pDisp->SetVoltsPerDiv(5);
		break;
	case IDM_VOLTS_64:
		pDisp->SetVoltsPerDiv(6);
		break;
	case IDM_VOLTS_128:
		pDisp->SetVoltsPerDiv(7);
		break;
	case IDM_VOLTS_256:
		pDisp->SetVoltsPerDiv(8);
		break;
	}
}

/*
	Time/Divの設定（メニュー）
*/
void SetTimePerDiv(HWND hWnd, UINT uItem)
{
}

/*
	グリッドサイズの設定（メニュー）
*/
void SetGridSize(HWND hWnd, UINT uItem)
{
}
