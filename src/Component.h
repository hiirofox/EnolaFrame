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

		Rectangle parentBounds;//��Щ��������ڴ��ڵ�
	protected:
		void AddAndMakeVisible(Component& child)
		{
			child.refreshMethod = refreshMethod;
			childs.emplace_back(&child);
		}
		void ResizeCbProc(Rectangle parentBounds, Rectangle windowBounds)
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
		int MouseMsgCbProc(int x, int y, int msg)
		{
			Rectangle myBounds = graph.GetBounds();
			if (x >= myBounds.x && y >= myBounds.y && x <= myBounds.x + myBounds.w && y <= myBounds.y + myBounds.h)//����ڷ�Χ��
			{
				int flag = 0;
				for (auto& child : childs)
				{
					if (child->MouseMsgCbProc(x, y, msg))flag = 1;
				}
				if (flag == 0)//������ҷ�Χ����û�ж���Ҫ
				{
					MouseMsg(x, y, msg);//����Ҫ��
				}
				return 1;
			}
			return 0;
		}
		void KbMsgCbProc(int key, int state)//��ʱû�����ô����Ⱥ���ɡ�
		{
			KbMsg(key, state);
			for (auto& child : childs)child->KbMsgCbProc(key, state);
		}

	public:
		virtual void Resize(Rectangle parentBounds, Rectangle windowBounds) {};//Resize����
		virtual void Paint(Graphics& g) {};//���ƴ���
		virtual void MouseMsg(int x, int y, int msg) {};//����¼�����
		virtual void KbMsg(int key, int msg) {};

		//void Repaint();//�ֶ������ػ�
		void SetBounds(Rectangle bounds) { graph.SetBounds({ bounds.x + parentBounds.x,bounds.y + parentBounds.y,bounds.w,bounds.h }); }//����ڸ��ؼ���bounds
		Rectangle GetBounds()
		{
			Rectangle myBounds = graph.GetBounds();
			return { myBounds.x - parentBounds.x,myBounds.y - parentBounds.y,myBounds.w,myBounds.h };
		}
	};
}