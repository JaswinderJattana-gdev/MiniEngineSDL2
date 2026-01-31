#include "engine/Engine.h"

#include <SDL.h>

int main(int, char**)
{
    // Absolute proof binary is running
    /*SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_INFORMATION,
        "PROOF",
        "main() started. Click OK to continue.",
        nullptr
    );*/

    Engine engine;
    if (!engine.Init())
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "INIT FAILED",
            "Engine::Init() returned false. (We will log why next.)",
            nullptr
        );
        engine.Shutdown();
        return 1;
    }

    engine.Run();
    engine.Shutdown();
    return 0;
}
