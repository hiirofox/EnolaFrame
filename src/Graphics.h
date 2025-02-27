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
			// 计算视口在OpenGL坐标系中的位置
			GLint glViewportY = windowSize.h - drawBounds.y - drawBounds.h;
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			// 使用绘图区域尺寸作为正交投影范围
			glOrtho(0, drawBounds.w, drawBounds.h, 0, -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			// 设置视口和剪裁区域
			glScissor(
				drawBounds.x,
				glViewportY,  // 转换后的Y坐标
				drawBounds.w,
				drawBounds.h
			);
			glEnable(GL_SCISSOR_TEST);  // 启用剪裁测试
			glViewport(
				drawBounds.x,
				glViewportY,  // 转换后的Y坐标
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
		void SetBounds(Rectangle bounds) { drawBounds = bounds; }//相对于窗口的bounds
		Rectangle GetBounds() { return drawBounds; }
		void SetWindowSize(Rectangle windowSize) { this->windowSize = windowSize; }//component在resize callback时更新一下
	};
}