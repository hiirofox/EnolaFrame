#include "Type.h"
#include "WindowBase.h"
#include <mutex>

std::mutex GraphMtx;
namespace Enola {
	class GraphicsTest
	{
	private:
		HGLRC glContext;
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
		void SetDrawBounds(Rectangle bounds, Rectangle windowSize) { drawBounds = bounds; this->windowSize = windowSize; }//component在resize callback时更新一下
		Rectangle GetBounds() { return drawBounds; }
	};
}

void Paint1(Enola::GraphicsTest& g)
{
	int w = g.GetBounds().w, h = g.GetBounds().h;
	glColor3ub(255, 255, 255);
	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(w, 0);
	glVertex2i(w, h);
	glVertex2i(0, h);
	glEnd();
}
void Paint2(Enola::GraphicsTest& g)
{
	int w = g.GetBounds().w, h = g.GetBounds().h;
	glColor3ub(0, 255, 0);
	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(w, 0);
	glVertex2i(w, h);
	glVertex2i(0, h);
	glEnd();
}


Enola::WindowBase wnd;
Enola::GraphicsTest g1, g2;
int main()
{
	wnd.Create("hello!", 640, 480);
	//g.SetDrawBounds({ 0,0,640,480 }, { 0,0,640,480 });
	wnd.SetResizeCallbackFunc([&](int w, int h) {
		//DBG("w:%d,h:%d\n", w, h);
		g1.SetDrawBounds({ 32,96,64,64 }, { 0,0,w,h });
		g2.SetDrawBounds({ 64,64,64,64 }, { 0,0,w,h });
		});
	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(10));
		wnd.SendTaskToThread([&]() {//向窗口线程发送绘制
			//DBG("111");
			glClear(GL_COLOR_BUFFER_BIT);

			g1.BeginPaint();
			Paint1(g1);
			g1.EndPaint();

			g2.BeginPaint();
			Paint2(g2);
			g2.EndPaint();

			SwapBuffers(wnd.GetHDC());
			});
	}
}
