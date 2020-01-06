// ConsoleApplication2.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <cmath>
#include <Windows.h>
#include <random>
#include "plane.h"

#define BMP_Header_Length 54
#define Width 400
#define Height 400

using namespace std;

GLint tex_enermy, tex_back, tex_plane, tex_bullet, tex_Bullet;
vector<Enermy*>vec;
Plane* plane;
double stime, etime;
default_random_engine e;

void init();

inline bool power_of_two(int n) {
	if (n <= 0) return false;
	return (n & (n - 1)) == 0;
}

GLuint load_texture(const char* file_name) {
	GLint width, height, total_bytes;
	GLubyte* pixels = 0;
	GLuint last_texture_ID = 0, texture_ID = 0;

	// 打开文件，如果失败，返回
	FILE* pFile = fopen(file_name, "rb");
	if (pFile == 0)
		return 0;

	// 读取文件中图象的宽度和高度
	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);

	// 计算每行像素所占字节数，并根据此数据计算总像素字节数
	{
		GLint line_bytes = width * 3;
		while (line_bytes % 4 != 0)
			++line_bytes;
		total_bytes = line_bytes * height;
	}

	// 根据总像素字节数分配内存
	pixels = (GLubyte*)malloc(total_bytes);
	if (pixels == 0)
	{
		fclose(pFile);
		return 0;
	}

	// 读取像素数据
	if (fread(pixels, total_bytes, 1, pFile) <= 0) {
		free(pixels);
		fclose(pFile);
		return 0;
	}

	// 在旧版本的OpenGL中
	// 如果图象的宽度和高度不是的整数次方，则需要进行缩放
	// 这里并没有检查OpenGL版本，出于对版本兼容性的考虑，按旧版本处理
	// 另外，无论是旧版本还是新版本，
	// 当图象的宽度和高度超过当前OpenGL实现所支持的最大值时，也要进行缩放
	{
		GLint max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if (!power_of_two(width)
			|| !power_of_two(height)
			|| width > max
			|| height > max) {
			const GLint new_width = 256;
			const GLint new_height = 256; // 规定缩放后新的大小为边长的正方形
			GLint new_line_bytes, new_total_bytes;
			GLubyte* new_pixels = 0;

			// 计算每行需要的字节数和总字节数
			new_line_bytes = new_width * 3;
			while (new_line_bytes % 4 != 0)
				++new_line_bytes;
			new_total_bytes = new_line_bytes * new_height;

			// 分配内存
			new_pixels = (GLubyte*)malloc(new_total_bytes);
			if (new_pixels == 0) {
				free(pixels);
				fclose(pFile);
				return 0;
			}

			// 进行像素缩放
			gluScaleImage(GL_RGB,
				width, height, GL_UNSIGNED_BYTE, pixels,
				new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

			// 释放原来的像素数据，把pixels指向新的像素数据，并重新设置width和height
			free(pixels);
			pixels = new_pixels;
			width = new_width;
			height = new_height;
		}
	}

	// 分配一个新的纹理编号
	glGenTextures(1, &texture_ID);
	if (texture_ID == 0) {
		free(pixels);
		fclose(pFile);
		return 0;
	}
	GLint temp = last_texture_ID;
	// 绑定新的纹理，载入纹理并设置纹理参数
	// 在绑定前，先获得原来绑定的纹理编号，以便在最后进行恢复
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &temp);
	//使用第texture_ID幅纹理
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	//设置纹理参数
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//载入一个二维纹理
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, last_texture_ID);

	// 之前为pixels分配的内存可在使用glTexImage2D以后释放
	// 因为此时像素数据已经被OpenGL另行保存了一份（可能被保存到专门的图形硬件中）
	free(pixels);
	return texture_ID;
}

void texture_colorkey(GLubyte r, GLubyte g, GLubyte b,
	GLubyte absolute) {
	GLint width, height;
	GLubyte *pixels = nullptr;
	//获取纹理大小信息
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
	//分配空间并获得纹理像素
	pixels = (GLubyte*)malloc(width*height * 4);
	//pixels = new GLubyte[width*height * 4];
	if (pixels == nullptr)
		return;
	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
	//修改像素中的Alpha值
	{
		GLint i, count = width*height;
		for (i = 0; i < count; ++i) {
			if (abs(pixels[i * 4] - b) <= absolute
				&&abs(pixels[i * 4 + 1] - g) <= absolute
				&&abs(pixels[i * 4 + 2] - r) <= absolute)
				pixels[i * 4 + 3] = 0;
			else
				pixels[i * 4 + 3] = 255;
		}
	}
	//将修改后的像素重新设置到纹理中
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
	free(pixels);
}

//Judge if all the enermies are died
bool checkall() {
	for (auto& i : vec)
		if (i->exist())return false;
	return true;
}
//Creat new enermy
void newenermy() {
	vec.clear();
	uniform_real_distribution<double> u1(0.75, 1.00);
	uniform_real_distribution<double> u2(-1.00, 0.75);
	for (int i = 0; i < 10; ++i) {
		double u = u1(e), l = u2(e);
		Enermy *t = new Enermy(u, u - 0.25, l, l + 0.25, tex_enermy, tex_bullet);
		vec.push_back(t);
	}
}

void selectFont(int size, int charset, const char* face) {
	HFONT hFont = CreateFontA(size, 0, 0, 0, FW_MEDIUM, 0, 0, 0,
		charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);
	HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
	DeleteObject(hOldFont);
}

void drawCNString(const char* str) {
	int len, i;
	wchar_t* wstring;
	HDC hDC = wglGetCurrentDC();
	GLuint list = glGenLists(1);

	// 计算字符的个数
	// 如果是双字节字符的（比如中文字符），两个字节才算一个字符
	// 否则一个字节算一个字符
	len = 0;
	for (i = 0; str[i] != '\0'; ++i) {
		if (IsDBCSLeadByte(str[i]))
			++i;
		++len;
	}

	// 将混合字符转化为宽字符
	wstring = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, wstring, len);
	wstring[len] = L'\0';

	// 逐个输出字符
	for (i = 0; i < len; ++i) {
		wglUseFontBitmapsW(hDC, wstring[i], 1, list);
		glCallList(list);
	}

	// 回收所有临时资源
	free(wstring);
	glDeleteLists(list, 1);
}

void endgame() {
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.0f, 1.0f, 1.0f);
	glRasterPos2f(-0.80, 0.00);
	selectFont(50, DEFAULT_CHARSET, "黑体");
	drawCNString("You have died");
	glEnable(GL_TEXTURE_2D);
}

char* to_char(int t) {
	char *str = new char[12];
	int  i = 0;
	while (t) {
		str[i] = t % 10 + '0';
		++i;
		t /= 10;
	}
	str[i] = '\0';
	int len = i;
	for (i -= 1; i >= len >> 1; --i)
		swap(str[i], str[len - i - 1]);
	str = str;
	return str;
}

void showscore() {
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0f, 0.0f, 0.0f);
	glRasterPos2f(-1.00, 0.90);
	selectFont(20, DEFAULT_CHARSET, "华文仿宋");
	drawCNString("Your Score:");

	glRasterPos2f(-0.40, 0.90);
	selectFont(20, DEFAULT_CHARSET, "华文仿宋");
	char *temp = to_char(plane->GetScore());
	drawCNString(temp);
	delete temp;

	glRasterPos2f(-0.10, 0.90);
	selectFont(20, DEFAULT_CHARSET, "华文仿宋");
	drawCNString("Your Life:");

	glRasterPos2f(0.40, 0.90);
	selectFont(20, DEFAULT_CHARSET, "华文仿宋");
	temp = to_char(plane->GetLife());
	drawCNString(temp);
	delete temp;

	glEnable(GL_TEXTURE_2D);
}

//Judge if hit the enermy
void checkcollide() {
	plane->checkattack(vec);
	if (checkall()) newenermy();
}
//Judge if be hit
void checkcollide1() {
	plane->checkcollide(vec);
}
//attack the enermy
void drawbullet() {
	plane->attack();
	checkcollide();
}

void drawenermy() {
	for (auto &i : vec)i->draw();
}

void display() {	
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_back);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0), glVertex2f(-1, -1);
	glTexCoord2f(0, 1), glVertex2f(-1, 1);
	glTexCoord2f(1, 1), glVertex2f(1, 1);
	glTexCoord2f(1, 0), glVertex2f(1, -1);
	glEnd();

	checkcollide1();

	plane->draw();
	if (plane->exist()) drawbullet();
	else endgame();

	showscore();
	drawenermy();
	glutSwapBuffers();
}
//Control the plane
void keycontrol(unsigned char c, int x, int y) {
	double temp = plane->move(c);
	if (temp > 0) stime = temp;
	else if (temp < 0) init();
}

void move(int x, int y) {
	plane->moveto(double(x) / (glutGet(GLUT_WINDOW_WIDTH) >> 1) - 1.0,
		1.0 - double(y) / (glutGet(GLUT_WINDOW_HEIGHT) >> 1));
}

void attack(int button, int state, int x, int y) {
	if (state == GLUT_LEFT)plane->move(' ');
}

void idle() {
	display();
	for (auto& i : vec) {
		i->move();
		i->attack();
	}
}

void init() {
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(Width, Height);
	glutCreateWindow("Plane War");
	tex_back = load_texture("Back.bmp");
	tex_enermy = load_texture("Yellow.bmp");
	tex_plane = load_texture("Black.bmp");
	tex_bullet = load_texture("bullet.bmp");
	tex_Bullet = load_texture("Bomb.bmp");
    glBindTexture(GL_TEXTURE_2D, tex_plane);
	texture_colorkey(255, 255, 255, 10);
	glBindTexture(GL_TEXTURE_2D, tex_enermy);
	texture_colorkey(255, 255, 255, 10);
	glBindTexture(GL_TEXTURE_2D, tex_bullet);
	texture_colorkey(255, 255, 255, 10);
	glBindTexture(GL_TEXTURE_2D, tex_Bullet);
	texture_colorkey(255, 255, 255, 10);
    glEnable(GL_TEXTURE_2D);
	newenermy();
	plane = new Plane(-0.50, -0.75, -0.125, 0.125, 10, tex_plane, tex_Bullet);
	glutDisplayFunc(&display);
	glutKeyboardFunc(&keycontrol);
	glutPassiveMotionFunc(&move);
	glutMouseFunc(&attack);
	glutIdleFunc(&idle);
	glutMainLoop();
}

int main(int argc, char* argv[]) {
	// GLUT初始化
	glutInit(&argc, argv);
	ShowCursor(false);
	init();
	return 0;
}