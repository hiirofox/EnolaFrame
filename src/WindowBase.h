#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <gl/GL.h>
#include <queue>
#include <functional>
#include <thread>

#include "dbg.h"

namespace Enola
{
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
	};
}