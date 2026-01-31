#pragma once

class Input;
class Renderer;
struct GameConfig;

class IGame
{
public:
    virtual ~IGame() = default;

    virtual void Init(const GameConfig& cfg, int windowW, int windowH) = 0;
    virtual void Update(double dtSeconds, const Input& input, int windowW, int windowH) = 0;
    virtual void Render(Renderer& renderer) = 0;
};
