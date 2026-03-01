#include "Input.h"
#include <algorithm>

void Input::Init()
{
    // SDL_GetKeyboardState returns a pointer managed by SDL.
    keys_ = SDL_GetKeyboardState(&keyCount_);
    prevKeys_.assign(keyCount_, 0);

    // Initialize previous state to current so we don't get false "pressed"
    std::copy(keys_, keys_ + keyCount_, prevKeys_.begin());
}

void Input::BeginFrame()
{
    quitRequested_ = false;

    // Snapshot previous keyboard state
    std::copy(keys_, keys_ + keyCount_, prevKeys_.begin());

    // Ensure SDL updates internal input state (safe even if no events)
    SDL_PumpEvents();
    RefreshKeyboardState();
}

void Input::ProcessEvent(const SDL_Event& e)
{
    if (e.type == SDL_QUIT)
    {
        quitRequested_ = true;
        return;
    }

    if (e.type == SDL_WINDOWEVENT)
    {
        if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) hasFocus_ = true;
        if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)   hasFocus_ = false;
    }
}

void Input::RefreshKeyboardState()
{
    // SDL may change the pointer over time; refresh it just in case
    keys_ = SDL_GetKeyboardState(&keyCount_);
    if (static_cast<int>(prevKeys_.size()) != keyCount_)
        prevKeys_.assign(keyCount_, 0);
}

bool Input::KeyDown(SDL_Scancode sc) const
{
    if (!keys_ || sc >= keyCount_) return false;
    return keys_[sc] != 0;
}

bool Input::KeyPressed(SDL_Scancode sc) const
{
    if (!keys_ || sc >= keyCount_) return false;
    return (keys_[sc] != 0) && (prevKeys_[sc] == 0);
}

bool Input::KeyReleased(SDL_Scancode sc) const
{
    if (!keys_ || sc >= keyCount_) return false;
    return (keys_[sc] == 0) && (prevKeys_[sc] != 0);
}
