#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <list>

#include "Graphics.h"
#include "dbg.h"
#include "Type.h"
#include "MouseMsg.h"

namespace Enola {
	class Component
	{
	private:
		std::function<void()> refreshMethod = []() {};
		std::list<Component*> childs;
		Component* parent = nullptr;
		Graphics graph;

		Rectangle parentBounds;//这些都是相对于窗口的

		MouseMsg mouseMsg;
	protected:
		void AddAndMakeVisible(Component& child)
		{
			child.refreshMethod = refreshMethod;
			child.parent = this;
			childs.push_back(&child);
		}
		void ToTop()
		{
			if (parent == nullptr)return;
			// 获取当前组件在父容器链表中的迭代器
			auto& siblings = parent->childs;
			auto it = std::find(siblings.begin(), siblings.end(), this);
			if (it != siblings.end()) {
				// 将该节点移动到链表末尾（最后绘制）
				siblings.splice(siblings.end(), siblings, it);
			}
		}
		virtual void ResizeCbProc(Rectangle parentBounds, Rectangle windowBounds)
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

		virtual int MouseMsgCbProc(int x, int y, int msg, int hasTopTake)//通过这个函数递归所有子控件并调用子控件的这个函数
		{
			int flag = 0, parentFlag = 0;
			/*for (auto& child : childs)
			{
				if (child->MouseMsgCbProc(x, y, msg))flag = 1;
			}*/
			size_t count = childs.size();
			for (size_t i = 0; i < count; ++i) {
				auto* child = *std::next(childs.rbegin(), i);
				if (child->MouseMsgCbProc(x, y, msg, flag)) flag = 1;
			}
			if (parent != nullptr) parentFlag = parent->isFocus;
			return MouseMsgProc(x, y, msg, flag || hasTopTake, parentFlag);
		}
		int isFocus = 0;
		int MouseMsgProc(int x, int y, int msg, int hasChildTake, int hasParentTake)
		{
			Rectangle myBounds = graph.GetBounds();//全局bounds
			int mx = x - myBounds.x, my = y - myBounds.y;
			mouseMsg.SetCompXY(myBounds.x, myBounds.y);
			mouseMsg.SetXY(x, y);
			mouseMsg.SetMsg(msg);
			MouseMsgSync(mouseMsg);//广播鼠标事件，无论是否在控件内
			if ((myBounds.IsIn(x, y) && !hasChildTake && !hasParentTake) || isFocus)
			{
				if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN)isFocus = 1;
				else if (msg == WM_LBUTTONUP || msg == WM_RBUTTONUP)isFocus = 0;
				MouseMsgCb(mouseMsg);//这个在以后被废弃
				switch (msg)
				{
				case WM_LBUTTONDOWN:	LButtonDown(mouseMsg);	break;
				case WM_LBUTTONUP:		LButtonUp(mouseMsg);		break;
				case WM_RBUTTONDOWN:	RButtonDown(mouseMsg);	break;
				case WM_RBUTTONUP:		RButtonUp(mouseMsg);		break;
				case WM_MOUSEMOVE:		MouseMove(mouseMsg);		break;
				}
				return 1;
			}
			return 0;
		}
		void ClearAllFocus()
		{
			isFocus = 0;
			for (auto& child : childs)child->ClearAllFocus();
		}

		virtual void KbMsgCbProc(int key, int state)//(或者根据鼠标事件让控件自己决定焦点)
		{
			KbMsg(key, state);//群发
			for (auto& child : childs)child->KbMsgCbProc(key, state);
		}

	public:
		virtual void Resize(Rectangle parentBounds, Rectangle windowBounds) {};//Resize触发
		virtual void Paint(Graphics& g) {};//绘制触发
		virtual void MouseMsgSync(MouseMsg& m) {};//全局鼠标事件触发
		virtual void MouseMsgCb(MouseMsg& m) {};//鼠标事件触发
		virtual void KbMsg(int key, int msg) {};

		virtual void LButtonDown(MouseMsg& m) {};//(实际上应该用这个)
		virtual void LButtonUp(MouseMsg& m) {};
		virtual void RButtonDown(MouseMsg& m) {};
		virtual void RButtonUp(MouseMsg& m) {};
		virtual void MouseMove(MouseMsg& m) {};

		//void Repaint();//手动触发重绘
		void SetBounds(Rectangle bounds)
		{
			graph.SetBounds({ bounds.x + parentBounds.x,bounds.y + parentBounds.y,bounds.w,bounds.h });
			//bounds = graph.GetBounds();
			//mouseMsg.SetCompXY(bounds.x, bounds.y);
		}//相对于父控件的bounds
		Rectangle GetBounds()//x,y是相对于父控件的
		{
			Rectangle myBounds = graph.GetBounds();
			return { myBounds.x - parentBounds.x,myBounds.y - parentBounds.y,myBounds.w,myBounds.h };
		}
		void SetGlobalBounds(Rectangle bounds)
		{
			graph.SetBounds(bounds);
			//mouseMsg.SetCompXY(bounds.x, bounds.y);
		}//全局bounds
		Rectangle GetGlobalBounds()
		{
			return graph.GetBounds();
		}//全局bounds
		Rectangle ToGlobal(Rectangle bounds)
		{
			return { bounds.x + parentBounds.x,bounds.y + parentBounds.y,bounds.w,bounds.h };
		}//相对于父控件的bounds转全局
	};
}