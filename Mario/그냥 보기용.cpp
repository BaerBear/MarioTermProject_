
void Player_::Move() {
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
    if (State() == PAIRI) tempHitboxX += 14;
    else if (State() == LIZAMONG) tempHitboxX += 26;

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

        int playerLeft = playerHitboxX;
        int playerRight = playerHitboxX + playerHitboxWidth;
        int playerTop = playerHitboxY;
        int playerBottom = playerHitboxY + playerHitboxHeight;

        bool overlapX = playerRight > blockLeft && playerLeft < blockRight;
        bool overlapY = playerBottom > blockTop && playerTop < blockBottom;

        if (overlapX && overlapY) {
            int prevPlayerBottom = prevY + playerHitboxHeight;
            int prevPlayerTop = prevY;

            if (prevPlayerBottom <= blockTop && jumpVelocity_ > 0) {
                y_ = blockTop - playerHitboxHeight;
                isJumping_ = false;
                jumpVelocity_ = 0.0f;
                onBlock = true;
                newGroundY = blockTop - playerHitboxHeight;
            }
            else if (prevPlayerTop >= blockBottom && jumpVelocity_ < 0 && (State() == TINO || State() == LIZAD)) {
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

            int playerLeft = playerHitboxX;
            int playerRight = playerHitboxX + playerHitboxWidth;
            int playerTop = playerHitboxY;
            int playerBottom = playerHitboxY + playerHitboxHeight;

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
                    if (!qblock.hit && State() == TINO) {
                        qblock.hit = true;
                        eatMushroom_ = true; // Change to Lizard
                    }
                    if (State() == TINO || State() == LIZAD) {
                        y_ = qblockBottom;
                        jumpVelocity_ = 0.0f;
                    }
                }
            }
        }
    }

    // Check if player is still on a block after moving
    if (groundY_ != defaultGroundY_) {
        bool stillOnBlock = false;

        for (const auto& block : Pimage.blocks) {
            int blockLeft = block.x;
            int blockRight = block.x + block.width;
            int blockTop = block.y;

            int playerLeft = playerHitboxX;
            int playerRight = playerHitboxX + playerHitboxWidth;
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

                int playerLeft = playerHitboxX;
                int playerRight = playerHitboxX + playerHitboxWidth;
                bool overlapX = playerRight > qblockLeft && playerLeft < qblockRight;

                if (overlapX && groundY_ == qblockTop - playerHitboxHeight) {
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

    if (!onBlock && y_ >= defaultGroundY_) {
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

class Player_ {
public:
    Image_ Pimage;
    int imageNum;

    void PlayerInit();
    void ResetPosition();
    void DrawPlayer(HDC targetDC);
    void Move();
    bool Moving() { return move_; };

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

    void GetHitbox(int& hitboxX, int& hitboxY, int& hitboxWidth, int& hitboxHeight) {
        hitboxX = x_;
        hitboxY = y_;
        hitboxWidth = 0;
        hitboxHeight = 0;

        switch (State()) {
        case TINO:
            hitboxX += 14;
            hitboxWidth = 23;
            hitboxHeight = 39;
            break;
        case PAIRI:
            hitboxX += 14;
            hitboxWidth = 23;
            hitboxHeight = 39;
            break;
        case LIZAD:
            hitboxX += 18;
            hitboxWidth = 44;
            hitboxHeight = 45;
            break;
        case LIZAMONG:
            hitboxX += 26;
            hitboxWidth = 40;
            hitboxHeight = 50;
            break;
        default:
            hitboxWidth = 36;
            hitboxHeight = 39;
        }
    }
};