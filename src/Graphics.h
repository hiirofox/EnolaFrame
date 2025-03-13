#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <gl/GL.h>

#include "dbg.h"
#include "Type.h"
#include "Def.h"

extern std::mutex GraphMtx;
namespace Enola {
	class Graphics//gl��ͼ������
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

		void SetColor(int argb)
		{
			GLubyte r = (argb >> 16) & 0xff;
			GLubyte g = (argb >> 8) & 0xff;
			GLubyte b = argb & 0xff;
			GLubyte a = (argb >> 24) & 0xff;
			glColor4ub(r, g, b, a);
		}
		void SetLineWidth(float width = 1.0)
		{
			glLineWidth(width);
		}
		void Clear(int argb)
		{
			GLubyte r = (argb >> 16) & 0xff;
			GLubyte g = (argb >> 8) & 0xff;
			GLubyte b = argb & 0xff;
			GLubyte a = (argb >> 24) & 0xff;
			glClearColor(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
			glClear(GL_COLOR_BUFFER_BIT);
		}
		void DrawPixel(int x, int y)
		{
			glBegin(GL_POINTS);
			glVertex2i(x, y);
			glEnd();
		}
		void DrawLine(int x1, int y1, int x2, int y2)
		{
			glBegin(GL_LINES);
			glVertex2i(x1, y1);
			glVertex2i(x2, y2);
			glEnd();
		}
		void DrawRect(int x, int y, int w, int h)
		{
			glBegin(GL_LINE_LOOP);
			glVertex2i(x, y);
			glVertex2i(x + w, y);
			glVertex2i(x + w, y + h);
			glVertex2i(x, y + h);
			glEnd();
		}
		void DrawFillRect(int x, int y, int w, int h)
		{
			glBegin(GL_QUADS);
			glVertex2i(x, y);
			glVertex2i(x + w, y);
			glVertex2i(x + w, y + h);
			glVertex2i(x, y + h);
			glEnd();
		}
		void DrawCircle(int x, int y, int r, float precision = 0.1)
		{
			glBegin(GL_LINE_LOOP);
			for (float i = 0; i < 2 * M_PI; i += precision)
			{
				glVertex2f(x + r * cosf(i), y + r * sinf(i));
			}
			glEnd();
		}
		void DrawFillCircle(int x, int y, int r, float precision = 0.1)
		{
			glBegin(GL_TRIANGLE_FAN);
			glVertex2i(x, y);
			for (float i = 0; i < 2 * M_PI; i += precision)
			{
				glVertex2f(x + r * cosf(i), y + r * sinf(i));
			}
			glEnd();
		}
		void DrawEllipse(int x, int y, int a, int b, float precision = 0.1)
		{
			glBegin(GL_LINE_LOOP);
			for (float i = 0; i < 2 * M_PI; i += precision)
			{
				glVertex2f(x + a * cosf(i), y + b * sinf(i));
			}
			glEnd();
		}
		void DrawFillEllipse(int x, int y, int a, int b, float precision = 0.1)
		{
			glBegin(GL_TRIANGLE_FAN);
			glVertex2i(x, y);
			for (float i = 0; i < 2 * M_PI; i += precision)
			{
				glVertex2f(x + a * cosf(i), y + b * sinf(i));
			}
			glEnd();
		}
	};
}