#pragma once

#include <gl/GL.h>

#include "dbg.h"
#include "Type.h"
#include "Def.h"

extern std::mutex GraphMtx;
namespace Enola {
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
			// �����ӿ���OpenGL����ϵ�е�λ��
			GLint glViewportY = windowSize.h - drawBounds.y - drawBounds.h;
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			// ʹ�û�ͼ����ߴ���Ϊ����ͶӰ��Χ
			glOrtho(0, drawBounds.w, drawBounds.h, 0, -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			// �����ӿںͼ�������
			glScissor(
				drawBounds.x,
				glViewportY,  // ת�����Y����
				drawBounds.w,
				drawBounds.h
			);
			glEnable(GL_SCISSOR_TEST);  // ���ü��ò���
			glViewport(
				drawBounds.x,
				glViewportY,  // ת�����Y����
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
		void SetBounds(Rectangle bounds) { drawBounds = bounds; }//����ڴ��ڵ�bounds
		Rectangle GetBounds() { return drawBounds; }
		void SetWindowSize(Rectangle windowSize) { this->windowSize = windowSize; }//component��resize callbackʱ����һ��
	};
}