#ifndef PLANE_H
#define PLANE_H

#include <GL/glut.h>
#include <cstdlib>
#include <ctime>

class Plane {
public:
	double leftupx;
	double leftupy;
	double width;
	int life;
	bool exist;
	bool b_exist;
	double bulletx, bullety;
	Plane(double lux, double luy, double wid, int li)
		:leftupx(lux), leftupy(luy), width(wid), life(li),
		exist(true), b_exist(false) {
		srand(time(NULL));
	}
	~Plane() {}
	void move();    //Enermy
	double Move(unsigned char c);    //Myself
	void b_move();
	void b_Move();
	void attack(GLint tex_Bullet);
	void draw(GLint tex_enermy, GLint tex_bullet);
	void Draw(GLint tex_plane);
};

#endif // !PLANE_H

