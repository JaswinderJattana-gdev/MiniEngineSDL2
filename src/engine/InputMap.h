#pragma once
#include "Actions.h"
#include <SDL.h>
#include <unordered_map>
#include <vector>

class Input;

class InputMap
{
public:
    void Bind(Action action, SDL_Scancode key);
    bool Down(const Input& input, Action action) const;
    bool Pressed(const Input& input, Action action) const;
    bool Released(const Input& input, Action action) const;

private:
    const std::vector<SDL_Scancode>& KeysFor(Action action) const;

private:
    std::unordered_map<Action, std::vector<SDL_Scancode>> bindings_;
};
