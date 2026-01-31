#pragma once
#include <SDL.h>
#include <vector>

class Input
{
public:
    void Init();
    void BeginFrame();                 // call once per frame (before polling events is fine)
    void ProcessEvent(const SDL_Event& e);

    bool QuitRequested() const { return quitRequested_; }
    bool HasFocus() const { return hasFocus_; }

    // Key queries (scancodes are layout-independent, good for games)
    bool KeyDown(SDL_Scancode sc) const;
    bool KeyPressed(SDL_Scancode sc) const;   // went down this frame
    bool KeyReleased(SDL_Scancode sc) const;  // went up this frame

private:
    void RefreshKeyboardState();

private:
    bool quitRequested_ = false;
    bool hasFocus_ = true;

    int keyCount_ = 0;
    const Uint8* keys_ = nullptr;             // SDL-owned pointer
    std::vector<Uint8> prevKeys_;             // our copy
};
