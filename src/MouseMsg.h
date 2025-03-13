#pragma once

namespace Enola
{
	class MouseMsg
	{
	private:
		int compx, compy;//�ؼ�ȫ������
		int x, y;//���ָ��ȫ������
		int msg;
	public:
		void SetCompXY(int x, int y) { compx = x; compy = y; }//�ؼ�ȫ������
		void SetXY(int x, int y) { this->x = x; this->y = y; }//���ָ��ȫ������
		void SetMsg(int msg) { this->msg = msg; }
		int GetX() { return x - compx; }
		int GetY() { return y - compy; }
		int GetGlobalX() { return x; }
		int GetGlobalY() { return y; }
		int GetMsg() { return msg; }
	};
}