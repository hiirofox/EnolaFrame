#pragma once

#include "WindowBase.h"
#include "Component.h"

namespace Enola {
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
		Window()
		{
			window.SetResizeCallbackFunc([this](int w, int h) {SetBounds({ 0,0,w,h }); ResizeCbProc({ 0,0,w,h }, { 0,0,w,h }); });
			window.SetRepaintCallbackFunc([this]() {Window::RepaintCbProc(); });
			window.SetMouseMsgCallbackFunc([this](int x, int y, int msg) { MouseMsgCbProc(x, y, msg, 0); });
			window.SetKbMsgCallbackFunc([this](int key, int state) {KbMsgCbProc(key, state); });
			SetRefreshMethod([this]() {Window::RepaintCbProc(); /*window.Refresh();*/ });//按理来说刷新局部即可，但是swapbuffer会渲染之前未刷新的缓冲
		}
		void Create(const char* windowName, int width, int height)
		{
			window.Create(windowName, width, height);
		}
		void Close()
		{
			window.Close();
		}
	};
}