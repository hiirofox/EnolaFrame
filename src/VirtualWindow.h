#pragma once

#include "Component.h"

namespace Enola
{
	class VirtualWindow :public Component
	{
	private:
		int isClick = 0;
		int startX = 0, startY = 0;
	public:
		void LButtonDown(MouseMsg& m) override
		{
			isClick = 1;
			startX = m.GetX();
			startY = m.GetY();
			ToTop();
			Repaint();
		}
		void LButtonUp(MouseMsg& m) override
		{
			isClick = 0;
			Repaint();
		}
		void MouseMove(MouseMsg& m) override
		{
			if (isClick)
			{
				Rectangle bounds = GetGlobalBounds();
				SetGlobalBounds({ m.GetGlobalX() - startX, m.GetGlobalY() - startY,bounds.w,bounds.h });
			}
			Repaint();
		}
		/*
		void Paint(Graphics& g) override//test
		{
			Rectangle bounds = GetBounds();
			int w = bounds.w, h = bounds.h;
			if (isClick)g.SetColor(0xff00ff00);
			else g.SetColor(0xffffffff);
			g.DrawFillRect(0, 0, w, h);
		}
		void Resize(Rectangle parentBounds, Rectangle windowBounds) override//test
		{
			SetBounds({ 64,64,128,128 });
		}*/
	};
}