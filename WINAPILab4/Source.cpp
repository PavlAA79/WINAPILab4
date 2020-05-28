#pragma once
#include <windows.h>
#include <tchar.h>
#include <string>
#include <stdio.h>

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI Thread1(LPVOID t);
DWORD WINAPI Thread2(LPVOID t);
HANDLE hThread1, EventThread1;
HANDLE hThread2, EventThread21, EventThread22;
static int x = 100;
static int y = 100;

int Wx, Wy;
HDC hdc = 0;
HBITMAP hbitmap = 0;

WCHAR Textout1[250];
static int i = 0;

TCHAR WinName[] = _T("MainFrame");

int WINAPI _tWinMain(HINSTANCE This,		 // ���������� �������� ���������� 
	HINSTANCE Prev, 	// � ����������� �������� ������ 0 
	LPTSTR cmd, 		// ��������� ������ 
	int mode) 		// ����� ����������� ����
{
	HWND hWnd;		// ���������� �������� ���� ��������� 
	MSG msg; 		// ��������� ��� �������� ��������� 
	WNDCLASS wc; 	// ����� ����

	// ����������� ������ ���� 
	wc.hInstance = This;
	wc.lpszClassName = WinName; 				// ��� ������ ���� 
	wc.lpfnWndProc = WndProc; 					// ������� ���� 
	wc.style = CS_HREDRAW | CS_VREDRAW; 			// ����� ���� 
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); 		// ����������� ������ 
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); 		// ����������� ������ 
	wc.lpszMenuName = NULL; 					// ��� ���� 
	wc.cbClsExtra = 0; 						// ��� �������������� ������ ������ 
	wc.cbWndExtra = 0; 						// ��� �������������� ������ ���� 
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	//wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); 	// ���������� ���� ����� ������ 

		// ����������� ������ ����

	if (!RegisterClass(&wc)) return 0;

	// �������� ���� 
	hWnd = CreateWindow(WinName,			// ��� ������ ���� 
		_T("������ Windows-����������"), 		// ��������� ���� 
		WS_OVERLAPPEDWINDOW, 		// ����� ���� 
		CW_USEDEFAULT,				// x 
		CW_USEDEFAULT,				// y	 ������� ���� 
		500, 				// width 
		500, 				// Height 
		HWND_DESKTOP, 				// ���������� ������������� ���� 
		NULL, 						// ��� ���� 
		This, 						// ���������� ���������� 
		NULL); 					// �������������� ���������� ��� 
	//CW_USEDEFAULT
	ShowWindow(hWnd, mode); 				// �������� ����

	// ���� ��������� ��������� 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg); 		// ������� ���������� ����� ������� ������� 
		DispatchMessage(&msg); 		// �������� ��������� ������� WndProc() 
	}
	return 0;
}

DWORD WINAPI Thread1(LPVOID t)
{
	int k = 0;
	while (1)
	{
		WaitForSingleObject(EventThread1, INFINITE);

		if (hdc)
		{
			if (k + 100 > Wx)
			{
				k = 0;
				x = 100;
			}

			RECT wrect;
			wrect.left = 0;
			wrect.top = 0;
			wrect.right = Wx;
			wrect.bottom = Wy;
			HBRUSH brush1 = CreateSolidBrush(RGB(187, 187, 187));

			SelectObject(hdc, brush1);
			FillRect(hdc, &wrect, brush1);

			RECT rect;
			rect.left = k;
			rect.top = 0;
			rect.right = x;
			rect.bottom = y;
			HBRUSH brush = CreateSolidBrush(RGB(66, 170, 255));
			SelectObject(hdc, brush);
			FillRect(hdc, &rect, brush);
			k++; x++;

		}

		SetEvent(EventThread21);
	}
}

DWORD WINAPI Thread2(LPVOID t)
{
	while (1)
	{
		WaitForSingleObject(EventThread21, INFINITE);

		if (hdc)
		{
			SetBkMode(hdc, TRANSPARENT);
			TextOut(hdc, 0, Wy / 2, Textout1, lstrlen(Textout1));
		}

		SetEvent(EventThread22);
	}
}


static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		hThread1 = CreateThread(0, 0, Thread1, 0, 0, 0);
		hThread2 = CreateThread(0, 0, Thread2, 0, 0, 0);

		EventThread1 = CreateEvent(0, FALSE, FALSE, TEXT("CircleStartEvent"));
		EventThread21 = CreateEvent(0, FALSE, FALSE, TEXT("TextStartEvent"));
		EventThread22 = CreateEvent(0, FALSE, FALSE, TEXT("TextEndEvent"));
		SetTimer(hWnd, 0, 10, 0);
		break;
	}
	case WM_SIZE:
	{
		Wx = LOWORD(lParam);
		Wy = HIWORD(lParam);

		if (hdc) DeleteDC(hdc);
		if (hbitmap) DeleteObject(hbitmap);

		const HDC dc = GetDC(hWnd);

		hdc = CreateCompatibleDC(dc);
		hbitmap = CreateCompatibleBitmap(dc, Wx, Wy);
		SelectObject(hdc, hbitmap);

		ReleaseDC(hWnd, dc);
		break;
	}
	case WM_CHAR:
	{
		TCHAR c = (TCHAR)wParam;
		Textout1[i] = c;
		i++;
		break;
	}
	case WM_PAINT:
	{
		SetEvent(EventThread1);
		WaitForSingleObject(EventThread22, INFINITE);

		PAINTSTRUCT ps;
		const HDC hdc1 = BeginPaint(hWnd, &ps);

		BitBlt(hdc1, 0, 0, Wx, Wy, hdc, 0, 0, SRCCOPY);

		EndPaint(hWnd, &ps);
		return TRUE;
	}
	case WM_TIMER:
	{
		InvalidateRect(hWnd, 0, FALSE);
		break;
	}
	case WM_DESTROY:
	{
		TerminateThread(hThread1, 0);
		TerminateThread(hThread2, 0);

		CloseHandle(EventThread1);
		CloseHandle(EventThread21);
		CloseHandle(EventThread22);

		if (hdc) DeleteDC(hdc);
		if (hbitmap) DeleteObject(hbitmap);

		PostQuitMessage(0);
		break;
	}
	}


	return DefWindowProc(hWnd, message, wParam, lParam);
}

