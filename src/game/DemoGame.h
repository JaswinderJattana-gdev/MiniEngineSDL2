#pragma once

struct GameConfig;

class DemoGame : public IGame
{
public:
    void Init(const GameConfig& cfg, int windowW, int windowH) override;
    void Update(double dtSeconds, const Input& input, int windowW, int windowH) override;
    void Render(Renderer& renderer) override;

private:
    double x_ = 350.0;
    double y_ = 200.0;
    double speed_ = 300.0;

    int w_ = 100;
    int h_ = 50;
};
