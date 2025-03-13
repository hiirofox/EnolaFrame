#include <Windows.h>
#include <gl/GL.h>
#include "Component.h"
#include "VirtualWindow.h"
#include "Window.h"

#include "dbg.h"
#include "Type.h"

class Wnd1 :public Enola::VirtualWindow
{
private:
public:
	void Paint(Enola::Graphics& g)override
	{
		Enola::Rectangle bounds = GetBounds();
		int w = bounds.w, h = bounds.h;
		g.SetColor(color);
		g.DrawFillRect(0, 0, w, h);
	};
	void Resize(Enola::Rectangle parentBounds, Enola::Rectangle windowBounds)override
	{
		SetBounds({ 64,64,128,128 });
	}
	int color;
};
class App1 :public Enola::Window
{
private:
	Wnd1 wnds[5];
public:
	App1()
	{
		for (int i = 0; i < 5; ++i)
		{
			wnds[i].color = 0xff000000 | (rand() * rand() % 0xffffff);
			AddAndMakeVisible(wnds[i]);
		}
	}
	void Paint(Enola::Graphics& g)override
	{
		g.Clear(0x00000000);
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
