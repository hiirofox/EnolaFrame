#pragma once

namespace Enola
{
	class MouseMsg
	{
	private:
		int compx, compy;//控件全局坐标
		int x, y;//鼠标指针全局坐标
		int msg;
	public:
		void SetCompXY(int x, int y) { compx = x; compy = y; }//控件全局坐标
		void SetXY(int x, int y) { this->x = x; this->y = y; }//鼠标指针全局坐标
		void SetMsg(int msg) { this->msg = msg; }
		int GetX() { return x - compx; }
		int GetY() { return y - compy; }
		int GetGlobalX() { return x; }
		int GetGlobalY() { return y; }
		int GetMsg() { return msg; }
	};
}