#include "plane.h"
#include <cstdio>

void Plane::move() {
	leftupy -= 0.002;
}

void Plane::draw(GLint tex_enermy, GLint tex_bullet) {
	if (exist) {
		glBindTexture(GL_TEXTURE_2D, tex_enermy);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0), glVertex2f(leftupx, leftupy - width);
		glTexCoord2f(0, 1), glVertex2f(leftupx, leftupy);
		glTexCoord2f(1, 1), glVertex2f(leftupx + width, leftupy);
		glTexCoord2f(1, 0), glVertex2f(leftupx + width, leftupy - width);
		glEnd();
	}
	if (b_exist) {
		glBindTexture(GL_TEXTURE_2D, tex_bullet);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0), glVertex2f(bulletx - 0.05, bullety - 0.1);
		glTexCoord2f(0, 1), glVertex2f(bulletx - 0.05, bullety);
		glTexCoord2f(1, 1), glVertex2f(bulletx + 0.05, bullety);
		glTexCoord2f(1, 0), glVertex2f(bulletx + 0.05, bullety - 0.1);
		glEnd();
	}
}

void Plane::Draw(GLint tex_plane) {
	if (life) {
		glBindTexture(GL_TEXTURE_2D, tex_plane);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.5f);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0), glVertex2f(leftupx, leftupy - width);
		glTexCoord2f(0, 1), glVertex2f(leftupx, leftupy);
		glTexCoord2f(1, 1), glVertex2f(leftupx + width, leftupy);
		glTexCoord2f(1, 0), glVertex2f(leftupx + width, leftupy - width);
		glEnd();
	}
}

double Plane::Move(unsigned char c) {
	double stime = 0;
	switch (c) {
	case 'a':    //left
	case 'A':
		leftupx -= 0.05;
		if (leftupx < -1)
			leftupx = -1;
		break;
	case 'w':
	case 'W':
		leftupy += 0.05;
		if (leftupy > 1)
			leftupy = 1;
		break;
	case 'd':
	case 'D':
		leftupx += 0.05;
		if (leftupx > 1 - width)
			leftupx = 1 - width;
		break;
	case 's':
	case 'S':
		leftupy -= 0.05;
		if (leftupy < -1 + width)
			leftupy = -1 + width;
		break;
	case ' ':
		b_exist = true;
		bulletx = leftupx + 0.125;
		bullety = leftupy;
		break;
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

void Plane::b_move() {
	if (rand() % 80 == 3 && !b_exist && exist) {
		b_exist = true;
		bulletx = leftupx + width / 2;
		bullety = leftupy - width;
	}
	if (b_exist) {
		bullety -= 0.008;
		if (bullety < -1.20) {
			b_exist = false;
			bullety = leftupy - width - 0.02;
		}
	}
}

void Plane::b_Move() {
	if (b_exist) {
		bullety += 0.04;
		if (bullety > 1.00) {
			b_exist = false;
			bullety = leftupy - 0.02;
		}
	}
}

void Plane::attack(GLint tex_Bullet) {
	if (!b_exist)
		return;
	glBindTexture(GL_TEXTURE_2D, tex_Bullet);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0), glVertex2f(bulletx - 0.05, bullety - 0.1);
	glTexCoord2f(0, 1), glVertex2f(bulletx - 0.05, bullety);
	glTexCoord2f(1, 1), glVertex2f(bulletx + 0.05, bullety);
	glTexCoord2f(1, 0), glVertex2f(bulletx + 0.05, bullety - 0.1);
	glEnd();
	b_Move();
}

