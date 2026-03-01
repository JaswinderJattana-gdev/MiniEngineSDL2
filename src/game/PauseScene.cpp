#include "PauseScene.h"
#include "../engine/SceneManager.h"
#include "../engine/Log.h"
#include "../engine/InputMap.h"
#include "../engine/Actions.h"
#include "../engine/EngineContext.h"
#include "../engine/Renderer.h"
#include "../engine/Input.h"

PauseScene::PauseScene(SceneManager& scenes, const EngineContext& ctx)
    : scenes_(scenes), ctx_(ctx)
{
}

void PauseScene::OnEnter()
{
    Log::Info("PauseScene enter");
    if (ctx_.window) SDL_SetWindowTitle(ctx_.window, "MiniEngineSDL2 - PAUSED");
}

void PauseScene::OnExit()
{
    Log::Info("PauseScene exit");
}

void PauseScene::Update(double /*dtSeconds*/, const Input& input, const EngineContext& ctx)
{
    if (ctx.input && (ctx.input->Pressed(input, Action::Pause) || ctx.input->Pressed(input, Action::Back)))
    {
        scenes_.Pop();
    }
}

void PauseScene::Render(Renderer& renderer, const EngineContext& ctx)
{
    if (!pauseLoaded_)
    {
        pauseLoaded_ = pauseTex_.LoadFromBMP(
            ctx_.renderer->Raw(),
            "assets/pause.bmp",
            true, 255, 255, 255
        );

        if (!pauseLoaded_)
            Log::Warn("PauseScene: could not load assets/pause.bmp");
    }

    if (pauseLoaded_)
    {
        const int pw = 400;
        const int ph = 200;

        const int x = (ctx_.logicalW - pw) / 2;
        const int y = (ctx_.logicalH - ph) / 2;

        renderer.DrawTexture(
            pauseTex_.Raw(),
            x,
            y,
            pw,
            ph
        );
    }
    else
    {
        renderer.SetDrawColor(30, 30, 30, 255);

        const int w = 320;
        const int h = 160;
        const int x = (ctx.logicalW - w) / 2;
        const int y = (ctx.logicalH - h) / 2;

        renderer.DrawRect(x, y, w, h);
    }
}
