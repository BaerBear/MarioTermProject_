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

#define MOVE 10
#define JUMP_VELOCITY -13
#define GRAVITY 0.5

#define STAGE1 1
#define HIDDEN 2
#define STAGE2 3
#define STAGE3 4

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
    CImage questionBlockImage[2];
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
    std::vector<Block> blocks[4]{};
    std::vector<QuestionBlock> questionBlocks[4]{};
    std::vector<TBlock> tBlocks[4]{};
    std::vector<Hole> holes[4]{};
    std::vector<FlagBlock> flagBlocks[4]{};

    Image_() {
        for (int i = 0; i < 4; ++i) {
            blocks[i].clear();
            questionBlocks[i].clear();
            tBlocks[i].clear();
            holes[i].clear();
            flagBlocks[i].clear();
        }
        currentStage = 1;
    };
    ~Image_() {};

    void ImageInit();
    void BlockInit();
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
    int time{};

    void PlayerInit();
    void ResetPosition();
    void DrawPlayer(HDC targetDC);
    void Move();
    void Attack();
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
        eatMushroom_ = !eatMushroom_;
        State();
    };

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
    bool isTouchingFlag; // 깃발 블럭에 닿았는지 여부
    float flagSlideProgress; // 깃발을 타고 내려가는 진행률
    float flagMoveRightProgress; // 오른쪽으로 이동하는 진행률
    int flagBottomY; // 깃발 블럭의 맨 아래 y 좌표
    bool isMovingRightAfterFlag; // 깃발 내려간 후 오른쪽 이동 중인지 여부
    int flagBlockStage; // 깃발 블럭이 있는 스테이지 번호

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
        Images.BlockInit();
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
        else if (wParam == '1') {
            Player.turnFlower();
        }
        else if (wParam == '2') {
            Player.turnMushroom();
        }
        break;
    }
    case WM_MOUSEMOVE: {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        int cameraX = Player.x() - 400;
        if (cameraX < 0) cameraX = 0;
        int stageWidth = (Images.NowStage() == STAGE1 ? Images.mStage1.GetWidth() :
            Images.NowStage() == HIDDEN ? Images.mStageHidden.GetWidth() :
            Images.NowStage() == STAGE2 ? Images.mStage2.GetWidth() :
            Images.NowStage() == STAGE3 ? Images.mStage3.GetWidth() : 0);
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
    isTouchingFlag = false;
    flagSlideProgress = 0.0f;
    flagMoveRightProgress = 0.0f;
    flagBottomY = 0;
    isMovingRightAfterFlag = false;
    flagBlockStage = 0;
}

void Player_::ResetPosition() {
    move_ = false;
    isJumping_ = false;
    jumpVelocity_ = 0.0f;
    isFallingIntoHole = false;
    direct_ = RIGHT;
    fallProgress = 0.0f;
    time = 0;
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
        defaultGroundY_ = 447;
    }

    if (Images.NowStage() != 2) {
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
    if (!Pimage.Player_Move_Pairi.IsNull()) {
        int cameraX = x_ - 400;
        if (cameraX < 0) cameraX = 0;
        int stageWidth = (Images.NowStage() == STAGE1 ? Pimage.mStage1.GetWidth() :
            Images.NowStage() == HIDDEN ? Pimage.mStageHidden.GetWidth() :
            Images.NowStage() == STAGE2 ? Pimage.mStage2.GetWidth() :
            Images.NowStage() == STAGE3 ? Pimage.mStage3.GetWidth() : 0);

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

        if (move_) {
            if (!eatFlower_) {
                if (!eatMushroom_) { // TINO
                    if (direct_ == LEFT) {
                        if (isJumping_)
                            Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, 10, 177, 36, 39, RGB(0, 255, 0));
                        else
                            Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, (2 - imageNum) * 56 + 10, 125, 36, 39, RGB(0, 255, 0));
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
                            Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 49, 54, 10, 177, 36, 39, RGB(0, 255, 0));
                        else
                            Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 49, 54, (2 - imageNum) * 56 + 10, 125, 36, 39, RGB(0, 255, 0));
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
                            Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, 10, 177, 36, 39, RGB(0, 255, 0));
                        else
                            Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 36, 39, (2 - imageNum) * 56 + 10, 177, 36, 39, RGB(0, 255, 0));
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
                            Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 49, 54, 10, 177, 36, 39, RGB(0, 255, 0));
                        else
                            Pimage.Player_Move_Tino.TransparentBlt(targetDC, offsetX, y_, 49, 54, (2 - imageNum) * 56 + 10, 177, 36, 39, RGB(0, 255, 0));
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
    if (isFallingIntoHole) {
        const float fallSpeed = 2.0f;
        fallProgress += fallSpeed;
        y_ += static_cast<int>(fallSpeed);

        if (fallProgress >= 100.0f) {
            PostQuitMessage(0);
        }
        return;
    }

    // 깃발 블럭을 타고 내려가는 중이거나 오른쪽으로 이동 중이면 다른 입력 무시
    if (isTouchingFlag || isMovingRightAfterFlag) {
        if (isTouchingFlag) {
            const float slideSpeed = 2.0f; // 내려가는 속도
            flagSlideProgress += slideSpeed;
            y_ += static_cast<int>(slideSpeed);

            // 플레이어의 히트박스 높이를 고려하여 바닥에 도달했는지 확인
            if (y_ >= defaultGroundY_) {
                y_ = defaultGroundY_;
                isTouchingFlag = false;
                move_ = true;
                direct_ = RIGHT;
                move_ = false;
                isMovingRightAfterFlag = true;
                flagSlideProgress = 0.0f;
            }
        }
        else if (isMovingRightAfterFlag) {
            const float moveRightSpeed = 2.0f; // 오른쪽 이동 속도
            const float moveRightDistance = 100.0f; // 오른쪽으로 이동할 거리
            flagMoveRightProgress += moveRightSpeed;
            x_ += static_cast<int>(moveRightSpeed);

            if (flagMoveRightProgress >= moveRightDistance) {
                Images.NextStage();
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

    int newX = x_;
    bool intendToMoveLeft = false;
    bool intendToMoveRight = false;

    // 깃발 블럭 충돌 감지
    if (!isFallingIntoHole && !Images.flagBlocks[Images.currentStage - 1].empty()) {
        for (const auto& flagBlock : Images.flagBlocks[Images.currentStage - 1]) {
            int flagLeft = flagBlock.x;
            int flagRight = flagBlock.x + flagBlock.width;
            int flagTop = flagBlock.y;
            int flagBottom = flagBlock.y + flagBlock.height;

            int playerLeft = hitbox_.left;
            int playerRight = hitbox_.right;
            int playerTop = hitbox_.top;
            int playerBottom = hitbox_.bottom;

            bool overlapX = playerRight > flagLeft && playerLeft < flagRight;
            bool overlapY = playerBottom > flagTop && playerTop < flagBottom;

            if (overlapX && overlapY) {
                isTouchingFlag = true;
                flagBlockStage = Images.currentStage;
                flagSlideProgress = 0.0f;
                flagMoveRightProgress = 0.0f;
                isMovingRightAfterFlag = false;
                isJumping_ = false;
                jumpVelocity_ = 0.0f;
                break;
            }
        }
    }

   

    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        direct_ = LEFT;
        intendToMoveLeft = true;
        move_ = true;
        moved = true;
    }
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        direct_ = RIGHT;
        intendToMoveRight = true;
        move_ = true;
        moved = true;
    }

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
        if (Images.NowStage() == STAGE1 || Images.NowStage() == STAGE2 || Images.NowStage() == STAGE3) defaultGroundY_ = 433;
        else if (Images.NowStage() == HIDDEN) defaultGroundY_ = 421;
    }
    else {
        if (Images.NowStage() == STAGE1 || Images.NowStage() == STAGE2 || Images.NowStage() == STAGE3) defaultGroundY_ = 447;
        else if (Images.NowStage() == HIDDEN) defaultGroundY_ = 435;
    }
    bool canMoveHorizontally = true;
    if (intendToMoveLeft) newX -= MOVE;
    if (intendToMoveRight) newX += MOVE;

    if (intendToMoveLeft || intendToMoveRight) {
        int checkHitboxX = newX;
        switch (State()) {
        case TINO:  checkHitboxX += 10; break;
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
                // 히든 스테이지에서 tblock1_3 (인덱스 2)는 충돌 차단에서 제외
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
                        qblock.hit = true;
                        eatFlower_ = true;
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
            bool atHoleTop = prevPlayerBottom >= holeTop - 5 && isJumping_ == FALSE;

            if (withinHoleX && atHoleTop) {
                isFallingIntoHole = true;
                fallProgress = 0.0f;
                break;
            }
        }
    }

    // Logic: Go to Hidden Stage from Stage 1 when standing on tblock0_4 and pressing down
    if (!isFallingIntoHole && !Images.tBlocks[Images.currentStage - 1].empty() && Images.tBlocks[Images.currentStage - 1].size() >= 4) {
        const auto& tblock4 = Images.tBlocks[Images.currentStage - 1][3]; // tblock0_4 in Stage 1
        int tblockLeft = tblock4.x;
        int tblockRight = tblock4.x + tblock4.width;
        int tblockTop = tblock4.y;

        int playerLeft = playerHitboxX;
        int playerRight = playerHitboxX + playerHitboxWidth;
        bool overlapX = playerRight > tblockLeft && playerLeft < tblockRight;

        if (overlapX && y_ == tblockTop - playerHitboxHeight && GetAsyncKeyState(VK_DOWN) & 0x8000) {
            Images.NextStage();
        }
    }

    // Logic: Return to Stage 1 from Hidden Stage when colliding with tblock1_3's left face
    if (!isFallingIntoHole && Images.currentStage == HIDDEN && !Images.tBlocks[Images.currentStage - 1].empty()) {
        const auto& tblock3 = Images.tBlocks[Images.currentStage - 1][2]; // tblock1_3 in Hidden Stage
        int tblockLeft = tblock3.x; // 614
        int tblockTop = tblock3.y; // 388
        int tblockBottom = tblock3.y + tblock3.height; // 474

        // 현재 히트박스
        int playerRight = hitbox_.right;
        int playerTop = hitbox_.top;
        int playerBottom = hitbox_.bottom;

        // 이전 프레임의 히트박스 오른쪽 위치 계산
        int prevHitboxX, prevHitboxY, prevHitboxWidth, prevHitboxHeight;
        int tempX = x_;
        x_ = prevX; // 이전 위치로 임시 변경
        GetHitbox(prevHitboxX, prevHitboxY, prevHitboxWidth, prevHitboxHeight); // 이전 히트박스 계산
        int prevPlayerRight = prevHitboxX + prevHitboxWidth; // 이전 히트박스의 오른쪽 위치
        x_ = tempX; // 원래 위치로 복원

        // 충돌 조건: 오른쪽으로 이동 중, 현재 오른쪽 면이 tblockLeft에 도달, 이전 오른쪽 면은 tblockLeft보다 왼쪽에 있었음
        bool collideWithLeftFace = intendToMoveRight && playerRight >= tblockLeft && prevPlayerRight < tblockLeft;
        bool yOverlap = playerBottom > tblockTop && playerTop < tblockBottom;

        if (collideWithLeftFace && yOverlap) {
            Images.currentStage = STAGE1;
            Images.stage1 = true;
            Images.hidden = false;
            Images.stage2 = false;
            Images.stage3 = false;
            Images.BlockInit();
            x_ = 2616;
            y_ = 372;
            groundY_ = 447;
            defaultGroundY_ = 447;
            move_ = false;
            isJumping_ = true;
            jumpVelocity_ = 0.0f;
            isFallingIntoHole = false;
            direct_ = RIGHT;
            fallProgress = 0.0f;
            eatFlower_ = false;
            eatMushroom_ = false;
        }
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

    int stageWidth = (Images.NowStage() == STAGE1 ? Pimage.mStage1.GetWidth() :
        Images.NowStage() == HIDDEN ? Pimage.mStageHidden.GetWidth() :
        Images.NowStage() == STAGE2 ? Pimage.mStage2.GetWidth() :
        Images.NowStage() == STAGE3 ? Pimage.mStage3.GetWidth() : 0);

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
        x_ = stageWidth - playerWidth; // 맵 끝에 고정
        move_ = true;
        imageNum = 0;
    }

    if (!moved) {
        move_ = false;
        if (!isJumping_) imageNum = 0;
        else if (isJumping_) {
            if (State() == PAIRI) {
                imageNum = (imageNum + 1) % 3;
            }
            else if (State() == LIZAMONG) {
                imageNum = (imageNum + 1) % 4;
            }
        }
        time = 0;
    }
    else if (!canMoveHorizontally && (intendToMoveLeft || intendToMoveRight) && !isJumping_) {
        move_ = true;
        imageNum = 0;
    }

    State();
}

void Player_::Attack() {}

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
    questionBlockImage[0].Load(TEXT("Image/QuestionBlock.gif"));
    questionBlockImage[1].Load(TEXT("Image/HitQuestionBlock.png"));

    stage1 = hidden = stage2 = stage3 = false;
    currentStage = STAGE1;
}

void Image_::BlockInit() {
    if (currentStage == STAGE1) {
        if (!blocks[1].empty()) {
            blocks[1].clear();
            questionBlocks[1].clear();
            tBlocks[1].clear();
            holes[1].clear();
            flagBlocks[1].clear();
        }
        if (!blocks[2].empty()) {
            blocks[2].clear();
            questionBlocks[2].clear();
            tBlocks[2].clear();
            holes[2].clear();
            flagBlocks[2].clear();
        }
        if (!blocks[3].empty()) {
            blocks[3].clear();
            questionBlocks[3].clear();
            tBlocks[3].clear();
            holes[3].clear();
            flagBlocks[3].clear();
        }
        if (blocks[0].empty()) {
            // Stage 1 Objects (currentStage == STAGE1)
            Block block0_1 = { 320, 335, 16, 32 };
            Block block0_2 = { 352, 335, 16, 32 };
            Block block0_3 = { 384, 335, 16, 32 };
            Block block0_4 = { 256, 335, 16, 42 };
            Block block0_5 = { 352, 185, 16, 42 };
            Block block0_6 = { 1280, 185, 130, 42 };
            Block block0_7 = { 1232, 335, 16, 32 };
            Block block0_8 = { 1264, 335, 16, 32 };
            Block block0_9 = { 1456, 185, 16, 32 };
            Block block0_10 = { 1472, 185, 16, 32 };
            Block block0_11 = { 1488, 185, 16, 32 };
            Block block0_12 = { 1600, 335, 16, 32 };
            Block block0_13 = { 1616, 335, 16, 32 };
            Block block0_14 = { 1744, 185, 16, 42 };
            Block block0_15 = { 1888, 335, 16, 42 };
            Block block0_16 = { 1936, 185, 16, 42 };
            Block block0_17 = { 1952, 185, 16, 42 };
            Block block0_18 = { 1968, 185, 16, 42 };
            Block block0_19 = { 2048, 185, 16, 32 };
            Block block0_20 = { 2096, 185, 16, 32 };
            Block block0_21 = { 2064, 335, 16, 42 };
            Block block0_22 = { 2080, 335, 16, 42 };
            Block block0_23 = { 2688, 335, 16, 32 };
            Block block0_24 = { 2704, 335, 16, 32 };
            Block block0_25 = { 2736, 335, 16, 32 };
            blocks[0].push_back(block0_1);
            blocks[0].push_back(block0_2);
            blocks[0].push_back(block0_3);
            blocks[0].push_back(block0_4);
            blocks[0].push_back(block0_5);
            blocks[0].push_back(block0_6);
            blocks[0].push_back(block0_7);
            blocks[0].push_back(block0_8);
            blocks[0].push_back(block0_9);
            blocks[0].push_back(block0_10);
            blocks[0].push_back(block0_11);
            blocks[0].push_back(block0_12);
            blocks[0].push_back(block0_13);
            blocks[0].push_back(block0_14);
            blocks[0].push_back(block0_15);
            blocks[0].push_back(block0_16);
            blocks[0].push_back(block0_17);
            blocks[0].push_back(block0_18);
            blocks[0].push_back(block0_19);
            blocks[0].push_back(block0_20);
            blocks[0].push_back(block0_21);
            blocks[0].push_back(block0_22);
            blocks[0].push_back(block0_23);
            blocks[0].push_back(block0_24);
            blocks[0].push_back(block0_25);

            QuestionBlock qblock0_1 = { 336, 335, 16, 42, false };
            QuestionBlock qblock0_2 = { 368, 335, 16, 42, false };
            QuestionBlock qblock0_3 = { 1248, 335, 16, 42, false };
            QuestionBlock qblock0_4 = { 1504, 335, 16, 42, false };
            QuestionBlock qblock0_5 = { 1504, 185, 16, 42, false };
            QuestionBlock qblock0_6 = { 1696, 335, 16, 42, false };
            QuestionBlock qblock0_7 = { 1744, 335, 16, 42, false };
            QuestionBlock qblock0_8 = { 1792, 335, 16, 42, false };
            QuestionBlock qblock0_9 = { 2064, 185, 16, 42, false };
            QuestionBlock qblock0_10 = { 2080, 185, 16, 42, false };
            QuestionBlock qblock0_11 = { 2720, 335, 16, 42, false };
            questionBlocks[0].push_back(qblock0_1);
            questionBlocks[0].push_back(qblock0_2);
            questionBlocks[0].push_back(qblock0_3);
            questionBlocks[0].push_back(qblock0_4);
            questionBlocks[0].push_back(qblock0_5);
            questionBlocks[0].push_back(qblock0_6);
            questionBlocks[0].push_back(qblock0_7);
            questionBlocks[0].push_back(qblock0_8);
            questionBlocks[0].push_back(qblock0_9);
            questionBlocks[0].push_back(qblock0_10);
            questionBlocks[0].push_back(qblock0_11);

            TBlock tblock0_1 = { 448, 413, 32, 76 };
            TBlock tblock0_2 = { 608, 375, 32, 110 };
            TBlock tblock0_3 = { 736, 336, 32, 150 };
            TBlock tblock0_4 = { 912, 336, 32, 150 };
            TBlock tblock0_5 = { 2140, 448, 59, 38 };
            TBlock tblock0_6 = { 2156, 410, 43, 38 };
            TBlock tblock0_7 = { 2172, 372, 27, 38 };
            TBlock tblock0_8 = { 2188, 334, 11, 38 };
            TBlock tblock0_9 = { 2238, 334, 16, 38 };
            TBlock tblock0_10 = { 2238, 372, 32, 38 };
            TBlock tblock0_11 = { 2238, 410, 48, 38 };
            TBlock tblock0_12 = { 2238, 448, 64, 38 };
            TBlock tblock0_13 = { 2366, 448, 75, 38 };
            TBlock tblock0_14 = { 2382, 410, 59, 38 };
            TBlock tblock0_15 = { 2398, 372, 43, 38 };
            TBlock tblock0_16 = { 2414, 334, 27, 38 };
            TBlock tblock0_17 = { 2480, 334, 16, 38 };
            TBlock tblock0_18 = { 2480, 372, 32, 38 };
            TBlock tblock0_19 = { 2480, 410, 48, 38 };
            TBlock tblock0_20 = { 2480, 448, 64, 38 };
            TBlock tblock0_21 = { 2608, 413, 32, 76 };
            TBlock tblock0_22 = { 2864, 413, 32, 76 };
            TBlock tblock0_23 = { 2896, 448, 144, 38 };
            TBlock tblock0_24 = { 2912, 410, 128, 38 };
            TBlock tblock0_25 = { 2928, 372, 112, 38 };
            TBlock tblock0_26 = { 2944, 334, 96, 38 };
            TBlock tblock0_27 = { 2960, 296, 80, 38 };
            TBlock tblock0_28 = { 2976, 258, 64, 38 };
            TBlock tblock0_29 = { 2992, 220, 48, 38 };
            TBlock tblock0_30 = { 3008, 182, 32, 38 };
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
            tBlocks[0].push_back(tblock0_24);
            tBlocks[0].push_back(tblock0_25);
            tBlocks[0].push_back(tblock0_26);
            tBlocks[0].push_back(tblock0_27);
            tBlocks[0].push_back(tblock0_28);
            tBlocks[0].push_back(tblock0_29);
            tBlocks[0].push_back(tblock0_30);

            Hole hole0_1 = { 1104, 487, 32, 74 };
            Hole hole0_2 = { 1376, 487, 48, 74 };
            Hole hole0_3 = { 2448, 487, 32, 74 };
            holes[0].push_back(hole0_1);
            holes[0].push_back(hole0_2);
            holes[0].push_back(hole0_3);

            // 깃발 블럭 추가 (Stage 1 끝부분)
            FlagBlock flagBlock0_1 = { 3168, 94, 16, 392 }; // x=3100, 높이 286 (맨 아래 y=486
            flagBlocks[0].push_back(flagBlock0_1);
        }
        }
    else if (currentStage == HIDDEN) {
        if (!blocks[0].empty()) {
            blocks[0].clear();
            questionBlocks[0].clear();
            tBlocks[0].clear();
            holes[0].clear();
        }
        if (!blocks[2].empty()) {
            blocks[2].clear();
            questionBlocks[2].clear();
            tBlocks[2].clear();
            holes[2].clear();
        }
        if (!blocks[3].empty()) {
            blocks[3].clear();
            questionBlocks[0].clear();
            tBlocks[3].clear();
            holes[3].clear();
        }
        if (blocks[1].empty()) {
            TBlock tblock1_1 = { 188, 346, 330, 126 };
            TBlock tblock1_2 = { 0, 0, 48, 474 };
            TBlock tblock1_3 = { 614, 388, 102, 86 };
            TBlock tblock1_4 = { 716, 0, 84, 392 };

            tBlocks[1].push_back(tblock1_1);
            tBlocks[1].push_back(tblock1_2);
            tBlocks[1].push_back(tblock1_3);
            tBlocks[1].push_back(tblock1_4);
        }
    }
    else if (currentStage == STAGE2) {
        if (!blocks[0].empty()) {
            blocks[0].clear();
            questionBlocks[0].clear();
            tBlocks[0].clear();
            holes[0].clear();
        }
        if (!blocks[1].empty()) {
            blocks[1].clear();
            questionBlocks[1].clear();
            tBlocks[1].clear();
            holes[1].clear();
        }
        if (!blocks[3].empty()) {
            blocks[3].clear();
            questionBlocks[3].clear();
            tBlocks[3].clear();
            holes[3].clear();
        }
        if (blocks[2].empty()) {
            TBlock tblock2_1 = { 291, 450, 57, 32 };
            TBlock tblock2_2 = { 386, 338, 122, 32 };
            TBlock tblock2_3 = { 419, 188, 73, 32 };
            TBlock tblock2_4 = { 514, 450, 42, 32 };
            TBlock tblock2_5 = { 291, 450, 57, 32 };
            TBlock tblock2_6 = { 563, 300, 72, 32 };
            TBlock tblock2_7 = { 643, 151, 103, 32 };
            TBlock tblock2_8 = { 802, 488, 58, 32 };
            TBlock tblock2_9 = { 945, 488, 72, 32 };
            TBlock tblock2_10 = { 1042, 488, 72, 32 };
            TBlock tblock2_11 = { 962, 189, 57, 32 };
            TBlock tblock2_12 = { 1122, 339, 42, 32 };
            TBlock tblock2_13 = { 1218, 226, 90, 32 };
            TBlock tblock2_14 = { 1569, 414, 57, 32 };
            TBlock tblock2_15 = { 1810, 488, 42, 32 };
            TBlock tblock2_16 = { 1858, 339, 58, 32 };
            TBlock tblock2_17 = { 1666, 263, 122, 32 };
            TBlock tblock2_18 = { 1954, 339, 58, 32 };
            TBlock tblock2_19 = { 0, 488, 252, 32 };
            TBlock tblock2_20 = { 2209, 338, 93, 150 };
            TBlock tblock2_21 = { 2241, 261, 62, 78 };
            TBlock tblock2_22 = { 2271, 188, 32, 72 };
            TBlock tblock2_23 = { 2065, 487, 751, 72 };

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

            QuestionBlock qblock2_1 = { 944, 376, 16, 36, false };
            questionBlocks[2].push_back(qblock2_1);

            Hole hole2_1 = { 257, 500, 1803, 69 };
            holes[2].push_back(hole2_1);
        }
        }
    else if (currentStage == STAGE3) {
            if (!blocks[0].empty()) {
                blocks[0].clear();
                questionBlocks[0].clear();
                tBlocks[0].clear();
                holes[0].clear();
            }
            if (!blocks[1].empty()) {
                blocks[1].clear();
                questionBlocks[1].clear();
                tBlocks[1].clear();
                holes[1].clear();
            }
            if (!blocks[2].empty()) {
                blocks[2].clear();
                questionBlocks[2].clear();
                tBlocks[2].clear();
                holes[2].clear();
            }
            if (blocks[3].empty()) {
                QuestionBlock qblock3_1 = { 480, 226, 16, 36, false };
                questionBlocks[3].push_back(qblock3_1);

                TBlock tblock3_1 = { 0, 262, 48, 36 };
                TBlock tblock3_2 = { 48, 300, 16, 36 };
                TBlock tblock3_3 = { 64, 336, 16, 36 };
                TBlock tblock3_4 = { 80, 372, 128, 72 };
                TBlock tblock3_5 = { 240, 372, 176, 108 };
                TBlock tblock3_6 = { 0, 112, 386, 72 };
                TBlock tblock3_7 = { 368, 184, 16, 36 };
                TBlock tblock3_8 = { 464, 372, 48, 108 };
                TBlock tblock3_9 = { 386, 74, 204, 36 };
                TBlock tblock3_10 = { 590, 74, 562, 144 };
                TBlock tblock3_11 = { 560, 336, 594, 144 };
                TBlock tblock3_12 = { 1150, 74, 402, 36 };
                TBlock tblock3_13 = { 1150, 374, 514, 108 };
                TBlock tblock3_14 = { 1552, 74, 112, 108 };
                TBlock tblock3_15 = { 1280, 110, 16, 36 };
                TBlock tblock3_16 = { 1408, 110, 16, 36 };
                TBlock tblock3_17 = { 1664, 482, 192, 36 };
                TBlock tblock3_18 = { 1664, 74, 304, 36 };
                TBlock tblock3_19 = { 1856, 374, 64, 108 };
                TBlock tblock3_20 = { 1920, 482, 48, 36 };
                TBlock tblock3_21 = { 1968, 374, 80, 108 };
                TBlock tblock3_22 = { 1968, 74, 80, 108 };
                TBlock tblock3_23 = { 2048, 374, 206, 36 };
                TBlock tblock3_24 = { 2270, 74, 32, 144 };
                TBlock tblock3_25 = { 2048, 74, 222, 36 };
                TBlock tblock3_26 = { 2254, 336, 50, 144 };
                TBlock tblock3_27 = { 2302, 74, 254, 36 };
                TBlock tblock3_28 = { 2302, 484, 254, 36 };

                tBlocks[3].push_back(tblock3_1);
                tBlocks[3].push_back(tblock3_2);
                tBlocks[3].push_back(tblock3_3);
                tBlocks[3].push_back(tblock3_4);
                tBlocks[3].push_back(tblock3_5);
                tBlocks[3].push_back(tblock3_6);
                tBlocks[3].push_back(tblock3_7);
                tBlocks[3].push_back(tblock3_8);
                tBlocks[3].push_back(tblock3_9);
                tBlocks[3].push_back(tblock3_10);
                tBlocks[3].push_back(tblock3_11);
                tBlocks[3].push_back(tblock3_12);
                tBlocks[3].push_back(tblock3_13);
                tBlocks[3].push_back(tblock3_14);
                tBlocks[3].push_back(tblock3_15);
                tBlocks[3].push_back(tblock3_16);
                tBlocks[3].push_back(tblock3_17);
                tBlocks[3].push_back(tblock3_18);
                tBlocks[3].push_back(tblock3_19);
                tBlocks[3].push_back(tblock3_20);
                tBlocks[3].push_back(tblock3_21);
                tBlocks[3].push_back(tblock3_22);
                tBlocks[3].push_back(tblock3_23);
                tBlocks[3].push_back(tblock3_24);
                tBlocks[3].push_back(tblock3_25);
                tBlocks[3].push_back(tblock3_26);
                tBlocks[3].push_back(tblock3_27);
                tBlocks[3].push_back(tblock3_28);

                Hole hole3_1 = { 208, 442, 32, 32 };
                Hole hole3_2 = { 416, 480, 48, 32 };
                Hole hole3_3 = { 512, 480, 48, 32 };
                holes[3].push_back(hole3_1);
                holes[3].push_back(hole3_2);
                holes[3].push_back(hole3_3);
            }
            }
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
}

void Image_::NextStage() {
    currentStage++;
    if (currentStage > 4) currentStage = 1;
    stage1 = (currentStage == STAGE1);
    hidden = (currentStage == HIDDEN);
    stage2 = (currentStage == STAGE2);
    stage3 = (currentStage == STAGE3);
    Player.ResetPosition();
    BlockInit();
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
    questionBlockImage[0].Destroy();
    questionBlockImage[1].Destroy();
    for (int i = 0; i < 4; ++i) {
        if (!blocks[i].empty()) {
            blocks[i].clear();
            questionBlocks[i].clear();
            tBlocks[i].clear();
            holes[i].clear();
        }
    }
}

void Player_::DrawHitbox(HDC targetDC) {
    int hitboxX, hitboxY, hitboxWidth, hitboxHeight;
    GetHitbox(hitboxX, hitboxY, hitboxWidth, hitboxHeight); // hitbox_ 업데이트

    int cameraX = GetCameraX(x_, Images.NowStage() == 1 ? Images.mStage1.GetWidth() :
        Images.NowStage() == 2 ? Images.mStageHidden.GetWidth() :
        Images.NowStage() == 3 ? Images.mStage2.GetWidth() :
        Images.NowStage() == 4 ? Images.mStage3.GetWidth() : 0);

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
    int cameraX = GetCameraX(Player.x(), NowStage() == STAGE1 ? mStage1.GetWidth() :
        NowStage() == HIDDEN ? mStageHidden.GetWidth() :
        NowStage() == STAGE2 ? mStage2.GetWidth() :
        NowStage() == STAGE3 ? mStage3.GetWidth() : 0);

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
        if (!qblock.hit) {
            RECT screenHitbox = { qblock.x - cameraX, qblock.y, qblock.x + qblock.width - cameraX, qblock.y + qblock.height };
            Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
        }
    }
    DeleteObject(qblockPen);
    DeleteObject(qblockBrush);

    // 파이프, 계단 블록 히트박스 그리기
    HPEN tblockPen = CreatePen(PS_DASH, 3, RGB(255, 0, 255));
    HBRUSH tblockBrush = (HBRUSH)GetStockObject(NULL_BRUSH); // 내부 채우기 없음
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
    HBRUSH holeBrush = (HBRUSH)GetStockObject(NULL_BRUSH); // 내부 채우기 없음
    SelectObject(targetDC, holePen);
    SelectObject(targetDC, holeBrush);

    for (const auto& hole : holes[currentStage - 1]) {
        RECT screenHitbox = { hole.x - cameraX, hole.y, hole.x + hole.width - cameraX, hole.y + hole.height };
        Rectangle(targetDC, screenHitbox.left, screenHitbox.top, screenHitbox.right, screenHitbox.bottom);
    }
    DeleteObject(holePen);
    DeleteObject(holeBrush);
}

int GetCameraX(int playerX, int stageWidth) {
    int cameraX = playerX - 400; // 플레이어가 화면 중앙에 오도록 설정
    if (cameraX < 0) cameraX = 0; // 스테이지 왼쪽 경계
    if (stageWidth > wRect.right && cameraX > stageWidth - wRect.right) {
        cameraX = stageWidth - wRect.right; // 스테이지 오른쪽 경계
    }
    return cameraX;
}