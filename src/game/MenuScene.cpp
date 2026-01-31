#include "MenuScene.h"
#include "DemoScene.h"
#include "../engine/SceneManager.h"
#include "../engine/Input.h"
#include "../engine/Renderer.h"
#include "../engine/EngineContext.h" 
#include "../engine/Log.h"
#include "../engine/InputMap.h"
#include "../engine/Actions.h"
#include <memory>

MenuScene::MenuScene(SceneManager& scenes, const EngineContext& ctx)
    : scenes_(scenes), ctx_(ctx)
{
}

void MenuScene::Update(double /*dtSeconds*/, const Input& input, const EngineContext& ctx)
{
    // Enter starts the demo
    if (ctx.input && ctx.input->Pressed(input, Action::Confirm))
    {
        scenes_.Set(std::make_unique<DemoScene>(scenes_, ctx));
    }
}

void MenuScene::Render(Renderer& renderer, const EngineContext& ctx)
{
    if (!menuLoaded_)
    {
        menuLoaded_ = menuTex_.LoadFromBMP(
            ctx_.renderer->Raw(),
            "assets/menu.bmp",
            true, 255, 255, 255
        );

        if (!menuLoaded_)
            Log::Warn("MenuScene: could not load assets/menu.bmp");
    }

    if (menuLoaded_)
    {
        renderer.DrawTexture(
            menuTex_.Raw(),
            0,
            0,
            ctx_.logicalW,
            ctx_.logicalH
        );
    }
    else
    {
        const int w = 240;
        const int h = 110;

        const int x = (ctx.logicalW - w) / 2;
        const int y = (ctx.logicalH - h) / 2;

        renderer.DrawRect(x, y, w, h);
    }
}
void MenuScene::OnEnter()
{
    Log::Info("MenuScene enter");
    if (ctx_.window) SDL_SetWindowTitle(ctx_.window, "MiniEngineSDL2 - MENU");
}

void MenuScene::OnExit()
{
    Log::Info("MenuScene exit");
}

