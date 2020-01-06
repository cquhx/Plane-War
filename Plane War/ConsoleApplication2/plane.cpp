#include "plane.h"

void Spirit::draw()const {
	if (exist()) {
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.5f);
		glBindTexture(GL_TEXTURE_2D, Tex);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0), glVertex2f(left, down);
		glTexCoord2f(0, 1), glVertex2f(left, up);
		glTexCoord2f(1, 1), glVertex2f(right, up);
		glTexCoord2f(1, 0), glVertex2f(right, down);
		glEnd();
	}
}

void Bullet::move() {
	switch (dir) {
	case 0:
		up -= speed;
		down -= speed;
		break;
	case 1:
		up += speed;
		down += speed;
		break;
	case 2:
		left -= speed;
		right -= speed;
		break;
	case 3:
		left += speed;
		right += speed;
		break;
	default:
		break;
	}
}

void Enermy::draw()const {
	Spirit::draw();
	bullet->draw();
}

void Enermy::move() {
	up -= 0.002; down -= 0.002;
}

void Enermy::attack() {
	bullet->move();
	bullet->draw();
	if (!bullet->exist()&&exist()) {
		int t = rand() % 100;
		if (t < 99)return;
		double mid = (left + right) / 2;
		bullet = new Bullet(down, down - 0.1, mid - 0.05, mid + 0.05, 0, 0.006, TexBullet);
	}
}

double Plane::move(unsigned char c) {
	double stime = 0;
	switch (c) {
	case 'a':    //left
	case 'A':
		left -= 0.05;
		if (left >= -1.0)
			right -= 0.05;
		else {
			left = -1.0;
			right = -0.75;
		}
		break;
	case 'w':
	case 'W':
		up += 0.05;
		if (up <= 1.0)
			down += 0.05;
		else {
			up = 1.0;
			down = 0.75;
		}
		break;
	case 'd':
	case 'D':
		right += 0.05;
		if (right <= 1.0)
			left += 0.05;
		else {
			right = 1.0;
			left = 0.75;
		}
		break;
	case 's':
	case 'S':
		down -= 0.05;
		if (down >= -1.0)
			up -= 0.05;
		else {
			down = -1.0;
			up = -0.75;
		}
		break;
	case ' ': {
		double mid = (right + left) / 2;
		bullet = new Bullet(up + 0.06, up, mid - 0.03,
			mid + 0.03, 1, 0.02, TexBullet);
		attack();
		break;
	}
	case 'r':
	case 'R':
		return -1;
	case '\033':
		exit(0);
	default:
		break;
	}
	return stime;
}

void Plane::attack() {
	bullet->move();
	bullet->draw();
}

void Plane::checkcollide(const vector<Enermy*>& vec) {
	for (auto &i : vec) {
		if (i->bullet->l() > right || i->bullet->r() < left
			|| i->bullet->u() < down || i->bullet->d() > up || !i->bullet->exist());
		else {
			i->bullet->setexist(false);
			--life;
			if (life < 0)life = 0;
		}
		if (!i->exist()) continue;
		if (i->down >= up || i->left >= right ||
			i->right <= left || i->up <= down)
			continue;
		life = 0;
	}
}

void Plane::moveto(double x, double y) {
	double MidWidth = width / 2;
	left = x - MidWidth;
	right = x + MidWidth;
	up = y + MidWidth;
	down = y - MidWidth;
}

void Plane::checkattack(const vector<Enermy*>& vec) {
	double x1 = bullet->l(), x2 = bullet->r();
	double y1 = bullet->d(), y2 = bullet->u();
	if (!bullet->exist())return;
	for (auto& i : vec) {
		if (!i->exist())continue;
		if (y2 <= i->down || y1 >= i->up ||
			x1 >= i->right || x2 <= i->left)
			continue;
		i->life = 0;
		score += 10;
		bullet->setexist(false);
	}
}