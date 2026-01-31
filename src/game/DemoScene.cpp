#include "DemoScene.h"
#include "MenuScene.h"
#include "PauseScene.h"

#include "../engine/SceneManager.h"
#include "../engine/Input.h"
#include "../engine/Renderer.h"
#include "../engine/EngineContext.h"
#include "../engine/Log.h"
#include "../engine/InputMap.h"
#include "../engine/Actions.h"
#include "../engine/Texture2D.h"

#include <algorithm>
#include <memory>

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
        dirRow_ = DIR_TO_ROW[dirIndex];

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

    // Move in world space
    const Vec2 prev = worldPos_;

    worldPos_ += dir * (speed_ * dtSeconds);

    // Clamp to world bounds
    worldPos_.x = std::clamp(worldPos_.x, 0.0, static_cast<double>(worldW_ - w_));
    worldPos_.y = std::clamp(worldPos_.y, 0.0, static_cast<double>(worldH_ - h_));

    // Move in world space with collision 
    const double speedNow = speed_ * (dashing_ ? dashSpeedMult_ : 1.0);
    Vec2 desired = worldPos_ + dir * (speedNow * dtSeconds);

    // --- X axis ---
    worldPos_.x = desired.x;
    worldPos_.x = std::clamp(worldPos_.x, 0.0, static_cast<double>(worldW_ - w_));

    SDL_Rect feet = FeetRectWorld();
    for (const auto& o : obstacles_)
    {
        if (SDL_HasIntersection(&feet, &o))
        {
            // revert X movement
            worldPos_.x = prev.x;
            break;
        }
    }

    // --- Y axis ---
    worldPos_.y = desired.y;
    worldPos_.y = std::clamp(worldPos_.y, 0.0, static_cast<double>(worldH_ - h_));

    feet = FeetRectWorld();
    for (const auto& o : obstacles_)
    {
        if (SDL_HasIntersection(&feet, &o))
        {
            // revert Y movement
            worldPos_.y = prev.y;
            break;
        }
    }
    const bool moved = (worldPos_.x != prev.x) || (worldPos_.y != prev.y);
    moving_ = moving && moved;

    if (moving_)
        anim_.Update(dtSeconds, true);
    else
        anim_.Update(dtSeconds, false);

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


    // Dust spawn (only when actually moving)
    if (moving_)
    {
        dustSpawnAcc_ += dtSeconds;

        // spawn rate: 30 particles/sec (tweak later)
        const double spawnInterval = 1.0 / 30.0;

        while (dustSpawnAcc_ >= spawnInterval)
        {
            dustSpawnAcc_ -= spawnInterval;

            SDL_Rect feet = FeetRectWorld();
            const double fx = feet.x + feet.w * 0.5;
            const double fy = feet.y + feet.h - 30; // bottom of feet

            Particle p;
            p.pos = Vec2{ fx, fy };

            // small random-ish spread
            static int s = 0;
            const int m = s++ % 3;
            const double dir = (m == 0) ? -1.5 : (m == 1) ? 0.0 : 1.5;

            p.vel = Vec2{ 30.0 * dir, 20.0 }; // slight sideways + down
            p.maxLife = p.life = 0.25;        // quarter second
            p.size = 3;

            particles_.push_back(p);
        }
    }
    else
    {
        dustSpawnAcc_ = 0.0;
    }
    // Update particles
    for (auto& p : particles_)
    {
        p.life -= dtSeconds;
        if (p.life > 0.0)
        {
            p.pos += p.vel * dtSeconds;
            p.vel *= 0.85; // damp
        }
    }

    // Remove dead
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
            [](const Particle& p) { return p.life <= 0.0; }),
        particles_.end()
    );

    // Camera follow (center player)
    camPos_.x = worldPos_.x + w_ * 0.5 - ctx.logicalW * 0.5;
    camPos_.y = worldPos_.y + h_ * 0.5 - ctx.logicalH * 0.5;

    // Clamp camera so it doesn't show outside the world
    camPos_.x = std::clamp(camPos_.x, 0.0, static_cast<double>(worldW_ - ctx.logicalW));
    camPos_.y = std::clamp(camPos_.y, 0.0, static_cast<double>(worldH_ - ctx.logicalH));
}

void DemoScene::Render(Renderer& renderer, const EngineContext& ctx)
{
    (void)ctx;
    renderer.Clear(245, 245, 245);

    Vec2 cam = camPos_;

    if (shakeTimeLeft_ > 0.0)
    {
        // simple deterministic shake (no RNG)
        const int t = static_cast<int>(SDL_GetTicks() / 16);
        const double sx = (t % 2 == 0) ? -shakeStrength_ : shakeStrength_;
        const double sy = (t % 3 == 0) ? -shakeStrength_ : shakeStrength_;
        cam.x += sx;
        cam.y += sy;
    }

    const int cell = 64;
    const int majorEvery = 5;

    const int viewW = ctx.logicalW;
    const int viewH = ctx.logicalH;

    // Camera offset within a cell (0..cell-1)
    const int camX = static_cast<int>(std::floor(cam.x));
    const int camY = static_cast<int>(std::floor(cam.y));

    int offsetX = camX % cell;
    int offsetY = camY % cell;
    if (offsetX < 0) offsetX += cell;
    if (offsetY < 0) offsetY += cell;

    // Minor grid
    renderer.SetDrawColor(220, 220, 220, 255);
    for (int x = -offsetX; x <= viewW; x += cell)
        renderer.DrawLine(x, 0, x, viewH);

    for (int y = -offsetY; y <= viewH; y += cell)
        renderer.DrawLine(0, y, viewW, y);

    // Major grid
    const int major = cell * majorEvery;

    int offsetXMaj = camX % major;
    int offsetYMaj = camY % major;
    if (offsetXMaj < 0) offsetXMaj += major;
    if (offsetYMaj < 0) offsetYMaj += major;

    renderer.SetDrawColor(200, 200, 200, 255);
    for (int x = -offsetXMaj; x <= viewW; x += major)
        renderer.DrawLine(x, 0, x, viewH);

    for (int y = -offsetYMaj; y <= viewH; y += major)
        renderer.DrawLine(0, y, viewW, y);

    const int screenX = static_cast<int>(std::round(worldPos_.x - cam.x));
    const int screenY = static_cast<int>(std::round(worldPos_.y - cam.y));

    // Thick world boundary frame ("walls") 
    const int thickness = 32;

    // screen-space positions of world edges
    const int sxL = static_cast<int>(std::round(0.0 - cam.x));
    const int syT = static_cast<int>(std::round(0.0 - cam.y));
    const int sxR = static_cast<int>(std::round(static_cast<double>(worldW_) - cam.x));
    const int syB = static_cast<int>(std::round(static_cast<double>(worldH_) - cam.y));

    renderer.SetDrawColor(120, 120, 120, 255);

    // LEFT wall (inside the world): [sxL .. sxL+thickness)
    renderer.FillRect(sxL, 0, thickness, viewH);

    // TOP wall (inside the world): [syT .. syT+thickness)
    renderer.FillRect(0, syT, viewW, thickness);

    // RIGHT wall (inside): [sxR-thickness .. sxR)
    renderer.FillRect(sxR - thickness, 0, thickness, viewH);

    // BOTTOM wall (inside): [syB-thickness .. syB)
    renderer.FillRect(0, syB - thickness, viewW, thickness);

    // Optional darker edge lines
    renderer.SetDrawColor(70, 70, 70, 255);
    renderer.DrawLine(sxL + thickness, 0, sxL + thickness, viewH);
    renderer.DrawLine(sxR - thickness, 0, sxR - thickness, viewH);
    renderer.DrawLine(0, syT + thickness, viewW, syT + thickness);
    renderer.DrawLine(0, syB - thickness, viewW, syB - thickness);

    // Draw obstacles (world -> screen)
    renderer.SetDrawColor(60, 60, 60, 255);

    for (const auto& o : obstacles_)
    {
        const int ox = static_cast<int>(std::round(o.x - cam.x));
        const int oy = static_cast<int>(std::round(o.y - cam.y));
        renderer.DrawRect(ox, oy, o.w, o.h);
    }

    // Draw dust particles (world -> screen)
    renderer.SetDrawColor(120, 120, 120, 255);

    for (const auto& p : particles_)
    {
        const int px = static_cast<int>(std::round(p.pos.x - cam.x));
        const int py = static_cast<int>(std::round(p.pos.y - cam.y));

        renderer.FillRect(px, py, p.size, p.size);
    }

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

	// Draw player
    if (sheetLoaded_ && sheet_.Texture())
    {
        // Animator resets to frame 0 when idle (playing=false in Update).
        // idle pose set to column 3.
        const int col = moving_ ? anim_.CurrentCol() : idleCol_;
        const SDL_Rect src = sheet_.FrameRect(col, dirRow_);

        renderer.DrawTextureRegion(
            sheet_.Texture(),
            src,
            screenX,
            screenY,
            w_, h_
        );
    }
    else
    {
        renderer.DrawRect(
            screenX,
            screenY,
            w_, h_
        );
    }

    // HUD panel (screen space)
    SDL_SetRenderDrawBlendMode(renderer.Raw(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer.Raw(), 0, 0, 0, 140);
    SDL_Rect panel{ 10, 10, 160, 54 };
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

    // Speed bar (bottom of panel)
    const double speedNow = ctx.gameCfg.playerSpeed * (dashing_ ? dashSpeedMult_ : 1.0);
    const double maxShown = ctx.gameCfg.playerSpeed * dashSpeedMult_;
    const double t = std::clamp(speedNow / maxShown, 0.0, 1.0);

    renderer.SetDrawColor(200, 200, 200, 255);
    renderer.DrawRect(18, 40, 140, 8);

    renderer.SetDrawColor(120, 220, 120, 255);
    renderer.FillRect(18, 40, static_cast<int>(140 * t), 8);

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

        sheetLoaded_ = sheet_.LoadFromBMP(ctx_.renderer->Raw(), "assets/player_sheet.bmp",
            frameW, frameH, cols, rows);

        if (!sheetLoaded_)
            Log::Warn("Could not load assets/player_sheet.bmp. Falling back to rectangle.");

        // Walk animation: 8 frames at 10 fps (tweak later)
        anim_.SetFrames(walkStartCol_, walkFrameCount_, walkFps_);
    }
    else
    {
        Log::Warn("Renderer not available in context. Falling back to rectangle.");
    }

    obstacles_.clear();

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

void DemoScene::OnExit()
{
    Log::Info("DemoScene exit");
}
