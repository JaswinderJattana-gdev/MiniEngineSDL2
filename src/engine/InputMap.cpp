#include "InputMap.h"
#include "Input.h"

static const std::vector<SDL_Scancode> kEmptyKeys{};

void InputMap::Bind(Action action, SDL_Scancode key)
{
    bindings_[action].push_back(key);
}

const std::vector<SDL_Scancode>& InputMap::KeysFor(Action action) const
{
    auto it = bindings_.find(action);
    if (it == bindings_.end()) return kEmptyKeys;
    return it->second;
}

bool InputMap::Down(const Input& input, Action action) const
{
    for (SDL_Scancode sc : KeysFor(action))
        if (input.KeyDown(sc)) return true;
    return false;
}

bool InputMap::Pressed(const Input& input, Action action) const
{
    for (SDL_Scancode sc : KeysFor(action))
        if (input.KeyPressed(sc)) return true;
    return false;
}

bool InputMap::Released(const Input& input, Action action) const
{
    for (SDL_Scancode sc : KeysFor(action))
        if (input.KeyReleased(sc)) return true;
    return false;
}
