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