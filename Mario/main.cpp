#include <Windows.h>
#include <tchar.h>
#include <random>
#include <atlimage.h>
#include <iostream>
#include <vector>

#pragma comment (lib, "msimg32.lib")

#define LEFT 1
#define RIGHT 2

#define TINO 1
#define PAIRI 2
#define LIZAD 3
#define LIZAMONG 4
#define LARGETINO 5

#define MOVE 7
#define JUMP_VELOCITY -13
#define GRAVITY 0.5

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"Window Programming Lab";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int GetCameraX(int playerX, int stageWidth);

class Image_ {
public:
	bool stage1, hidden, stage2, stage3;
	int currentStage;

	CImage Player_Move_Tino, Player_Move_Pairi, Player_Move_Lizard, Player_Move_Lizamong;
	CImage Player_Attack_Tino, Player_Attack_Pairi, Player_Attack_Lizamong;
	CImage mStage1, mStageHidden, mStage2, mStage3;
	CImage blockImage;
	CImage questionBlockImage;
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
	std::vector<Block> blocks;
	std::vector<QuestionBlock> questionBlocks;
	std::vector<TBlock> tBlocks;
	std::vector<Hole> holes;

	void ImageInit();
	void DrawBackGround(int x, int y, HDC targetDC);
	void Destroy();
	void NextStage();
	int NowStage() { return currentStage; };
	void DrawHitBox(HDC targetDC);
};

class Player_ {
public:
	Image_ Pimage;
	int imageNum;


	void PlayerInit();
	void ResetPosition();
	void DrawPlayer(HDC targetDC);
	void Move();
	bool Moving() { return move_; };
	void DrawHitbox(HDC targetDC);

	int State();

	int x() { return x_; };
	int y() { return y_; };

private:
	int x_, y_;
	int direct_;
	bool move_;
	bool eatFlower_;
	bool eatMushroom_;
	bool isJumping_;
	float jumpVelocity_;
	int groundY_;
	int defaultGroundY_;
	bool isFallingIntoHole;
	float fallProgress;
	RECT hitbox_;

	void GetHitbox(int& hitboxX, int& hitboxY, int& hitboxWidth, int& hitboxHeight) {
		hitboxX = x_;
		hitboxY = y_;
		hitboxWidth = 0;
		hitboxHeight = 0;

		switch (State()) {
		case TINO:
		case LARGETINO:
			if (direct_ == RIGHT) hitboxX += 14;
			else hitboxX = x_;
			hitboxWidth = 18;
			hitboxHeight = 39;
			break;
		case PAIRI:
			if (direct_ == RIGHT) hitboxX += 14;
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
			else hitboxX = x_;
			hitboxWidth = 30;
			hitboxHeight = 51;
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
		hDC = GetDC(hWnd);
		mDC = CreateCompatibleDC(hDC);
		GetClientRect(hWnd, &wRect);
		Images.ImageInit();
		Player.PlayerInit();
		hBitmap = CreateCompatibleBitmap(hDC, wRect.right, wRect.bottom);
		SelectObject(mDC, hBitmap);
		SetTimer(hWnd, 1, 16, NULL);
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
		else if (wParam == 'h' || wParam == 'H') {
			DrawAllHitBox = !DrawAllHitBox;
		}
		break;
	}
	case WM_MOUSEMOVE: {
		int mouseX = LOWORD(lParam);
		int mouseY = HIWORD(lParam);

		int cameraX = Player.x() - 400;
		if (cameraX < 0) cameraX = 0;
		int stageWidth = (Images.NowStage() == 1 ? Images.mStage1.GetWidth() :
			Images.NowStage() == 2 ? Images.mStageHidden.GetWidth() :
			Images.NowStage() == 3 ? Images.mStage2.GetWidth() :
			Images.NowStage() == 4 ? Images.mStage3.GetWidth() : 0);
		if (stageWidth <= wRect.right) {
			cameraX = 0;
		}
		else if (cameraX > stageWidth - wRect.right) {
			cameraX = stageWidth - wRect.right;
		}

		mouseBackgroundX = mouseX + cameraX;
		mouseBackgroundY = mouseY;

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	case WM_TIMER: {
		switch (wParam) {
		case 1: {
			if (Player.Moving()) {
				if (Player.State() == TINO || Player.State() == LARGETINO || Player.State() == LIZAMONG || Player.State() == LIZAD) {
					Player.imageNum = (Player.imageNum + 1) % 3;
				}
				else if (Player.State() == PAIRI) {
					Player.imageNum = (Player.imageNum + 1) % 6;
				}
			}
			Player.Move();
			break;
		}
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	case WM_PAINT: {
		hDC = BeginPaint(hWnd, &ps);
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
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void Player_::PlayerInit() {
	x_ = 10;
	y_ = 447;
	direct_ = RIGHT;
	move_ = false;
	eatFlower_ = false;
	eatMushroom_ = true;
	imageNum = 0;
	isJumping_ = false;
	jumpVelocity_ = 0.0f;
	groundY_ = 447;
	defaultGroundY_ = 447;
	isFallingIntoHole = false;
	fallProgress = 0.0f;
	Pimage = Images;
	hitbox_ = { x_ + 14, y_, x_ + 39, y_ + 39 };
}

void Player_::ResetPosition() {
	move_ = false;
	isJumping_ = false;
	jumpVelocity_ = 0.0f;
	isFallingIntoHole = false;
	fallProgress = 0.0f;
	if (Images.NowStage() == 1) {
		x_ = 10;
		y_ = 447;
		groundY_ = 447;
		defaultGroundY_ = 447;
	}
	else if (Images.NowStage() == 2) {
		x_ = 50;
		y_ = 435;
		groundY_ = 435;
		defaultGroundY_ = 435;
	}
	else if (Images.NowStage() == 3) {
		x_ = 30;
		y_ = 447;
		groundY_ = 447;
		defaultGroundY_ = 447;
	}
	else if (Images.NowStage() == 4) {
		x_ = 5;
		y_ = 223;
		groundY_ = 223;
		defaultGroundY_ = 223;
	}

	if (Images.NowStage() != 2) {
		eatFlower_ = false;
		eatMushroom_ = false;
	}
}

void Player_::DrawPlayer(HDC targetDC) {
	if (!Pimage.Player_Move_Pairi.IsNull()) {
		int cameraX = x_ - 400;
		if (cameraX < 0) cameraX = 0;
		int stageWidth = (Images.NowStage() == 1 ? Pimage.mStage1.GetWidth() :
			Images.NowStage() == 2 ? Pimage.mStageHidden.GetWidth() :
			Images.NowStage() == 3 ? Pimage.mStage2.GetWidth() :
			Images.NowStage() == 4 ? Pimage.mStage3.GetWidth() : 0);

		if (stageWidth <= wRect.right) {
			cameraX = 0;
		}
		else if (cameraX > stageWidth - wRect.right) {
			cameraX = stageWidth - wRect.right;
		}
		int playerWidth = 0;
		switch (State()) {
		case TINO: playerWidth = 36; break;
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

		if (move_) {
			if (!eatFlower_) {
				if (!eatMushroom_) {
					// 꽃 x 버섯 x - 스몰 티노
					if (direct_ == LEFT)
						Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 10, 36, 39, RGB(0, 255, 0));
					else
						Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 10, 36, 39, RGB(0, 255, 0));
				}
				else if (eatMushroom_) {
					// 꽃 o 버섯 x (아직 크기 미구현) - 라지 티노
					if (direct_ == LEFT)
						Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 10, 36, 39, RGB(0, 255, 0));
					else
						Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 10, 36, 39, RGB(0, 255, 0));
				}
			}
			else if (eatFlower_) {
				if (!eatMushroom_) {
					// 꽃 o 버섯 x - 파이리
					if (direct_ == LEFT)
						Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 40, 75, 39, 39, RGB(199, 225, 209));
					else
						Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 40, 75, 39, 39, RGB(199, 225, 209));
				}
				else if (eatMushroom_) {
					// 꽃 o 버섯 o - 리자몽
					if (direct_ == LEFT)
						Pimage.Player_Move_Lizamong.TransparentBlt(targetDC, offsetX, y_, 64, 51, imageNum * 62, 0, 65, 51, RGB(199, 225, 209));
					else
						Pimage.Player_Move_Lizamong.TransparentBlt(targetDC, offsetX, y_, 64, 51, imageNum * 62, 0, 65, 51, RGB(199, 225, 209));
				}
			}
		}
		else if (!move_) {
			if (!eatFlower_) {
				if (!eatMushroom_) {
					// 꽃 x 버섯 x - 스몰 티노
					if (direct_ == LEFT)
						Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
					else
						Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
				}
				else if (eatMushroom_) {
					// 꽃 x 버섯 o (크기 아직 미구현) - 라지 티노
					if (direct_ == LEFT)
						Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
					else
						Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
					/* if (direct_ == LEFT)
						 Pimage.Player_Move_Lizard.TransparentBlt(targetDC, offsetX, y_, 46, 45, 0, 0, 46, 45, RGB(255, 255, 255));
					 else
						 Pimage.Player_Move_Lizard.TransparentBlt(targetDC, offsetX, y_, 46, 45, 0, 0, 46, 45, RGB(255, 255, 255));*/
				}
			}
			else if (eatFlower_) {
				if (!eatMushroom_) {
					// 꽃 o 버섯 x - 파이리
					if (direct_ == LEFT)
						Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 40, 75, 39, 39, RGB(199, 225, 209));
					else
						Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 40, 75, 39, 39, RGB(199, 225, 209));
				}
				else if (eatMushroom_) {
					// 꽃 o 버섯 o - 리자몽
					if (direct_ == LEFT)
						Pimage.Player_Move_Lizamong.TransparentBlt(targetDC, offsetX, y_, 64, 51, imageNum * 62, 0, 65, 51, RGB(199, 225, 209));
					else
						Pimage.Player_Move_Lizamong.TransparentBlt(targetDC, offsetX, y_, 64, 51, imageNum * 62, 0, 65, 51, RGB(199, 225, 209));
				}
			}
		}
	}
	else {
		OutputDebugString(L"Player_Move_Pairi is NULL\n");
	}
}

void Player_::Move() {
	// If player is falling into a hole, continue the falling animation
	if (isFallingIntoHole) {
		const float fallSpeed = 5.0f; // Adjust this to control falling speed (pixels per frame)
		fallProgress += fallSpeed;
		y_ += static_cast<int>(fallSpeed); // Increase y_ to simulate falling

		// Check if falling is complete (fallen by 100 units)
		if (fallProgress >= 100.0f) {
			PostQuitMessage(0); // Terminate the program (game over)
		}
		return; // Skip normal movement logic while falling
	}

	bool moved = false;

	int prevX = x_;
	int prevY = y_;

	int playerHitboxX, playerHitboxY, playerHitboxWidth, playerHitboxHeight;
	GetHitbox(playerHitboxX, playerHitboxY, playerHitboxWidth, playerHitboxHeight);

	int newX = x_;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		direct_ = LEFT;
		newX -= MOVE;
		move_ = true;
		moved = true;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		direct_ = RIGHT;
		newX += MOVE;
		move_ = true;
		moved = true;
	}

	int tempHitboxX = newX;
	switch (State()) {
	case TINO: tempHitboxX += 14; break;
	case PAIRI: tempHitboxX += 14; break;
	case LIZAD: tempHitboxX += 20; break;
	case LIZAMONG: tempHitboxX += 17; break;
	default: tempHitboxX += 14; break;
	}

	bool canMoveHorizontally = true;
	for (const auto& block : Pimage.blocks) {
		int blockLeft = block.x;
		int blockRight = block.x + block.width;
		int blockTop = block.y;
		int blockBottom = block.y + block.height;

		int newPlayerLeft = tempHitboxX;
		int newPlayerRight = tempHitboxX + playerHitboxWidth;
		int playerTop = playerHitboxY;
		int playerBottom = playerHitboxY + playerHitboxHeight;

		bool overlapX = newPlayerRight > blockLeft && newPlayerLeft < blockRight;
		bool overlapY = playerBottom > blockTop && playerTop < blockBottom;

		if (overlapX && overlapY) {
			canMoveHorizontally = false;
			if (newX < x_) {
				x_ = blockRight - (playerHitboxX - x_);
			}
			else if (newX > x_) {
				x_ = blockLeft - playerHitboxWidth - (playerHitboxX - x_);
			}
			break;
		}
	}

	if (canMoveHorizontally) {
		for (const auto& qblock : Pimage.questionBlocks) {
			int qblockLeft = qblock.x;
			int qblockRight = qblock.x + qblock.width;
			int qblockTop = qblock.y;
			int qblockBottom = qblock.y + qblock.height;

			int newPlayerLeft = tempHitboxX;
			int newPlayerRight = tempHitboxX + playerHitboxWidth;
			int playerTop = playerHitboxY;
			int playerBottom = playerHitboxY + playerHitboxHeight;

			bool overlapX = newPlayerRight > qblockLeft && newPlayerLeft < qblockRight;
			bool overlapY = playerBottom > qblockTop && playerTop < qblockBottom;

			if (overlapX && overlapY) {
				canMoveHorizontally = false;
				if (newX < x_) {
					x_ = qblockRight - (playerHitboxX - x_);
				}
				else if (newX > x_) {
					x_ = qblockLeft - playerHitboxWidth - (playerHitboxX - x_);
				}
				break;
			}
		}
	}

	if (canMoveHorizontally) {
		for (const auto& tblock : Pimage.tBlocks) {
			int tblockLeft = tblock.x;
			int tblockRight = tblock.x + tblock.width;
			int tblockTop = tblock.y;
			int tblockBottom = tblock.y + tblock.height;

			int newPlayerLeft = tempHitboxX;
			int newPlayerRight = tempHitboxX + playerHitboxWidth;
			int playerTop = playerHitboxY;
			int playerBottom = playerHitboxY + playerHitboxHeight;

			bool overlapX = newPlayerRight > tblockLeft && newPlayerLeft < tblockRight;
			bool overlapY = playerBottom > tblockTop && playerTop < tblockBottom;

			if (overlapX && overlapY) {
				canMoveHorizontally = false;
				if (newX < x_) {
					x_ = tblockRight - (playerHitboxX - x_);
				}
				else if (newX > x_) {
					x_ = tblockLeft - playerHitboxWidth - (playerHitboxX - x_);
				}
				break;
			}
		}
	}

	if (canMoveHorizontally) {
		x_ = newX;
	}

	if (GetAsyncKeyState(VK_UP) & 0x8000 && !isJumping_) {
		isJumping_ = true;
		jumpVelocity_ = JUMP_VELOCITY;
	}

	if (isJumping_) {
		y_ += static_cast<int>(jumpVelocity_);
		jumpVelocity_ += GRAVITY;
	}

	GetHitbox(playerHitboxX, playerHitboxY, playerHitboxWidth, playerHitboxHeight);

	bool onBlock = false;
	int newGroundY = defaultGroundY_;

	// Check collisions with Blocks (vertical)
	for (const auto& block : Pimage.blocks) {
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
			WCHAR debug[100];
			wsprintf(debug, L"Block Collision: playerBottom=%d, blockTop=%d, y_=%d\n",
				playerBottom, blockTop, y_);
			OutputDebugString(debug);

			if (prevPlayerBottom <= blockTop + 5 && jumpVelocity_ > 0) { // 허용 범위 추가
				y_ = blockTop - playerHitboxHeight;
				isJumping_ = false;
				jumpVelocity_ = 0.0f;
				onBlock = true;
				newGroundY = blockTop - playerHitboxHeight;
			}
			else if (prevY >= blockBottom && jumpVelocity_ < 0) { // prevPlayerTop 대신 prevY 사용
				y_ = blockBottom;
				jumpVelocity_ = 0.0f;
			}
		}
	}

	// Check collisions with QuestionBlocks (vertical)
	if (!onBlock) {
		for (auto& qblock : Pimage.questionBlocks) {
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
				WCHAR debug[100];
				wsprintf(debug, L"QBlock Collision: playerBottom=%d, qblockTop=%d, y_=%d\n",
					playerBottom, qblockTop, y_);
				OutputDebugString(debug);

				if (prevPlayerBottom <= qblockTop && jumpVelocity_ > 0) {
					y_ = qblockTop - playerHitboxHeight;
					isJumping_ = false;
					jumpVelocity_ = 0.0f;
					onBlock = true;
					newGroundY = qblockTop - playerHitboxHeight;
				}
				else if (prevPlayerTop >= qblockBottom && jumpVelocity_ < 0) {
					if (!qblock.hit) {
						qblock.hit = true;
						eatFlower_ = true;
					}
					y_ = qblockBottom;
					jumpVelocity_ = 0.0f;
				}
			}
		}
	}

	// Check collisions with TBlocks (vertical)
	if (!onBlock) {
		for (const auto& tblock : Pimage.tBlocks) {
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

	// Check if player is on a Hole (vertical)
	if (!onBlock) {
		for (const auto& hole : Pimage.holes) {
			int holeLeft = hole.x;
			int holeRight = hole.x + hole.width;
			int holeTop = hole.y;
			int holeBottom = hole.y + hole.height;

			int playerLeft = playerHitboxX;
			int playerRight = playerHitboxX + playerHitboxWidth;
			int playerTop = playerHitboxY;
			int playerBottom = playerHitboxY + playerHitboxHeight;

			bool overlapX = playerRight > holeLeft && playerLeft < holeRight;
			bool overlapY = playerBottom > holeTop && playerTop < holeBottom;

			if (overlapX && overlapY) {
				int prevPlayerBottom = prevY + playerHitboxHeight;
				if (prevPlayerBottom <= holeTop && jumpVelocity_ > 0) {
					// Player lands on the hole, start falling
					isFallingIntoHole = true;
					fallProgress = 0.0f;
					y_ = holeTop - playerHitboxHeight; // Snap to hole top initially
					break; // Exit loop to start falling animation
				}
			}
		}
	}

	// Check if player is on TBlock4 and down key is pressed to enter hidden stage
	if (!isFallingIntoHole && !Pimage.tBlocks.empty() && Pimage.tBlocks.size() >= 4) { // Ensure TBlock4 exists
		const auto& tblock4 = Pimage.tBlocks[3]; // TBlock4 is at index 3 (0-based)
		int tblockLeft = tblock4.x;
		int tblockRight = tblock4.x + tblock4.width;
		int tblockTop = tblock4.y;

		int playerLeft = playerHitboxX;
		int playerRight = playerHitboxX + playerHitboxWidth;
		bool overlapX = playerRight > tblockLeft && playerLeft < tblockRight;

		// Check if player is standing on TBlock4 and down key is pressed
		if (overlapX && y_ == tblockTop - playerHitboxHeight && GetAsyncKeyState(VK_DOWN) & 0x8000) {

			Pimage.currentStage = 2; // Reset to stage 1 first to ensure NextStage moves to hidden
			Images.NextStage();
		}
	}

	// Check if player is still on a block after moving
	if (!isFallingIntoHole && groundY_ != defaultGroundY_) {
		bool stillOnBlock = false;

		for (const auto& block : Pimage.blocks) {
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
			for (const auto& qblock : Pimage.questionBlocks) {
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
			for (const auto& tblock : Pimage.tBlocks) {
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

	int stageWidth = (Images.NowStage() == 1 ? Pimage.mStage1.GetWidth() :
		Images.NowStage() == 2 ? Pimage.mStageHidden.GetWidth() :
		Images.NowStage() == 3 ? Pimage.mStage2.GetWidth() :
		Images.NowStage() == 4 ? Pimage.mStage3.GetWidth() : 0);

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
	}
	if (x_ + playerWidth > stageWidth) {
		x_ = stageWidth - playerWidth;
	}

	if (!moved) {
		move_ = false;
		imageNum = 0;
	}
}

int Player_::State() {
	if (!eatFlower_) {
		if (!eatMushroom_) return TINO;
		else if (eatMushroom_) return LARGETINO;
	}
	else {
		if (!eatMushroom_) return PAIRI;
		else return LIZAMONG;
	}
}

void Image_::ImageInit() {
	Player_Move_Tino.Load(TEXT("Image/티노 스프라이트.png"));
	Player_Move_Pairi.Load(TEXT("Image/파이리 스프라이트.png"));
	Player_Move_Lizard.Load(TEXT("Image/리자드 스프라이트.png"));
	Player_Move_Lizamong.Load(TEXT("Image/리자몽 스프라이트.png"));

	Player_Attack_Tino.Load(TEXT("Image/티노 스프라이트.png"));
	Player_Attack_Pairi.Load(TEXT("Image/파이리 스프라이트.png"));
	Player_Attack_Lizamong.Load(TEXT("Image/리자몽 스프라이트.png"));

	mStage1.Load(TEXT("Image/월드 1.png"));
	mStageHidden.Load(TEXT("Image/히든 스테이지.png"));
	mStage2.Load(TEXT("Image/월드 2.png"));
	mStage3.Load(TEXT("Image/월드 3.png"));

	blockImage.Load(TEXT("Image/BrickBlockBrown.png"));
	questionBlockImage.Load(TEXT("Image/QuestionBlock.gif"));
	Block block1 = { 320, 335, 16, 32 };
	Block block2 = { 352, 335, 16, 32 };
	Block block3 = { 384, 335, 16, 32 };
	Block block4 = { 256, 335, 16, 42 };
	Block block5 = { 352, 185, 16, 42 };
	Block block6 = { 1280, 185, 130, 42 };
	Block block7 = { 1232, 335, 16, 32 };
	Block block8 = { 1264, 335, 16, 32 };
	Block block9 = { 1456, 185, 16, 32 };
	Block block10 = { 1472, 185, 16, 32 };
	Block block11 = { 1488, 185, 16, 32 };
	Block block12 = { 1600, 335, 16, 32 };
	Block block13 = { 1616, 335, 16, 32 };
	Block block14 = { 1744, 185, 16, 42 };
	Block block15 = { 1888, 335, 16, 42 };
	Block block16 = { 1936, 185, 16, 42 };
	Block block17 = { 1952, 185, 16, 42 };
	Block block18 = { 1968, 185, 16, 42 };
	Block block19 = { 2048, 185, 16, 32 };
	Block block20 = { 2096, 185, 16, 32 };
	Block block21 = { 2064, 335, 16, 42 };
	Block block22 = { 2080, 335, 16, 42 };
	Block block23 = { 2688, 335, 16, 32 };
	Block block24 = { 2704, 335, 16, 32 };
	Block block25 = { 2736, 335, 16, 32 };
	blocks.push_back(block1);
	blocks.push_back(block2);
	blocks.push_back(block3);
	blocks.push_back(block4);
	blocks.push_back(block5);
	blocks.push_back(block6);
	blocks.push_back(block7);
	blocks.push_back(block8);
	blocks.push_back(block9);
	blocks.push_back(block10);
	blocks.push_back(block11);
	blocks.push_back(block12);
	blocks.push_back(block13);
	blocks.push_back(block14);
	blocks.push_back(block15);
	blocks.push_back(block16);
	blocks.push_back(block17);
	blocks.push_back(block18);
	blocks.push_back(block19);
	blocks.push_back(block20);
	blocks.push_back(block21);
	blocks.push_back(block22);
	blocks.push_back(block23);
	blocks.push_back(block24);
	blocks.push_back(block25);
	QuestionBlock qblock1 = { 336, 335, 16, 42, false };
	QuestionBlock qblock2 = { 368, 335, 16, 42, false };
	QuestionBlock qblock3 = { 1248, 335, 16, 42, false };
	QuestionBlock qblock4 = { 1504, 335, 16, 42, false };
	QuestionBlock qblock5 = { 1504, 185, 16, 42, false };
	QuestionBlock qblock6 = { 1696, 335, 16, 42, false };
	QuestionBlock qblock7 = { 1744, 335, 16, 42, false };
	QuestionBlock qblock8 = { 1792, 335, 16, 42, false };
	QuestionBlock qblock9 = { 2064, 185, 16, 42, false };
	QuestionBlock qblock10 = { 2080, 185, 16, 42, false };
	QuestionBlock qblock11 = { 2720, 335, 16, 42, false };
	questionBlocks.push_back(qblock1);
	questionBlocks.push_back(qblock2);
	questionBlocks.push_back(qblock3);
	questionBlocks.push_back(qblock4);
	questionBlocks.push_back(qblock5);
	questionBlocks.push_back(qblock6);
	questionBlocks.push_back(qblock7);
	questionBlocks.push_back(qblock8);
	questionBlocks.push_back(qblock9);
	questionBlocks.push_back(qblock10);
	questionBlocks.push_back(qblock11);

	TBlock tblock1 = { 448, 413, 32, 76 };
	TBlock tblock2 = { 608, 375, 32, 110 };
	TBlock tblock3 = { 736, 336, 32, 150 };  // 파이프들
	TBlock tblock4 = { 912, 336, 32, 150 };

	TBlock tblock5 = { 2140, 448, 59, 38 };
	TBlock tblock6 = { 2156, 410, 43, 38 };
	TBlock tblock7 = { 2172, 372, 27, 38 }; // 첫번째 계단
	TBlock tblock8 = { 2188, 334, 11, 38 };

	TBlock tblock9 = { 2238, 334, 16, 38 };
	TBlock tblock10 = { 2238, 372, 32, 38 }; // 두번째 계단
	TBlock tblock11 = { 2238, 410, 48, 38 };
	TBlock tblock12 = { 2238, 448, 64, 38 };

	TBlock tblock13 = { 2366, 448, 75, 38 };
	TBlock tblock14 = { 2382, 410, 59, 38 };
	TBlock tblock15 = { 2398, 372, 43, 38 };  // 세번째 계단
	TBlock tblock16 = { 2414, 334, 27, 38 };

	TBlock tblock17 = { 2480, 334, 16, 38 };
	TBlock tblock18 = { 2480, 372, 32, 38 };  // 네번째 계단
	TBlock tblock19 = { 2480, 410, 48, 38 };
	TBlock tblock20 = { 2480, 448, 64, 38 };

	TBlock tblock21 = { 2602, 413, 32, 76 }; // 파이프
	TBlock tblock22 = { 2864, 413, 32, 76 };

	TBlock tblock23 = { 2896, 448, 144, 38 };
	TBlock tblock24 = { 2912, 410, 128, 38 };
	TBlock tblock25 = { 2928, 372, 112, 38 };
	TBlock tblock26 = { 2944, 334, 96, 38 };
	TBlock tblock27 = { 2960, 296, 80, 38 };  // 마지막 계단
	TBlock tblock28 = { 2976, 258, 64, 38 };
	TBlock tblock29 = { 2992, 220, 48, 38 };
	TBlock tblock30 = { 3008, 182, 32, 38 };
	tBlocks.push_back(tblock1);
	tBlocks.push_back(tblock2);
	tBlocks.push_back(tblock3);
	tBlocks.push_back(tblock4);
	tBlocks.push_back(tblock5);
	tBlocks.push_back(tblock6);
	tBlocks.push_back(tblock7);
	tBlocks.push_back(tblock8);
	tBlocks.push_back(tblock9);
	tBlocks.push_back(tblock10);
	tBlocks.push_back(tblock11);
	tBlocks.push_back(tblock12);
	tBlocks.push_back(tblock13);
	tBlocks.push_back(tblock14);
	tBlocks.push_back(tblock15);
	tBlocks.push_back(tblock16);
	tBlocks.push_back(tblock17);
	tBlocks.push_back(tblock18);
	tBlocks.push_back(tblock19);
	tBlocks.push_back(tblock20);
	tBlocks.push_back(tblock21);
	tBlocks.push_back(tblock22);
	tBlocks.push_back(tblock23);
	tBlocks.push_back(tblock24);
	tBlocks.push_back(tblock25);
	tBlocks.push_back(tblock26);
	tBlocks.push_back(tblock27);
	tBlocks.push_back(tblock28);
	tBlocks.push_back(tblock29);
	tBlocks.push_back(tblock30);

	Hole hole1 = { 1100, 486, 34, 74 };
	holes.push_back(hole1);

	stage1 = hidden = stage2 = stage3 = false;
	currentStage = 1;
}

void Image_::DrawBackGround(int x, int y, HDC targetDC) {
	int cameraX = x - 400;
	if (cameraX < 0) cameraX = 0;

	int srcWidth = (stage1 ? mStage1.GetWidth() : stage2 ? mStage2.GetWidth() :
		stage3 ? mStage3.GetWidth() : mStageHidden.GetWidth());
	int srcHeight = (stage1 ? mStage1.GetHeight() : stage2 ? mStage2.GetHeight() :
		stage3 ? mStage3.GetHeight() : mStageHidden.GetHeight());

	if (srcWidth <= wRect.right) {
		cameraX = 0;
	}
	else if (cameraX > srcWidth - wRect.right) {
		cameraX = srcWidth - wRect.right;
	}

	int destWidth = wRect.right;
	int destHeight = wRect.bottom;

	if (stage1 && !mStage1.IsNull()) {
		mStage1.StretchBlt(targetDC, 0, 0, destWidth, destHeight, cameraX, 0, wRect.right, srcHeight, SRCCOPY);
	}
	else if (stage2 && !mStage2.IsNull()) {
		mStage2.StretchBlt(targetDC, 0, 0, destWidth, destHeight, cameraX, 0, wRect.right, srcHeight, SRCCOPY);
	}
	else if (stage3 && !mStage3.IsNull()) {
		mStage3.StretchBlt(targetDC, 0, 0, destWidth, destHeight, cameraX, 0, wRect.right, srcHeight, SRCCOPY);
	}
	else if (hidden && !mStageHidden.IsNull()) {
		mStageHidden.StretchBlt(targetDC, 0, 0, destWidth, destHeight, cameraX, 0, wRect.right, srcHeight, SRCCOPY);
	}
	else {
		OutputDebugString(L"No valid background image\n");
	}

	if (!blockImage.IsNull()) {
		for (const auto& block : blocks) {
			int offsetX = block.x - cameraX;
			if (offsetX + block.width > 0 && offsetX < wRect.right) {
				blockImage.StretchBlt(targetDC, offsetX, block.y, block.width, block.height, 0, 0, blockImage.GetWidth(), blockImage.GetHeight(), SRCCOPY);
			}
		}
	}

	if (!questionBlockImage.IsNull()) {
		for (const auto& qblock : questionBlocks) {
			int offsetX = qblock.x - cameraX;
			if (offsetX + qblock.width > 0 && offsetX < wRect.right) {
				questionBlockImage.StretchBlt(targetDC, offsetX, qblock.y, qblock.width, qblock.height, 0, 0, questionBlockImage.GetWidth(), questionBlockImage.GetHeight(), SRCCOPY);
			}
		}
	}
	// TBlocks are not rendered as they are invisible
}

void Image_::NextStage() {
	currentStage++;
	if (currentStage > 4) currentStage = 1;
	stage1 = (currentStage == 1);
	hidden = (currentStage == 2);
	stage2 = (currentStage == 3);
	stage3 = (currentStage == 4);
	Player.ResetPosition();
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
	blockImage.Destroy();
	questionBlockImage.Destroy();
}

void Player_::DrawHitbox(HDC targetDC) {
	int hitboxX, hitboxY, hitboxWidth, hitboxHeight;
	GetHitbox(hitboxX, hitboxY, hitboxWidth, hitboxHeight); // hitbox_ 업데이트

	int cameraX = GetCameraX(x_, Pimage.NowStage() == 1 ? Pimage.mStage1.GetWidth() :
		Pimage.NowStage() == 2 ? Pimage.mStageHidden.GetWidth() :
		Pimage.NowStage() == 3 ? Pimage.mStage2.GetWidth() :
		Pimage.NowStage() == 4 ? Pimage.mStage3.GetWidth() : 0);

	// 화면 좌표로 변환
	RECT screenHitbox = { hitbox_.left - cameraX, hitbox_.top, hitbox_.right - cameraX, hitbox_.bottom };

	HPEN pen = CreatePen(PS_DASH, 3, RGB(255, 0, 0));
	HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH); // 내부 채우기 없음
	SelectObject(targetDC, pen);
	SelectObject(targetDC, hBrush);
	Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
	DeleteObject(pen);
}

void Image_::DrawHitBox(HDC targetDC) {
	int cameraX = GetCameraX(Player.x(), NowStage() == 1 ? mStage1.GetWidth() :
		NowStage() == 2 ? mStageHidden.GetWidth() :
		NowStage() == 3 ? mStage2.GetWidth() :
		NowStage() == 4 ? mStage3.GetWidth() : 0);

	// 블록 히트박스 그리기
	HPEN blockPen = CreatePen(PS_DASH, 3, RGB(255, 255, 0));
	HBRUSH blockBrush = (HBRUSH)GetStockObject(NULL_BRUSH); // 내부 채우기 없음
	SelectObject(targetDC, blockPen);
	SelectObject(targetDC, blockBrush);

	for (const auto& block : blocks) {
		RECT screenHitbox = { block.x - cameraX, block.y, block.x + block.width - cameraX, block.y + block.height };
		Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
	}
	DeleteObject(blockPen);

	// 물음표 블록 히트박스 그리기
	HPEN qblockPen = CreatePen(PS_DASH, 3, RGB(0, 255, 0));
	HBRUSH qblockBrush = (HBRUSH)GetStockObject(NULL_BRUSH); // 내부 채우기 없음
	SelectObject(targetDC, qblockPen);
	SelectObject(targetDC, qblockBrush);

	for (const auto& qblock : questionBlocks) {
		if (!qblock.hit) {
			RECT screenHitbox = { qblock.x - cameraX, qblock.y, qblock.x + qblock.width - cameraX, qblock.y + qblock.height };
			Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
		}
	}
	DeleteObject(qblockPen);

	// 파이프, 계단 블록 히트박스 그리기
	HPEN tblockPen = CreatePen(PS_DASH, 3, RGB(255, 0, 255));
	HBRUSH tblockBrush = (HBRUSH)GetStockObject(NULL_BRUSH); // 내부 채우기 없음
	SelectObject(targetDC, tblockPen);
	SelectObject(targetDC, tblockBrush);

	for (const auto& tblock : tBlocks) {
		RECT screenHitbox = { tblock.x - cameraX, tblock.y, tblock.x + tblock.width - cameraX, tblock.y + tblock.height };
		Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
	}
	DeleteObject(tblockPen);
}

int GetCameraX(int playerX, int stageWidth) {
	int cameraX = playerX - 400;
	if (cameraX < 0) cameraX = 0;
	if (stageWidth <= wRect.right) {
		cameraX = 0;
	}
	else if (cameraX > stageWidth - wRect.right) {
		cameraX = stageWidth - wRect.right;
	}
	return cameraX;
}