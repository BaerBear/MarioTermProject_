#include <Windows.h>
#include <tchar.h>
#include <random>
#include <atlimage.h>
#include <iostream>
//#pragma comment (lib, "msimg32.lib")
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Window Programming Lab";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

//김경준 왔다감

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 1200, 1000, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) // 콜백으로 등록
{
	PAINTSTRUCT ps;
	HDC hDC;
	static HDC mDC;
	COLORREF RGB;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	static HBITMAP hBitmap;
	static RECT wRect;

	//std::mt19937 rd(std::random_device{}());
	//std::uniform_int_distribution<int>Ran(0, 100);

	/* 브러시
	HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
	HBRUSH oldBrush = (HBRUSH)SelectObject(mDC, hBrush);
	SelectObject(mDC, oldBrush);
	DeleteObject(hBrush);

	펜
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	HPEN oldPen = (HPEN)SelectObject(mDC, hPen);
	SelectObject(mDC, oldPen);
	DeleteObject(hPen);
	*/

	switch (iMessage) {
	case WM_CREATE: {

		break;
	}
	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, wRect.right, wRect.bottom);
		(HBITMAP)SelectObject(mDC, (HBITMAP)hBitmap);



		BitBlt(hDC, 0, 0, wRect.right, wRect.bottom, mDC, 0, 0, SRCCOPY);
		DeleteDC(mDC);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY: {
		DeleteObject(hBitmap);
		PostQuitMessage(0);
		return 0;
	}
	}
	return (DefWindowProc(hWnd, iMessage, wParam, lParam)); // 항상 얘를 호출 후 종료해야 함
}