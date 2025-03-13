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

		Rectangle parentBounds;//��Щ��������ڴ��ڵ�

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
			// ��ȡ��ǰ����ڸ����������еĵ�����
			auto& siblings = parent->childs;
			auto it = std::find(siblings.begin(), siblings.end(), this);
			if (it != siblings.end()) {
				// ���ýڵ��ƶ�������ĩβ�������ƣ�
				siblings.splice(siblings.end(), siblings, it);
			}
		}
		virtual void ResizeCbProc(Rectangle parentBounds, Rectangle windowBounds)
		{//������ʵ���ӿؼ��ݹ���ã�������ÿ���ؼ���Resize()
			this->parentBounds = parentBounds;
			graph.SetWindowSize(windowBounds);
			Resize(parentBounds, windowBounds);
			Rectangle myBounds = graph.GetBounds();
			for (auto& child : childs) child->ResizeCbProc(myBounds, windowBounds);
		}

		void SetRefreshMethod(std::function<void()> refreshMethod) { this->refreshMethod = refreshMethod; };
		void Repaint()
		{
			//���ÿ����û������������������ġ��ҽ���������������ã������Ϳ���ȷ��ֻ�ڴ����߳��ڵ��á�
			//����
			RepaintProc();
			refreshMethod();
			//����
		}
		void RepaintProc()
		{
			graph.BeginPaint();
			Paint(graph);
			graph.EndPaint();
			for (auto& child : childs)child->RepaintProc();//�ݹ����
		}
		virtual void RepaintCbProc() { RepaintProc(); };//ʹ��ʱӦ����д���������SwapBuffer

		virtual int MouseMsgCbProc(int x, int y, int msg, int hasTopTake)//ͨ����������ݹ������ӿؼ��������ӿؼ����������
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
			Rectangle myBounds = graph.GetBounds();//ȫ��bounds
			int mx = x - myBounds.x, my = y - myBounds.y;
			mouseMsg.SetCompXY(myBounds.x, myBounds.y);
			mouseMsg.SetXY(x, y);
			mouseMsg.SetMsg(msg);
			MouseMsgSync(mouseMsg);//�㲥����¼��������Ƿ��ڿؼ���
			if ((myBounds.IsIn(x, y) && !hasChildTake && !hasParentTake) || isFocus)
			{
				if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN)isFocus = 1;
				else if (msg == WM_LBUTTONUP || msg == WM_RBUTTONUP)isFocus = 0;
				MouseMsgCb(mouseMsg);//������Ժ󱻷���
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

		virtual void KbMsgCbProc(int key, int state)//(���߸�������¼��ÿؼ��Լ���������)
		{
			KbMsg(key, state);//Ⱥ��
			for (auto& child : childs)child->KbMsgCbProc(key, state);
		}

	public:
		virtual void Resize(Rectangle parentBounds, Rectangle windowBounds) {};//Resize����
		virtual void Paint(Graphics& g) {};//���ƴ���
		virtual void MouseMsgSync(MouseMsg& m) {};//ȫ������¼�����
		virtual void MouseMsgCb(MouseMsg& m) {};//����¼�����
		virtual void KbMsg(int key, int msg) {};

		virtual void LButtonDown(MouseMsg& m) {};//(ʵ����Ӧ�������)
		virtual void LButtonUp(MouseMsg& m) {};
		virtual void RButtonDown(MouseMsg& m) {};
		virtual void RButtonUp(MouseMsg& m) {};
		virtual void MouseMove(MouseMsg& m) {};

		//void Repaint();//�ֶ������ػ�
		void SetBounds(Rectangle bounds)
		{
			graph.SetBounds({ bounds.x + parentBounds.x,bounds.y + parentBounds.y,bounds.w,bounds.h });
			//bounds = graph.GetBounds();
			//mouseMsg.SetCompXY(bounds.x, bounds.y);
		}//����ڸ��ؼ���bounds
		Rectangle GetBounds()//x,y������ڸ��ؼ���
		{
			Rectangle myBounds = graph.GetBounds();
			return { myBounds.x - parentBounds.x,myBounds.y - parentBounds.y,myBounds.w,myBounds.h };
		}
		void SetGlobalBounds(Rectangle bounds)
		{
			graph.SetBounds(bounds);
			//mouseMsg.SetCompXY(bounds.x, bounds.y);
		}//ȫ��bounds
		Rectangle GetGlobalBounds()
		{
			return graph.GetBounds();
		}//ȫ��bounds
		Rectangle ToGlobal(Rectangle bounds)
		{
			return { bounds.x + parentBounds.x,bounds.y + parentBounds.y,bounds.w,bounds.h };
		}//����ڸ��ؼ���boundsתȫ��
	};
}