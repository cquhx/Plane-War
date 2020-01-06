#ifndef PLANE_H
#define PLANE_H

#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#define safe_delete(p) if(p){delete p;p=nullptr;}
using std::vector;

class Spirit {
protected:
	double left, right, up, down;
	double length, width;
	double midx, midy;
	int life;
	GLint Tex;
public:
	Spirit(double u, double d, double l, double r, int li, GLint tex) :
		up(u), down(d), left(l), right(r), life(li), Tex(tex) {
		width = up - down; length = right - left;
		midx = (left + right) / 2; midy = (up + down) / 2;
	}
	Spirit() {}
	virtual ~Spirit() {}
	virtual bool exist()const = 0;
	virtual void move() = 0;
	void draw()const;
};

class Bullet :public Spirit {
private:
	int dir;
	double speed;
	bool ex;
public:
	Bullet(double u, double d, double l, double r, int di, double s, GLint tex)
		:dir(di), speed(s), ex(true) {
		up = u; down = d; left = l; right = r;Tex = tex; 
	}
	~Bullet() {}
	void move();
	void setexist(const bool b) { ex = b; }
	bool exist()const { 
		return ex&&left >= -1.0&&right <= 1.0&&up <= 1.0&&down >= -1.0;
	}
	inline double l() { return left; }
	inline double r() { return right; }
	inline double u() { return up; }
	inline double d() { return down; }
};

class Plane;
class Enermy:public Spirit {
	friend class Plane;
public:
	Enermy(double u, double d, double l, double r, GLint texself, GLint tex, int li = 1) {
		up = u; down = d; left = l; right = r; Tex = texself; life = li;
		double mid = (l + r) / 2; TexBullet = tex;
		bullet = new Bullet(d, d - 0.1, mid - 0.05, mid + 0.05, 0, 0.006, tex);
		srand(time(NULL));
	}
	~Enermy() { safe_delete(bullet); }
	void draw()const;
	void attack();
	void move();
	bool exist()const { return life != 0 && up > -1.0;}
private:
	Bullet* bullet;
	GLint TexBullet;
};

class Plane:public Spirit {
public:
	Plane(double u, double d, double l, double r, int li, GLint texself, GLint tex) :
		score(0), TexBullet(tex) {
		up = u; down = d; left = l; right = r; width = r - l;
		Tex = texself; life = li; TexBullet = tex;
		double mid = (l + r) / 2;
		bullet = new Bullet(u + 0.004, u, l + mid - 0.002, r - mid + 0.002, 5, 0.04, tex);
		srand(time(NULL));
	}
	~Plane() { safe_delete(bullet); }
	void move() {}
	double move(unsigned char c);   
	bool exist()const { return life != 0; }
	void attack();
	void moveto(double, double);
	const int GetLife()const { return life; }
	const int GetScore()const { return score; }
	void checkattack(const vector<Enermy*>&);
	void checkcollide(const vector<Enermy*>&);
private:
	Bullet* bullet;
	GLint TexBullet;
	int score;
};

#endif // !PLANE_H

