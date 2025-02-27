#include <Windows.h>
#include <windowsx.h>
#include <gl/GL.h>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>

#include "dbg.h"
#include "Type.h"

std::mutex GraphMtx;
namespace Enola {
	class WindowBase
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
			WindowBase* my = (WindowBase*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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
				// 处理鼠标消息
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

			// 处理键盘消息
			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_CHAR:
			{
				SendTaskToThread([uMsg, wParam, lParam, this]() {KbMsgCallback(wParam, uMsg); });
				return 0;
			}

			// 处理重绘消息
			case WM_PAINT:
			{
				SendTaskToThread([uMsg, wParam, lParam, this]() {RepaintCallback(); });
				//return 0;
				return DefWindowProc(hwnd, uMsg, wParam, lParam);//这个不知道为什么不发送到window
			}

			// 处理窗口调整大小消息
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

			while (true)
			{
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
					if (task) task();//窗口线程内处理任务
				}
				std::this_thread::sleep_for(std::chrono::microseconds(100));
			}
		}
	protected:
		std::function<void(int, int)> ResizeCallback = [](int, int) {};
		std::function<void()> RepaintCallback = []() {};
		std::function<void(int, int, int)> MouseMsgCallback = [](int, int, int) {};
		std::function<void(int, int)> KbMsgCallback = [](int, int) {};

	public:
		void Create(const char* windowName, int width, int height)
		{
			windowClass.lpfnWndProc = WindowProc;
			windowClass.hInstance = GetModuleHandle(NULL);
			windowClass.lpszClassName = windowName;
			this->width = width;
			this->height = height;
			windowThread = std::thread(&WindowBase::WindowThread, this);
		}
		void Close()
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hglrc);
			ReleaseDC(hwnd, hdc);
			DestroyWindow(hwnd);
		}
		void SendTaskToThread(std::function<void(void)> task) { taskQueue.push(task); }

		void SetResizeCallbackFunc(std::function<void(int, int)> callback) { ResizeCallback = callback; }
		void SetRepaintCallbackFunc(std::function<void()> callback) { RepaintCallback = callback; }
		void SetMouseMsgCallbackFunc(std::function<void(int, int, int)> callback) { MouseMsgCallback = callback; }
		void SetKbMsgCallbackFunc(std::function<void(int, int)> callback) { KbMsgCallback = callback; }

		HWND	GetHWND() { return hwnd; }
		HDC		GetHDC() { return hdc; }
		HGLRC	GetHGLRC() { return hglrc; }

		void MakeContextCorrect() { wglMakeCurrent(hdc, hglrc); }
		void Refresh() { SwapBuffers(hdc); }
	};
	class Graphics
	{
	private:
		Rectangle drawBounds;
		Rectangle windowSize;
	public:
		void BeginPaint()
		{
			GraphMtx.lock();
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glPushMatrix();
			// 计算视口在OpenGL坐标系中的位置
			GLint glViewportY = windowSize.h - drawBounds.y - drawBounds.h;
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			// 使用绘图区域尺寸作为正交投影范围
			glOrtho(0, drawBounds.w, drawBounds.h, 0, -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			// 设置视口和剪裁区域
			glScissor(
				drawBounds.x,
				glViewportY,  // 转换后的Y坐标
				drawBounds.w,
				drawBounds.h
			);
			glEnable(GL_SCISSOR_TEST);  // 启用剪裁测试
			glViewport(
				drawBounds.x,
				glViewportY,  // 转换后的Y坐标
				drawBounds.w,
				drawBounds.h
			);
		}
		void EndPaint()
		{
			glPopAttrib();
			glPopMatrix();
			GraphMtx.unlock();
		}
		void SetBounds(Rectangle bounds) { drawBounds = bounds; }//相对于窗口的bounds
		Rectangle GetBounds() { return drawBounds; }
		void SetWindowSize(Rectangle windowSize) { this->windowSize = windowSize; }//component在resize callback时更新一下
	};
	class Component
	{
	private:
		std::vector<std::unique_ptr<Component>> childs;
		Graphics graph;

		Rectangle parentBounds;//这些都是相对于窗口的
	protected:
		void AddAndMakeVisible(Component& child)
		{
			childs.emplace_back(&child);
		}
		void ResizeCbProc(Rectangle parentBounds, Rectangle windowBounds)
		{//在里面实现子控件递归调用，并触发每个控件的Resize()
			this->parentBounds = parentBounds;
			graph.SetWindowSize(windowBounds);
			Resize(parentBounds, windowBounds);
			Rectangle myBounds = graph.GetBounds();
			for (auto& child : childs) child->ResizeCbProc(myBounds, windowBounds);
		}
		void Repaint()
		{
			graph.BeginPaint();
			Paint(graph);
			graph.EndPaint();
			for (auto& child : childs)child->Repaint();//递归绘制
		}
		virtual void RepaintCbProc() { Repaint(); };//使用时应该重写这个，加上SwapBuffer
		int MouseMsgCbProc(int x, int y, int msg)
		{
			Rectangle myBounds = graph.GetBounds();
			if (x >= myBounds.x && y >= myBounds.y && x <= myBounds.x + myBounds.w && y <= myBounds.h)//如果在范围内
			{
				int flag = 0;
				for (auto& child : childs)
				{
					if (child->MouseMsgCbProc(x, y, msg))flag = 1;
				}
				if (flag == 0)//如果在我范围，且没有儿子要
				{
					MouseMsg(x, y, msg);//那我要了
				}
				return 1;
			}
			return 0;
		}
		void KbMsgCbProc(int key, int state)//暂时没想好怎么处理，群发吧。
		{
			KbMsg(key, state);
			for (auto& child : childs)child->KbMsgCbProc(key, state);
		}

	public:
		virtual void Resize(Rectangle parentBounds, Rectangle windowBounds) {};//Resize触发
		virtual void Paint(Graphics& g) {};//绘制触发
		virtual void MouseMsg(int x, int y, int msg) {};//鼠标事件触发
		virtual void KbMsg(int key, int msg) {};

		//void Repaint();//手动触发重绘
		void SetBounds(Rectangle bounds) { graph.SetBounds({ bounds.x + parentBounds.x,bounds.y + parentBounds.y,bounds.w,bounds.h }); }//相对于父控件的bounds
		Rectangle GetBounds()
		{
			Rectangle myBounds = graph.GetBounds();
			return { myBounds.x - parentBounds.x,myBounds.y - parentBounds.y,myBounds.w,myBounds.h };
		}
	};
	class Window :public Component
	{
	private:
		WindowBase window;
		std::mutex repaintMtx;
	protected:
		void RepaintCbProc() override
		{
			repaintMtx.lock();//防止多线程冲突（一般不会）
			window.MakeContextCorrect();
			Component::RepaintCbProc();
			window.Refresh();
			repaintMtx.unlock();
		}
	public:
		void Create(const char* windowName, int width, int height)
		{
			window.SetResizeCallbackFunc([this](int w, int h) {SetBounds({ 0,0,w,h }); ResizeCbProc({ 0,0,w,h }, { 0,0,w,h }); });
			window.SetRepaintCallbackFunc([this]() {Window::RepaintCbProc(); });
			window.SetMouseMsgCallbackFunc([this](int x, int y, int msg) {MouseMsgCbProc(x, y, msg); });
			window.SetKbMsgCallbackFunc([this](int key, int state) {KbMsgCbProc(key, state); });
			window.Create(windowName, width, height);
		}
		void Close()
		{
			window.Close();
		}
	};
}

class Comp1 :public Enola::Component
{
private:
	int isClick = 0;
public:
	void Paint(Enola::Graphics& g)override
	{
		Enola::Rectangle bounds = GetBounds();
		int w = bounds.w, h = bounds.h;
		if (isClick)glColor3ui(0x00, 0xff, 0x00);
		else glColor3ui(0xff, 0xff, 0xff);
		glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(w, 0);
		glVertex2i(w, h);
		glVertex2i(0, h);
		glEnd();
	}
	void Resize(Enola::Rectangle parentBounds, Enola::Rectangle windowBounds)override
	{
		SetBounds({ 64,64,128,128 });
	}
	void MouseMsg(int x, int y, int msg)override
	{
		if (msg == WM_LBUTTONDOWN)isClick = 1;
		else if (msg == WM_LBUTTONUP)isClick = 0;
	}
};

class App1 :public Enola::Window
{
private:
	Comp1 testComp;
	int isClick;
public:
	App1()
	{
		AddAndMakeVisible(testComp);
	}
	void Paint(Enola::Graphics& g)override
	{
		//DBG("Repaint!\n");
		Enola::Rectangle bounds = GetBounds();
		int w = bounds.w, h = bounds.h;
		if (isClick)glColor3ui(0x00, 0xff, 0xff);
		else glColor3ui(0x00, 0x00, 0xff);
		glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(w, 0);
		glVertex2i(w, h);
		glVertex2i(0, h);
		glEnd();
	}
	void MouseMsg(int x, int y, int msg)override
	{
		if (msg == WM_LBUTTONDOWN)isClick = 1;
		else if (msg == WM_LBUTTONUP)isClick = 0;
	}
};

App1 app;
int main()
{
	app.Create("hello enola!", 640, 480);
	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
}
