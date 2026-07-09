#include "DemoScene.h"
#include "MenuScene.h"
#include "PauseScene.h"
#include "GamePaths.h"

#include "../engine/SceneManager.h"
#include "../engine/Input.h"
#include "../engine/Renderer.h"
#include "../engine/EngineContext.h"
#include "../engine/Log.h"
#include "../engine/InputMap.h"
#include "../engine/Actions.h"
#include "../engine/Texture2D.h"
#include "../engine/WorldRender2D.h"
#include "../engine/LevelData.h"
#include "../engine/LevelIO.h"
#include "../engine/AssetManager.h"
#include "../engine/SpriteSheet.h"

#include <algorithm>
#include <memory>
#include <cmath>
#include <string>

namespace
{
    constexpr double PI = 3.14159265358979323846;

    double AngleDegreesFromDir(const Vec2& dir)
    {
        // 0 degrees = East/right
        // positive angle goes counter-clockwise in math space
        double deg = std::atan2(-dir.y, dir.x) * 180.0 / PI;
        if (deg < 0.0)
            deg += 360.0;
        return deg;
    }

    int DirectionIndex16FromDir(const Vec2& dir, int offset, bool clockwise)
    {
        if (dir.x == 0.0 && dir.y == 0.0)
            return 0;

        double deg = AngleDegreesFromDir(dir);

        // 16 directions = 22.5 degrees each
        int idx = static_cast<int>(std::floor((deg + 11.25) / 22.5)) % 16;

        if (clockwise)
            idx = (16 - idx) % 16;

        idx = (idx + offset) % 16;
        if (idx < 0)
            idx += 16;

        return idx;
    }

    int TurretFrame90FromDir(const Vec2& dir, int offset, bool clockwise)
    {
        if (dir.x == 0.0 && dir.y == 0.0)
            return 0;

        double deg = AngleDegreesFromDir(dir);

        // 90 frames = 4 degrees each
        int frame = static_cast<int>(std::round(deg / 4.0)) % 90;

        if (clockwise)
            frame = (90 - frame) % 90;

        frame = (frame + offset) % 90;
        if (frame < 0)
            frame += 90;

        return frame;
    }

    SDL_Point PointOnCircle(double degrees, double radius, double sideOffset)
    {
        const double rad = degrees * PI / 180.0;

        // Direction: 0 degrees = right/east
        const double dx = std::cos(rad);
        const double dy = -std::sin(rad);

        // Perpendicular vector
        const double px = -dy;
        const double py = dx;

        const double cx = 256.0;
        const double cy = 256.0;

        return SDL_Point{
            static_cast<int>(std::round(cx + dx * radius + px * sideOffset)),
            static_cast<int>(std::round(cy + dy * radius + py * sideOffset))
        };
    }
}

DemoScene::DemoScene(SceneManager& scenes, const EngineContext& ctx)
    : scenes_(scenes), ctx_(ctx)
{}


void DemoScene::Update(double dtSeconds, const Input& input, const EngineContext& ctx)
{
    // ESC goes back to menu (engine keeps running)
    if (ctx.input && ctx.input->Pressed(input, Action::Back))
    {
        scenes_.Set(std::make_unique<MenuScene>(scenes_, ctx_));
        return;
    }

	// P pauses
    if (ctx.input && ctx.input->Pressed(input, Action::Pause))
    {
        scenes_.Push(std::make_unique<PauseScene>(scenes_, ctx_));
        return;
    }

    if (!input.HasFocus())
        return;

    Vec2 dir{ 0.0, 0.0 };

    // Toggle debug draw with F1 (direct keyboard check)
    const Uint8* ks = SDL_GetKeyboardState(nullptr);
    static bool prevF1 = false;
    const bool f1 = (ks[SDL_SCANCODE_F1] != 0);
    if (f1 && !prevF1)
        debugDraw_ = !debugDraw_;
    prevF1 = f1;

    // Map direction index -> sprite sheet row
// Direction index order from pickDirRow: 0:E,1:NE,2:N,3:NW,4:W,5:SW,6:S,7:SE
    static const int DIR_TO_ROW[8] =
    {
        0, // E  -> row ?
        2, // NE -> row ?
        1, // N  -> row ?
        3, // NW -> row ?
        7, // W  -> row ?
        6, // SW -> row ?
        4, // S  -> row ?
        5  // SE -> row ?
    };

    if (ctx.input && ctx.input->Down(input, Action::MoveLeft))  dir.x -= 1.0;
    if (ctx.input && ctx.input->Down(input, Action::MoveRight)) dir.x += 1.0;
    if (ctx.input && ctx.input->Down(input, Action::MoveUp))    dir.y -= 1.0;
    if (ctx.input && ctx.input->Down(input, Action::MoveDown))  dir.y += 1.0;

    if (dir.x != 0.0 || dir.y != 0.0)
        dir = dir.Normalized();

    auto pickDirRow = [](double x, double y) -> int
        {
            // x,y are assumed normalized-ish. Y- is up.
            // Returns row index using: E,NE,N,NW,W,SW,S,SE
            if (x == 0.0 && y == 0.0) return -1;

            // Angle where 0 is East, counter-clockwise positive
            const double ang = std::atan2(-y, x); // -y because screen y grows downward
            // Convert to 0..2pi
            double a = ang;
            if (a < 0.0) a += 6.283185307179586;

            // 8 sectors (each 45 degrees)
            const int sector = static_cast<int>(std::floor((a + 0.392699081698724) / 0.785398163397448)); // +22.5deg then /45deg
            const int s = sector % 8;

            // Map sector (0..7) to rows: E,NE,N,NW,W,SW,S,SE
            return s;
        };

    primaryWeapon_.Update(dtSeconds);

	// Dash
    dashCooldownLeft_ = std::max(0.0, dashCooldownLeft_ - dtSeconds);
    if (dashing_)
    {
        dashTimeLeft_ -= dtSeconds;
        if (dashTimeLeft_ <= 0.0)
            dashing_ = false;
    }

    if (shakeTimeLeft_ > 0.0)
        shakeTimeLeft_ = std::max(0.0, shakeTimeLeft_ - dtSeconds);
    //

    const bool moving = (dir.x != 0.0 || dir.y != 0.0);
    const int dirIndex = pickDirRow(dir.x, dir.y);
    if (dirIndex != -1)
    {
        dirRow_ = DIR_TO_ROW[dirIndex];
        facingDir_ = dir;
    }

    if (moving)
    {
        anim_.Update(dtSeconds, true);
    }
    else
    {
        // When idle, stop animation and force idle frame later in Render (idleCol_ = 3)
        anim_.Update(dtSeconds, false);
    }

    moving_ = moving;

    w_ = ctx.gameCfg.playerW;
    h_ = ctx.gameCfg.playerH;

    h_ = w_;

    speed_ = ctx.gameCfg.playerSpeed;

    const Vec2 prev = worldPos_;

    // Move in world space with collision (engine helper)
    const double speedNow = speed_ * (dashing_ ? dashSpeedMult_ : 1.0);
    Vec2 desired = prev + dir * (speedNow * dtSeconds);

    // feet rect callback (static) used by CollisionWorld2D
    auto FeetRectFromTopLeft = [](const Vec2& topLeft, int objW, int objH) -> SDL_Rect
        {
            // matches DemoScene::FeetRectWorld() logic, but uses provided topLeft/size
            const int cw = static_cast<int>(objW * 0.28);
            const int ch = static_cast<int>(objH * 0.35);

            const int cx = static_cast<int>(std::round(topLeft.x + (objW - cw) * 0.5));
            const double anchor = 0.7;
            const int cy = static_cast<int>(std::round(topLeft.y + objH * anchor - ch * 0.7));

            return SDL_Rect{ cx, cy, cw, ch };
        };

    worldPos_ = collision_.MoveWithCollisions(prev, desired, w_, h_, FeetRectFromTopLeft);

    const bool moved = (worldPos_.x != prev.x) || (worldPos_.y != prev.y);
    moving_ = moving && moved;

    if (moving_)
        anim_.Update(dtSeconds, true);
    else
        anim_.Update(dtSeconds, false);

    // Mech lower-body direction and walk animation
    if (moving_)
    {
        mechLowerDir_ = DirectionIndex16FromDir(dir, mechLowerDirOffset_, mechLowerClockwise_);

        mechLowerAnimTimer_ += dtSeconds;
        const double frameTime = 1.0 / mechLowerFps_;

        while (mechLowerAnimTimer_ >= frameTime)
        {
            mechLowerAnimTimer_ -= frameTime;
            mechLowerFrame_ = (mechLowerFrame_ + 1) % MECH_LOWER_FRAMES;
        }
    }
    else
    {
        mechLowerAnimTimer_ = 0.0;
        mechLowerFrame_ = 0;
    }

	// Dash input
    const bool dashPressed = (ctx.input && ctx.input->Pressed(input, Action::Dash));

    if (dashPressed && moving && !dashing_ && dashCooldownLeft_ <= 0.0)
    {
        dashing_ = true;
        dashTimeLeft_ = dashDuration_;
        dashCooldownLeft_ = dashCooldown_;

        // small camera shake
        shakeTimeLeft_ = 0.10;
    }

    // Fire input
    const bool fireHeld = (ctx.input && ctx.input->Down(input, Action::Fire));

    if (fireHeld)
    {
        SDL_Point mouseWindow = input.MousePosition();

        int mouseLogicalX = mouseWindow.x;
        int mouseLogicalY = mouseWindow.y;

        if (ctx.windowW > 0 && ctx.windowH > 0)
        {
            mouseLogicalX = static_cast<int>((static_cast<double>(mouseWindow.x) / static_cast<double>(ctx.windowW)) * ctx.logicalW);
            mouseLogicalY = static_cast<int>((static_cast<double>(mouseWindow.y) / static_cast<double>(ctx.windowH)) * ctx.logicalH);
        }

        SDL_Point mouseWorld = camera_.ScreenToWorldPoint(SDL_Point{ mouseLogicalX, mouseLogicalY });

        Vec2 playerCenter{
            worldPos_.x + w_ * 0.5,
            worldPos_.y + h_ * 0.5
        };

        Vec2 aimDir{
            static_cast<double>(mouseWorld.x) - playerCenter.x,
            static_cast<double>(mouseWorld.y) - playerCenter.y
        };

        if (aimDir.Length() > 0.0001)
        {
            Vec2 fireDir = aimDir.Normalized();

            const double mechWorldX = worldPos_.x + w_ * 0.5 - mechRenderSize_ * 0.5;
            const double mechWorldY = worldPos_.y + h_ * 0.5 - mechRenderSize_ * 0.5;
            const double mechScale = static_cast<double>(mechRenderSize_) / static_cast<double>(MECH_FRAME_SIZE);

            std::vector<Vec2> muzzles;
            muzzles.reserve(MECH_GUN_COUNT);

            for (int gun = 0; gun < MECH_GUN_COUNT; ++gun)
            {
                const SDL_Point muzzleLocal = mechMuzzleMap_[mechUpperFrame_][gun];

                muzzles.push_back(Vec2{
                    mechWorldX + muzzleLocal.x * mechScale - 4.0,
                    mechWorldY + muzzleLocal.y * mechScale - 4.0
                    });
            }

            primaryWeapon_.TryFire(bullets_, muzzles, fireDir);
        }
    }

    // Dust spawn (only when actually moving)
    if (moving_)
    {
        SDL_Rect feet = FeetRectWorld();
        particles_.SpawnDustFromFeet(feet, dtSeconds, 30.0);
    }
    else
    {
        particles_.ResetSpawnAccumulator();
    }

    // Update particles
    particles_.Update(dtSeconds);

    // Update bullets
    bullets_.Update(dtSeconds);

    // Update enemy damage cooldown
    playerDamageCooldown_ = std::max(0.0, playerDamageCooldown_ - dtSeconds);

    // Bullet vs target hit detection
    auto& activeBullets = bullets_.Bullets();

    activeBullets.erase(
        std::remove_if(activeBullets.begin(), activeBullets.end(),
            [this](const BulletSystem2D::Bullet& b)
            {
                if (targets_.HitAndDamageFirst(b.entity, b.damage))
                    return true;

                if (enemies_.HitAndDamageFirst(b.entity, b.damage))
                    return true;

                return false;
            }),
        activeBullets.end()
    );

    targets_.RemoveDead();
    enemies_.RemoveDead();

	// Check win condition (all enemies dead)
    if (!levelWon_ && enemies_.AllDead())
    {
        levelWon_ = true;
    }

    // Camera follow (center player)
    camera_.SetViewSize(ctx.logicalW, ctx.logicalH);

    camera_.FollowTargetInstant(
        Vec2{ worldPos_.x + w_ * 0.5, worldPos_.y + h_ * 0.5 },
        worldW_,
        worldH_
    );

    camPos_ = camera_.Position();

    // Update turret aim frame from mouse position
    SDL_Point mouseWindow = input.MousePosition();

    int mouseLogicalX = mouseWindow.x;
    int mouseLogicalY = mouseWindow.y;

    if (ctx.windowW > 0 && ctx.windowH > 0)
    {
        mouseLogicalX = static_cast<int>((static_cast<double>(mouseWindow.x) / static_cast<double>(ctx.windowW)) * ctx.logicalW);
        mouseLogicalY = static_cast<int>((static_cast<double>(mouseWindow.y) / static_cast<double>(ctx.windowH)) * ctx.logicalH);
    }

    SDL_Point mouseWorld = camera_.ScreenToWorldPoint(SDL_Point{ mouseLogicalX, mouseLogicalY });

    Vec2 playerCenter{
        worldPos_.x + w_ * 0.5,
        worldPos_.y + h_ * 0.5
    };

	// Update enemies to move toward player (only if level not won)
    if (!levelWon_)
    {
        enemies_.SetVelocityTowardPoint(playerCenter, 80.0);
        enemies_.Update(dtSeconds);
    }

	// Update player entity for collision detection
    playerEntity_.position = worldPos_;
    playerEntity_.velocity = Vec2{ 0.0, 0.0 };
    playerEntity_.bounds = FeetRectWorld();

	// Check enemy vs player collisions and apply damage
    if (playerDamageCooldown_ <= 0.0 && enemies_.AnyEnemyIntersects(playerEntity_))
    {
        playerHealth_.ApplyDamage(enemyContactDamage_);
        playerDamageCooldown_ = playerDamageInterval_;
    }

	// If player died, respawn at start with full health and clear bullets/targets/enemies/particles
    if (playerHealth_.IsDead())
    {
        ResetDemo();
    }

    Vec2 aimDir{
        static_cast<double>(mouseWorld.x) - playerCenter.x,
        static_cast<double>(mouseWorld.y) - playerCenter.y
    };

    if (aimDir.Length() > 0.0001)
    {
        mechUpperFrame_ = TurretFrame90FromDir(aimDir.Normalized(), mechUpperFrameOffset_, mechUpperClockwise_);
    }
}

void DemoScene::Render(Renderer& renderer, const EngineContext& ctx)
{
    (void)ctx;
    renderer.Clear(245, 245, 245);

    const int cell = 64;
    const int majorEvery = 5;

    const int viewW = ctx.logicalW;
    const int viewH = ctx.logicalH;

    const int thickness = 32;

    Vec2 cam = camera_.Position();;

    if (shakeTimeLeft_ > 0.0)
    {
        // simple deterministic shake (no RNG)
        const int t = static_cast<int>(SDL_GetTicks() / 16);
        const double sx = (t % 2 == 0) ? -shakeStrength_ : shakeStrength_;
        const double sy = (t % 3 == 0) ? -shakeStrength_ : shakeStrength_;
        cam.x += sx;
        cam.y += sy;
    }

    // Apply shake to camera for this frame only (render-only)
    Camera2D camForRender = camera_;
    camForRender.SetPosition(cam);

    // Grid + world frame
    WorldRender2D::DrawGrid(renderer, camForRender, viewW, viewH, cell, majorEvery);

    WorldRender2D::DrawWorldBoundsFrame(renderer, camForRender, viewW, viewH, worldW_, worldH_, thickness);

    // Draw obstacles (world -> screen)
    renderer.SetDrawColor(60, 60, 60, 255);

    for (const auto& o : obstacles_)
    {
        const int ox = static_cast<int>(std::round(o.x - cam.x));
        const int oy = static_cast<int>(std::round(o.y - cam.y));
        renderer.DrawRect(ox, oy, o.w, o.h);
    }

    // Draw dust particles (world -> screen)
    particles_.RenderSquares(renderer, camForRender);

    // Draw targets (world -> screen)
    targets_.Render(renderer, camForRender);

	// Draw enemies (world -> screen)
    enemies_.Render(renderer, camForRender);

	// Draw bullets (world -> screen)
    bullets_.Render(renderer, camForRender);

	// Debug draw
    if (debugDraw_)
    {
        // Draw obstacles filled (so collision space is obvious)
        renderer.SetDrawColor(80, 80, 80, 80);
        SDL_SetRenderDrawBlendMode(renderer.Raw(), SDL_BLENDMODE_BLEND);
        for (const auto& o : obstacles_)
        {
            const int ox = static_cast<int>(std::round(o.x - cam.x));
            const int oy = static_cast<int>(std::round(o.y - cam.y));
            SDL_Rect r{ ox, oy, o.w, o.h };
            SDL_SetRenderDrawColor(renderer.Raw(), 80, 80, 80, 80);
            SDL_RenderFillRect(renderer.Raw(), &r);
        }

        // Feet collider outline
        SDL_Rect feet = FeetRectWorld();
        const int fx = static_cast<int>(std::round(feet.x - cam.x));
        const int fy = static_cast<int>(std::round(feet.y - cam.y));
        renderer.SetDrawColor(255, 0, 0, 255);
        renderer.DrawRect(fx, fy, feet.w, feet.h);
    }

    // Player screen position (world -> screen, with shake)
    const int screenX = static_cast<int>(std::round(worldPos_.x - cam.x));
    const int screenY = static_cast<int>(std::round(worldPos_.y - cam.y));

    // Draw player / mech
    const int mechX = static_cast<int>(std::round(worldPos_.x + w_ * 0.5 - mechRenderSize_ * 0.5 - cam.x));
    const int mechY = static_cast<int>(std::round(worldPos_.y + h_ * 0.5 - mechRenderSize_ * 0.5 - cam.y));

    if (mechLowerLoaded_ && mechLowerSheet_.Texture())
    {
        SDL_Rect lowerSrc = mechLowerFrameMap_[mechLowerDir_][mechLowerFrame_];

        renderer.DrawTextureRegion(
            mechLowerSheet_.Texture(),
            lowerSrc,
            mechX,
            mechY,
            mechRenderSize_,
            mechRenderSize_
        );
    }

    if (mechUpperLoaded_ && mechUpperSheet_.Texture())
    {
        const int upperCol = mechUpperFrame_ % 10;
        const int upperRow = mechUpperFrame_ / 10;
        SDL_Rect upperSrc = mechUpperSheet_.FrameRect(upperCol, upperRow);

        renderer.DrawTextureRegion(
            mechUpperSheet_.Texture(),
            upperSrc,
            mechX,
            mechY,
            mechRenderSize_,
            mechRenderSize_
        );
    }

    // Fallback to old player sprite if mech failed to load
    if (!mechLowerLoaded_ && !mechUpperLoaded_ && sheetLoaded_ && sheet_.Texture())
    {
        const int col = moving_ ? anim_.CurrentCol() : idleCol_;
        const SDL_Rect src = sheet_.FrameRect(col, dirRow_);

        renderer.DrawTextureRegion(
            sheet_.Texture(),
            src,
            screenX,
            screenY,
            w_,
            h_
        );
    }
    else if (!mechLowerLoaded_ && !mechUpperLoaded_)
    {
        renderer.DrawRect(
            screenX,
            screenY,
            w_,
            h_
        );
    }

    // Debug muzzle points
    if (drawMuzzleDebug_ && mechUpperLoaded_)
    {
        const int mechX = static_cast<int>(std::round(worldPos_.x + w_ * 0.5 - mechRenderSize_ * 0.5 - cam.x));
        const int mechY = static_cast<int>(std::round(worldPos_.y + h_ * 0.5 - mechRenderSize_ * 0.5 - cam.y));
        const double mechScale = static_cast<double>(mechRenderSize_) / static_cast<double>(MECH_FRAME_SIZE);

        renderer.SetDrawColor(0, 255, 0, 255);

        for (int gun = 0; gun < MECH_GUN_COUNT; ++gun)
        {
            const SDL_Point muzzleLocal = mechMuzzleMap_[mechUpperFrame_][gun];

            const int mx = mechX + static_cast<int>(std::round(muzzleLocal.x * mechScale));
            const int my = mechY + static_cast<int>(std::round(muzzleLocal.y * mechScale));

            renderer.FillRect(mx - 2, my - 2, 5, 5);
        }
    }

    // HUD panel (screen space)
    SDL_SetRenderDrawBlendMode(renderer.Raw(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer.Raw(), 0, 0, 0, 140);
    SDL_Rect panel{ 10, 10, 160, 76 };
    SDL_RenderFillRect(renderer.Raw(), &panel);

    // State indicators
    int x = 18, y = 18;

    // MOVING indicator (green when moving)
    renderer.SetDrawColor(moving_ ? 0 : 70, moving_ ? 200 : 70, 0, 255);
    renderer.FillRect(x, y, 14, 14);

    x += 22;
    // DASH indicator (blue when dashing)
    renderer.SetDrawColor(dashing_ ? 0 : 70, dashing_ ? 140 : 70, dashing_ ? 255 : 70, 255);
    renderer.FillRect(x, y, 14, 14);

    x += 22;
    // DEBUG indicator (yellow when debugDraw)
    renderer.SetDrawColor(debugDraw_ ? 220 : 70, debugDraw_ ? 220 : 70, 0, 255);
    renderer.FillRect(x, y, 14, 14);

    x += 22;
    // WIN indicator (cyan when level cleared)
    renderer.SetDrawColor(levelWon_ ? 0 : 70, levelWon_ ? 220 : 70, levelWon_ ? 220 : 70, 255);
    renderer.FillRect(x, y, 14, 14);

    // Speed bar
    const double speedNow = ctx.gameCfg.playerSpeed * (dashing_ ? dashSpeedMult_ : 1.0);
    const double maxShown = ctx.gameCfg.playerSpeed * dashSpeedMult_;
    const double t = std::clamp(speedNow / maxShown, 0.0, 1.0);

    renderer.SetDrawColor(200, 200, 200, 255);
    renderer.DrawRect(18, 40, 140, 8);

    renderer.SetDrawColor(120, 220, 120, 255);
    renderer.FillRect(18, 40, static_cast<int>(140 * t), 8);

    // Player health bar
    const double hpT =
        (playerHealth_.maxHp > 0)
        ? static_cast<double>(playerHealth_.hp) / static_cast<double>(playerHealth_.maxHp)
        : 0.0;

    renderer.SetDrawColor(200, 200, 200, 255);
    renderer.DrawRect(18, 56, 140, 8);

    renderer.SetDrawColor(220, 70, 70, 255);
    renderer.FillRect(18, 56, static_cast<int>(140 * hpT), 8);

    // DEBUG: prove FillRect works
    // renderer.SetDrawColor(255, 0, 255, 255);
    // renderer.FillRect(10, 10, 80, 40);

}

SDL_Rect DemoScene::FeetRectWorld() const
{
    // Feet collider: centered, small, near bottom (ignores shadow/head)
    const int cw = static_cast<int>(w_ * 0.28);
    const int ch = static_cast<int>(h_ * 0.35);

    const int cx = static_cast<int>(std::round(worldPos_.x + (w_ - cw) * 0.5));
    const double anchor = 0.7;
    const int cy = static_cast<int>(std::round(worldPos_.y + h_ * anchor - ch * 0.7));

    return SDL_Rect{ cx, cy, cw, ch };
}

void DemoScene::OnEnter()
{
    Log::Info("DemoScene enter");
    if (ctx_.window) SDL_SetWindowTitle(ctx_.window, "MiniEngineSDL2 - DEMO");
    sheetLoaded_ = false;

    if (ctx_.renderer && ctx_.renderer->Raw())
    {
        // IMPORTANT: match these to your actual sheet!
        const int frameW = 138;
        const int frameH = 138;
        const int cols = 8; // frames per direction
        const int rows = 8; // directions

        sheetLoaded_ = false;

        if (ctx_.assets)
        {
            ctx_.assets->LoadTextureBMP("player_sheet", "assets/player_sheet.bmp", 255, 255, 255);

            SDL_Texture* tex = ctx_.assets->GetTexture("player_sheet");

            if (tex)
            {
                sheet_.SetTexture(tex, frameW, frameH, cols, rows);
                sheetLoaded_ = true;
            }
        }

        if (!sheetLoaded_)
        {
            Log::Warn("Could not load assets/player_sheet.bmp. Falling back to rectangle.");
        }

        if (!sheetLoaded_)
            Log::Warn("Could not load assets/player_sheet.bmp. Falling back to rectangle.");

        // Walk animation: 8 frames at 10 fps (tweak later)
        anim_.SetFrames(walkStartCol_, walkFrameCount_, walkFps_);
    }
    else
    {
        Log::Warn("Renderer not available in context. Falling back to rectangle.");
    }

    // Load mech sheets
    mechLowerLoaded_ = false;
    mechUpperLoaded_ = false;

    if (ctx_.assets)
    {
        // White color key because the sheets came from transparent PNGs exported to BMP
        ctx_.assets->LoadTextureBMP("mech_lower", "assets/mechlowersheet.bmp", 255, 255, 255);
        ctx_.assets->LoadTextureBMP("mech_upper", "assets/mechuppersheet.bmp", 255, 255, 255);

        SDL_Texture* lowerTex = ctx_.assets->GetTexture("mech_lower");
        SDL_Texture* upperTex = ctx_.assets->GetTexture("mech_upper");

        if (lowerTex)
        {
            mechLowerSheet_.SetTexture(
                lowerTex,
                MECH_FRAME_SIZE,
                MECH_FRAME_SIZE,
                MECH_LOWER_FRAMES,
                MECH_LOWER_DIRS
            );
            mechLowerLoaded_ = true;
        }
        else
        {
            Log::Warn("Could not load assets/mechlowersheet.bmp.");
        }

        if (upperTex)
        {
            mechUpperSheet_.SetTexture(
                upperTex,
                MECH_FRAME_SIZE,
                MECH_FRAME_SIZE,
                10,
                9
            );
            mechUpperLoaded_ = true;
        }
        else
        {
            Log::Warn("Could not load assets/mechuppersheet.bmp. If this fails, re-export as 10x9 grid instead of 1x90 vertical.");
        }
    }

    // Build default lower-body frame map.
    // You can manually edit any individual entry later if the sheet generator misplaced frames.
    for (int dir = 0; dir < MECH_LOWER_DIRS; ++dir)
    {
        for (int frame = 0; frame < MECH_LOWER_FRAMES; ++frame)
        {
            mechLowerFrameMap_[dir][frame] = SDL_Rect{
                frame * MECH_FRAME_SIZE,
                dir * MECH_FRAME_SIZE,
                MECH_FRAME_SIZE,
                MECH_FRAME_SIZE
            };
        }
    }

    // Build default dual-muzzle map for turret.
    // These are source-frame pixel positions inside each 512x512 turret frame.
    // Can override any individual frame below.
    for (int frame = 0; frame < MECH_UPPER_FRAMES; ++frame)
    {
        const int mappedFrame = mechUpperClockwise_
            ? (MECH_UPPER_FRAMES - frame) % MECH_UPPER_FRAMES
            : frame;

        // +22 shifts ~90 degrees clockwise
        const int offsetFrame = (mappedFrame - 22) % MECH_UPPER_FRAMES;

        const double degrees = static_cast<double>(offsetFrame) * 4.0;

        // Tune these values after visually checking the muzzle dots.
        const double barrelLength = 110.0;
        const double barrelSideOffset = 105.0;

        // Format: mechMuzzleMap_[frame][gun] = SDL_Point{ x, y };
        mechMuzzleMap_[frame][0] = PointOnCircle(degrees, barrelLength, -barrelSideOffset);
        mechMuzzleMap_[frame][1] = PointOnCircle(degrees, barrelLength, barrelSideOffset);
    }

    obstacles_.clear();
    particles_.Clear();

    loadedEnemies_.clear();
    loadedDestructibles_.clear();

    // Try loading a level from file first
    LevelData loadedLevel;
    const bool levelLoaded = LevelIO::LoadFromFile(GamePaths::PlayableLevel, loadedLevel);

    if (levelLoaded)
    {
        worldW_ = loadedLevel.worldW;
        worldH_ = loadedLevel.worldH;

        worldPos_.x = static_cast<double>(loadedLevel.playerSpawn.x);
        worldPos_.y = static_cast<double>(loadedLevel.playerSpawn.y);

        obstacles_ = loadedLevel.obstacles;

        loadedEnemies_ = loadedLevel.enemies;
        loadedDestructibles_ = loadedLevel.destructibles;
    }
    else
    {
        Log::Warn(std::string("Could not load ") + GamePaths::PlayableLevel + ". Falling back to procedural obstacle generation.");
        // --- Tuning knobs ---
        const int cell = 320;        // spacing between obstacle "cells" (bigger = less dense)
        const int margin = 120;      // keep obstacles away from world edges
        const int maxPerRow = (worldW_ - 2 * margin) / cell;
        const int maxPerCol = (worldH_ - 2 * margin) / cell;

        // Deterministic hash (no RNG) -> returns 0..(mod-1)
        auto hash2 = [](int x, int y, int mod) -> int
            {
                // cheap integer hash
                unsigned v = static_cast<unsigned>(x * 73856093u) ^ static_cast<unsigned>(y * 19349663u);
                v ^= (v >> 13);
                v *= 1274126177u;
                return static_cast<int>(v % static_cast<unsigned>(mod));
            };

        // Helper to avoid spawning inside a protected "spawn area"
        SDL_Rect spawnSafe{ 0, 0, 0, 0 };
        {
            // keep a clear area around the initial player spawn
            const int safeW = 500;
            const int safeH = 350;
            const int sx = 200;   // if your spawn is different, adjust these two lines
            const int sy = 200;
            spawnSafe = SDL_Rect{ sx - safeW / 2, sy - safeH / 2, safeW, safeH };
        }

        auto overlaps = [](const SDL_Rect& a, const SDL_Rect& b) -> bool
            {
                return SDL_HasIntersection(&a, &b);
            };

        // Generating obstacles across the world
        for (int gy = 0; gy <= maxPerCol; ++gy)
        {
            for (int gx = 0; gx <= maxPerRow; ++gx)
            {
                // Probability gate (lower => fewer obstacles)
                // 0..99; place only if < threshold
                const int roll = hash2(gx, gy, 100);
                if (roll >= 30) // ~% filled;decrease for sparser, increase for denser
                    continue;

                // Base position in world
                int baseX = margin + gx * cell;
                int baseY = margin + gy * cell;

                // Jitter within the cell 
                int jx = hash2(gx + 11, gy + 7, 120) - 60;   // -60..+59
                int jy = hash2(gx + 19, gy + 3, 120) - 60;

                int x = baseX + jx;
                int y = baseY + jy;

                // Size variety
                int w = 70 + hash2(gx + 3, gy + 5, 140);     // 70..209
                int h = 40 + hash2(gx + 9, gy + 2, 110);     // 40..149

                // some tall pillars
                if (hash2(gx + 2, gy + 9, 10) == 0)
                {
                    w = 60 + hash2(gx + 5, gy + 1, 60);      // 60..119
                    h = 220 + hash2(gx + 7, gy + 4, 180);    // 220..399
                }

                SDL_Rect r{ x, y, w, h };

                // Keep inside world
                if (r.x < margin) r.x = margin;
                if (r.y < margin) r.y = margin;
                if (r.x + r.w > worldW_ - margin) r.x = worldW_ - margin - r.w;
                if (r.y + r.h > worldH_ - margin) r.y = worldH_ - margin - r.h;

                // Keep spawn area clear
                if (overlaps(r, spawnSafe))
                    continue;

                // Avoiding heavy overlap with existing obstacles 
                bool bad = false;
                for (const auto& o : obstacles_)
                {
                    if (SDL_HasIntersection(&r, &o))
                    {
                        bad = true;
                        break;
                    }
                }
                if (bad)
                    continue;

                obstacles_.push_back(r);
            }
        }
        
    }

    collision_.SetWorldSize(worldW_, worldH_);
    collision_.SetObstacles(obstacles_);

    bullets_.Clear();
    bullets_.SetWorldSize(worldW_, worldH_);
    bullets_.SetObstacles(&obstacles_);

    WeaponDefinition2D mechGun;
    mechGun.fireInterval = 0.08;
    mechGun.bulletSpeed = 1000.0;
    mechGun.bulletLife = 1.0;
    mechGun.bulletW = 8;
    mechGun.bulletH = 8;
    mechGun.bulletDamage = 1;
    mechGun.fireMode = WeaponFireMode::AlternatingMuzzles;

    primaryWeapon_.SetDefinition(mechGun);
    primaryWeapon_.Reset();

	// Player state
    playerHealth_.SetMax(100);
    playerEntity_.id = 1;
    playerEntity_.tag = EntityTag::Player;
    playerEntity_.layer = CollisionLayer2D::Player;
    playerEntity_.collisionMask = CollisionLayer2D::Enemy | CollisionLayer2D::Pickup | CollisionLayer2D::Trigger;
    playerEntity_.active = true;

    targets_.Clear();

	// Load destructibles from level file if available, otherwise fallback to test destructibles
    if (!loadedDestructibles_.empty())
    {
        for (const auto& d : loadedDestructibles_)
        {
            targets_.AddTarget(d.rect, d.hp);
        }
    }
    else
    {
        // Fallback destructible test targets
        targets_.AddTarget(SDL_Rect{ 900, 500, 48, 48 }, 3);
        targets_.AddTarget(SDL_Rect{ 1100, 700, 48, 48 }, 3);
        targets_.AddTarget(SDL_Rect{ 1400, 900, 48, 48 }, 3);
    }

    enemies_.Clear();

	// Load enemies from level file if available, otherwise fallback to test enemies
    if (!loadedEnemies_.empty())
    {
        for (const auto& e : loadedEnemies_)
        {
            enemies_.TryAddEnemy(e.rect, e.hp, &obstacles_);
        }
    }
    else
    {
        // Fallback test enemies
        enemies_.TryAddEnemy(SDL_Rect{ 800, 900, 56, 56 }, 5, &obstacles_);
        enemies_.TryAddEnemy(SDL_Rect{ 1200, 1100, 56, 56 }, 3, &obstacles_);
        enemies_.TryAddEnemy(SDL_Rect{ 1200, 1300, 56, 56 }, 3, &obstacles_);
    }

    levelWon_ = false;
}

void DemoScene::ResetDemo()
{
    worldPos_ = Vec2{ 10.0, 10.0 };

    playerHealth_.SetMax(100);

    enemies_.Clear();

    enemies_.TryAddEnemy(SDL_Rect{ 800, 900, 56, 56 }, 10, &obstacles_);
    enemies_.TryAddEnemy(SDL_Rect{ 500, 100, 56, 56 }, 3, &obstacles_);
    enemies_.TryAddEnemy(SDL_Rect{ 200, 300, 56, 56 }, 3, &obstacles_);
}

void DemoScene::OnExit()
{
    Log::Info("DemoScene exit");
}
