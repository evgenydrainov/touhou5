#include "Game.h"

int main()
{
	for (;;) {
		th::Game game;
		game.run();

		if (!game.restart) {
			break;
		}
	}

	return 0;
}

#ifdef NDEBUG
int __stdcall WinMain(void* hInstance, void* hPrevInstance, char* pCmdLine, int nCmdShow)
{
	return main();
}
#endif
