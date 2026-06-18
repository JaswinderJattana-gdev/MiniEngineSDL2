#pragma once
#include "../engine/IScene.h"
#include "../engine/math/Vec2.h"

#include "../engine/Texture2D.h"
#include "../engine/SpriteSheet.h"
#include "../engine/Animator.h"
#include "../engine/Camera2D.h"
#include "../engine/ParticleSystem2D.h"
#include "../engine/CollisionWorld2D.h"
#include "../engine/BulletSystem2D.h"
#include "../engine/TargetSystem2D.h"
#include "../engine/EnemySystem2D.h"
#include "../engine/WeaponSystem2D.h"
#include <vector>
#include <SDL.h>
#include <array>


class SceneManager;

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

    // Mech visual sheets
    SpriteSheet mechLowerSheet_;
    SpriteSheet mechUpperSheet_;

    bool mechLowerLoaded_ = false;
    bool mechUpperLoaded_ = false;

    // Mech sheet constants
    static constexpr int MECH_FRAME_SIZE = 512;
    static constexpr int MECH_LOWER_DIRS = 16;
    static constexpr int MECH_LOWER_FRAMES = 5;
    static constexpr int MECH_UPPER_FRAMES = 90;

    // Manual lower-body frame map:
    // [direction][animationFrame] -> source rect
    std::array<std::array<SDL_Rect, MECH_LOWER_FRAMES>, MECH_LOWER_DIRS> mechLowerFrameMap_{};

    // Lower body animation
    int mechLowerDir_ = 0;
    int mechLowerFrame_ = 0;
    double mechLowerAnimTimer_ = 0.0;
    double mechLowerFps_ = 10.0;

    // Upper turret
    int mechUpperFrame_ = 0;

    // Render tuning
    int mechRenderSize_ = 128;

    // Frame-map / angle tuning controls
    int mechUpperFrameOffset_ = 67;      // change this if turret frame 0 does not face right/east
    int mechLowerDirOffset_ = 4;        // starting guess: row 0 faces south
    bool mechUpperClockwise_ = true;   // flip if turret rotates opposite direction
    bool mechLowerClockwise_ = true;   // flip if lower rows rotate opposite direction

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

    BulletSystem2D bullets_;
    Vec2 facingDir_{ 0.0, 1.0 }; // default facing down

    static constexpr int MECH_GUN_COUNT = 2;

    // [turretFrame][gunIndex] -> muzzle position inside 512x512 source frame
    std::array<std::array<SDL_Point, MECH_GUN_COUNT>, MECH_UPPER_FRAMES> mechMuzzleMap_{};

    bool drawMuzzleDebug_ = true;

    TargetSystem2D targets_;

    EnemySystem2D enemies_;

    WeaponInstance2D primaryWeapon_;

    Camera2D camera_;

    ParticleSystem2D particles_;

    CollisionWorld2D collision_;

    // camera shake
    double shakeTimeLeft_ = 0.0;
    double shakeStrength_ = 2.0;
};