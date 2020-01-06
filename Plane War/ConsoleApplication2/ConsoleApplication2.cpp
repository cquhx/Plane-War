// ConsoleApplication2.cpp : �������̨Ӧ�ó������ڵ㡣
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

	// ���ļ������ʧ�ܣ�����
	FILE* pFile = fopen(file_name, "rb");
	if (pFile == 0)
		return 0;

	// ��ȡ�ļ���ͼ��Ŀ�Ⱥ͸߶�
	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);

	// ����ÿ��������ռ�ֽ����������ݴ����ݼ����������ֽ���
	{
		GLint line_bytes = width * 3;
		while (line_bytes % 4 != 0)
			++line_bytes;
		total_bytes = line_bytes * height;
	}

	// �����������ֽ��������ڴ�
	pixels = (GLubyte*)malloc(total_bytes);
	if (pixels == 0)
	{
		fclose(pFile);
		return 0;
	}

	// ��ȡ��������
	if (fread(pixels, total_bytes, 1, pFile) <= 0) {
		free(pixels);
		fclose(pFile);
		return 0;
	}

	// �ھɰ汾��OpenGL��
	// ���ͼ��Ŀ�Ⱥ͸߶Ȳ��ǵ������η�������Ҫ��������
	// ���ﲢû�м��OpenGL�汾�����ڶ԰汾�����ԵĿ��ǣ����ɰ汾����
	// ���⣬�����Ǿɰ汾�����°汾��
	// ��ͼ��Ŀ�Ⱥ͸߶ȳ�����ǰOpenGLʵ����֧�ֵ����ֵʱ��ҲҪ��������
	{
		GLint max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if (!power_of_two(width)
			|| !power_of_two(height)
			|| width > max
			|| height > max) {
			const GLint new_width = 256;
			const GLint new_height = 256; // �涨���ź��µĴ�СΪ�߳���������
			GLint new_line_bytes, new_total_bytes;
			GLubyte* new_pixels = 0;

			// ����ÿ����Ҫ���ֽ��������ֽ���
			new_line_bytes = new_width * 3;
			while (new_line_bytes % 4 != 0)
				++new_line_bytes;
			new_total_bytes = new_line_bytes * new_height;

			// �����ڴ�
			new_pixels = (GLubyte*)malloc(new_total_bytes);
			if (new_pixels == 0) {
				free(pixels);
				fclose(pFile);
				return 0;
			}

			// ������������
			gluScaleImage(GL_RGB,
				width, height, GL_UNSIGNED_BYTE, pixels,
				new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

			// �ͷ�ԭ�����������ݣ���pixelsָ���µ��������ݣ�����������width��height
			free(pixels);
			pixels = new_pixels;
			width = new_width;
			height = new_height;
		}
	}

	// ����һ���µ�������
	glGenTextures(1, &texture_ID);
	if (texture_ID == 0) {
		free(pixels);
		fclose(pFile);
		return 0;
	}
	GLint temp = last_texture_ID;
	// ���µ������������������������
	// �ڰ�ǰ���Ȼ��ԭ���󶨵������ţ��Ա��������лָ�
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &temp);
	//ʹ�õ�texture_ID������
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	//�����������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//����һ����ά����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, last_texture_ID);

	// ֮ǰΪpixels������ڴ����ʹ��glTexImage2D�Ժ��ͷ�
	// ��Ϊ��ʱ���������Ѿ���OpenGL���б�����һ�ݣ����ܱ����浽ר�ŵ�ͼ��Ӳ���У�
	free(pixels);
	return texture_ID;
}

void texture_colorkey(GLubyte r, GLubyte g, GLubyte b,
	GLubyte absolute) {
	GLint width, height;
	GLubyte *pixels = nullptr;
	//��ȡ�����С��Ϣ
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
	//����ռ䲢�����������
	pixels = (GLubyte*)malloc(width*height * 4);
	//pixels = new GLubyte[width*height * 4];
	if (pixels == nullptr)
		return;
	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
	//�޸������е�Alphaֵ
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
	//���޸ĺ�������������õ�������
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

	// �����ַ��ĸ���
	// �����˫�ֽ��ַ��ģ����������ַ����������ֽڲ���һ���ַ�
	// ����һ���ֽ���һ���ַ�
	len = 0;
	for (i = 0; str[i] != '\0'; ++i) {
		if (IsDBCSLeadByte(str[i]))
			++i;
		++len;
	}

	// ������ַ�ת��Ϊ���ַ�
	wstring = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, wstring, len);
	wstring[len] = L'\0';

	// �������ַ�
	for (i = 0; i < len; ++i) {
		wglUseFontBitmapsW(hDC, wstring[i], 1, list);
		glCallList(list);
	}

	// ����������ʱ��Դ
	free(wstring);
	glDeleteLists(list, 1);
}

void endgame() {
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.0f, 1.0f, 1.0f);
	glRasterPos2f(-0.80, 0.00);
	selectFont(50, DEFAULT_CHARSET, "����");
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
	selectFont(20, DEFAULT_CHARSET, "���ķ���");
	drawCNString("Your Score:");

	glRasterPos2f(-0.40, 0.90);
	selectFont(20, DEFAULT_CHARSET, "���ķ���");
	char *temp = to_char(plane->GetScore());
	drawCNString(temp);
	delete temp;

	glRasterPos2f(-0.10, 0.90);
	selectFont(20, DEFAULT_CHARSET, "���ķ���");
	drawCNString("Your Life:");

	glRasterPos2f(0.40, 0.90);
	selectFont(20, DEFAULT_CHARSET, "���ķ���");
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
	// GLUT��ʼ��
	glutInit(&argc, argv);
	ShowCursor(false);
	init();
	return 0;
}