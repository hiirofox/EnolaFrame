#include <Windows.h>
#include <windowsx.h>
#include <gl/GL.h>
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <string>
#include "waveout.h"
#include "dbg.h"

#include "untitled_0049.h"

#define M_PI 3.1415926535897932384626
class Window
{
private:
	WNDCLASS windowClass;
	HWND hwnd;
	HDC hdc;
	PIXELFORMATDESCRIPTOR pfd;
	HGLRC hglrc;
	std::thread windowThread;
	std::queue<std::function<void(void)>>taskQueue;
	int width, height;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Window* my = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (my)
		{
			return my->MessageProc(hwnd, uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}
	LRESULT MessageProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
			// ���������Ϣ
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		{
			SendTaskToThread([uMsg, wParam, lParam, this]() {MouseMsgCallback(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), uMsg); });
			return 0;
		}

		// ���������Ϣ
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		{
			SendTaskToThread([uMsg, wParam, lParam, this]() {KbMsgCallback(wParam, uMsg); });
			return 0;
		}

		// �����ػ���Ϣ
		case WM_PAINT:
		{
			SendTaskToThread([uMsg, wParam, lParam, this]() {RepaintCallback(); });
			//return 0;
			return DefWindowProc(hwnd, uMsg, wParam, lParam);//�����֪��Ϊʲô�����͵�window
		}

		// �����ڵ�����С��Ϣ
		case WM_SIZE:
		{
			SendTaskToThread([uMsg, wParam, lParam, this]() {ResizeCallback(LOWORD(lParam), HIWORD(lParam)); });
			return 0;
		}
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	void WindowThread()
	{
		RegisterClass(&windowClass);

		hwnd = CreateWindowEx(0, windowClass.lpszClassName, windowClass.lpszClassName,
			WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			width, height, NULL, NULL, windowClass.hInstance, NULL);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

		hdc = GetDC(hwnd);

		pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
			32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 8, 0, 0, 0, 0 };
		int pf = ChoosePixelFormat(hdc, &pfd);
		SetPixelFormat(hdc, pf, &pfd);

		hglrc = wglCreateContext(hdc);
		wglMakeCurrent(hdc, hglrc);

		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);

		Resize(width, height);
		while (true)
		{
			Paint(hdc);//component�Ķ������Ȳ���
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					Close();
					return;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			while (taskQueue.size())
			{
				auto task = taskQueue.front();
				taskQueue.pop();
				task();//�����߳��ڴ�������
			}
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}
	}
protected:
	virtual void ResizeCallback(int width, int height) {}
	virtual void RepaintCallback() {}
	virtual void MouseMsgCallback(int x, int y, int msg) {};
	virtual void KbMsgCallback(int key, int state) {};

public:
	void Create(const char* windowName, int width, int height)
	{
		windowClass.lpfnWndProc = WindowProc;
		windowClass.hInstance = GetModuleHandle(NULL);
		windowClass.lpszClassName = windowName;
		this->width = width;
		this->height = height;
		windowThread = std::thread(&Window::WindowThread, this);
	}
	void Close()
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hglrc);
		ReleaseDC(hwnd, hdc);
		DestroyWindow(hwnd);
	}
	void SendTaskToThread(std::function<void(void)> task)
	{
		taskQueue.push(task);
	}
	virtual void Resize(int w, int h)
	{

	}
	virtual void Paint(HDC& hdc)
	{

	}
};

GLfloat vertices[][3] = {
	{-1.0f, -1.0f, 1.0f},  // ǰ����
	{1.0f, -1.0f, 1.0f},   // ǰ����
	{1.0f, 1.0f, 1.0f},    // ǰ����
	{-1.0f, 1.0f, 1.0f},   // ǰ����
	{-1.0f, -1.0f, -1.0f}, // ������
	{1.0f, -1.0f, -1.0f},  // ������
	{1.0f, 1.0f, -1.0f},   // ������
	{-1.0f, 1.0f, -1.0f}   // ������
};
GLint faces[][4] = {
	{0, 1, 2, 3}, // ǰ��
	{1, 5, 6, 2}, // ����
	{5, 4, 7, 6}, // ����
	{4, 0, 3, 7}, // ����
	{3, 2, 6, 7}, // ����
	{4, 5, 1, 0}  // ����
};
GLfloat colors[][4] = {
	{1.0f, 0.0f, 0.0f, 1.0f}, // ��
	{0.0f, 1.0f, 0.0f, 1.0f}, // ��
	{0.0f, 0.0f, 1.0f, 1.0f}, // ��
	{1.0f, 1.0f, 0.0f, 1.0f}, // ��
	{0.0f, 1.0f, 1.0f, 1.0f}, // ��
	{1.0f, 0.0f, 1.0f, 1.0f}  // ��
};
class AppCube :public Window
{
private:
	void drawCube() {
		for (int i = 0; i < 6; i++) {
			glBegin(GL_QUADS);
			glColor4fv(colors[i]);
			for (int j = 0; j < 4; j++) {
				glVertex3fv(vertices[faces[i][j]]);
			}
			glEnd();
		}
	}
	float time = 0.0;
protected:
	void MouseMsgCallback(int x, int y, int msg) { DBG("Cube:  %d %d %d\n", x, y, msg); }
public:
	void Resize(int w, int h) override
	{
		glEnable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport(0, 0, w, h);
		float aspect = (float)w / h;
		float fov = 45.0f;
		float nearv = 0.1f;
		float farv = 100.0f;
		float top = nearv * tanf(fov * 3.14159265f / 360.0f);
		float right = top * aspect;
		glFrustum(-right, right, -top, top, nearv, farv);
	}
	void Paint(HDC& hdc) override
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ������ͼ����
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0.0f, 0.0f, -6.0f);  // ���λ��

		glRotatef(time += 0.5, 0.5f, 1.0f, 0.0f);

		drawCube();
		SwapBuffers(hdc);
	}
};

class Torus {
private:
	const float R = 1.0f;  // ���뾶
	const float r = 0.3f;  // �ܰ뾶
	const int N = 50;      // ��Բ�ֶ���
	const int n = 30;      // ��Բ�ֶ���

public:
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> texCoords;  // ������������
	std::vector<GLuint> indices;

	Torus() {
		generateVertices();
		generateIndices();
	}

private:
	void generateVertices() {
		for (int i = 0; i <= N; i++) {
			float theta = 2 * M_PI * i / N;
			for (int j = 0; j <= n; j++) {
				float phi = 2 * M_PI * j / n;

				// ��������
				float x = (R + r * cos(phi)) * cos(theta);
				float y = (R + r * cos(phi)) * sin(theta);
				float z = r * sin(phi);
				vertices.insert(vertices.end(), { x, y, z });

				// �������꣨����Բ����
				texCoords.push_back((float)i / N);  // U�����0��1����
			}
		}
	}

	void generateIndices() {
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < n; j++) {
				int first = i * (n + 1) + j;
				int second = first + n + 1;

				indices.push_back(first);
				indices.push_back(second);
				indices.push_back(second + 1);

				indices.push_back(first);
				indices.push_back(second + 1);
				indices.push_back(first + 1);
			}
		}
	}
};

class AppTorus : public Window {
private:
	Torus torus;
	float rotationAngle = 0.0f;
	GLuint textureID;  // ����ID

	// ���ɲʺ�����
	void createRainbowTexture() {
		const int TEX_SIZE = 256;
		GLubyte texData[TEX_SIZE][3];

		// ���ɲʺ�ɫ����
		for (int i = 0; i < TEX_SIZE; ++i) {
			float t = (float)i / TEX_SIZE;
			texData[i][0] = 255 * (0.5f * sin(2 * M_PI * t) + 0.5f);  // R
			texData[i][1] = 255 * (0.5f * sin(2 * M_PI * t + 2 * M_PI / 3) + 0.5f);  // G
			texData[i][2] = 255 * (0.5f * sin(2 * M_PI * t + 4 * M_PI / 3) + 0.5f);  // B
		}

		// ��������
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_1D, textureID);

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, TEX_SIZE, 0,
			GL_RGB, GL_UNSIGNED_BYTE, texData);
	}
protected:
	void MouseMsgCallback(int x, int y, int msg) { DBG("Torus: %d %d %d\n", x, y, msg); }
public:
	void Resize(int w, int h) override {
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_1D);

		// ��ʼ��������һ�Σ�
		static bool texInitialized = false;
		if (!texInitialized) {
			createRainbowTexture();
			texInitialized = true;
		}

		// ����ͶӰ
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport(0, 0, w, h);
		float aspect = (float)w / h;
		float fov = 45.0f;
		float nearv = 0.1f;
		float farv = 100.0f;
		float top = nearv * tanf(fov * 3.14159265f / 360.0f);
		float right = top * aspect;
		glFrustum(-right, right, -top, top, nearv, farv);
	}

	void Paint(HDC& hdc) override {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ������ͼ����
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0.0f, 0.0f, -6.0f);  // ���λ��

		// ������ת
		rotationAngle += 1.0f;
		if (rotationAngle >= 360) rotationAngle -= 360;

		// ��������
		glEnable(GL_TEXTURE_1D);
		glBindTexture(GL_TEXTURE_1D, textureID);

		// ���ò�������
		glColor3f(1, 1, 1);  // ʹ�ô�����ɫ��������
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		// Ӧ�ñ任
		glPushMatrix();
		glRotatef(rotationAngle, 0.5f, 1.0f, 0.0f);
		drawTexturedTorus();
		glPopMatrix();

		glDisable(GL_TEXTURE_1D);
		SwapBuffers(hdc);
	}

private:
	void drawTexturedTorus() {
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		// ���ö������������
		glVertexPointer(3, GL_FLOAT, 0, torus.vertices.data());
		glTexCoordPointer(1, GL_FLOAT, 0, torus.texCoords.data());

		// ����
		glDrawElements(GL_TRIANGLES, torus.indices.size(),
			GL_UNSIGNED_INT, torus.indices.data());

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
};

#define MaxPolyN 32
class PolyManager
{
private:
	int32_t notebuf[MaxPolyN];
	int32_t vbuf[MaxPolyN];
	float freqbuf[MaxPolyN];
	float velobuf[MaxPolyN];
	int statebuf[MaxPolyN];
	int tickbuf[MaxPolyN];
	int pos = 0;
public:
	void NoteOn(int note, int velo, int tick)
	{
		notebuf[pos] = note;
		vbuf[pos] = velo;
		freqbuf[pos] = 440.0 * powf(2.0, (float)(note - 60 + 3) / 12.0);
		velobuf[pos] = (float)velo / 128.0;
		statebuf[pos] = 1;
		pos++;
		if (pos >= MaxPolyN)pos = 0;
	}
	void NoteOff(int note, int velo)
	{
		int maxTick = -999, maxi = -1;
		for (int i = 0; i < MaxPolyN; ++i)
		{
			if (notebuf[i] == note)
			{
				statebuf[i] = 0;
			}
		}
	}
	float GetFreq(int poly)
	{
		return freqbuf[poly];
	}
	float GetVelocity(int poly)
	{
		return velobuf[poly];
	}
	float GetState(int poly)
	{
		return statebuf[poly];
	}
};

AppCube app1;
AppTorus app2;
WaveOut wo;
#define numSamples 512
float wavbufl[numSamples];
float wavbufr[numSamples];
PolyManager poly;
float oscts[MaxPolyN];
int main()
{
	app1.Create("cube", 640, 480);
	app2.Create("torus", 640, 480);
	wo.Init();
	wo.Start();
	int sampleRate = wo.GetSampleRate();

	int tickSamples = 200;
	int numInfo = sizeof(ticks) / sizeof(uint32_t);
	int samplesCount = 0;
	int tickCount = 15634;
	int readPos = 0;
	for (;;)
	{
		for (int i = 0; i < numSamples; ++i)
		{
			samplesCount++;
			if (samplesCount >= tickSamples)
			{
				samplesCount -= tickSamples;
				tickCount++;
				if (tickCount >= ticks[numInfo])
				{
					//tickCount = 0;
					//readPos = 0;
				}
				while (ticks[readPos] <= tickCount)
				{
					//todo:updata midi
					if (cmds[readPos] == 0x90) poly.NoteOn(notes[readPos], velos[readPos], tickCount);
					if (cmds[readPos] == 0x80) poly.NoteOff(notes[readPos], velos[readPos]);
					readPos++;
				}
			}
			float outl = 0, outr = 0;
			for (int j = 0; j < MaxPolyN; ++j)
			{
				oscts[j] += poly.GetFreq(j) / sampleRate;
				oscts[j] -= 2 * (int)oscts[j];
				outl += oscts[j] * poly.GetVelocity(j) * poly.GetState(j);
				outr += oscts[j] * poly.GetVelocity(j) * poly.GetState(j);
			}
			wavbufl[i] = outl * 0.05;
			wavbufr[i] = outr * 0.05;
		}
		wo.FillBuffer(wavbufl, wavbufr, numSamples);
		DBG("tick:%d\n", tickCount);
		//std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
	wo.Close();
	return 0;
}
