#include <Windows.h>
#include <gl/GL.h>
#include "Component.h"
#include "Window.h"

#include "dbg.h"
#include "Type.h"

class Comp1 :public Enola::Component
{
private:
	int isClick = 0;
public:
	void Paint(Enola::Graphics& g)override
	{
		Enola::Rectangle bounds = GetBounds();
		int w = bounds.w, h = bounds.h;
		if (isClick)glColor3ub(0x00, 0xff, 0x00);
		else glColor3ub(0xff, 0xff, 0xff);
		glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(w, 0);
		glVertex2i(w, h);
		glVertex2i(0, h);
		glEnd();
	}
	void Resize(Enola::Rectangle parentBounds, Enola::Rectangle windowBounds)override
	{
		SetBounds({ 64,64,128,128 });
	}
	void MouseMsg(int x, int y, int msg)override
	{
		if (msg == WM_LBUTTONDOWN)isClick = 1;
		else if (msg == WM_LBUTTONUP)isClick = 0;
		Repaint();
	}
};

class App1 :public Enola::Window
{
private:
	Comp1 testComp;
	int isClick;
public:
	App1()
	{
		AddAndMakeVisible(testComp);
	}
	void Paint(Enola::Graphics& g)override
	{
		//DBG("Repaint!\n");
		Enola::Rectangle bounds = GetBounds();
		int w = bounds.w, h = bounds.h;
		if (isClick)glColor3ub(0x00, 0xff, 0xff);
		else glColor3ub(0x00, 0x00, 0xff);
		glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(w, 0);
		glVertex2i(w, h);
		glVertex2i(0, h);
		glEnd();
	}
	void MouseMsg(int x, int y, int msg)override
	{
		if (msg == WM_LBUTTONDOWN)isClick = 1;
		else if (msg == WM_LBUTTONUP)isClick = 0;
		Repaint();
	}
};

App1 app;
int main()
{
	app.Create("hello enola!", 640, 480);
	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
}
