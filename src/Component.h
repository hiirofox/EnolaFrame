#pragma once

#include <functional>
#include <thread>
#include <mutex>

#include "Graphics.h"
#include "dbg.h"
#include "Type.h"

namespace Enola {
	class Component
	{
	private:
		std::function<void()> refreshMethod = []() {};
		std::vector<std::unique_ptr<Component>> childs;
		Graphics graph;

		Rectangle parentBounds;//这些都是相对于窗口的
	protected:
		void AddAndMakeVisible(Component& child)
		{
			child.refreshMethod = refreshMethod;
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
		void SetRefreshMethod(std::function<void()> refreshMethod) { this->refreshMethod = refreshMethod; };
		void Repaint()
		{
			//还得考虑用户在哪里调用这个方法的。我仅限于在类里面调用，这样就可以确保只在窗口线程内调用。
			//加锁
			RepaintProc();
			refreshMethod();
			//解锁
		}
		void RepaintProc()
		{
			graph.BeginPaint();
			Paint(graph);
			graph.EndPaint();
			for (auto& child : childs)child->RepaintProc();//递归绘制
		}
		virtual void RepaintCbProc() { RepaintProc(); };//使用时应该重写这个，加上SwapBuffer
		int MouseMsgCbProc(int x, int y, int msg)
		{
			Rectangle myBounds = graph.GetBounds();
			if (x >= myBounds.x && y >= myBounds.y && x <= myBounds.x + myBounds.w && y <= myBounds.y + myBounds.h)//如果在范围内
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
}