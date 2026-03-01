#pragma once

struct EngineConfig
{
    // Window
    const char* windowTitle = "PROOF_BUILD_271";
    int windowW = 800;
    int windowH = 450;
    bool resizable = true;

    bool useVsync = true;
    bool useHardwareAcceleration = true;

    int logicalW = 800;
    int logicalH = 450;
    bool useLogicalSize = true;

    // Timing
    double fixedDt = 1.0 / 60.0;   // 60 Hz update
    double maxFrameTime = 0.25;    // clamp hitches (seconds)
};

struct GameConfig
{
    // Player rectangle
    int playerW = 138;
    int playerH = 138;
    double playerSpeed = 200.0; // pixels/sec
};
