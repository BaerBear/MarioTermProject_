#include <Windows.h>
#include <tchar.h>
#include <random>
#include <atlimage.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <gdiplus.h> // GDI+ 추가

#pragma comment (lib, "msimg32.lib")
#pragma comment (lib, "gdiplus.lib")

#define LEFT 1
#define RIGHT 2

#define TINO 1
#define PAIRI 2
#define LIZAD 3
#define LIZAMONG 4
#define LARGETINO 5

#define MOVE 13
#define JUMP_VELOCITY -13
#define GRAVITY 0.5

#define TUTORIAL 1
#define STAGE1 2
#define STAGE2 3
#define HIDDEN 4

#define MUSHROOM 0
#define FLOWER 1

#define LEFT 0
#define RIGHT 1

std::mt19937 Item_type(std::random_device{}());
std::uniform_int_distribution<int> Type(0, 1); // 0: 버섯, 1: 꽃

Gdiplus::GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken; // GDI+ 토큰

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Window Programming Lab";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int GetCameraX(int playerX, int stageWidth);

class Image_ {
public:
	bool stage1, hidden, tutorial, stage2;
	int currentStage;
	bool isStartScreen;
	bool isTransitioning; // 전환 중 여부
	float transitionTimer; // 전환 타이머 (초 단위)

	CImage mStartScreen;
	CImage Player_Move_Tino, Player_Move_Pairi, Player_Move_Lizamong;
	CImage Player_Attack_Tino, Player_Attack_Pairi, Player_Attack_Lizamong;
	CImage Item_Mushroom, Item_Flower;
	CImage FireballImage;
	CImage mStage1, mStageHidden, mStageTutorial, mStage2;
	CImage blockImage;
	CImage questionBlockImage[2];
	CImage monster;
	CImage coupaImage;
	struct Block {
		int x, y;
		int width, height;
	};
	struct QuestionBlock {
		int x, y;
		int width, height;
		bool hit;
	};
	struct TBlock {
		int x, y;
		int width, height;
	};
	struct Hole {
		int x, y;
		int width, height;
	};
	struct FlagBlock {
		int x, y;
		int width, height;
	};
	struct Fireball {
		int x, y;
		float velocityX;
		float velocityY;
		bool active;
		int width, height;
		int imageNum{};
		int time{};
		bool isPlayerFireball;
	};
	struct Monster {
		int x, y;
		int width, height;
		int direction; // LEFT(1) 또는 RIGHT(2)
		float speed; // 이동 속도
		bool isAlive; // 살아있는지 여부
		bool isFalling; // 구멍에 빠졌는지 여부
		float fallProgress; // 낙하 진행률
		float directionTimer; // 방향 변경 타이머
		float directionChangeInterval; // 방향 변경 간격 (5~10초)
	};
	struct Item {
		int x, y;
		int width, height;
		int type; // 0: 버섯, 1: 꽃
		bool isActive;
		int direction;
		float speed;
		float directionTimer;
		float directionChangeInterval;
	};
	struct Coupa {
		int x, y;
		int width, height;
		int direction; // LEFT(1) 또는 RIGHT(2)
		bool isAlive;
		float coupaJumpVelocity;
		bool isJumping;
		float jumpVelocity;
		float jumpTimer; // 점프 주기
		float fireballTimer; // 파이어볼 발사 주기
		int health; // 체력 (5)
		int invincibleTime; // 무적 시간 (프레임)
		bool isInvincible; // 무적 상태
		bool isFalling; // 구멍 낙하 여부
		float fallProgress; // 낙하 진행률
		float directionTimer; // 방향 변경 타이머
		float directionChangeInterval; // 방향 변경 간격
		float speed; // 이동 속도
	};
	std::vector<Coupa> coupas[4]{}; // 스테이지별 쿠파

	std::vector<Monster> monsters[4]{};
	std::vector<Block> blocks[4]{};
	std::vector<QuestionBlock> questionBlocks[4]{};
	std::vector<TBlock> tBlocks[4]{};
	std::vector<Hole> holes[4]{};
	std::vector<FlagBlock> flagBlocks[4]{};
	std::vector<Fireball> fireballs;
	std::vector<Item> items[4]{};

	Image_() {
		for (int i = 0; i < 4; ++i) {
			blocks[i].clear();
			questionBlocks[i].clear();
			tBlocks[i].clear();
			holes[i].clear();
			flagBlocks[i].clear();
			monsters[i].clear();
			coupas[i].clear();
		}
		fireballs.clear();
		currentStage = 0; // 시작 화면 (0으로 설정)
		isStartScreen = true;
		isTransitioning = false;
		transitionTimer = 0.0f;
	};
	~Image_() {};

	void ImageInit();
	void BlockInit();
	void DrawBackGround(int x, int y, HDC targetDC);
	void Destroy();
	void NextStage();
	int NowStage() { return currentStage; };
	void DrawHitBox(HDC targetDC);
	void EnterHiddenStage();
	void QuitHiddenStage();
};

class Player_ {
public:
	Image_ Pimage;
	int imageNum;
	int time{};
	int fireballCooldown;
	bool attackKeyPressed;
	bool jumpKeyPressed; // 점프 키 상태 추적
	int invincibleTime;  // 무적 시간 카운터 (프레임 단위)
	bool isInvincible;   // 무적 상태 플래그

	void PlayerInit();
	void ResetPosition();
	void DrawPlayer(HDC targetDC);
	void Move();
	void Attack();
	void FireballMove();
	bool Moving() { return move_; };
	void DrawHitbox(HDC targetDC);

	int State();

	int x() { return x_; };
	int y() { return y_; };

	// 확인용
	void turnFlower() {
		eatFlower_ = !eatFlower_;
		State();
	};
	void turnMushroom() {
		if (State() == TINO || State() == PAIRI) {
			groundY_ -= 15;
			y_ -= 15;
		}
		eatMushroom_ = !eatMushroom_;
		State();
	};
	void turnInvicible() {
		if (!isTouchingFlag || !isMovingRightAfterFlag) {
			return; // 깃발에 닿지 않았거나 깃발에서 내려온 후에는 무적 상태 전환 불가
		}
		isInvincible = !isInvincible;
		if (isInvincible) {
			invincibleTime = 3000;
		}
		else {
			invincibleTime = 0; // 무적 해제
		}
	}

	// isFallingIntoHole에 대한 접근자 및 설정자 추가
	bool isFalling() const { return isFallingIntoHole; } // 상태 확인
	void setFalling(bool value) { isFallingIntoHole = value; } // 상태 설정

	float getFallProgress() const { return fallProgress_; }
	void setFallProgress(float value) { fallProgress_ = value; }

private:
	int x_, y_;
	int direct_;
	bool move_;
	bool shoot_;
	bool eatFlower_;
	bool eatMushroom_;
	bool isJumping_;
	float jumpVelocity_;
	int groundY_;
	int defaultGroundY_;
	bool isFallingIntoHole;
	float fallProgress_;
	RECT hitbox_;
	bool isTouchingFlag;
	float flagSlideProgress;
	float flagMoveRightProgress;
	int flagBottomY;
	bool isMovingRightAfterFlag;
	int flagBlockStage;

	void GetHitbox(int& hitboxX, int& hitboxY, int& hitboxWidth, int& hitboxHeight) {
		hitboxX = x_;
		hitboxY = y_;
		hitboxWidth = 0;
		hitboxHeight = 0;

		switch (State()) {
		case TINO:
			if (direct_ == RIGHT) hitboxX += 12;
			else hitboxX += 8;
			hitboxWidth = 18;
			hitboxHeight = 39;
			break;
		case LARGETINO:
			if (direct_ == RIGHT) hitboxX += 12;
			else hitboxX += 8;
			hitboxWidth = 31;
			hitboxHeight = 54;
			break;
		case PAIRI:
			if (direct_ == RIGHT) hitboxX += 21;
			else hitboxX = x_;
			hitboxWidth = 18;
			hitboxHeight = 39;
			break;
		case LIZAD:
			if (direct_ == RIGHT) hitboxX += 26;
			else hitboxX = x_;
			hitboxWidth = 44;
			hitboxHeight = 45;
			break;
		case LIZAMONG:
			if (direct_ == RIGHT) hitboxX += 26;
			else hitboxX += 6;
			hitboxWidth = 32;
			hitboxHeight = 54;
			break;
		default:
			hitboxWidth = 36;
			hitboxHeight = 39;
		}

		hitbox_.left = hitboxX;
		hitbox_.top = hitboxY;
		hitbox_.right = hitboxX + hitboxWidth;
		hitbox_.bottom = hitboxY + hitboxHeight;
	}
};

HDC hDC;
HDC mDC;
RECT wRect;
Image_ Images;
Player_ Player;

bool DrawAllHitBox = false;

static int mouseBackgroundX = 0;
static int mouseBackgroundY = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	static HBITMAP hBitmap;

	switch (iMessage) {
	case WM_CREATE: {
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		hDC = GetDC(hWnd);
		mDC = CreateCompatibleDC(hDC);
		GetClientRect(hWnd, &wRect);
		Images.ImageInit();
		Player.PlayerInit();
		hBitmap = CreateCompatibleBitmap(hDC, wRect.right, wRect.bottom);
		SelectObject(mDC, hBitmap);
		SetTimer(hWnd, 1, 16, NULL);
		Images.isStartScreen = true; // 시작 화면으로 초기화
		ReleaseDC(hWnd, hDC);
		break;
	}
	case WM_LBUTTONDOWN: {
		if (Images.isStartScreen) {
			int mouseX = LOWORD(lParam);
			int mouseY = HIWORD(lParam);
			if (mouseX >= 310 && mouseX <= 500 && mouseY >= 480 && mouseY <= 520) { // Start 버튼
				Images.isStartScreen = false;
				Images.currentStage = TUTORIAL;
				Images.tutorial = true;
				Images.stage1 = Images.stage2 = Images.hidden = false;
				Images.isTransitioning = false;
				Images.transitionTimer = 0.0f;
				Images.blocks[0].clear();
				Images.questionBlocks[0].clear();
				Images.tBlocks[0].clear();
				Images.holes[0].clear();
				Images.flagBlocks[0].clear();
				Images.monsters[0].clear();
				Images.items[0].clear();
				Images.coupas[0].clear();
				Images.fireballs.clear();
				Player.PlayerInit();
				Images.BlockInit();
				InvalidateRect(hWnd, NULL, FALSE);
			}
			else if (mouseX >= 310 && mouseX <= 500 && mouseY >= 525 && mouseY <= 555) { // Quit 버튼
				PostQuitMessage(0);
			}
		}
		break;
	}
	case WM_KEYDOWN: {
		if (Images.isStartScreen) break;
		// ... (기존 키 처리 코드 유지)
		break;
	}
	case WM_MOUSEMOVE: {
		if (Images.isStartScreen) break;
		// ... (기존 마우스 이동 코드 유지)
		break;
	}
	case WM_TIMER: {
		if (Images.isStartScreen) break;
		switch (wParam) {
		case 1: {
			Player.Move();
			if (Images.isTransitioning) {
				Images.transitionTimer += 0.016f;
				if (Images.transitionTimer >= 2.0f && Images.transitionTimer < 2.016f) { // 페이드 아웃 완료 시 초기화
					int currentIndex = Images.currentStage - 1;
					Images.blocks[currentIndex].clear();
					Images.questionBlocks[currentIndex].clear();
					Images.tBlocks[currentIndex].clear();
					Images.holes[currentIndex].clear();
					Images.flagBlocks[currentIndex].clear();
					Images.monsters[currentIndex].clear();
					Images.items[currentIndex].clear();
					Images.coupas[currentIndex].clear();
					Images.fireballs.clear();
					Player.ResetPosition();
					Images.BlockInit();
				}
				if (Images.transitionTimer >= 4.0f) { // 페이드 인 완료
					Images.isTransitioning = false;
					Images.transitionTimer = 0.0f;
				}
			}
			Player.FireballMove();
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
		}
		break;
	}
	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
		Gdiplus::Graphics graphics(mDC);

		if (Images.isStartScreen) {
			if (!Images.mStartScreen.IsNull()) {
				Images.mStartScreen.StretchBlt(mDC, 0, 0, wRect.right, wRect.bottom, 0, 0, Images.mStartScreen.GetWidth(), Images.mStartScreen.GetHeight(), SRCCOPY);
				if (DrawAllHitBox) {
					HPEN buttonPen = CreatePen(PS_DASH, 2, RGB(255, 0, 0));
					HBRUSH buttonBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
					SelectObject(mDC, buttonPen);
					SelectObject(mDC, buttonBrush);
					Rectangle(mDC, 310, 480, 500, 520); // Start 버튼
					Rectangle(mDC, 310, 525, 500, 555); // Quit 버튼
					DeleteObject(buttonPen);
					DeleteObject(buttonBrush);
				}
			}
		}
		else {
			// 게임 화면
			HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
			FillRect(mDC, &wRect, whiteBrush);
			DeleteObject(whiteBrush);
			Images.DrawBackGround(Player.x(), Player.y(), mDC);
			Player.DrawPlayer(mDC);
			WCHAR buffer[100];
			wsprintf(buffer, L"Mouse: (%d, %d)", mouseBackgroundX, mouseBackgroundY);
			SetTextColor(mDC, RGB(255, 255, 255));
			SetBkMode(mDC, TRANSPARENT);
			TextOut(mDC, 10, 10, buffer, lstrlen(buffer));
			if (DrawAllHitBox) {
				Player.DrawHitbox(mDC);
				Images.DrawHitBox(mDC);
			}

			// 전환 중 페이드 효과
			if (Images.isTransitioning) {
				float progress = Images.transitionTimer / 4.0f;
				BYTE alpha;
				if (Images.transitionTimer <= 2.0f) {
					// 페이드 아웃: 0 -> 255
					alpha = static_cast<BYTE>(Images.transitionTimer / 2.0f * 255);
				}
				else {
					// 페이드 인: 255 -> 0
					alpha = static_cast<BYTE>((4.0f - Images.transitionTimer) / 2.0f * 255);
				}
				if (alpha > 255) alpha = 255;
				if (alpha < 0) alpha = 0;
				Gdiplus::SolidBrush brush(Gdiplus::Color(alpha, 0, 0, 0));
				graphics.FillRectangle(&brush, 0, 0, wRect.right, wRect.bottom);
			}
		}

		BitBlt(hDC, 0, 0, wRect.right, wRect.bottom, mDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY: {
		Images.Destroy();
		DeleteDC(mDC);
		DeleteObject(hBitmap);
		ReleaseDC(hWnd, hDC);
		Gdiplus::GdiplusShutdown(gdiplusToken);
		PostQuitMessage(0);
		return 0;
	}
	}
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void Player_::PlayerInit() {
	x_ = 35;
	y_ = 447;
	direct_ = RIGHT;
	move_ = false;
	eatFlower_ = false;
	eatMushroom_ = false;
	imageNum = 0;
	isJumping_ = false;
	jumpVelocity_ = 0.0f;
	groundY_ = 447;
	defaultGroundY_ = 447;
	isFallingIntoHole = false;
	fallProgress_ = 0.0f;
	Pimage = Images;
	hitbox_ = { x_ + 14, y_, x_ + 39, y_ + 39 };
	isTouchingFlag = false;
	flagSlideProgress = 0.0f;
	flagMoveRightProgress = 0.0f;
	flagBottomY = 0;
	isMovingRightAfterFlag = false;
	flagBlockStage = 0;
	fireballCooldown = 0;
	attackKeyPressed = false;
	jumpKeyPressed = false;
	invincibleTime = 0;  // 초기 무적 시간 0
	isInvincible = false;  // 초기 무적 상태 해제
}

void Player_::ResetPosition() {
	move_ = false;
	isJumping_ = false;
	jumpVelocity_ = 0.0f;
	jumpKeyPressed = false;
	isFallingIntoHole = false;
	direct_ = RIGHT;
	fallProgress_ = 0.0f;
	time = 0;
	fireballCooldown = 0;
	attackKeyPressed = false;
	if (Images.NowStage() == TUTORIAL) {
		x_ = 35;
		y_ = 449;
		groundY_ = 449;
		defaultGroundY_ = 465;
	}
	else if (Images.NowStage() == STAGE1) {
		x_ = 10;
		y_ = 447;
		groundY_ = 447;
		defaultGroundY_ = 447;
	}
	else if (Images.NowStage() == STAGE2) {
		x_ = 5;
		y_ = 223;
		groundY_ = 223;
		defaultGroundY_ = 447;
	}
	else if (Images.NowStage() == HIDDEN) {
		x_ = 50;
		y_ = 0;
		isJumping_ = true;
		groundY_ = 435;
		defaultGroundY_ = 435;
	}

	if (Images.NowStage() != HIDDEN) {
		eatFlower_ = false;
		eatMushroom_ = false;
	}
	isTouchingFlag = false;
	flagSlideProgress = 0.0f;
	flagMoveRightProgress = 0.0f;
	flagBottomY = 0;
	isMovingRightAfterFlag = false;
	flagBlockStage = 0;
}

void Player_::DrawPlayer(HDC targetDC) {
	if (!Pimage.Player_Move_Tino.IsNull()) {
		int cameraX = x_ - 400;
		if (cameraX < 0) cameraX = 0;
		int stageWidth = (Images.NowStage() == TUTORIAL ? Pimage.mStageTutorial.GetWidth() :
			Images.NowStage() == STAGE1 ? Pimage.mStage1.GetWidth() :
			Images.NowStage() == STAGE2 ? Pimage.mStage2.GetWidth() :
			Images.NowStage() == HIDDEN ? Pimage.mStageHidden.GetWidth() : 0);

		if (stageWidth <= wRect.right) {
			cameraX = 0;
		}
		else if (cameraX > stageWidth - wRect.right) {
			cameraX = stageWidth - wRect.right;
		}
		int playerWidth = 0;
		switch (State()) {
		case TINO: playerWidth = 36; break;
		case LARGETINO: playerWidth = 49; break;
		case PAIRI: playerWidth = 39; break;
		case LIZAD: playerWidth = 46; break;
		case LIZAMONG: playerWidth = 66; break;
		default: playerWidth = 36;
		}

		int offsetX = x_ - cameraX;
		if (offsetX + playerWidth > wRect.right) {
			offsetX = wRect.right - playerWidth;
		}
		if (offsetX < 0) {
			offsetX = 0;
		}

		if (!Images.FireballImage.IsNull() && !Images.fireballs.empty()) {
			for (auto& fireball : Images.fireballs) {
				if (fireball.active) {
					int fireballOffsetX = fireball.x - cameraX;
					if (fireballOffsetX + fireball.width > 0 && fireballOffsetX < wRect.right) {
						Images.FireballImage.TransparentBlt(targetDC, fireballOffsetX, fireball.y, fireball.width, fireball.height,
							fireball.imageNum * 8, 0, 8, 8, RGB(146, 144, 255));
					}
				}
			}
		}

		// 무적 상태에서 깜빡임 효과
		if (isInvincible && (invincibleTime / 10) % 2 == 0) {
			return; // 짝수 프레임마다 렌더링 생략 (깜빡임)
		}

		if (move_) {
			if (!eatFlower_) {
				if (!eatMushroom_) { // TINO
					if (direct_ == LEFT) {
						if (isJumping_)
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, 10, 175, 36, 39, RGB(0, 255, 0));
						else
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, (2 - imageNum) * 56 + 10, 124, 36, 39, RGB(0, 255, 0));
					}
					else if (direct_ == RIGHT) {
						if (isJumping_)
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, 2 * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
						else
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 10, 36, 39, RGB(0, 255, 0));
					}
				}
				else if (eatMushroom_) { // LARGETINO
					if (direct_ == LEFT) {
						if (isJumping_)
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 49, 54, 10, 175, 36, 39, RGB(0, 255, 0));
						else
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 49, 54, (2 - imageNum) * 56 + 10, 124, 36, 39, RGB(0, 255, 0));
					}
					else if (direct_ == RIGHT) {
						if (isJumping_)
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 49, 54, 2 * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
						else
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 49, 54, imageNum * 56 + 10, 10, 36, 39, RGB(0, 255, 0));
					}
				}
			}
			else if (eatFlower_) {
				if (!eatMushroom_) { // PAIRI
					if (direct_ == LEFT) {
						if (isJumping_)
							Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 42, 222, 41, 39, RGB(199, 225, 209));
						else
							Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 40, 83, 39, 39, RGB(199, 225, 209));
					}
					else if (direct_ == RIGHT) {
						if (isJumping_)
							Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 42 + 5, 268, 41, 39, RGB(199, 225, 209));
						else
							Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 40 + 2, 42, 39, 39, RGB(199, 225, 209));
					}
				}
				else if (eatMushroom_) { // LIZAMONG
					if (direct_ == LEFT) {
						if (isJumping_)
							Pimage.Player_Move_Lizamong.TransparentBlt(targetDC, offsetX, y_, 64, 54, (3 - imageNum) * 75 + 2, 227, 75, 47, RGB(199, 225, 209));
						else
							Pimage.Player_Move_Lizamong.TransparentBlt(targetDC, offsetX, y_, 64, 54, (2 - imageNum) * 63 + 1, 54, 65, 54, RGB(199, 225, 209));
					}
					else if (direct_ == RIGHT) {
						if (isJumping_)
							Pimage.Player_Move_Lizamong.TransparentBlt(targetDC, offsetX, y_, 64, 54, imageNum * 75, 277, 75, 47, RGB(199, 225, 209));
						else
							Pimage.Player_Move_Lizamong.TransparentBlt(targetDC, offsetX, y_, 64, 54, imageNum * 62, 0, 65, 54, RGB(199, 225, 209));
					}
				}
			}
		}
		else if (!move_) {
			if (!eatFlower_) {
				if (!eatMushroom_) { // TINO
					if (direct_ == LEFT) {
						if (isJumping_)
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, 10, 175, 36, 39, RGB(0, 255, 0));
						else
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, (2 - imageNum) * 56 + 10, 175, 36, 39, RGB(0, 255, 0));
					}
					else if (direct_ == RIGHT) {
						if (isJumping_)
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, 2 * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
						else
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
					}
				}
				else if (eatMushroom_) { // LARGETINO
					if (direct_ == LEFT) {
						if (isJumping_)
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 49, 54, 10, 175, 36, 39, RGB(0, 255, 0));
						else
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 49, 54, (2 - imageNum) * 56 + 10, 175, 36, 39, RGB(0, 255, 0));
					}
					else if (direct_ == RIGHT) {
						if (isJumping_)
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 49, 54, 2 * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
						else
							Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 49, 54, imageNum * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
					}
				}
			}
			else if (eatFlower_) {
				if (!eatMushroom_) { // PAIRI
					if (direct_ == LEFT) {
						if (isJumping_)
							Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 42, 222, 41, 39, RGB(199, 225, 209));
						else
							Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 40, 0, 39, 39, RGB(199, 225, 209));
					}
					else if (direct_ == RIGHT) {
						if (isJumping_)
							Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 42 + 5, 268, 41, 39, RGB(199, 225, 209));
						else
							Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, 40, 0, 39, 39, RGB(199, 225, 209));
					}
				}
				else if (eatMushroom_) { // LIZAMONG
					if (direct_ == LEFT) {
						if (isJumping_)
							Pimage.Player_Move_Lizamong.TransparentBlt(targetDC, offsetX, y_, 64, 54, (3 - imageNum) * 75 + 2, 227, 75, 47, RGB(199, 225, 209));
						else
							Pimage.Player_Move_Lizamong.TransparentBlt(targetDC, offsetX, y_, 64, 54, 2 * 63 + 1, 54, 65, 54, RGB(199, 225, 209));
					}
					else if (direct_ == RIGHT) {
						if (isJumping_)
							Pimage.Player_Move_Lizamong.TransparentBlt(targetDC, offsetX, y_, 64, 54, imageNum * 75, 277, 75, 47, RGB(199, 225, 209));
						else
							Pimage.Player_Move_Lizamong.TransparentBlt(targetDC, offsetX, y_, 64, 54, imageNum * 62, 0, 65, 54, RGB(199, 225, 209));
					}
				}
			}
		}
	}
	else {
		OutputDebugString(L"Player_Move_Pairi is NULL\n");
	}
}

void Player_::Move() {
	if (Pimage.isStartScreen || Images.isTransitioning) return;

	if (isFallingIntoHole) {
		const float fallSpeed = 2.0f;
		fallProgress_ += fallSpeed;
		y_ += static_cast<int>(fallSpeed);
		if (fallProgress_ >= 100.0f) {
			Images.isTransitioning = true;
			Images.transitionTimer = 0.0f;
			setFalling(false);
			setFallProgress(0.0f);
			return;
		}
		return;
	}

	int newX = x_;
	bool intendToMoveLeft = (GetAsyncKeyState(VK_LEFT) & 0x8000) != 0;
	bool intendToMoveRight = (GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0;
	bool intendToJump = (GetAsyncKeyState(VK_UP) & 0x8000) != 0 && !jumpKeyPressed;
	bool intendToAttack = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0 && !attackKeyPressed;

	if (isTouchingFlag || isMovingRightAfterFlag) {
		if (isTouchingFlag) {
			Player.invincibleTime = 0;
			Player.isInvincible = false;
			if (Images.NowStage() == TUTORIAL) defaultGroundY_ = 447;
			const float slideSpeed = 4.0f;
			flagSlideProgress += slideSpeed;
			y_ += static_cast<int>(slideSpeed);
			if (y_ >= defaultGroundY_) {
				y_ = defaultGroundY_;
				isTouchingFlag = false;
				direct_ = RIGHT;
				move_ = false;
				isMovingRightAfterFlag = true;
				flagSlideProgress = 0.0f;
			}
		}
		else if (isMovingRightAfterFlag) {
			const float moveRightSpeed = 2.0f;
			const float moveRightDistance = 100.0f;
			flagMoveRightProgress += moveRightSpeed;
			x_ += static_cast<int>(moveRightSpeed);
			move_ = true;
			time++;
			if (time == 3) {
				if (State() == TINO || State() == LARGETINO || State() == LIZAMONG || State() == LIZAD) {
					imageNum = (imageNum + 1) % 3;
				}
				else if (State() == PAIRI) {
					if (isJumping_) imageNum = (imageNum + 1) % 5;
					else imageNum = (imageNum + 1) % 6;
				}
				time = 0;
			}
			if (flagMoveRightProgress >= moveRightDistance) {
				Images.NextStage();
				Images.isTransitioning = true;
				Images.transitionTimer = 0.0f;
				isMovingRightAfterFlag = false;
				flagMoveRightProgress = 0.0f;

			}
		}
		return;
	}

	bool moved = false;
	int prevX = x_;
	int prevY = y_;
	int playerHitboxX, playerHitboxY, playerHitboxWidth, playerHitboxHeight;
	GetHitbox(playerHitboxX, playerHitboxY, playerHitboxWidth, playerHitboxHeight);

	if (invincibleTime > 0) {
		invincibleTime--;
		if (invincibleTime <= 0) {
			isInvincible = false;
		}
	}

	// 쿠파 동작 처리
	if (!Images.coupas[Images.currentStage - 1].empty()) {
		for (auto& coupa : Images.coupas[Images.currentStage - 1]) {
			if (!coupa.isAlive) continue;

			coupa.direction = (x_ < coupa.x) ? LEFT : RIGHT;

			if (coupa.isFalling) {
				const float fallSpeed = 2.0f;
				coupa.fallProgress += fallSpeed;
				coupa.y += static_cast<int>(fallSpeed);
				if (coupa.fallProgress >= 100.0f) {
					coupa.isAlive = false;
				}
				continue;
			}

			bool onGround = false;
			int newCoupaY = coupa.y;
			const float gravity = 2.0f;

			for (const auto& tblock : Images.tBlocks[Images.currentStage - 1]) {
				int tblockLeft = tblock.x;
				int tblockRight = tblock.x + tblock.width;
				int tblockTop = tblock.y;

				int coupaLeft = coupa.x;
				int coupaRight = coupa.x + coupa.width;
				int coupaBottom = newCoupaY + coupa.height;

				bool overlapX = coupaRight > tblockLeft && coupaLeft < tblockRight;
				bool overlapY = coupaBottom + static_cast<int>(gravity) >= tblockTop && coupaBottom <= tblockTop + 5;

				if (overlapX && overlapY) {
					newCoupaY = tblockTop - coupa.height;
					onGround = true;
					coupa.isJumping = false;
					coupa.jumpVelocity = 0.0f;
					break;
				}
			}

			for (const auto& block : Images.blocks[Images.currentStage - 1]) {
				int blockLeft = block.x;
				int blockRight = block.x + block.width;
				int blockTop = block.y;

				int coupaLeft = coupa.x;
				int coupaRight = coupa.x + coupa.width;
				int coupaBottom = newCoupaY + coupa.height;

				bool overlapX = coupaRight > blockLeft && coupaLeft < blockRight;
				bool overlapY = coupaBottom + static_cast<int>(gravity) >= blockTop && coupaBottom <= blockTop + 5;

				if (overlapX && overlapY) {
					newCoupaY = blockTop - coupa.height;
					onGround = true;
					coupa.isJumping = false;
					coupa.jumpVelocity = 0.0f;
					break;
				}
			}

			for (const auto& qblock : Images.questionBlocks[Images.currentStage - 1]) {
				int qblockLeft = qblock.x;
				int qblockRight = qblock.x + qblock.width;
				int qblockTop = qblock.y;

				int coupaLeft = coupa.x;
				int coupaRight = coupa.x + coupa.width;
				int coupaBottom = newCoupaY + coupa.height;

				bool overlapX = coupaRight > qblockLeft && coupaLeft < qblockRight;
				bool overlapY = coupaBottom + static_cast<int>(gravity) >= qblockTop && coupaBottom <= qblockTop + 5;

				if (overlapX && overlapY) {
					newCoupaY = qblockTop - coupa.height;
					onGround = true;
					coupa.isJumping = false;
					coupa.jumpVelocity = 0.0f;
					break;
				}
			}

			for (const auto& hole : Images.holes[Images.currentStage - 1]) {
				int holeLeft = hole.x;
				int holeRight = hole.x + hole.width;
				int holeTop = hole.y;

				int coupaBottomCenterX = coupa.x + coupa.width / 2;
				int coupaBottomY = newCoupaY + coupa.height;

				bool withinHoleX = coupaBottomCenterX >= holeLeft && coupaBottomCenterX <= holeRight;
				bool atHoleTop = coupaBottomY >= holeTop - 5;

				if (withinHoleX && atHoleTop) {
					coupa.isFalling = true;
					coupa.fallProgress = 0.0f;
					onGround = false;
					break;
				}
			}

			coupa.jumpTimer += 0.016f;
			if (coupa.jumpTimer >= 3.0f && !coupa.isJumping && onGround) {
				coupa.isJumping = true;
				coupa.jumpVelocity = coupa.coupaJumpVelocity;
				coupa.jumpTimer = 0.0f;
			}
			if (coupa.isJumping) {
				newCoupaY += static_cast<int>(coupa.jumpVelocity);
				coupa.jumpVelocity += GRAVITY;
			}

			if (!onGround && !coupa.isJumping) {
				newCoupaY += static_cast<int>(gravity);
			}
			coupa.y = newCoupaY;

			coupa.directionTimer += 0.016f;
			if (coupa.directionTimer >= coupa.directionChangeInterval) {
				coupa.directionChangeInterval = static_cast<float>(rand() % 6 + 5);
				coupa.directionTimer = 0.0f;
				coupa.direction = (coupa.direction == LEFT) ? RIGHT : LEFT;
			}

			float newCoupaX = coupa.x + (coupa.direction == LEFT ? -coupa.speed : coupa.speed);

			bool canMove = true;
			for (const auto& block : Images.blocks[Images.currentStage - 1]) {
				int blockLeft = block.x;
				int blockRight = block.x + block.width;
				int blockTop = block.y;
				int blockBottom = block.y + block.height;

				int coupaLeft = static_cast<int>(newCoupaX);
				int coupaRight = static_cast<int>(newCoupaX) + coupa.width;
				int coupaTop = coupa.y;
				int coupaBottom = coupa.y + coupa.height;

				bool overlapX = coupaRight > blockLeft && coupaLeft < blockRight;
				bool overlapY = coupaBottom > blockTop && coupaTop < blockBottom;

				if (overlapX && overlapY) {
					int prevCoupaRight = coupa.x + coupa.width;
					int prevCoupaLeft = coupa.x;
					if (coupa.direction == RIGHT && prevCoupaRight <= blockLeft && coupaRight > blockLeft) {
						coupa.direction = LEFT;
						newCoupaX = blockLeft - coupa.width;
						canMove = false;
					}
					else if (coupa.direction == LEFT && prevCoupaLeft >= blockRight && coupaLeft < blockRight) {
						coupa.direction = RIGHT;
						newCoupaX = blockRight;
						canMove = false;
					}
					break;
				}
			}

			if (canMove) {
				for (const auto& qblock : Images.questionBlocks[Images.currentStage - 1]) {
					int qblockLeft = qblock.x;
					int qblockRight = qblock.x + qblock.width;
					int qblockTop = qblock.y;
					int qblockBottom = qblock.y + qblock.height;

					int coupaLeft = static_cast<int>(newCoupaX);
					int coupaRight = static_cast<int>(newCoupaX) + coupa.width;
					int coupaTop = coupa.y;
					int coupaBottom = coupa.y + coupa.height;

					bool overlapX = coupaRight > qblockLeft && coupaLeft < qblockRight;
					bool overlapY = coupaBottom > qblockTop && coupaTop < qblockBottom;

					if (overlapX && overlapY) {
						int prevCoupaRight = coupa.x + coupa.width;
						int prevCoupaLeft = coupa.x;
						if (coupa.direction == RIGHT && prevCoupaRight <= qblockLeft && coupaRight > qblockLeft) {
							coupa.direction = LEFT;
							newCoupaX = qblockLeft - coupa.width;
							canMove = false;
						}
						else if (coupa.direction == LEFT && prevCoupaLeft >= qblockRight && coupaLeft < qblockRight) {
							coupa.direction = RIGHT;
							newCoupaX = qblockRight;
							canMove = false;
						}
						break;
					}
				}
			}

			if (canMove) {
				for (size_t i = 0; i < Images.tBlocks[Images.currentStage - 1].size(); i++) {
					if (Images.currentStage == HIDDEN && i == 2) continue;

					const auto& tblock = Images.tBlocks[Images.currentStage - 1][i];
					int tblockLeft = tblock.x;
					int tblockRight = tblock.x + tblock.width;
					int tblockTop = tblock.y;
					int tblockBottom = tblock.y + tblock.height;

					int coupaLeft = static_cast<int>(newCoupaX);
					int coupaRight = static_cast<int>(newCoupaX) + coupa.width;
					int coupaTop = coupa.y;
					int coupaBottom = coupa.y + coupa.height;

					bool overlapX = coupaRight > tblockLeft && coupaLeft < tblockRight;
					bool overlapY = coupaBottom > tblockTop && coupaTop < tblockBottom;

					if (overlapX && overlapY) {
						int prevCoupaRight = coupa.x + coupa.width;
						int prevCoupaLeft = coupa.x;
						if (coupa.direction == RIGHT && prevCoupaRight <= tblockLeft && coupaRight > tblockLeft) {
							coupa.direction = LEFT;
							newCoupaX = tblockLeft - coupa.width;
							canMove = false;
						}
						else if (coupa.direction == LEFT && prevCoupaLeft >= tblockRight && coupaLeft < tblockRight) {
							coupa.direction = RIGHT;
							newCoupaX = tblockRight;
							canMove = false;
						}
						break;
					}
				}
			}

			if (canMove) {
				coupa.x = newCoupaX;
			}

			coupa.fireballTimer += 0.016f;
			if (coupa.fireballTimer >= 2.0f) {
				Image_::Fireball fireball;
				fireball.width = 20;
				fireball.height = 20;
				fireball.active = true;
				fireball.isPlayerFireball = false; // 쿠파 파이어볼로 설정
				fireball.x = (coupa.direction == RIGHT) ? coupa.x + coupa.width : coupa.x - fireball.width;
				fireball.y = coupa.y + coupa.height / 4;
				fireball.velocityX = (coupa.direction == RIGHT) ? 6.0f : -6.0f;
				fireball.velocityY = 0.0f;
				Images.fireballs.push_back(fireball); // Pimage 대신 Images 사용
				coupa.fireballTimer = 0.0f;
			}

			if (coupa.invincibleTime > 0) {
				coupa.invincibleTime--;
				if (coupa.invincibleTime <= 0) {
					coupa.isInvincible = false;
				}
			}

			if (!isInvincible && !coupa.isInvincible) {
				int coupaLeft = coupa.x;
				int coupaRight = coupa.x + coupa.width;
				int coupaTop = coupa.y;
				int coupaBottom = coupa.y + coupa.height;

				int playerLeft = hitbox_.left;
				int playerRight = hitbox_.right;
				int playerTop = hitbox_.top;
				int playerBottom = hitbox_.bottom;

				bool overlapX = playerRight > coupaLeft && playerLeft < coupaRight;
				bool overlapY = playerBottom > coupaTop && playerTop < coupaBottom;

				if (overlapX && overlapY) {
					int prevPlayerBottom = prevY + playerHitboxHeight;
					if (prevPlayerBottom <= coupaTop + 5 && jumpVelocity_ > 0) {
						coupa.health--;
						coupa.isInvincible = true;
						coupa.invincibleTime = 120;
						jumpVelocity_ = JUMP_VELOCITY / 2;
						if (coupa.health <= 0) {
							coupa.isAlive = false;
						}
					}
					else {
						if (State() == TINO) {
							ResetPosition();
						}
						else if (State() == LARGETINO) {
							eatMushroom_ = false;
							isInvincible = true;
							invincibleTime = 180;
						}
						else if (State() == PAIRI) {
							eatFlower_ = false;
							eatMushroom_ = false;
							isInvincible = true;
							invincibleTime = 180;
						}
						else if (State() == LIZAMONG) {
							eatFlower_ = true;
							eatMushroom_ = false;
							isInvincible = true;
							invincibleTime = 180;
						}
					}
				}
			}
		}

		auto& coupaList = Images.coupas[Images.currentStage - 1];
		coupaList.erase(std::remove_if(coupaList.begin(), coupaList.end(),
			[](const Image_::Coupa& c) { return !c.isAlive; }), coupaList.end());
	}

	if (!Images.monsters[Images.currentStage - 1].empty() && !isInvincible) {
		for (auto& monster : Images.monsters[Images.currentStage - 1]) {
			if (!monster.isAlive) continue;

			if (monster.isFalling) {
				const float fallSpeed = 2.0f;
				monster.fallProgress += fallSpeed;
				monster.y += static_cast<int>(fallSpeed);
				if (monster.fallProgress >= 100.0f) {
					monster.isAlive = false;
				}
				continue;
			}

			bool onGround = false;
			int newMonsterY = monster.y;
			const float gravity = 2.0f;

			for (const auto& tblock : Images.tBlocks[Images.currentStage - 1]) {
				int tblockLeft = tblock.x;
				int tblockRight = tblock.x + tblock.width;
				int tblockTop = tblock.y;

				int monsterLeft = monster.x;
				int monsterRight = monster.x + monster.width;
				int monsterBottom = newMonsterY + monster.height;

				bool overlapX = monsterRight > tblockLeft && monsterLeft < tblockRight;
				bool overlapY = monsterBottom + static_cast<int>(gravity) >= tblockTop && monsterBottom <= tblockTop + 5;

				if (overlapX && overlapY) {
					newMonsterY = tblockTop - monster.height;
					onGround = true;
					break;
				}
			}

			for (const auto& block : Images.blocks[Images.currentStage - 1]) {
				int blockLeft = block.x;
				int blockRight = block.x + block.width;
				int blockTop = block.y;

				int monsterLeft = monster.x;
				int monsterRight = monster.x + monster.width;
				int monsterBottom = newMonsterY + monster.height;

				bool overlapX = monsterRight > blockLeft && monsterLeft < blockRight;
				bool overlapY = monsterBottom + static_cast<int>(gravity) >= blockTop && monsterBottom <= blockTop + 5;

				if (overlapX && overlapY) {
					newMonsterY = blockTop - monster.height;
					onGround = true;
					break;
				}
			}

			for (const auto& qblock : Images.questionBlocks[Images.currentStage - 1]) {
				int qblockLeft = qblock.x;
				int qblockRight = qblock.x + qblock.width;
				int qblockTop = qblock.y;

				int monsterLeft = monster.x;
				int monsterRight = monster.x + monster.width;
				int monsterBottom = newMonsterY + monster.height;

				bool overlapX = monsterRight > qblockLeft && monsterLeft < qblockRight;
				bool overlapY = monsterBottom + static_cast<int>(gravity) >= qblockTop && monsterBottom <= qblockTop + 5;

				if (overlapX && overlapY) {
					newMonsterY = qblockTop - monster.height;
					onGround = true;
					break;
				}
			}

			for (const auto& hole : Images.holes[Images.currentStage - 1]) {
				int holeLeft = hole.x;
				int holeRight = hole.x + hole.width;
				int holeTop = hole.y;

				int monsterBottomCenterX = monster.x + monster.width / 2;
				int monsterBottomY = newMonsterY + monster.height;

				bool withinHoleX = monsterBottomCenterX >= holeLeft && monsterBottomCenterX <= holeRight;
				bool atHoleTop = monsterBottomY >= holeTop - 5;

				if (withinHoleX && atHoleTop) {
					monster.isFalling = true;
					monster.fallProgress = 0.0f;
					onGround = false;
					break;
				}
			}

			if (!onGround) {
				monster.y += static_cast<int>(gravity);
			}
			else {
				monster.y = newMonsterY;
			}

			monster.directionTimer += 0.016f;
			if (monster.directionTimer >= monster.directionChangeInterval) {
				monster.directionChangeInterval = static_cast<float>(rand() % 6 + 5);
				monster.directionTimer = 0.0f;
				monster.direction = (monster.direction == LEFT) ? RIGHT : LEFT;
			}

			float newMonsterX = monster.x + (monster.direction == LEFT ? -monster.speed : monster.speed);

			bool canMove = true;
			for (const auto& block : Images.blocks[Images.currentStage - 1]) {
				int blockLeft = block.x;
				int blockRight = block.x + block.width;
				int blockTop = block.y;
				int blockBottom = block.y + block.height;

				int monsterLeft = static_cast<int>(newMonsterX);
				int monsterRight = static_cast<int>(newMonsterX) + monster.width;
				int monsterTop = monster.y;
				int monsterBottom = monster.y + monster.height;

				bool overlapX = monsterRight > blockLeft && monsterLeft < blockRight;
				bool overlapY = monsterBottom > blockTop && monsterTop < blockBottom;

				if (overlapX && overlapY) {
					int prevMonsterRight = monster.x + monster.width;
					int prevMonsterLeft = monster.x;
					if (monster.direction == RIGHT && prevMonsterRight <= blockLeft && monsterRight > blockLeft) {
						monster.direction = LEFT;
						newMonsterX = blockLeft - monster.width;
						canMove = false;
					}
					else if (monster.direction == LEFT && prevMonsterLeft >= blockRight && monsterLeft < blockRight) {
						monster.direction = RIGHT;
						newMonsterX = blockRight;
						canMove = false;
					}
					break;
				}
			}

			if (canMove) {
				for (const auto& qblock : Images.questionBlocks[Images.currentStage - 1]) {
					int qblockLeft = qblock.x;
					int qblockRight = qblock.x + qblock.width;
					int qblockTop = qblock.y;
					int qblockBottom = qblock.y + qblock.height;

					int monsterLeft = static_cast<int>(newMonsterX);
					int monsterRight = static_cast<int>(newMonsterX) + monster.width;
					int monsterTop = monster.y;
					int monsterBottom = monster.y + monster.height;

					bool overlapX = monsterRight > qblockLeft && monsterLeft < qblockRight;
					bool overlapY = monsterBottom > qblockTop && monsterTop < qblockBottom;

					if (overlapX && overlapY) {
						int prevMonsterRight = monster.x + monster.width;
						int prevMonsterLeft = monster.x;
						if (monster.direction == RIGHT && prevMonsterRight <= qblockLeft && monsterRight > qblockLeft) {
							monster.direction = LEFT;
							newMonsterX = qblockLeft - monster.width;
							canMove = false;
						}
						else if (monster.direction == LEFT && prevMonsterLeft >= qblockRight && monsterLeft < qblockRight) {
							monster.direction = RIGHT;
							newMonsterX = qblockRight;
							canMove = false;
						}
						break;
					}
				}
			}

			if (canMove) {
				for (size_t i = 0; i < Images.tBlocks[Images.currentStage - 1].size(); i++) {
					if (Images.currentStage == HIDDEN && i == 2) continue;

					const auto& tblock = Images.tBlocks[Images.currentStage - 1][i];
					int tblockLeft = tblock.x;
					int tblockRight = tblock.x + tblock.width;
					int tblockTop = tblock.y;
					int tblockBottom = tblock.y + tblock.height;

					int monsterLeft = static_cast<int>(newMonsterX);
					int monsterRight = static_cast<int>(newMonsterX) + monster.width;
					int monsterTop = monster.y;
					int monsterBottom = monster.y + monster.height;

					bool overlapX = monsterRight > tblockLeft && monsterLeft < tblockRight;
					bool overlapY = monsterBottom > tblockTop && monsterTop < tblockBottom;

					if (overlapX && overlapY) {
						int prevMonsterRight = monster.x + monster.width;
						int prevMonsterLeft = monster.x;
						if (monster.direction == RIGHT && prevMonsterRight <= tblockLeft && monsterRight > tblockLeft) {
							monster.direction = LEFT;
							newMonsterX = tblockLeft - monster.width;
							canMove = false;
						}
						else if (monster.direction == LEFT && prevMonsterLeft >= tblockRight && monsterLeft < tblockRight) {
							monster.direction = RIGHT;
							newMonsterX = tblockRight;
							canMove = false;
						}
						break;
					}
				}
			}

			if (canMove) {
				monster.x = newMonsterX;
			}

			int monsterLeft = monster.x;
			int monsterRight = monster.x + monster.width;
			int monsterTop = monster.y;
			int monsterBottom = monster.y + monster.height;

			int playerLeft = hitbox_.left;
			int playerRight = hitbox_.right;
			int playerTop = hitbox_.top;
			int playerBottom = hitbox_.bottom;

			bool overlapX = playerRight > monsterLeft && playerLeft < monsterRight;
			bool overlapY = playerBottom > monsterTop && playerTop < monsterBottom;

			if (overlapX && overlapY && !isInvincible) {
				int prevPlayerBottom = prevY + playerHitboxHeight;
				if (prevPlayerBottom <= monsterTop + 5 && jumpVelocity_ > 0) {
					monster.isAlive = false;
					jumpVelocity_ = JUMP_VELOCITY / 2;
				}
				else {
					if (State() == TINO) {
						ResetPosition();
					}
					else if (State() == LARGETINO) {
						eatMushroom_ = false;
						isInvincible = true;
						invincibleTime = 180;
					}
					else if (State() == PAIRI) {
						eatFlower_ = false;
						eatMushroom_ = false;
						isInvincible = true;
						invincibleTime = 180;
					}
					else if (State() == LIZAMONG) {
						eatFlower_ = true;
						eatMushroom_ = false;
						isInvincible = true;
						invincibleTime = 180;
					}
				}
			}
		}

		auto& monsterList = Images.monsters[Images.currentStage - 1];
		monsterList.erase(std::remove_if(monsterList.begin(), monsterList.end(),
			[](const Image_::Monster& m) { return !m.isAlive; }), monsterList.end());
	}

	if (intendToMoveLeft) {
		direct_ = LEFT;
		move_ = true;
		moved = true;
	}
	if (intendToMoveRight) {
		direct_ = RIGHT;
		move_ = true;
		moved = true;
	}
	if (intendToJump && !isJumping_ && !isFallingIntoHole) {
		isJumping_ = true;
		jumpVelocity_ = JUMP_VELOCITY;
		jumpKeyPressed = true;
	}
	if (intendToAttack && fireballCooldown <= 0) {
		Attack();
		attackKeyPressed = true;
	}

	if (!(GetAsyncKeyState(VK_UP) & 0x8000)) jumpKeyPressed = false;
	if (!(GetAsyncKeyState(VK_SPACE) & 0x8000)) attackKeyPressed = false;

	if (move_) {
		time++;
		if (time == 3) {
			if (State() == TINO || State() == LARGETINO || State() == LIZAMONG || State() == LIZAD) {
				imageNum = (imageNum + 1) % 3;
			}
			else if (State() == PAIRI) {
				if (isJumping_) imageNum = (imageNum + 1) % 5;
				else imageNum = (imageNum + 1) % 6;
			}
			time = 0;
		}
	}

	if (State() == LIZAMONG || State() == LARGETINO) {
		if (Images.NowStage() == STAGE1 || Images.NowStage() == STAGE2) defaultGroundY_ = 433;
		else if (Images.NowStage() == HIDDEN) defaultGroundY_ = 421;
		else if (Images.NowStage() == TUTORIAL) defaultGroundY_ = 451;
	}
	else {
		if (Images.NowStage() == STAGE1 || Images.NowStage() == STAGE2) defaultGroundY_ = 447;
		else if (Images.NowStage() == HIDDEN) defaultGroundY_ = 435;
		else if (Images.NowStage() == TUTORIAL) defaultGroundY_ = 465;
	}

	bool canMoveHorizontally = true;
	if (intendToMoveLeft) newX -= MOVE;
	if (intendToMoveRight) newX += MOVE;

	if (intendToMoveLeft || intendToMoveRight) {
		int checkHitboxX = newX;
		switch (State()) {
		case TINO: checkHitboxX += 10; break;
		case LARGETINO:
			if (direct_ == RIGHT) checkHitboxX += 10;
			else checkHitboxX += 12;
			break;
		case PAIRI: if (direct_ == RIGHT) checkHitboxX += 14; break;
		case LIZAD: if (direct_ == RIGHT) checkHitboxX += 20; break;
		case LIZAMONG:
			if (direct_ == RIGHT) checkHitboxX += 26;
			else checkHitboxX += 8;
			break;
		default: if (direct_ == RIGHT) checkHitboxX += 14; break;
		}

		for (const auto& block : Images.blocks[Images.currentStage - 1]) {
			int blockLeft = block.x;
			int blockRight = block.x + block.width;
			int blockTop = block.y;
			int blockBottom = block.y + block.height;

			int newPlayerLeft = checkHitboxX;
			int newPlayerRight = checkHitboxX + playerHitboxWidth;
			int playerTop = playerHitboxY;
			int playerBottom = playerHitboxY + playerHitboxHeight;

			bool overlapX = newPlayerRight > blockLeft && newPlayerLeft < blockRight;
			bool overlapY = playerBottom > blockTop && playerTop < blockBottom;

			if (overlapX && overlapY) {
				canMoveHorizontally = false;
				break;
			}
		}

		if (canMoveHorizontally) {
			for (const auto& qblock : Images.questionBlocks[Images.currentStage - 1]) {
				int qblockLeft = qblock.x;
				int qblockRight = qblock.x + qblock.width;
				int qblockTop = qblock.y;
				int qblockBottom = qblock.y + qblock.height;

				int newPlayerLeft = checkHitboxX;
				int newPlayerRight = checkHitboxX + playerHitboxWidth;
				int playerTop = playerHitboxY;
				int playerBottom = playerHitboxY + playerHitboxHeight;

				bool overlapX = newPlayerRight > qblockLeft && newPlayerLeft < qblockRight;
				bool overlapY = playerBottom > qblockTop && playerTop < qblockBottom;

				if (overlapX && overlapY) {
					canMoveHorizontally = false;
					break;
				}
			}
		}

		if (canMoveHorizontally) {
			for (size_t i = 0; i < Images.tBlocks[Images.currentStage - 1].size(); i++) {
				if (Images.currentStage == HIDDEN && i == 2) continue;

				const auto& tblock = Images.tBlocks[Images.currentStage - 1][i];
				int tblockLeft = tblock.x;
				int tblockRight = tblock.x + tblock.width;
				int tblockTop = tblock.y;
				int tblockBottom = tblock.y + tblock.height;

				int newPlayerLeft = checkHitboxX;
				int newPlayerRight = checkHitboxX + playerHitboxWidth;
				int playerTop = playerHitboxY;
				int playerBottom = playerHitboxY + playerHitboxHeight;

				bool overlapX = newPlayerRight > tblockLeft && newPlayerLeft < tblockRight;
				bool overlapY = playerBottom > tblockTop && playerTop < tblockBottom;

				if (overlapX && overlapY) {
					canMoveHorizontally = false;
					break;
				}
			}
		}

		if (canMoveHorizontally) {
			x_ = newX;
		}
		else {
			move_ = true;
			imageNum = 0;
		}
	}

	if (isJumping_) {
		y_ += static_cast<int>(jumpVelocity_);
		jumpVelocity_ += GRAVITY;
	}

	GetHitbox(playerHitboxX, playerHitboxY, playerHitboxWidth, playerHitboxHeight);

	bool onBlock = false;
	int newGroundY = defaultGroundY_;

	for (const auto& block : Images.blocks[Images.currentStage - 1]) {
		int blockLeft = block.x;
		int blockRight = block.x + block.width;
		int blockTop = block.y;
		int blockBottom = block.y + block.height;

		int playerLeft = hitbox_.left;
		int playerRight = hitbox_.right;
		int playerTop = hitbox_.top;
		int playerBottom = hitbox_.bottom;

		bool overlapX = playerRight > blockLeft && playerLeft < blockRight;
		bool overlapY = playerBottom > blockTop && playerTop < blockBottom;

		if (overlapX && overlapY) {
			int prevPlayerBottom = prevY + playerHitboxHeight;
			if (prevPlayerBottom <= blockTop + 5 && jumpVelocity_ > 0) {
				y_ = blockTop - playerHitboxHeight;
				isJumping_ = false;
				jumpVelocity_ = 0.0f;
				onBlock = true;
				newGroundY = blockTop - playerHitboxHeight;
			}
			else if (prevY >= blockBottom && jumpVelocity_ < 0) {
				y_ = blockBottom;
				jumpVelocity_ = 0.0f;
			}
		}
	}

	if (!onBlock) {
		for (auto& qblock : Images.questionBlocks[Images.currentStage - 1]) {
			int qblockLeft = qblock.x;
			int qblockRight = qblock.x + qblock.width;
			int qblockTop = qblock.y;
			int qblockBottom = qblock.y + qblock.height;

			int playerLeft = hitbox_.left;
			int playerRight = hitbox_.right;
			int playerTop = hitbox_.top;
			int playerBottom = hitbox_.bottom;

			bool overlapX = playerRight > qblockLeft && playerLeft < qblockRight;
			bool overlapY = playerBottom > qblockTop && playerTop < qblockBottom;

			if (overlapX && overlapY) {
				int prevPlayerBottom = prevY + playerHitboxHeight;
				int prevPlayerTop = prevY;
				if (prevPlayerBottom <= qblockTop && jumpVelocity_ > 0) {
					y_ = qblockTop - playerHitboxHeight;
					isJumping_ = false;
					jumpVelocity_ = 0.0f;
					onBlock = true;
					newGroundY = qblockTop - playerHitboxHeight;
				}
				else if (prevPlayerTop >= qblockBottom && jumpVelocity_ < 0) {
					if (!qblock.hit) {
						// 아이템 생성
						qblock.hit = true;
						Image_::Item item;
						item.x = qblock.x;
						item.y = qblockTop - 32; // 아이템을 QuestionBlock 위에 배치
						item.width = 32;
						item.height = 32;
						item.type = Type(Item_type); // 랜덤 타입 (0: 버섯, 1: 꽃)
						item.isActive = true;
						item.direction = (rand() % 2 == 0) ? LEFT : RIGHT;
						item.speed = 1.0f;
						item.directionTimer = 0.0f;
						item.directionChangeInterval = static_cast<float>(rand() % 6 + 5);
						Images.items[Images.currentStage - 1].push_back(item);
					}
					y_ = qblockBottom;
					jumpVelocity_ = 0.0f;
				}
			}
		}
	}

	if (!onBlock) {
		for (const auto& tblock : Images.tBlocks[Images.currentStage - 1]) {
			int tblockLeft = tblock.x;
			int tblockRight = tblock.x + tblock.width;
			int tblockTop = tblock.y;
			int tblockBottom = tblock.y + tblock.height;

			int playerLeft = playerHitboxX;
			int playerRight = playerHitboxX + playerHitboxWidth;
			int playerTop = playerHitboxY;
			int playerBottom = playerHitboxY + playerHitboxHeight;

			bool overlapX = playerRight > tblockLeft && playerLeft < tblockRight;
			bool overlapY = playerBottom > tblockTop && playerTop < tblockBottom;

			if (overlapX && overlapY) {
				int prevPlayerBottom = prevY + playerHitboxHeight;
				int prevPlayerTop = prevY;

				if (prevPlayerBottom <= tblockTop && jumpVelocity_ > 0) {
					y_ = tblockTop - playerHitboxHeight;
					isJumping_ = false;
					jumpVelocity_ = 0.0f;
					onBlock = true;
					newGroundY = tblockTop - playerHitboxHeight;
				}
				else if (prevPlayerTop >= tblockBottom && jumpVelocity_ < 0) {
					y_ = tblockBottom;
					jumpVelocity_ = 0.0f;
				}
			}
		}
	}

	if (!onBlock) {
		for (const auto& hole : Images.holes[Images.currentStage - 1]) {
			int holeLeft = hole.x;
			int holeRight = hole.x + hole.width;
			int holeTop = hole.y;

			int playerBottomCenterX = hitbox_.left + (hitbox_.right - hitbox_.left) / 2;
			int playerBottomY = hitbox_.bottom;

			bool withinHoleX = playerBottomCenterX >= holeLeft && playerBottomCenterX <= holeRight;

			int prevPlayerBottom = prevY + playerHitboxHeight;
			bool atHoleTop = prevPlayerBottom >= holeTop - 5 && !isJumping_;

			if (withinHoleX && atHoleTop) {
				isFallingIntoHole = true;
				fallProgress_ = 0.0f;
				break;
			}
		}
	}

	if (!isFallingIntoHole && !isTouchingFlag) {
		for (const auto& flag : Images.flagBlocks[Images.currentStage - 1]) {
			int flagLeft = flag.x;
			int flagRight = flag.x + flag.width;
			int flagTop = flag.y;
			int flagBottom = flag.y + flag.height;

			int playerLeft = hitbox_.left;
			int playerRight = hitbox_.right;
			int playerTop = hitbox_.top;
			int playerBottom = hitbox_.bottom;

			bool overlapX = playerRight > flagLeft && playerLeft < flagRight;
			bool overlapY = playerBottom > flagTop && playerTop < flagBottom;

			if (overlapX && overlapY) {
				isTouchingFlag = true;
				flagBlockStage = Images.currentStage;
				flagBottomY = flagBottom;
				// 플레이어 x 좌표를 깃발 왼쪽에 고정
				x_ = flagLeft - playerHitboxWidth;
				// y 좌표는 충돌 시점의 y_ 유지 (깃발 상단 대신)
				// y_는 이미 현재 위치이므로 변경 불필요
				jumpVelocity_ = 0.0f;
				isJumping_ = false;
				move_ = false;
				direct_ = RIGHT;
				break;
			}
		}
	}

	if (!isFallingIntoHole && !Images.tBlocks[Images.currentStage - 1].empty() && Images.tBlocks[Images.currentStage - 1].size() >= 4) {
		const auto& tblock4 = Images.tBlocks[Images.currentStage - 1][3];
		int tblockLeft = tblock4.x;
		int tblockRight = tblock4.x + tblock4.width;
		int tblockTop = tblock4.y;

		int playerLeft = playerHitboxX;
		int playerRight = playerHitboxX + playerHitboxWidth;
		bool overlapX = playerRight > tblockLeft && playerLeft < tblockRight;

		if (overlapX && y_ == tblockTop - playerHitboxHeight && GetAsyncKeyState(VK_DOWN) & 0x8000) {
			Images.isTransitioning = true;
			Images.transitionTimer = 0.0f;
			Images.EnterHiddenStage();
			//Player.ResetPosition(); // 초기 위치 설정만 유지
		}
	}

	if (!isFallingIntoHole && Images.currentStage == HIDDEN && !Images.tBlocks[Images.currentStage - 1].empty()) {
		const auto& tblock3 = Images.tBlocks[Images.currentStage - 1][2];
		int tblockLeft = tblock3.x;
		int tblockTop = tblock3.y;
		int tblockBottom = tblock3.y + tblock3.height;

		int playerRight = hitbox_.right;
		int playerTop = hitbox_.top;
		int playerBottom = hitbox_.bottom;

		int prevHitboxX, prevHitboxY, prevHitboxWidth, prevHitboxHeight;
		int tempX = x_;
		x_ = prevX;
		GetHitbox(prevHitboxX, prevHitboxY, prevHitboxWidth, prevHitboxHeight);
		int prevPlayerRight = prevHitboxX + prevHitboxWidth;
		x_ = tempX;

		bool collideWithLeftFace = intendToMoveRight && playerRight + 5 >= tblockLeft && prevPlayerRight < tblockLeft;
		bool yOverlap = playerBottom > tblockTop && playerTop < tblockBottom;

		if (collideWithLeftFace && yOverlap) {
			Images.currentStage = STAGE1;
			Images.hidden = false;
			Images.stage1 = true;
			x_ = 2605; // 1스테이지 파이프 출구 근처
			y_ = 350;  // 적절한 y 좌표
			Images.isTransitioning = true;
			Images.transitionTimer = 0.0f;
			// ResetPosition() 호출 제거, 상태 유지
		}
	}

	// 아이템 움직임 및 충돌 처리
	if (!Images.items[Images.currentStage - 1].empty()) {
		for (auto& item : Images.items[Images.currentStage - 1]) {
			if (!item.isActive) continue;

			// 중력 적용
			int newItemY = item.y;
			bool onGround = false;
			const float gravity = 2.0f;

			for (const auto& tblock : Images.tBlocks[Images.currentStage - 1]) {
				int tblockLeft = tblock.x;
				int tblockRight = tblock.x + tblock.width;
				int tblockTop = tblock.y;

				int itemLeft = item.x;
				int itemRight = item.x + item.width;
				int itemBottom = newItemY + item.height;

				bool overlapX = itemRight > tblockLeft && itemLeft < tblockRight;
				bool overlapY = itemBottom + static_cast<int>(gravity) >= tblockTop && itemBottom <= tblockTop + 5;

				if (overlapX && overlapY) {
					newItemY = tblockTop - item.height;
					onGround = true;
					break;
				}
			}

			for (const auto& block : Images.blocks[Images.currentStage - 1]) {
				int blockLeft = block.x;
				int blockRight = block.x + block.width;
				int blockTop = block.y;

				int itemLeft = item.x;
				int itemRight = item.x + item.width;
				int itemBottom = newItemY + item.height;

				bool overlapX = itemRight > blockLeft && itemLeft < blockRight;
				bool overlapY = itemBottom + static_cast<int>(gravity) >= blockTop && itemBottom <= blockTop + 5;

				if (overlapX && overlapY) {
					newItemY = blockTop - item.height;
					onGround = true;
					break;
				}
			}

			for (const auto& qblock : Images.questionBlocks[Images.currentStage - 1]) {
				int qblockLeft = qblock.x;
				int qblockRight = qblock.x + qblock.width;
				int qblockTop = qblock.y;

				int itemLeft = item.x;
				int itemRight = item.x + item.width;
				int itemBottom = newItemY + item.height;

				bool overlapX = itemRight > qblockLeft && itemLeft < qblockRight;
				bool overlapY = itemBottom + static_cast<int>(gravity) >= qblockTop && itemBottom <= qblockTop + 5;

				if (overlapX && overlapY) {
					newItemY = qblockTop - item.height;
					onGround = true;
					break;
				}
			}

			if (!onGround) {
				item.y += static_cast<int>(gravity);
			}
			else {
				item.y = newItemY;
			}

			// 방향 변경 로직
			item.directionTimer += 0.016f;
			if (item.directionTimer >= item.directionChangeInterval) {
				item.directionChangeInterval = static_cast<float>(rand() % 6 + 5);
				item.directionTimer = 0.0f;
				item.direction = (item.direction == LEFT) ? RIGHT : LEFT;
			}

			float newItemX = item.x + (item.direction == LEFT ? -item.speed : item.speed);

			bool canMove = true;
			for (const auto& block : Images.blocks[Images.currentStage - 1]) {
				int blockLeft = block.x;
				int blockRight = block.x + block.width;
				int blockTop = block.y;
				int blockBottom = block.y + block.height;

				int itemLeft = static_cast<int>(newItemX);
				int itemRight = static_cast<int>(newItemX) + item.width;
				int itemTop = item.y;
				int itemBottom = item.y + item.height;

				bool overlapX = itemRight > blockLeft && itemLeft < blockRight;
				bool overlapY = itemBottom > blockTop && itemTop < blockBottom;

				if (overlapX && overlapY) {
					int prevItemRight = item.x + item.width;
					int prevItemLeft = item.x;
					if (item.direction == RIGHT && prevItemRight <= blockLeft && itemRight > blockLeft) {
						item.direction = LEFT;
						newItemX = blockLeft - item.width;
						canMove = false;
					}
					else if (item.direction == LEFT && prevItemLeft >= blockRight && itemLeft < blockRight) {
						item.direction = RIGHT;
						newItemX = blockRight;
						canMove = false;
					}
					break;
				}
			}

			if (canMove) {
				for (const auto& qblock : Images.questionBlocks[Images.currentStage - 1]) {
					int qblockLeft = qblock.x;
					int qblockRight = qblock.x + qblock.width;
					int qblockTop = qblock.y;
					int qblockBottom = qblock.y + qblock.height;

					int itemLeft = static_cast<int>(newItemX);
					int itemRight = static_cast<int>(newItemX) + item.width;
					int itemTop = item.y;
					int itemBottom = item.y + item.height;

					bool overlapX = itemRight > qblockLeft && itemLeft < qblockRight;
					bool overlapY = itemBottom > qblockTop && itemTop < qblockBottom;

					if (overlapX && overlapY) {
						int prevItemRight = item.x + item.width;
						int prevItemLeft = item.x;
						if (item.direction == RIGHT && prevItemRight <= qblockLeft && itemRight > qblockLeft) {
							item.direction = LEFT;
							newItemX = qblockLeft - item.width;
							canMove = false;
						}
						else if (item.direction == LEFT && prevItemLeft >= qblockRight && itemLeft < qblockRight) {
							item.direction = RIGHT;
							newItemX = qblockRight;
							canMove = false;
						}
						break;
					}
				}
			}

			if (canMove) {
				item.x = newItemX;
			}

			// 플레이어와 아이템 충돌 체크
			int itemLeft = item.x;
			int itemRight = item.x + item.width;
			int itemTop = item.y;
			int itemBottom = item.y + item.height;

			int playerLeft = hitbox_.left;
			int playerRight = hitbox_.right;
			int playerTop = hitbox_.top;
			int playerBottom = hitbox_.bottom;

			bool overlapX = playerRight > itemLeft && playerLeft < itemRight;
			bool overlapY = playerBottom > itemTop && playerTop < itemBottom;

			if (overlapX && overlapY && item.isActive) {
				if (item.type == 0) { // 버섯
					if (!eatMushroom_) {
						groundY_ -= 15; // 크기 증가 반영
						y_ -= 15;
					}
					eatMushroom_ = true;
				}
				else if (item.type == 1) { // 꽃
					eatFlower_ = true;
				}
				item.isActive = false;
			}
		}

		// 비활성화된 아이템 제거
		auto& itemList = Images.items[Images.currentStage - 1];
		itemList.erase(std::remove_if(itemList.begin(), itemList.end(),
			[](const Image_::Item& i) { return !i.isActive; }), itemList.end());
	}

	if (!isFallingIntoHole && groundY_ != defaultGroundY_) {
		bool stillOnBlock = false;

		for (const auto& block : Images.blocks[Images.currentStage - 1]) {
			int blockLeft = block.x;
			int blockRight = block.x + block.width;
			int blockTop = block.y;

			int playerLeft = hitbox_.left;
			int playerRight = hitbox_.right;
			bool overlapX = playerRight > blockLeft && playerLeft < blockRight;

			if (overlapX && groundY_ == blockTop - playerHitboxHeight) {
				stillOnBlock = true;
				break;
			}
		}

		if (!stillOnBlock) {
			for (const auto& qblock : Images.questionBlocks[Images.currentStage - 1]) {
				int qblockLeft = qblock.x;
				int qblockRight = qblock.x + qblock.width;
				int qblockTop = qblock.y;

				int playerLeft = hitbox_.left;
				int playerRight = hitbox_.right;
				bool overlapX = playerRight > qblockLeft && playerLeft < qblockRight;

				if (overlapX && groundY_ == qblockTop - playerHitboxHeight) {
					stillOnBlock = true;
					break;
				}
			}
		}

		if (!stillOnBlock) {
			for (const auto& tblock : Images.tBlocks[Images.currentStage - 1]) {
				int tblockLeft = tblock.x;
				int tblockRight = tblock.x + tblock.width;
				int tblockTop = tblock.y;

				int playerLeft = playerHitboxX;
				int playerRight = playerHitboxX + playerHitboxWidth;
				bool overlapX = playerRight > tblockLeft && playerLeft < tblockRight;

				if (overlapX && groundY_ == tblockTop - playerHitboxHeight) {
					stillOnBlock = true;
					break;
				}
			}
		}

		if (!stillOnBlock) {
			groundY_ = defaultGroundY_;
			if (y_ < defaultGroundY_) {
				isJumping_ = true;
			}
		}
	}

	if (onBlock) {
		groundY_ = newGroundY;
	}

	if (!onBlock && !isFallingIntoHole && y_ >= defaultGroundY_) {
		y_ = defaultGroundY_;
		isJumping_ = false;
		jumpVelocity_ = 0.0f;
	}

	int stageWidth = (Images.NowStage() == TUTORIAL ? Pimage.mStageTutorial.GetWidth() :
		Images.NowStage() == STAGE1 ? Pimage.mStage1.GetWidth() :
		Images.NowStage() == STAGE2 ? Pimage.mStage2.GetWidth() :
		Images.NowStage() == HIDDEN ? Pimage.mStageHidden.GetWidth() : 0);

	int playerWidth = 0;
	switch (State()) {
	case TINO: playerWidth = 36; break;
	case PAIRI: playerWidth = 39; break;
	case LIZAD: playerWidth = 46; break;
	case LIZAMONG: playerWidth = 66; break;
	default: playerWidth = 36;
	}

	if (x_ < 0) {
		x_ = 0;
		move_ = true;
		imageNum = 0;
	}
	if (x_ + playerWidth > stageWidth) {
		x_ = stageWidth - playerWidth;
		move_ = true;
		imageNum = 0;
	}

	if (!moved) {
		move_ = false;
		if (!isJumping_) imageNum = 0;
		else if (isJumping_) {
			time++;
			if (time == 3) {
				if (State() == PAIRI) {
					imageNum = (imageNum + 1) % 3;
				}
				else if (State() == LIZAMONG) {
					imageNum = (imageNum + 1) % 4;
				}
				time = 0;
			}
		}
	}
	else if (!canMoveHorizontally && (intendToMoveLeft || intendToMoveRight) && !isJumping_) {
		move_ = true;
		imageNum = 0;
	}
	if (fireballCooldown > 0) {
		fireballCooldown--;
	}
	State();
}


void Player_::Attack() {
	if (!eatFlower_) return;

	int playerHitboxX, playerHitboxY, playerHitboxWidth, playerHitboxHeight;
	GetHitbox(playerHitboxX, playerHitboxY, playerHitboxWidth, playerHitboxHeight);

	Image_::Fireball fireball;
	fireball.width = 20;
	fireball.height = 20;
	fireball.active = true;
	fireball.isPlayerFireball = true; // 플레이어 파이어볼로 설정

	if (direct_ == RIGHT) {
		fireball.x = hitbox_.right - fireball.width;
		fireball.velocityX = 6.0f;
	}
	else {
		fireball.x = hitbox_.left;
		fireball.velocityX = -6.0f;
	}
	fireball.y = playerHitboxY + playerHitboxHeight / 4;
	fireball.velocityY = 0.0f;

	Images.fireballs.push_back(fireball); // Pimage 대신 Images 사용
	fireballCooldown = 15;

	WCHAR buffer[100];
	swprintf_s(buffer, L"Fireball shot: x=%d, y=%d, velocityX=%.2f\n", fireball.x, fireball.y, fireball.velocityX);
	OutputDebugStringW(buffer);
}

void Player_::FireballMove() {
	if (!Images.fireballs.empty()) {
		auto& fireballs = Images.fireballs;
		for (auto it = fireballs.begin(); it != fireballs.end(); /* 증가 연산은 내부에서 */) {
			if (!it->active) {
				++it;
				continue;
			}

			it->x += static_cast<int>(it->velocityX);
			it->y += static_cast<int>(it->velocityY);
			it->velocityY += GRAVITY;

			int cameraX = x_ - 400;
			if (cameraX < 0) cameraX = 0;
			int stageWidth = (Images.NowStage() == TUTORIAL ? Images.mStageTutorial.GetWidth() :
				Images.NowStage() == STAGE1 ? Images.mStage1.GetWidth() :
				Images.NowStage() == STAGE2 ? Images.mStage2.GetWidth() :
				Images.NowStage() == HIDDEN ? Images.mStageHidden.GetWidth() : 0);
			if (cameraX > stageWidth - wRect.right) {
				cameraX = stageWidth - wRect.right;
			}

			int playerHitboxX, playerHitboxY, playerHitboxWidth, playerHitboxHeight;
			GetHitbox(playerHitboxX, playerHitboxY, playerHitboxWidth, playerHitboxHeight);

			// 구멍 체크
			bool isOverHole = false;
			for (const auto& hole : Images.holes[Images.currentStage - 1]) {
				int holeLeft = hole.x;
				int holeRight = hole.x + hole.width;
				int fireballCenterX = it->x + it->width / 2;
				if (fireballCenterX >= holeLeft && fireballCenterX <= holeRight) {
					isOverHole = true;
					break;
				}
			}

			// 지면 튕김
			if (!isOverHole && it->y + it->height <= defaultGroundY_ + playerHitboxHeight + 10 && it->y + it->height >= defaultGroundY_ + playerHitboxHeight - 5) {
				it->y = defaultGroundY_ + playerHitboxHeight - it->height;
				it->velocityY = -6.0f;
			}

			int fireballScreenX = it->x - cameraX;
			if (fireballScreenX + it->width < 0 || fireballScreenX > wRect.right) {
				it = fireballs.erase(it);
				continue;
			}

			// 파이어볼과 쿠파 충돌 체크
			for (auto& coupa : Images.coupas[Images.currentStage - 1]) {
				if (!coupa.isAlive || coupa.isInvincible) continue;

				int fireballLeft = it->x;
				int fireballRight = it->x + it->width;
				int fireballTop = it->y;
				int fireballBottom = it->y + it->height;

				int coupaLeft = coupa.x;
				int coupaRight = coupa.x + coupa.width;
				int coupaTop = coupa.y;
				int coupaBottom = coupa.y + coupa.height;

				bool overlapX = fireballRight > coupaLeft && fireballLeft < coupaRight;
				bool overlapY = fireballBottom > coupaTop && fireballTop < coupaBottom;

				if (overlapX && overlapY && it->isPlayerFireball) { // 플레이어 파이어볼만 쿠파에 영향
					coupa.health--;
					coupa.isInvincible = true;
					coupa.invincibleTime = 120; // 2초
					it->active = false;
					if (coupa.health <= 0) {
						coupa.isAlive = false;
					}
					break;
				}
			}

			// 파이어볼과 플레이어 충돌 체크 (쿠파의 파이어볼)
			if (!isInvincible && !it->isPlayerFireball) { // 쿠파 파이어볼만 플레이어에 영향
				int fireballLeft = it->x;
				int fireballRight = it->x + it->width;
				int fireballTop = it->y;
				int fireballBottom = it->y + it->height;

				int playerLeft = hitbox_.left;
				int playerRight = hitbox_.right;
				int playerTop = hitbox_.top;
				int playerBottom = hitbox_.bottom;

				bool overlapX = fireballRight > playerLeft && fireballLeft < playerRight;
				bool overlapY = fireballBottom > playerTop && fireballTop < playerBottom;

				if (overlapX && overlapY) {
					it->active = false;
					if (State() == TINO) {
						ResetPosition();
					}
					else if (State() == LARGETINO) {
						eatMushroom_ = false;
						isInvincible = true;
						invincibleTime = 180;
					}
					else if (State() == PAIRI) {
						eatFlower_ = false;
						eatMushroom_ = false;
						isInvincible = true;
						invincibleTime = 180;
					}
					else if (State() == LIZAMONG) {
						eatFlower_ = true;
						eatMushroom_ = false;
						isInvincible = true;
						invincibleTime = 180;
					}
				}
			}

			// 파이어볼과 몬스터 충돌 체크
			auto& monsters = Images.monsters[Images.currentStage - 1];
			for (auto monsterIt = monsters.begin(); monsterIt != monsters.end(); /* 증가 연산은 내부에서 */) {
				if (!monsterIt->isAlive) {
					++monsterIt;
					continue;
				}

				int fireballLeft = it->x;
				int fireballRight = it->x + it->width;
				int fireballTop = it->y;
				int fireballBottom = it->y + it->height;

				int monsterLeft = monsterIt->x;
				int monsterRight = monsterIt->x + monsterIt->width;
				int monsterTop = monsterIt->y;
				int monsterBottom = monsterIt->y + monsterIt->height;

				bool overlapX = fireballRight > monsterLeft && fireballLeft < monsterRight;
				bool overlapY = fireballBottom > monsterTop && fireballTop < monsterBottom;

				if (overlapX && overlapY) {
					monsterIt->isAlive = false;
					it->active = false;
					break;
				}
				++monsterIt;
			}

			bool hitWall = false;
			for (const auto& block : Images.blocks[Images.currentStage - 1]) {
				int blockLeft = block.x;
				int blockRight = block.x + block.width;
				int blockTop = block.y;
				int blockBottom = block.y + block.height;

				int fireballLeft = it->x;
				int fireballRight = it->x + it->width;
				int fireballTop = it->y;
				int fireballBottom = it->y + it->height;

				bool overlapX = fireballRight > blockLeft && fireballLeft < blockRight;
				bool overlapY = fireballBottom > blockTop && fireballTop < blockBottom;

				if (overlapX && overlapY) {
					int prevFireballBottom = fireballBottom - static_cast<int>(it->velocityY);
					int prevFireballTop = fireballTop - static_cast<int>(it->velocityY);
					int prevFireballRight = fireballRight - static_cast<int>(it->velocityX);
					int prevFireballLeft = fireballLeft - static_cast<int>(it->velocityX);

					if (prevFireballBottom <= blockTop && fireballBottom > blockTop && it->velocityY > 0) {
						it->y = blockTop - it->height;
						it->velocityY = -4.0f;
					}
					else if ((prevFireballRight <= blockLeft && fireballRight > blockLeft) ||
						(prevFireballLeft >= blockRight && fireballLeft < blockRight) ||
						(prevFireballTop >= blockBottom && fireballTop < blockBottom)) {
						hitWall = true;
					}
					break;
				}
			}

			if (!hitWall) {
				for (const auto& qblock : Images.questionBlocks[Images.currentStage - 1]) {
					int qblockLeft = qblock.x;
					int qblockRight = qblock.x + qblock.width;
					int qblockTop = qblock.y;
					int qblockBottom = qblock.y + qblock.height;

					int fireballLeft = it->x;
					int fireballRight = it->x + it->width;
					int fireballTop = it->y;
					int fireballBottom = it->y + it->height;

					bool overlapX = fireballRight > qblockLeft && fireballLeft < qblockRight;
					bool overlapY = fireballBottom > qblockTop && fireballTop < qblockBottom;

					if (overlapX && overlapY) {
						int prevFireballBottom = fireballBottom - static_cast<int>(it->velocityY);
						int prevFireballTop = fireballTop - static_cast<int>(it->velocityY);
						int prevFireballRight = fireballRight - static_cast<int>(it->velocityX);
						int prevFireballLeft = fireballLeft - static_cast<int>(it->velocityX);

						if (prevFireballBottom <= qblockTop && fireballBottom > qblockTop && it->velocityY > 0) {
							it->y = qblockTop - it->height;
							it->velocityY = -4.0f;
						}
						else if ((prevFireballRight <= qblockLeft && fireballRight > qblockLeft) ||
							(prevFireballLeft >= qblockRight && fireballLeft < qblockRight) ||
							(prevFireballTop >= qblockBottom && fireballTop < qblockBottom)) {
							hitWall = true;
						}
						break;
					}
				}
			}

			if (!hitWall) {
				for (size_t i = 0; i < Images.tBlocks[Images.currentStage - 1].size(); i++) {
					const auto& tblock = Images.tBlocks[Images.currentStage - 1][i];
					int tblockLeft = tblock.x;
					int tblockRight = tblock.x + tblock.width;
					int tblockTop = tblock.y;
					int tblockBottom = tblock.y + tblock.height;

					int fireballLeft = it->x;
					int fireballRight = it->x + it->width;
					int fireballTop = it->y;
					int fireballBottom = it->y + it->height;

					bool overlapX = fireballRight > tblockLeft && fireballLeft < tblockRight;
					bool overlapY = fireballBottom > tblockTop && fireballTop < tblockBottom;

					if (overlapX && overlapY) {
						int prevFireballBottom = fireballBottom - static_cast<int>(it->velocityY);
						int prevFireballTop = fireballTop - static_cast<int>(it->velocityY);
						int prevFireballRight = fireballRight - static_cast<int>(it->velocityX);
						int prevFireballLeft = fireballLeft - static_cast<int>(it->velocityX);

						if (prevFireballBottom <= tblockTop && fireballBottom > tblockTop && it->velocityY > 0) {
							it->y = tblockTop - it->height;
							it->velocityY = -4.0f;
						}
						else if ((prevFireballRight <= tblockLeft && fireballRight > tblockLeft) ||
							(prevFireballLeft >= tblockRight && fireballLeft < tblockRight) ||
							(prevFireballTop >= tblockBottom && fireballTop < tblockBottom)) {
							hitWall = true;
						}
						break;
					}
				}
			}

			if (hitWall || it->x < 0 || it->x + it->width > stageWidth || !it->active) {
				it = fireballs.erase(it);
				continue;
			}

			it->time++;
			if (it->time == 3) {
				it->imageNum = (it->imageNum + 1) % 4;
				it->time = 0;
			}
			++it;
		}

		// 죽은 몬스터 제거
		auto& monsterList = Images.monsters[Images.currentStage - 1];
		monsterList.erase(std::remove_if(monsterList.begin(), monsterList.end(),
			[](const Image_::Monster& m) { return !m.isAlive; }), monsterList.end());

		// 죽은 쿠파 제거
		auto& coupaList = Images.coupas[Images.currentStage - 1];
		coupaList.erase(std::remove_if(coupaList.begin(), coupaList.end(),
			[](const Image_::Coupa& c) { return !c.isAlive; }), coupaList.end());
	}
}

int Player_::State() {
	if (!eatFlower_) {
		if (!eatMushroom_) return TINO;
		else if (eatMushroom_) {
			return LARGETINO;
		}
	}
	else {
		if (!eatMushroom_) return PAIRI;
		else return LIZAMONG;
	}
}

void Image_::ImageInit() {
	mStartScreen.Load(TEXT("Image/시작.png"));
	Player_Move_Tino.Load(TEXT("Image/티노 스프라이트.png"));
	Player_Move_Pairi.Load(TEXT("Image/파이리 스프라이트.png"));
	Player_Move_Lizamong.Load(TEXT("Image/리자몽 스프라이트.png"));

	Player_Attack_Tino.Load(TEXT("Image/티노 스프라이트.png"));
	Player_Attack_Pairi.Load(TEXT("Image/파이리 스프라이트.png"));
	Player_Attack_Lizamong.Load(TEXT("Image/리자몽 스프라이트.png"));

	mStageTutorial.Load(TEXT("Image/튜토리얼.png"));
	mStage1.Load(TEXT("Image/월드 1.png"));
	mStage2.Load(TEXT("Image/월드 2.png"));
	mStageHidden.Load(TEXT("Image/히든 스테이지.png"));

	blockImage.Load(TEXT("Image/BrickBlockBrown.png"));
	questionBlockImage[0].Load(TEXT("Image/QuestionBlock.gif"));
	questionBlockImage[1].Load(TEXT("Image/HitQuestionBlock.png"));

	monster.Load(TEXT("Image/굼바.png"));

	FireballImage.Load(TEXT("Image/Fireball.png"));

	coupaImage.Load(TEXT("Image/찐쿠파.png"));

	Item_Flower.Load(TEXT("Image/꽃.png"));
	Item_Mushroom.Load(TEXT("Image/버섯.png"));

	tutorial = stage1 = stage2 = hidden = false;
	currentStage = 0; // 시작 화면
	isStartScreen = true;
}

void Image_::BlockInit() {
	if (currentStage == TUTORIAL) {
		if (!blocks[1].empty()) {
			blocks[1].clear();
			questionBlocks[1].clear();
			tBlocks[1].clear();
			holes[1].clear();
			flagBlocks[1].clear();
			monsters[1].clear();
		}
		if (!blocks[2].empty()) {
			blocks[2].clear();
			questionBlocks[2].clear();
			tBlocks[2].clear();
			holes[2].clear();
			flagBlocks[2].clear();
			monsters[2].clear();
			coupas[2].clear();
		}
		if (!blocks[3].empty()) {
			blocks[3].clear();
			questionBlocks[3].clear();
			tBlocks[3].clear();
			holes[3].clear();
			flagBlocks[3].clear();
			monsters[3].clear();
		}
		if (blocks[0].empty()) {
			TBlock tblock0_1 = { 291, 450, 57, 32 };
			TBlock tblock0_2 = { 386, 338, 122, 32 };
			TBlock tblock0_3 = { 419, 188, 73, 32 };
			TBlock tblock0_4 = { 514, 450, 42, 32 };
			TBlock tblock0_5 = { 291, 450, 57, 32 };
			TBlock tblock0_6 = { 563, 300, 72, 32 };
			TBlock tblock0_7 = { 643, 151, 103, 32 };
			TBlock tblock0_8 = { 802, 488, 58, 32 };
			TBlock tblock0_9 = { 945, 488, 72, 32 };
			TBlock tblock0_10 = { 1042, 488, 72, 32 };
			TBlock tblock0_11 = { 962, 189, 57, 32 };
			TBlock tblock0_12 = { 1122, 339, 42, 32 };
			TBlock tblock0_13 = { 1218, 226, 90, 32 };
			TBlock tblock0_14 = { 1569, 414, 57, 32 };
			TBlock tblock0_15 = { 1810, 488, 42, 32 };
			TBlock tblock0_16 = { 1858, 339, 58, 32 };
			TBlock tblock0_17 = { 1666, 263, 122, 32 };
			TBlock tblock0_18 = { 1954, 339, 58, 32 };
			TBlock tblock0_19 = { 0, 488, 252, 32 };
			TBlock tblock0_20 = { 2209, 338, 93, 150 };
			TBlock tblock0_21 = { 2241, 261, 62, 78 };
			TBlock tblock0_22 = { 2271, 188, 32, 72 };
			TBlock tblock0_23 = { 2065, 487, 751, 72 };

			tBlocks[0].push_back(tblock0_1);
			tBlocks[0].push_back(tblock0_2);
			tBlocks[0].push_back(tblock0_3);
			tBlocks[0].push_back(tblock0_4);
			tBlocks[0].push_back(tblock0_5);
			tBlocks[0].push_back(tblock0_6);
			tBlocks[0].push_back(tblock0_7);
			tBlocks[0].push_back(tblock0_8);
			tBlocks[0].push_back(tblock0_9);
			tBlocks[0].push_back(tblock0_10);
			tBlocks[0].push_back(tblock0_11);
			tBlocks[0].push_back(tblock0_12);
			tBlocks[0].push_back(tblock0_13);
			tBlocks[0].push_back(tblock0_14);
			tBlocks[0].push_back(tblock0_15);
			tBlocks[0].push_back(tblock0_16);
			tBlocks[0].push_back(tblock0_17);
			tBlocks[0].push_back(tblock0_18);
			tBlocks[0].push_back(tblock0_19);
			tBlocks[0].push_back(tblock0_20);
			tBlocks[0].push_back(tblock0_21);
			tBlocks[0].push_back(tblock0_22);
			tBlocks[0].push_back(tblock0_23);

			QuestionBlock qblock0_1 = { 944, 376, 16, 36, false };
			questionBlocks[0].push_back(qblock0_1);

			Hole hole0_1 = { 257, 500, 1803, 69 };
			holes[0].push_back(hole0_1);

			FlagBlock flagBlock0_1 = { 2432, 94, 16, 392 }; // x=3100, 높이 286 (맨 아래 y=486
			flagBlocks[0].push_back(flagBlock0_1);

			Monster monster0_1;
			monster0_1.x = 424; // 초기 위치Add commentMore actions
			monster0_1.y = 290;
			monster0_1.width = 32;
			monster0_1.height = 32;
			monster0_1.direction = LEFT;
			monster0_1.speed = 1.0f;
			monster0_1.isAlive = true;
			monster0_1.isFalling = false;
			monster0_1.fallProgress = 0.0f;
			monster0_1.directionTimer = 0.0f;
			monster0_1.directionChangeInterval = static_cast<float>(rand() % 6 + 5); // 5~10초
			monsters[0].push_back(monster0_1);
		}
	}
	else if (currentStage == STAGE1) {
		if (!blocks[0].empty()) {
			blocks[0].clear();
			questionBlocks[0].clear();
			tBlocks[0].clear();
			holes[0].clear();
			monsters[0].clear();
		}
		if (!blocks[2].empty()) {
			blocks[2].clear();
			questionBlocks[2].clear();
			tBlocks[2].clear();
			holes[2].clear();
			monsters[2].clear();
			coupas[2].clear();
		}
		if (!blocks[3].empty()) {
			blocks[3].clear();
			questionBlocks[3].clear();
			tBlocks[3].clear();
			holes[3].clear();
			monsters[3].clear();
		}
		if (blocks[1].empty()) {
			// Stage 1 Objects (currentStage == STAGE1)
			Block block1_1 = { 320, 335, 16, 32 };
			Block block1_2 = { 352, 335, 16, 32 };
			Block block1_3 = { 384, 335, 16, 32 };
			Block block1_4 = { 256, 335, 16, 42 };
			Block block1_5 = { 352, 185, 16, 42 };
			Block block1_6 = { 1280, 185, 130, 42 };
			Block block1_7 = { 1232, 335, 16, 32 };
			Block block1_8 = { 1264, 335, 16, 32 };
			Block block1_9 = { 1456, 185, 16, 32 };
			Block block1_10 = { 1472, 185, 16, 32 };
			Block block1_11 = { 1488, 185, 16, 32 };
			Block block1_12 = { 1600, 335, 16, 32 };
			Block block1_13 = { 1616, 335, 16, 32 };
			Block block1_14 = { 1744, 185, 16, 42 };
			Block block1_15 = { 1888, 335, 16, 42 };
			Block block1_16 = { 1936, 185, 16, 42 };
			Block block1_17 = { 1952, 185, 16, 42 };
			Block block1_18 = { 1968, 185, 16, 42 };
			Block block1_19 = { 2048, 185, 16, 32 };
			Block block1_20 = { 2096, 185, 16, 32 };
			Block block1_21 = { 2064, 335, 16, 42 };
			Block block1_22 = { 2080, 335, 16, 42 };
			Block block1_23 = { 2688, 335, 16, 32 };
			Block block1_24 = { 2704, 335, 16, 32 };
			Block block1_25 = { 2736, 335, 16, 32 };
			blocks[1].push_back(block1_1);
			blocks[1].push_back(block1_2);
			blocks[1].push_back(block1_3);
			blocks[1].push_back(block1_4);
			blocks[1].push_back(block1_5);
			blocks[1].push_back(block1_6);
			blocks[1].push_back(block1_7);
			blocks[1].push_back(block1_8);
			blocks[1].push_back(block1_9);
			blocks[1].push_back(block1_10);
			blocks[1].push_back(block1_11);
			blocks[1].push_back(block1_12);
			blocks[1].push_back(block1_13);
			blocks[1].push_back(block1_14);
			blocks[1].push_back(block1_15);
			blocks[1].push_back(block1_16);
			blocks[1].push_back(block1_17);
			blocks[1].push_back(block1_18);
			blocks[1].push_back(block1_19);
			blocks[1].push_back(block1_20);
			blocks[1].push_back(block1_21);
			blocks[1].push_back(block1_22);
			blocks[1].push_back(block1_23);
			blocks[1].push_back(block1_24);
			blocks[1].push_back(block1_25);

			QuestionBlock qblock1_1 = { 336, 335, 16, 42, false };
			QuestionBlock qblock1_2 = { 368, 335, 16, 42, false };
			QuestionBlock qblock1_3 = { 1248, 335, 16, 42, false };
			QuestionBlock qblock1_4 = { 1504, 335, 16, 42, false };
			QuestionBlock qblock1_5 = { 1504, 185, 16, 42, false };
			QuestionBlock qblock1_6 = { 1696, 335, 16, 42, false };
			QuestionBlock qblock1_7 = { 1744, 335, 16, 42, false };
			QuestionBlock qblock1_8 = { 1792, 335, 16, 42, false };
			QuestionBlock qblock1_9 = { 2064, 185, 16, 42, false };
			QuestionBlock qblock1_10 = { 2080, 185, 16, 42, false };
			QuestionBlock qblock1_11 = { 2720, 335, 16, 42, false };
			questionBlocks[1].push_back(qblock1_1);
			questionBlocks[1].push_back(qblock1_2);
			questionBlocks[1].push_back(qblock1_3);
			questionBlocks[1].push_back(qblock1_4);
			questionBlocks[1].push_back(qblock1_5);
			questionBlocks[1].push_back(qblock1_6);
			questionBlocks[1].push_back(qblock1_7);
			questionBlocks[1].push_back(qblock1_8);
			questionBlocks[1].push_back(qblock1_9);
			questionBlocks[1].push_back(qblock1_10);
			questionBlocks[1].push_back(qblock1_11);

			TBlock tblock1_1 = { 448, 413, 32, 76 };
			TBlock tblock1_2 = { 608, 375, 32, 110 };
			TBlock tblock1_3 = { 736, 336, 32, 150 };
			TBlock tblock1_4 = { 912, 336, 32, 150 };
			TBlock tblock1_5 = { 2140, 448, 59, 38 };
			TBlock tblock1_6 = { 2156, 410, 43, 38 };
			TBlock tblock1_7 = { 2172, 372, 27, 38 };
			TBlock tblock1_8 = { 2188, 334, 11, 38 };
			TBlock tblock1_9 = { 2238, 334, 16, 38 };
			TBlock tblock1_10 = { 2238, 372, 32, 38 };
			TBlock tblock1_11 = { 2238, 410, 48, 38 };
			TBlock tblock1_12 = { 2238, 448, 64, 38 };
			TBlock tblock1_13 = { 2366, 448, 75, 38 };
			TBlock tblock1_14 = { 2382, 410, 59, 38 };
			TBlock tblock1_15 = { 2398, 372, 43, 38 };
			TBlock tblock1_16 = { 2414, 334, 27, 38 };
			TBlock tblock1_17 = { 2480, 334, 16, 38 };
			TBlock tblock1_18 = { 2480, 372, 32, 38 };
			TBlock tblock1_19 = { 2480, 410, 48, 38 };
			TBlock tblock1_20 = { 2480, 448, 64, 38 };
			TBlock tblock1_21 = { 2608, 413, 32, 76 };
			TBlock tblock1_22 = { 2864, 413, 32, 76 };
			TBlock tblock1_23 = { 2896, 448, 144, 38 };
			TBlock tblock1_24 = { 2912, 410, 128, 38 };
			TBlock tblock1_25 = { 2928, 372, 112, 38 };
			TBlock tblock1_26 = { 2944, 334, 96, 38 };
			TBlock tblock1_27 = { 2960, 296, 80, 38 };
			TBlock tblock1_28 = { 2976, 258, 64, 38 };
			TBlock tblock1_29 = { 2992, 220, 48, 38 };
			TBlock tblock1_30 = { 3008, 182, 32, 38 };
			TBlock tblock1_floor1 = { 0, 487, 1104, 38 };
			TBlock tblock1_floor2 = { 1136, 487, 240, 38 };
			TBlock tblock1_floor3 = { 1424, 487, 722, 38 };
			TBlock tblock1_floor4 = { 2544, 487, 331, 38 };
			tBlocks[1].push_back(tblock1_1);
			tBlocks[1].push_back(tblock1_2);
			tBlocks[1].push_back(tblock1_3);
			tBlocks[1].push_back(tblock1_4);
			tBlocks[1].push_back(tblock1_5);
			tBlocks[1].push_back(tblock1_6);
			tBlocks[1].push_back(tblock1_7);
			tBlocks[1].push_back(tblock1_8);
			tBlocks[1].push_back(tblock1_9);
			tBlocks[1].push_back(tblock1_10);
			tBlocks[1].push_back(tblock1_11);
			tBlocks[1].push_back(tblock1_12);
			tBlocks[1].push_back(tblock1_13);
			tBlocks[1].push_back(tblock1_14);
			tBlocks[1].push_back(tblock1_15);
			tBlocks[1].push_back(tblock1_16);
			tBlocks[1].push_back(tblock1_17);
			tBlocks[1].push_back(tblock1_18);
			tBlocks[1].push_back(tblock1_19);
			tBlocks[1].push_back(tblock1_20);
			tBlocks[1].push_back(tblock1_21);
			tBlocks[1].push_back(tblock1_22);
			tBlocks[1].push_back(tblock1_23);
			tBlocks[1].push_back(tblock1_24);
			tBlocks[1].push_back(tblock1_25);
			tBlocks[1].push_back(tblock1_26);
			tBlocks[1].push_back(tblock1_27);
			tBlocks[1].push_back(tblock1_28);
			tBlocks[1].push_back(tblock1_29);
			tBlocks[1].push_back(tblock1_30);
			tBlocks[1].push_back(tblock1_floor1);
			tBlocks[1].push_back(tblock1_floor2);
			tBlocks[1].push_back(tblock1_floor3);
			tBlocks[1].push_back(tblock1_floor4);

			Hole hole1_1 = { 1104, 487, 32, 74 };
			Hole hole1_2 = { 1376, 487, 48, 74 };
			Hole hole1_3 = { 2448, 487, 32, 74 };
			holes[1].push_back(hole1_1);
			holes[1].push_back(hole1_2);
			holes[1].push_back(hole1_3);

			// 깃발 블럭 추가 (Stage 1 끝부분)
			FlagBlock flagBlock1_1 = { 3168, 94, 16, 392 }; // x=3100, 높이 286 (맨 아래 y=486
			flagBlocks[1].push_back(flagBlock1_1);

			Monster monster1_1;
			monster1_1.x = 500; // 초기 위치Add commentMore actions
			monster1_1.y = 450; // 바닥에 위치
			monster1_1.width = 32;
			monster1_1.height = 32;
			monster1_1.direction = RIGHT;
			monster1_1.speed = 1.0f;
			monster1_1.isAlive = true;
			monster1_1.isFalling = false;
			monster1_1.fallProgress = 0.0f;
			monster1_1.directionTimer = 0.0f;
			monster1_1.directionChangeInterval = static_cast<float>(rand() % 6 + 5); // 5~10초
			monsters[1].push_back(monster1_1);

			Monster monster1_2;
			monster1_2.x = 830; // 초기 위치Add commentMore actions
			monster1_2.y = 450; // 바닥에 위치
			monster1_2.width = 32;
			monster1_2.height = 32;
			monster1_2.direction = RIGHT;
			monster1_2.speed = 1.0f;
			monster1_2.isAlive = true;
			monster1_2.isFalling = false;
			monster1_2.fallProgress = 0.0f;
			monster1_2.directionTimer = 0.0f;
			monster1_2.directionChangeInterval = static_cast<float>(rand() % 6 + 5); // 5~10초
			monsters[1].push_back(monster1_2);

			Monster monster1_3;
			monster1_3.x = 1900; // 초기 위치Add commentMore actions
			monster1_3.y = 440; // 바닥에 위치
			monster1_3.width = 32;
			monster1_3.height = 32;
			monster1_3.direction = RIGHT;
			monster1_3.speed = 1.0f;
			monster1_3.isAlive = true;
			monster1_3.isFalling = false;
			monster1_3.fallProgress = 0.0f;
			monster1_3.directionTimer = 0.0f;
			monster1_3.directionChangeInterval = static_cast<float>(rand() % 6 + 5); // 5~10초
			monsters[1].push_back(monster1_3);

			Monster monster1_4;
			monster1_4.x = 2730; // 초기 위치Add commentMore actions
			monster1_4.y = 440; // 바닥에 위치
			monster1_4.width = 32;
			monster1_4.height = 32;
			monster1_4.direction = RIGHT;
			monster1_4.speed = 1.0f;
			monster1_4.isAlive = true;
			monster1_4.isFalling = false;
			monster1_4.fallProgress = 0.0f;
			monster1_4.directionTimer = 0.0f;
			monster1_4.directionChangeInterval = static_cast<float>(rand() % 6 + 5); // 5~10초
			monsters[1].push_back(monster1_4);

			Monster monster1_5;
			monster1_5.x = 2680; // 초기 위치Add commentMore actions
			monster1_5.y = 440; // 바닥에 위치
			monster1_5.width = 32;
			monster1_5.height = 32;
			monster1_5.direction = LEFT;
			monster1_5.speed = 1.0f;
			monster1_5.isAlive = true;
			monster1_5.isFalling = false;
			monster1_5.fallProgress = 0.0f;
			monster1_5.directionTimer = 0.0f;
			monster1_5.directionChangeInterval = static_cast<float>(rand() % 6 + 5); // 5~10초
			monsters[1].push_back(monster1_5);

			Monster monster1_6;
			monster1_6.x = 350; // 초기 위치Add commentMore actions
			monster1_6.y = 280; // 바닥에 위치
			monster1_6.width = 32;
			monster1_6.height = 32;
			monster1_6.direction = LEFT;
			monster1_6.speed = 1.0f;
			monster1_6.isAlive = true;
			monster1_6.isFalling = false;
			monster1_6.fallProgress = 0.0f;
			monster1_6.directionTimer = 0.0f;
			monster1_6.directionChangeInterval = static_cast<float>(rand() % 6 + 5); // 5~10초
			monsters[1].push_back(monster1_6);

			Monster monster1_7;
			monster1_7.x = 1940; // 초기 위치Add commentMore actions
			monster1_7.y = 440; // 바닥에 위치
			monster1_7.width = 32;
			monster1_7.height = 32;
			monster1_7.direction = RIGHT;
			monster1_7.speed = 1.0f;
			monster1_7.isAlive = true;
			monster1_7.isFalling = false;
			monster1_7.fallProgress = 0.0f;
			monster1_7.directionTimer = 0.0f;
			monster1_7.directionChangeInterval = static_cast<float>(rand() % 6 + 5); // 5~10초
			monsters[1].push_back(monster1_7);

			Monster monster1_8;
			monster1_8.x = 1920; // 초기 위치Add commentMore actions
			monster1_8.y = 440; // 바닥에 위치
			monster1_8.width = 32;
			monster1_8.height = 32;
			monster1_8.direction = LEFT;
			monster1_8.speed = 1.0f;
			monster1_8.isAlive = true;
			monster1_8.isFalling = false;
			monster1_8.fallProgress = 0.0f;
			monster1_8.directionTimer = 0.0f;
			monster1_8.directionChangeInterval = static_cast<float>(rand() % 6 + 5); // 5~10초
			monsters[1].push_back(monster1_8);


		}
	}
	else if (currentStage == STAGE2) {
		if (!blocks[0].empty()) {
			blocks[0].clear();
			questionBlocks[0].clear();
			tBlocks[0].clear();
			holes[0].clear();
			monsters[0].clear();
		}
		if (!blocks[1].empty()) {
			blocks[1].clear();
			questionBlocks[1].clear();
			tBlocks[1].clear();
			holes[1].clear();
			monsters[1].clear();
		}
		if (!blocks[3].empty()) {
			blocks[3].clear();
			questionBlocks[3].clear();
			tBlocks[3].clear();
			holes[3].clear();
			monsters[3].clear();
		}
		if (blocks[2].empty()) {
			QuestionBlock qblock2_1 = { 480, 226, 16, 36, false };
			questionBlocks[2].push_back(qblock2_1);

			TBlock tblock2_1 = { 0, 262, 48, 36 };
			TBlock tblock2_2 = { 48, 300, 16, 36 };
			TBlock tblock2_3 = { 64, 336, 16, 36 };
			TBlock tblock2_4 = { 80, 372, 128, 72 };
			TBlock tblock2_5 = { 240, 372, 176, 108 };
			TBlock tblock2_6 = { 0, 112, 386, 72 };
			TBlock tblock2_7 = { 368, 184, 16, 36 };
			TBlock tblock2_8 = { 464, 372, 48, 108 };
			TBlock tblock2_9 = { 386, 74, 204, 36 };
			TBlock tblock2_10 = { 590, 74, 562, 144 };
			TBlock tblock2_11 = { 560, 336, 594, 144 };
			TBlock tblock2_12 = { 1150, 74, 402, 36 };
			TBlock tblock2_13 = { 1150, 374, 514, 108 };
			TBlock tblock2_14 = { 1552, 74, 112, 108 };
			TBlock tblock2_15 = { 1280, 110, 16, 36 };
			TBlock tblock2_16 = { 1408, 110, 16, 36 };
			TBlock tblock2_17 = { 1664, 482, 192, 36 };
			TBlock tblock2_18 = { 1664, 74, 304, 36 };
			TBlock tblock2_19 = { 1856, 374, 64, 108 };
			TBlock tblock2_20 = { 1920, 482, 48, 36 };
			TBlock tblock2_21 = { 1968, 374, 80, 108 };
			TBlock tblock2_22 = { 1968, 74, 80, 108 };
			TBlock tblock2_23 = { 2048, 374, 206, 36 };
			TBlock tblock2_24 = { 2270, 74, 32, 144 };
			TBlock tblock2_25 = { 2048, 74, 222, 36 };
			TBlock tblock2_26 = { 2254, 336, 50, 144 };
			TBlock tblock2_27 = { 2302, 74, 254, 36 };
			TBlock tblock2_28 = { 2302, 484, 254, 36 };

			tBlocks[2].push_back(tblock2_1);
			tBlocks[2].push_back(tblock2_2);
			tBlocks[2].push_back(tblock2_3);
			tBlocks[2].push_back(tblock2_4);
			tBlocks[2].push_back(tblock2_5);
			tBlocks[2].push_back(tblock2_6);
			tBlocks[2].push_back(tblock2_7);
			tBlocks[2].push_back(tblock2_8);
			tBlocks[2].push_back(tblock2_9);
			tBlocks[2].push_back(tblock2_10);
			tBlocks[2].push_back(tblock2_11);
			tBlocks[2].push_back(tblock2_12);
			tBlocks[2].push_back(tblock2_13);
			tBlocks[2].push_back(tblock2_14);
			tBlocks[2].push_back(tblock2_15);
			tBlocks[2].push_back(tblock2_16);
			tBlocks[2].push_back(tblock2_17);
			tBlocks[2].push_back(tblock2_18);
			tBlocks[2].push_back(tblock2_19);
			tBlocks[2].push_back(tblock2_20);
			tBlocks[2].push_back(tblock2_21);
			tBlocks[2].push_back(tblock2_22);
			tBlocks[2].push_back(tblock2_23);
			tBlocks[2].push_back(tblock2_24);
			tBlocks[2].push_back(tblock2_25);
			tBlocks[2].push_back(tblock2_26);
			tBlocks[2].push_back(tblock2_27);
			tBlocks[2].push_back(tblock2_28);

			Hole hole2_1 = { 208, 442, 32, 32 };
			Hole hole2_2 = { 416, 480, 48, 32 };
			Hole hole2_3 = { 512, 480, 48, 32 };
			holes[2].push_back(hole2_1);
			holes[2].push_back(hole2_2);
			holes[2].push_back(hole2_3);

			Monster monster2_1;
			monster2_1.x = 668; // 초기 위치Add commentMore actions
			monster2_1.y = 295; // 바닥에 위치
			monster2_1.width = 32;
			monster2_1.height = 32;
			monster2_1.direction = RIGHT;
			monster2_1.speed = 1.0f;
			monster2_1.isAlive = true;
			monster2_1.isFalling = false;
			monster2_1.fallProgress = 0.0f;
			monster2_1.directionTimer = 0.0f;
			monster2_1.directionChangeInterval = static_cast<float>(rand() % 6 + 5); // 5~10초
			monsters[2].push_back(monster2_1);

			Monster monster2_2;
			monster2_2.x = 690; // 초기 위치Add commentMore actions
			monster2_2.y = 295; // 바닥에 위치
			monster2_2.width = 32;
			monster2_2.height = 32;
			monster2_2.direction = RIGHT;
			monster2_2.speed = 1.0f;
			monster2_2.isAlive = true;
			monster2_2.isFalling = false;
			monster2_2.fallProgress = 0.0f;
			monster2_2.directionTimer = 0.0f;
			monster2_2.directionChangeInterval = static_cast<float>(rand() % 6 + 5); // 5~10초
			monsters[2].push_back(monster2_2);



			Coupa coupa;
			coupa.x = 2140; // 테스트용, tblock2_11(x=560, width=594) 내
			coupa.y = 250; // tblock2_11(y=336) - height(64)
			coupa.width = 100;
			coupa.height = 100;
			coupa.direction = LEFT;
			coupa.isAlive = true;
			coupa.isJumping = false;
			coupa.jumpVelocity = 0.0f;
			coupa.coupaJumpVelocity = -10.0f;
			coupa.jumpTimer = 0.0f;
			coupa.fireballTimer = 0.0f;
			coupa.health = 5;
			coupa.invincibleTime = 0;
			coupa.isInvincible = false;
			coupa.isFalling = false;
			coupa.fallProgress = 0.0f;
			coupa.directionTimer = 0.0f;
			coupa.directionChangeInterval = static_cast<float>(rand() % 6 + 5);
			coupa.speed = 0.0f;
			coupas[2].push_back(coupa);

		}
	}
	else if (currentStage == HIDDEN) {
		if (!blocks[0].empty()) {
			blocks[0].clear();
			questionBlocks[0].clear();
			tBlocks[0].clear();
			holes[0].clear();
			monsters[0].clear();
		}
		if (!blocks[2].empty()) {
			blocks[2].clear();
			questionBlocks[2].clear();
			tBlocks[2].clear();
			holes[2].clear();
			monsters[2].clear();
			coupas[2].clear();
		}
		if (blocks[3].empty()) {
			TBlock tblock3_1 = { 188, 346, 330, 126 };
			TBlock tblock3_2 = { 0, 0, 48, 474 };
			TBlock tblock3_3 = { 614, 388, 102, 86 };
			TBlock tblock3_4 = { 716, 0, 84, 392 };
			TBlock tblock3_5 = { 188, 0, 330, 40 };

			tBlocks[3].push_back(tblock3_1);
			tBlocks[3].push_back(tblock3_2);
			tBlocks[3].push_back(tblock3_3);
			tBlocks[3].push_back(tblock3_4);
			tBlocks[3].push_back(tblock3_5);
			// 아이템 생성
			Image_::Item item3_1;
			item3_1.width = 32;
			item3_1.height = 32;
			item3_1.x = 282;
			item3_1.y = 346 - item3_1.height; // 아이템을 QuestionBlock 위에 배치
			item3_1.type = MUSHROOM;
			item3_1.isActive = true;
			item3_1.direction = (rand() % 2 == 0) ? LEFT : RIGHT;
			item3_1.speed = 0.f;
			item3_1.directionTimer = 0.0f;
			item3_1.directionChangeInterval = static_cast<float>(rand() % 6 + 5);
			Images.items[3].push_back(item3_1);

			Image_::Item item3_2;
			item3_2.width = 32;
			item3_2.height = 32;
			item3_2.x = 392;
			item3_2.y = 346 - item3_2.height; // 아이템을 QuestionBlock 위에 배치
			item3_2.type = FLOWER;
			item3_2.isActive = true;
			item3_2.direction = (rand() % 2 == 0) ? LEFT : RIGHT;
			item3_2.speed = 0.f;
			item3_2.directionTimer = 0.0f;
			item3_2.directionChangeInterval = static_cast<float>(rand() % 6 + 5);
			Images.items[3].push_back(item3_2);
		}
	}
}

void Image_::DrawBackGround(int x, int y, HDC targetDC) {
	int cameraX = x - 400;
	if (cameraX < 0) cameraX = 0;

	int srcWidth = (stage1 ? mStage1.GetWidth() : tutorial ? mStageTutorial.GetWidth() :
		stage2 ? mStage2.GetWidth() : mStageHidden.GetWidth());
	int srcHeight = (stage1 ? mStage1.GetHeight() : tutorial ? mStageTutorial.GetHeight() :
		stage2 ? mStage2.GetHeight() : mStageHidden.GetHeight());

	if (srcWidth <= wRect.right) {
		cameraX = 0;
	}
	else if (cameraX > srcWidth - wRect.right) {
		cameraX = srcWidth - wRect.right;
	}

	int destWidth = wRect.right;
	int destHeight = wRect.bottom;

	if (tutorial && !mStageTutorial.IsNull()) {
		mStageTutorial.StretchBlt(targetDC, 0, 0, destWidth, destHeight, cameraX, 0, wRect.right, srcHeight, SRCCOPY);
	}
	else if (stage1 && !mStage1.IsNull()) {
		mStage1.StretchBlt(targetDC, 0, 0, destWidth, destHeight, cameraX, 0, wRect.right, srcHeight, SRCCOPY);
	}
	else if (stage2 && !mStage2.IsNull()) {
		mStage2.StretchBlt(targetDC, 0, 0, destWidth, destHeight, cameraX, 0, wRect.right, srcHeight, SRCCOPY);
	}
	else if (hidden && !mStageHidden.IsNull()) {
		mStageHidden.StretchBlt(targetDC, 0, 0, destWidth, destHeight, cameraX, 0, wRect.right, srcHeight, SRCCOPY);
	}
	else {
		OutputDebugString(L"No valid background image\n");
	}

	if (!blockImage.IsNull()) {
		for (const auto& block : blocks[currentStage - 1]) {
			int offsetX = block.x - cameraX;
			if (offsetX + block.width > 0 && offsetX < wRect.right) {
				blockImage.StretchBlt(targetDC, offsetX, block.y, block.width, 42, 0, 0, blockImage.GetWidth(), blockImage.GetHeight(), SRCCOPY);
			}
		}
	}


	if (!questionBlockImage[0].IsNull() && !questionBlockImage[1].IsNull()) {
		for (const auto& qblock : questionBlocks[currentStage - 1]) {
			int offsetX = qblock.x - cameraX;
			if (offsetX + qblock.width > 0 && offsetX < wRect.right && !qblock.hit) {
				questionBlockImage[0].StretchBlt(targetDC, offsetX, qblock.y, qblock.width, qblock.height, 0, 0, questionBlockImage[0].GetWidth(), questionBlockImage[0].GetHeight(), SRCCOPY);
			}
			else if (offsetX + qblock.width > 0 && offsetX < wRect.right && qblock.hit) {
				questionBlockImage[1].StretchBlt(targetDC, offsetX, qblock.y, qblock.width, qblock.height, 0, 0, questionBlockImage[0].GetWidth(), questionBlockImage[0].GetHeight(), SRCCOPY);
			}
		}
	}

	// 몬스터 그리기 추가
	if (!monster.IsNull()) {
		for (const auto& m : monsters[currentStage - 1]) {
			if (!m.isAlive) continue; // 죽은 몬스터는 그리지 않음

			int offsetX = m.x - cameraX;
			if (offsetX + m.width > 0 && offsetX < wRect.right) {
				if (m.direction == RIGHT) {
					// 오른쪽 방향: 원본 이미지 그대로
					monster.TransparentBlt(targetDC, offsetX, m.y, m.width, m.height, 0, 0, monster.GetWidth(), monster.GetHeight(), RGB(0, 255, 0));
				}
				else if (m.direction == LEFT) {
					// 왼쪽 방향: 이미지 좌우 반전
					monster.TransparentBlt(targetDC, offsetX, m.y, m.width, m.height, 0, 0, monster.GetWidth(), monster.GetHeight(), RGB(0, 255, 0));
				}

			}
		}
	}

	// 아이템
	for (const auto& item : items[currentStage - 1]) {
		if (item.isActive) {
			int offsetX = item.x - cameraX;
			bool mushroom = (item.type == 0); // 0: 버섯, 1: 꽃
			bool flower = (item.type == 1); // 0: 버섯, 1: 꽃
			if (mushroom)
				Item_Mushroom.TransparentBlt(targetDC, offsetX, item.y, item.width, item.height,
					0, 0, Item_Mushroom.GetWidth(), Item_Mushroom.GetHeight(), RGB(146, 144, 255));
			else if (flower)
				Item_Flower.TransparentBlt(targetDC, offsetX, item.y, item.width, item.height,
					0, 0, Item_Flower.GetWidth(), Item_Flower.GetHeight(), RGB(146, 144, 255));
		}
	}
	// 쿠파 그리기
	if (!coupaImage.IsNull()) {
		for (const auto& coupa : coupas[currentStage - 1]) {
			if (!coupa.isAlive) continue;
			int offsetX = coupa.x - cameraX;
			WCHAR buffer[100];
			swprintf_s(buffer, L"Coupa at x=%d, y=%d, cameraX=%d, offsetX=%d\n", coupa.x, coupa.y, cameraX, offsetX);
			OutputDebugStringW(buffer);
			if (offsetX + coupa.width > 0 && offsetX < wRect.right) {
				if (!(coupa.isInvincible && (coupa.invincibleTime / 10) % 2 == 0)) {
					coupaImage.TransparentBlt(targetDC, offsetX, coupa.y, coupa.width, coupa.height,
						0, 0, coupaImage.GetWidth(), coupaImage.GetHeight(), RGB(255, 255, 255));
				}
			}
		}
	}
}

void Image_::NextStage() {
	if (currentStage == TUTORIAL) {
		currentStage = STAGE1;
		tutorial = false;
		stage1 = true;
	}
	else if (currentStage == STAGE1) {
		currentStage = STAGE2;
		stage1 = false;
		stage2 = true;
	}
	// 모든 스테이지의 객체 벡터 초기화
	for (int i = 0; i < 4; ++i) {
		blocks[i].clear();
		questionBlocks[i].clear();
		tBlocks[i].clear();
		holes[i].clear();
		flagBlocks[i].clear();
		monsters[i].clear();
		items[i].clear();
		coupas[i].clear();
	}
	fireballs.clear();
	Player.ResetPosition();
	BlockInit(); // 새 스테이지 객체 로드
}

void Image_::EnterHiddenStage() {
	currentStage = HIDDEN;
	tutorial = (currentStage == TUTORIAL);
	stage1 = (currentStage == STAGE1);
	stage2 = (currentStage == STAGE2);
	hidden = (currentStage == HIDDEN);
	Player.ResetPosition();
	BlockInit();
}

void Image_::QuitHiddenStage() {
	currentStage = STAGE1;
	tutorial = (currentStage == TUTORIAL);
	stage1 = (currentStage == STAGE1);
	stage2 = (currentStage == STAGE2);
	hidden = (currentStage == HIDDEN);
}

void Image_::Destroy() {
	mStartScreen.Destroy(); // 시작 화면 이미지 해제
	Player_Move_Pairi.Destroy();
	Player_Move_Lizamong.Destroy();
	Player_Attack_Pairi.Destroy();
	Player_Attack_Lizamong.Destroy();
	mStageTutorial.Destroy();
	mStage1.Destroy();
	mStage2.Destroy();
	mStageHidden.Destroy();
	blockImage.Destroy();
	questionBlockImage[0].Destroy();
	coupaImage.Destroy();
	Item_Mushroom.Destroy();
	Item_Flower.Destroy();
	fireballs.clear();
	for (int i = 0; i < 4; ++i) {
		blocks[i].clear();
		questionBlocks[i].clear();
		tBlocks[i].clear();
		holes[i].clear();
		flagBlocks[i].clear();
		coupas[i].clear();
		items[i].clear();
	}
}

void Player_::DrawHitbox(HDC targetDC) {
	int hitboxX, hitboxY, hitboxWidth, hitboxHeight;
	GetHitbox(hitboxX, hitboxY, hitboxWidth, hitboxHeight);

	int cameraX = GetCameraX(x_, Images.NowStage() == TUTORIAL ? Images.mStageTutorial.GetWidth() :
		Images.NowStage() == STAGE1 ? Images.mStage1.GetWidth() :
		Images.NowStage() == STAGE2 ? Images.mStage2.GetWidth() :
		Images.NowStage() == HIDDEN ? Images.mStageHidden.GetWidth() : 0);

	RECT screenHitbox = { hitbox_.left - cameraX, hitbox_.top, hitbox_.right - cameraX, hitbox_.bottom };

	HPEN pen = CreatePen(PS_DASH, 3, RGB(255, 0, 0));
	HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	SelectObject(targetDC, pen);
	SelectObject(targetDC, hBrush);
	Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
	DeleteObject(pen);
}

void Image_::DrawHitBox(HDC targetDC) {
	int cameraX = GetCameraX(Player.x(), NowStage() == TUTORIAL ? mStageTutorial.GetWidth() :
		NowStage() == STAGE1 ? mStage1.GetWidth() :
		NowStage() == STAGE2 ? mStage2.GetWidth() :
		NowStage() == HIDDEN ? mStageHidden.GetWidth() : 0);

	// 블록 히트박스 그리기
	HPEN blockPen = CreatePen(PS_DASH, 3, RGB(255, 255, 0));
	HBRUSH blockBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	SelectObject(targetDC, blockPen);
	SelectObject(targetDC, blockBrush);

	for (const auto& block : blocks[currentStage - 1]) {
		RECT screenHitbox = { block.x - cameraX, block.y, block.x + block.width - cameraX, block.y + block.height };
		Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
	}
	DeleteObject(blockPen);
	DeleteObject(blockBrush);

	// 물음표 블럭 히트박스 그리기
	HPEN qblockPen = CreatePen(PS_DASH, 3, RGB(0, 255, 0));
	HBRUSH qblockBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	SelectObject(targetDC, qblockPen);
	SelectObject(targetDC, qblockBrush);

	for (const auto& qblock : questionBlocks[currentStage - 1]) {
		RECT screenHitbox = { qblock.x - cameraX, qblock.y, qblock.x + qblock.width - cameraX, qblock.y + qblock.height };
		Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
	}
	DeleteObject(qblockPen);
	DeleteObject(qblockBrush);

	// 파이프, 계단 블록 히트박스 그리기
	HPEN tblockPen = CreatePen(PS_DASH, 3, RGB(255, 0, 255));
	HBRUSH tblockBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	SelectObject(targetDC, tblockPen);
	SelectObject(targetDC, tblockBrush);

	for (const auto& tblock : tBlocks[currentStage - 1]) {
		RECT screenHitbox = { tblock.x - cameraX, tblock.y, tblock.x + tblock.width - cameraX, tblock.y + tblock.height };
		Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
	}
	DeleteObject(tblockPen);
	DeleteObject(tblockBrush);

	// 구멍 히트박스 그리기
	HPEN holePen = CreatePen(PS_DASH, 3, RGB(0, 222, 0));
	HBRUSH holeBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	SelectObject(targetDC, holePen);
	SelectObject(targetDC, holeBrush);

	for (const auto& hole : holes[currentStage - 1]) {
		RECT screenHitbox = { hole.x - cameraX, hole.y, hole.x + hole.width - cameraX, hole.y + hole.height };
		Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
	}
	DeleteObject(holePen);
	DeleteObject(holeBrush);

	// 깃발 히트박스 그리기
	HPEN flagPen = CreatePen(PS_DASH, 3, RGB(0, 222, 0));
	HBRUSH flagBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	SelectObject(targetDC, flagPen);
	SelectObject(targetDC, flagBrush);

	for (const auto& flag : flagBlocks[currentStage - 1]) {
		RECT screenHitbox = { flag.x - cameraX, flag.y, flag.x + flag.width - cameraX, flag.y + flag.height };
		Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
	}
	DeleteObject(flagPen);
	DeleteObject(flagBrush);

	// 파이어볼 히트박스 그리기
	HPEN fireballPen = CreatePen(PS_DASH, 3, RGB(0, 222, 0));
	HBRUSH fireballBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	SelectObject(targetDC, fireballPen);
	SelectObject(targetDC, fireballBrush);

	for (const auto& fireball : fireballs) {
		RECT screenHitbox = { fireball.x - cameraX, fireball.y, fireball.x + fireball.width - cameraX, fireball.y + fireball.height };
		Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
	}
	DeleteObject(fireballPen);
	DeleteObject(fireballBrush);
	for (const auto& flag : flagBlocks[currentStage - 1]) {
		RECT screenHitbox = { flag.x - cameraX, flag.y, flag.x + flag.width - cameraX, flag.y + flag.height };
		Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
	}
	DeleteObject(flagPen);
	DeleteObject(flagBrush);

	// 몬스터 히트박스 그리기
	HPEN monsterPen = CreatePen(PS_DASH, 4, RGB(255, 0, 0)); // 빨간색으로 표시
	HBRUSH monsterBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	SelectObject(targetDC, monsterPen);
	SelectObject(targetDC, monsterBrush);

	for (const auto& monster : monsters[currentStage - 1]) {
		if (monster.isAlive) {
			RECT screenHitbox = { monster.x - cameraX, monster.y, monster.x + monster.width - cameraX, monster.y + monster.height };
			Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
		}
	}
	DeleteObject(monsterPen);
	DeleteObject(monsterBrush);

	// 쿠파 히트박스 그리기
	HPEN coupaPen = CreatePen(PS_DASH, 4, RGB(0, 0, 255)); // 파란색
	HBRUSH coupaBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	SelectObject(targetDC, coupaPen);
	SelectObject(targetDC, coupaBrush);
	for (const auto& coupa : coupas[currentStage - 1]) {
		if (coupa.isAlive) {
			RECT screenHitbox = { coupa.x - cameraX, coupa.y, coupa.x + coupa.width - cameraX, coupa.y + coupa.height };
			Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
		}
	}
	DeleteObject(coupaPen);
	DeleteObject(coupaBrush);

	// 아이템 히트박스 그리기
	HPEN itemPen = CreatePen(PS_DASH, 4, RGB(255, 100, 100)); // 빨간색으로 표시
	HBRUSH itemBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	SelectObject(targetDC, itemPen);
	SelectObject(targetDC, itemBrush);

	for (const auto& item : items[currentStage - 1]) {
		if (item.isActive) {
			RECT screenHitbox = { item.x - cameraX, item.y, item.x + item.width - cameraX, item.y + item.height };
			Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
		}
	}
	DeleteObject(itemPen);
	DeleteObject(itemBrush);
}

int GetCameraX(int playerX, int stageWidth) {
	int cameraX = playerX - 400; // 플레이어가 화면 중앙에 오도록 설정
	if (cameraX < 0) cameraX = 0; // 스테이지 왼쪽 경계
	if (stageWidth > wRect.right && cameraX > stageWidth - wRect.right) {
		cameraX = stageWidth - wRect.right; // 스테이지 오른쪽 경계
	}
	return cameraX;
}