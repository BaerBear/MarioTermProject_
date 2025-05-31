#include <Windows.h>
#include <tchar.h>
#include <random>
#include <atlimage.h>
#include <iostream>
#pragma comment (lib, "msimg32.lib")
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#define LEFT 1
#define RIGHT 2
#define MOVE 10

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Window Programming Lab";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

// 파이리 이미지 39 * 39
// 파이리 히트박스 25 * 39 (플레이어 x + 14)

// 리자드 이미지 46 * 45

// 리자몽 이미지 66 * 54 (플레이어 x + 26)
// 리자몽 히트박스 40 * 50

class Image_
{
public:
	bool stage1, hidden, stage2, stage3;
	int currentStage;

	// move
	CImage Player_Move_Pairi, Player_Move_Lizard, Player_Move_Lizamong;
	// attack
	CImage Player_Attack_Pairi, Player_Attack_Lizamong;
	// stage
	CImage mStage1, mStageHidden, mStage2, mStage3;

	void ImageInit();
	void DrawBackGround(int x, int y);
	void Destroy();
	void NextStage();
};

class Player_
{
public:
	Image_ Pimage;
	int imageNum;

	void PlayerInit();
	void ResetPosition();
	void DrawPlayer();
	void Move();
	bool Moving() { return move_; };

	int x() { return x_; };
	int y() { return y_; };

private:
	int x_, y_;
	int direct_;
	bool move_;
	bool eatFlower_;
	bool eatMushroom_;
};

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

	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 800, 600, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

HDC hDC;
HDC mDC;
RECT wRect;
Image_ Images;
Player_ Player;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) // 콜백으로 등록
{
	PAINTSTRUCT ps;

	COLORREF RGB;
	static HBITMAP hBitmap;

	//std::mt19937 rd(std::random_device{}());
	//std::uniform_int_distribution<int>Ran(0, 100);

	switch (iMessage) {
	case WM_CREATE: {
		hDC = GetDC(hWnd);
		mDC = CreateCompatibleDC(hDC);
		GetClientRect(hWnd, &wRect);
		Images.ImageInit();
		Player.PlayerInit();
		hBitmap = CreateCompatibleBitmap(hDC, wRect.right, wRect.bottom);
		SelectObject(mDC, hBitmap);
		SetTimer(hWnd, 1, 50, NULL);
		Images.stage1 = true;
		ReleaseDC(hWnd, hDC);
		break;
	}
	case WM_KEYDOWN: {
		if (wParam == 'S' || wParam == 's') {
			Images.NextStage();
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'q' || wParam == 'Q') {
			PostQuitMessage(0);
		}
		break;
	}
	case WM_TIMER: {
		switch (wParam) {
		case 1: {
			if(Player.Moving())Player.imageNum = (Player.imageNum + 1) % 4; // 4 프레임 반복
			Player.Move();
			break;
		}
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		Images.DrawBackGround(Player.x(), Player.y());
		Player.DrawPlayer();
		BitBlt(hDC, 0, 0, wRect.right, wRect.bottom, mDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY: {
		Images.Destroy();
		DeleteDC(mDC);
		DeleteObject(hBitmap);
		ReleaseDC(hWnd, hDC);
		PostQuitMessage(0);
		return 0;
	}
	}
	return (DefWindowProc(hWnd, iMessage, wParam, lParam)); // 항상 얘를 호출 후 종료해야 함
}

void Player_::PlayerInit() {
	x_ = 0;
	y_ = 168;
	direct_ = RIGHT;
	move_ = false;
	eatFlower_ = false;
	eatMushroom_ = false;
	imageNum = 0;
	Pimage = Images; // 전역 Images 객체로 초기화
}

void Player_::ResetPosition() {
	x_ = 0;
	y_ = 168;
	direct_ = RIGHT;
	move_ = false;
}

void Player_::DrawPlayer() {
	if (!Pimage.Player_Move_Pairi.IsNull()) {
		int cameraX = Player.x() - 400;
		if (cameraX < 0) cameraX = 0;
		int stageWidth = (Pimage.currentStage == 1 ? Pimage.mStage1.GetWidth() :
			Pimage.currentStage == 2 ? Pimage.mStage2.GetWidth() :
			Pimage.currentStage == 3 ? Pimage.mStage3.GetWidth() :
			Pimage.mStageHidden.GetWidth());
		if (cameraX > stageWidth - wRect.right) {
			cameraX = stageWidth - wRect.right;
		}
		int offsetX = x_ - cameraX; // 플레이어 실제 x 위치
		if (move_) {
			if(direct_ == LEFT) Pimage.Player_Move_Pairi.TransparentBlt(mDC, offsetX, y_, 39, 39, imageNum * 40, 75, 39, 39, RGB(199, 225, 209));
			else if (direct_ == RIGHT) Pimage.Player_Move_Pairi.TransparentBlt(mDC, offsetX + 39, y_, -39, 39, imageNum * 40, 75, 39, 39, RGB(199, 225, 209));
		}
		else {
			if (direct_ == LEFT) Pimage.Player_Move_Pairi.TransparentBlt(mDC, offsetX, y_, 39, 39, 0, 0, 39, 39, RGB(199, 225, 209));
			else if (direct_ == RIGHT) Pimage.Player_Move_Pairi.TransparentBlt(mDC, offsetX + 39, y_, -39, 39, 0, 0, 39, 39, RGB(199, 225, 209));
		}
	}
	else {
		OutputDebugString(L"Player_Move_Pairi is NULL\n");
	}
}

void Player_::Move() {
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		direct_ = LEFT;
		x_ -= MOVE;
		move_ = true;
	}
	else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		direct_ = RIGHT;
		x_ += MOVE;
		move_ = true;
	}
	else {
		move_ = false;
		imageNum = 0;
	}
	if (x_ <= 0) x_ = 0;
	int stageWidth = (Pimage.currentStage == 1 ? Pimage.mStage1.GetWidth() :
		Pimage.currentStage == 2 ? Pimage.mStage2.GetWidth() :
		Pimage.currentStage == 3 ? Pimage.mStage3.GetWidth() :
		Pimage.mStageHidden.GetWidth());
	if (x_ >= stageWidth) {
		x_ = stageWidth;
	}
}

void Image_::ImageInit() {
	//Player_Move_Tino;
	Player_Move_Pairi.Load(TEXT("Image/파이리 스프라이트.png"));
	Player_Move_Lizard.Load(TEXT("Image/리자드 스프라이트.png"));
	Player_Move_Lizamong.Load(TEXT("Image/리자몽 스프라이트.png"));

	//Player_Attack_Tino;
	Player_Attack_Pairi.Load(TEXT("Image/파이리 스프라이트.png"));
	Player_Attack_Lizamong.Load(TEXT("Image/리자몽 스프라이트.png"));

	mStage1.Load(TEXT("Image/월드 1.png"));
	mStageHidden.Load(TEXT("Image/히든 스테이지.png"));
	mStage2.Load(TEXT("Image/월드 2.png"));
	mStage3.Load(TEXT("Image/월드 3.png"));

	stage1 = hidden = stage2 = stage3 = false;
	currentStage = 1;
}

void Image_::DrawBackGround(int x, int y) {
	int cameraX = x - 400; // 플레이어 중심
	if (cameraX < 0) cameraX = 0; // 왼쪽 끝 고정

	int srcWidth = (stage1 ? mStage1.GetWidth() : stage2 ? mStage2.GetWidth() :
		stage3 ? mStage3.GetWidth() : mStageHidden.GetWidth());
	int srcHeight = (stage1 ? mStage1.GetHeight() : stage2 ? mStage2.GetHeight() :
		stage3 ? mStage3.GetHeight() : mStageHidden.GetHeight());
	if (cameraX > srcWidth - wRect.right) {
		cameraX = srcWidth - wRect.right; // 오른쪽 끝 제한
	}

	float scaleX = static_cast<float>(wRect.right) / srcWidth; // 800 / 2304 ≈ 0.3472
	float scaleY = static_cast<float>(wRect.bottom) / srcHeight; // 600 / 240 = 2.5
	int destWidth = wRect.right; // 800
	int destHeight = wRect.bottom; // 600

	if (stage1 && !mStage1.IsNull()) {
		mStage1.StretchBlt(mDC, 0, 0, destWidth, destHeight, cameraX, 0, wRect.right, srcHeight, SRCCOPY);
	}
	else if (stage2 && !mStage2.IsNull()) {
		mStage2.StretchBlt(mDC, 0, 0, destWidth, destHeight, cameraX, 0, wRect.right, srcHeight, SRCCOPY);
	}
	else if (stage3 && !mStage3.IsNull()) {
		mStage3.StretchBlt(mDC, 0, 0, destWidth, destHeight, cameraX, 0, wRect.right, srcHeight, SRCCOPY);
	}
	else if (hidden && !mStageHidden.IsNull()) {
		mStageHidden.Draw(mDC,0,0,wRect.right,wRect.bottom,0,0, mStageHidden.GetWidth(), mStageHidden.GetHeight());
	}
	else {
		OutputDebugString(L"No valid background image\n");
	}
}

void Image_::NextStage() {
	currentStage++;
	if (currentStage > 4) currentStage = 1;
	stage1 = (currentStage == 1);
	hidden = (currentStage == 2);
	stage2 = (currentStage == 3);
	stage3 = (currentStage == 4);
	Player.ResetPosition(); // 스테이지 전환 시 플레이어 위치 초기화
}

void Image_::Destroy() {
	Player_Move_Pairi.Destroy();
	Player_Move_Lizard.Destroy();
	Player_Move_Lizamong.Destroy();
	Player_Attack_Pairi.Destroy();
	Player_Attack_Lizamong.Destroy();
	mStage1.Destroy();
	mStageHidden.Destroy();
	mStage2.Destroy();
	mStage3.Destroy();
}