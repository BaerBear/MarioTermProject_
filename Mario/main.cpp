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

#define MOVE 10
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
    std::vector<Block> blocks[4];
    std::vector<QuestionBlock> questionBlocks[4];
    std::vector<TBlock> tBlocks[4];
    std::vector<Hole> holes[4];

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
			if(direct_ == RIGHT) hitboxX += 14;
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
                if (Player.State() == TINO || Player.State() == LIZAMONG || Player.State() == LIZAD) {
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
	eatMushroom_ = false;
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

    direct_ = RIGHT;
    move_ = false;
    eatFlower_ = false;
    eatMushroom_ = false;
    isJumping_ = false;
    jumpVelocity_ = 0.0f;
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
                    if (direct_ == LEFT)
                        Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 10, 36, 39, RGB(0, 255, 0));
                    else
                        Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 10, 36, 39, RGB(0, 255, 0));
                }
                else if (eatMushroom_) {
                    if (direct_ == LEFT)
                        Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 10, 36, 39, RGB(0, 255, 0));
                    else
                        Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 10, 36, 39, RGB(0, 255, 0));
                }
            }
            else if (eatFlower_) {
                if (!eatMushroom_) {
                    if (direct_ == LEFT)
                        Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 40, 75, 39, 39, RGB(199, 225, 209));
                    else
                        Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 40, 75, 39, 39, RGB(199, 225, 209));
                }
                else if (eatMushroom_) {
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
                    if (direct_ == LEFT)
                        Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
                    else
                        Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
                }
                else if (eatMushroom_) {
                    if (direct_ == LEFT)
                        Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
                    else
                        Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, imageNum * 56 + 10, 69, 36, 39, RGB(0, 255, 0));
                }
            }
            else if (eatFlower_) {
                if (!eatMushroom_) {
                    if (direct_ == LEFT)
                        Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 40, 75, 39, 39, RGB(199, 225, 209));
                    else
                        Pimage.Player_Move_Pairi.TransparentBlt(targetDC, offsetX, y_, 39, 39, imageNum * 40, 75, 39, 39, RGB(199, 225, 209));
                }
                else if (eatMushroom_) {
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
    if (isFallingIntoHole) {
        const float fallSpeed = 3.0f;
        fallProgress += fallSpeed;
        y_ += static_cast<int>(fallSpeed);

        if (fallProgress >= 100.0f) {
            PostQuitMessage(0);
        }
        return;
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
    for (const auto& block : Pimage.blocks[Pimage.currentStage - 1]) {
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
        for (const auto& qblock : Pimage.questionBlocks[Pimage.currentStage - 1]) {
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
        for (const auto& tblock : Pimage.tBlocks[Pimage.currentStage - 1]) {
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

    for (const auto& block : Pimage.blocks[Pimage.currentStage - 1]) {
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
        for (auto& qblock : Pimage.questionBlocks[Pimage.currentStage - 1]) {
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
		for (const auto& tblock : Pimage.tBlocks[Pimage.currentStage - 1]) {
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
        for (const auto& hole : Pimage.holes[Pimage.currentStage - 1]) {
            int holeLeft = hole.x;
            int holeRight = hole.x + hole.width;
            int holeTop = hole.y;

            int playerBottomCenterX = hitbox_.left + (hitbox_.right - hitbox_.left) / 2;
            int playerBottomY = hitbox_.bottom;

            bool withinHoleX = playerBottomCenterX >= holeLeft && playerBottomCenterX <= holeRight;

            int prevPlayerBottom = prevY + playerHitboxHeight;
            bool atHoleTop = prevPlayerBottom >= holeTop - 5 && isJumping_ == FALSE;

            if (withinHoleX && atHoleTop) {
                isFallingIntoHole = true;
                fallProgress = 0.0f;
                y_ = holeTop - playerHitboxHeight;
                break;
            }
        }
    }

    if (!isFallingIntoHole && !Pimage.tBlocks[Pimage.currentStage - 1].empty() && Pimage.tBlocks[Pimage.currentStage - 1].size() >= 4) {
        const auto& tblock4 = Pimage.tBlocks[Pimage.currentStage - 1][3];
        int tblockLeft = tblock4.x;
        int tblockRight = tblock4.x + tblock4.width;
        int tblockTop = tblock4.y;

		int playerLeft = playerHitboxX;
		int playerRight = playerHitboxX + playerHitboxWidth;
		bool overlapX = playerRight > tblockLeft && playerLeft < tblockRight;

        if (overlapX && y_ == tblockTop - playerHitboxHeight && GetAsyncKeyState(VK_DOWN) & 0x8000) {
            Pimage.currentStage = 2;
            Images.NextStage();
        }
    }

    if (!isFallingIntoHole && groundY_ != defaultGroundY_) {
        bool stillOnBlock = false;

        for (const auto& block : Pimage.blocks[Pimage.currentStage - 1]) {
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
            for (const auto& qblock : Pimage.questionBlocks[Pimage.currentStage - 1]) {
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
            for (const auto& tblock : Pimage.tBlocks[Pimage.currentStage - 1]) {
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

    WCHAR debug[100];
    wsprintf(debug, L"Player Hitbox: (%d, %d, %d, %d), State=%d\n",
        hitbox_.left, hitbox_.top, hitbox_.right, hitbox_.bottom, State());
    OutputDebugString(debug);

    State();
}

int Player_::State() {
    if (!eatFlower_) {
        return TINO;
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

    // Stage 1 Objects (currentStage == 1)
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
    blocks[0].push_back(block1_1);
    blocks[0].push_back(block1_2);
    blocks[0].push_back(block1_3);
    blocks[0].push_back(block1_4);
    blocks[0].push_back(block1_5);
    blocks[0].push_back(block1_6);
    blocks[0].push_back(block1_7);
    blocks[0].push_back(block1_8);
    blocks[0].push_back(block1_9);
    blocks[0].push_back(block1_10);
    blocks[0].push_back(block1_11);
    blocks[0].push_back(block1_12);
    blocks[0].push_back(block1_13);
    blocks[0].push_back(block1_14);
    blocks[0].push_back(block1_15);
    blocks[0].push_back(block1_16);
    blocks[0].push_back(block1_17);
    blocks[0].push_back(block1_18);
    blocks[0].push_back(block1_19);
    blocks[0].push_back(block1_20);
    blocks[0].push_back(block1_21);
    blocks[0].push_back(block1_22);
    blocks[0].push_back(block1_23);
    blocks[0].push_back(block1_24);
    blocks[0].push_back(block1_25);

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
    questionBlocks[0].push_back(qblock1_1);
    questionBlocks[0].push_back(qblock1_2);
    questionBlocks[0].push_back(qblock1_3);
    questionBlocks[0].push_back(qblock1_4);
    questionBlocks[0].push_back(qblock1_5);
    questionBlocks[0].push_back(qblock1_6);
    questionBlocks[0].push_back(qblock1_7);
    questionBlocks[0].push_back(qblock1_8);
    questionBlocks[0].push_back(qblock1_9);
    questionBlocks[0].push_back(qblock1_10);
    questionBlocks[0].push_back(qblock1_11);

    TBlock tblock1_1 = { 442, 413, 32, 76 };
    TBlock tblock1_2 = { 602, 375, 32, 110 };
    TBlock tblock1_3 = { 730, 336, 32, 150 };
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
    TBlock tblock1_21 = { 2602, 413, 32, 76 };
    TBlock tblock1_22 = { 2864, 413, 32, 76 };
    TBlock tblock1_23 = { 2896, 448, 144, 38 };
    TBlock tblock1_24 = { 2912, 410, 128, 38 };
    TBlock tblock1_25 = { 2928, 372, 112, 38 };
    TBlock tblock1_26 = { 2944, 334, 96, 38 };
    TBlock tblock1_27 = { 2960, 296, 80, 38 };
    TBlock tblock1_28 = { 2976, 258, 64, 38 };
    TBlock tblock1_29 = { 2992, 220, 48, 38 };
    TBlock tblock1_30 = { 3008, 182, 32, 38 };
    tBlocks[0].push_back(tblock1_1);
    tBlocks[0].push_back(tblock1_2);
    tBlocks[0].push_back(tblock1_3);
    tBlocks[0].push_back(tblock1_4);
    tBlocks[0].push_back(tblock1_5);
    tBlocks[0].push_back(tblock1_6);
    tBlocks[0].push_back(tblock1_7);
    tBlocks[0].push_back(tblock1_8);
    tBlocks[0].push_back(tblock1_9);
    tBlocks[0].push_back(tblock1_10);
    tBlocks[0].push_back(tblock1_11);
    tBlocks[0].push_back(tblock1_12);
    tBlocks[0].push_back(tblock1_13);
    tBlocks[0].push_back(tblock1_14);
    tBlocks[0].push_back(tblock1_15);
    tBlocks[0].push_back(tblock1_16);
    tBlocks[0].push_back(tblock1_17);
    tBlocks[0].push_back(tblock1_18);
    tBlocks[0].push_back(tblock1_19);
    tBlocks[0].push_back(tblock1_20);
    tBlocks[0].push_back(tblock1_21);
    tBlocks[0].push_back(tblock1_22);
    tBlocks[0].push_back(tblock1_23);
    tBlocks[0].push_back(tblock1_24);
    tBlocks[0].push_back(tblock1_25);
    tBlocks[0].push_back(tblock1_26);
    tBlocks[0].push_back(tblock1_27);
    tBlocks[0].push_back(tblock1_28);
    tBlocks[0].push_back(tblock1_29);
    tBlocks[0].push_back(tblock1_30);

    Hole hole1_1 = { 1104, 487, 32, 74 };
    Hole hole1_2 = { 1376, 487, 48, 74 };
    Hole hole1_3 = { 2448, 487, 32, 74 };
    holes[0].push_back(hole1_1);
    holes[0].push_back(hole1_2);
    holes[0].push_back(hole1_3);

    // Hidden Stage Objects (currentStage == 2)
   TBlock tblock2_1 = { 188, 346, 330, 126 };
   TBlock tblock2_2 = { 0, 0, 48, 474 };
   TBlock tblock2_3 = { 614, 388, 102, 86 };
   
   tBlocks[1].push_back(tblock2_1);
   tBlocks[1].push_back(tblock2_2);
   tBlocks[1].push_back(tblock2_3);
   
   //QuestionBlock qblock2_1 = { 120, 335, 16, 42, false };
   //questionBlocks[1].push_back(qblock2_1);
   //
   //TBlock tblock2_1 = { 200, 413, 32, 76 };
   //tBlocks[1].push_back(tblock2_1);

    //Hole hole2_1 = { 250, 435, 32, 74 };
    //holes[1].push_back(hole2_1);

    

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
        for (const auto& block : blocks[currentStage - 1]) {
            int offsetX = block.x - cameraX;
            if (offsetX + block.width > 0 && offsetX < wRect.right) {
                blockImage.StretchBlt(targetDC, offsetX, block.y, block.width, block.height, 0, 0, blockImage.GetWidth(), blockImage.GetHeight(), SRCCOPY);
            }
        }
    }

    if (!questionBlockImage.IsNull()) {
        for (const auto& qblock : questionBlocks[currentStage - 1]) {
            int offsetX = qblock.x - cameraX;
            if (offsetX + qblock.width > 0 && offsetX < wRect.right) {
                questionBlockImage.StretchBlt(targetDC, offsetX, qblock.y, qblock.width, qblock.height, 0, 0, questionBlockImage.GetWidth(), questionBlockImage.GetHeight(), SRCCOPY);
            }
        }
    }
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
    GetHitbox(hitboxX, hitboxY, hitboxWidth, hitboxHeight);

    int cameraX = GetCameraX(x_, Pimage.NowStage() == 1 ? Pimage.mStage1.GetWidth() :
        Pimage.NowStage() == 2 ? Pimage.mStageHidden.GetWidth() :
        Pimage.NowStage() == 3 ? Pimage.mStage2.GetWidth() :
        Pimage.NowStage() == 4 ? Pimage.mStage3.GetWidth() : 0);

    RECT screenHitbox = { hitbox_.left - cameraX, hitbox_.top, hitbox_.right - cameraX, hitbox_.bottom };

    HPEN pen = CreatePen(PS_DASH, 3, RGB(255, 0, 0));
    HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
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

    HPEN blockPen = CreatePen(PS_DASH, 3, RGB(255, 255, 0));
    HBRUSH blockBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    SelectObject(targetDC, blockPen);
    SelectObject(targetDC, blockBrush);

    for (const auto& block : blocks[currentStage - 1]) {
        RECT screenHitbox = { block.x - cameraX, block.y, block.x + block.width - cameraX, block.y + block.height };
        Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
    }
    DeleteObject(blockPen);

    HPEN qblockPen = CreatePen(PS_DASH, 3, RGB(0, 255, 0));
    HBRUSH qblockBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    SelectObject(targetDC, qblockPen);
    SelectObject(targetDC, qblockBrush);

    for (const auto& qblock : questionBlocks[currentStage - 1]) {
        if (!qblock.hit) {
            RECT screenHitbox = { qblock.x - cameraX, qblock.y, qblock.x + qblock.width - cameraX, qblock.y + qblock.height };
            Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
        }
    }
    DeleteObject(qblockPen);
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