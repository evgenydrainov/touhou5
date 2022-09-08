#include "Game.h"

int main()
{
	th::Game game;
	game.run();
	return 0;
}

#ifdef NDEBUG
int __stdcall WinMain(void* hInstance, void* hPrevInstance, char* pCmdLine, int nCmdShow)
{
	return main();
}
#endif
