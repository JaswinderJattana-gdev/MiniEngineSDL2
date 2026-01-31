#pragma once
#include "../engine/IScene.h"
#include "../engine/math/Vec2.h"

#include "../engine/Texture2D.h"
#include "../engine/SpriteSheet.h"
#include "../engine/Animator.h"

#include <vector>
#include <SDL.h>


class SceneManager;

struct Particle
{
    Vec2 pos;
    Vec2 vel;
    double life = 0.0;   // seconds remaining
    double maxLife = 0.0;
    int size = 2;        // pixels
};

class DemoScene : public IScene
{
public:
    DemoScene(SceneManager& scenes, const EngineContext& ctx);

    void OnEnter() override;
    void OnExit() override;

    void Update(double dtSeconds, const Input& input, const EngineContext& ctx) override;
    void Render(Renderer& renderer, const EngineContext& ctx) override;

    SpriteSheet sheet_;
    Animator anim_;
    bool sheetLoaded_ = false;

    int dirRow_ = 5; // default South row (idle facing down)

    int idleCol_ = 2;        // idle frame column
    int walkStartCol_ = 0;   // first walking frame
    int walkFrameCount_ = 8; // frames per row
    double walkFps_ = 10.0;

    bool playerTexLoaded_ = false;
    bool moving_ = false;
    bool debugDraw_ = false;

    std::vector<SDL_Rect> obstacles_;

    SDL_Rect FeetRectWorld() const;

    std::vector<Particle> particles_;
    double dustSpawnAcc_ = 0.0;

private:
    SceneManager& scenes_;
    const EngineContext& ctx_;

    Vec2 pos_{ 350.0, 200.0 };
    double speed_ = 300.0;
    int w_ = 100;
    int h_ = 50;

    Vec2 worldPos_{ 10.0, 10.0 };

    Vec2 camPos_{ 0.0, 0.0 }; // top-left of camera in world space
    int worldW_ = 4000;
    int worldH_ = 4000;

    bool dashing_ = false;
    double dashTimeLeft_ = 0.0;
    double dashCooldownLeft_ = 0.0;

    double dashDuration_ = 0.30;
    double dashCooldown_ = 0.35;
    double dashSpeedMult_ = 3;

    // camera shake
    double shakeTimeLeft_ = 0.0;
    double shakeStrength_ = 2.0;
};
