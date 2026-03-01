#pragma once

class Input;
class Renderer;
struct EngineContext;

class IScene
{
public:
    virtual ~IScene() = default;

    virtual void OnEnter() {}
    virtual void OnExit() {}

    virtual void Update(double dtSeconds, const Input& input, const EngineContext& ctx) = 0;
    virtual void Render(Renderer& renderer, const EngineContext& ctx) = 0;
};
