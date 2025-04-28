#include "pch.h"
#include "Application.h"

int main()
{
    Application app;
    app.Init();
    app.InitImGui();
    app.Run();
    app.ShutdownImGui();

	return 0;
}