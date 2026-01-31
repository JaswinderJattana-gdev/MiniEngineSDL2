#pragma once
#include "../engine/IScene.h"
#include "../engine/EngineContext.h"
#include "../engine/Texture2D.h"


class SceneManager;

class PauseScene : public IScene
{
public:
    explicit PauseScene(SceneManager& scenes, const EngineContext& ctx);
    const EngineContext& ctx_;

    void OnEnter() override;
    void OnExit() override;

    void Update(double dtSeconds, const Input& input, const EngineContext& ctx) override;
    void Render(Renderer& renderer, const EngineContext& ctx) override;

    Texture2D pauseTex_;
    bool pauseLoaded_ = false;

private:
    SceneManager& scenes_;
};
