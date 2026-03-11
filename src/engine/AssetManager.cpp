#include "AssetManager.h"

#include <SDL.h>

AssetManager::~AssetManager()
{
    Shutdown();
}

void AssetManager::Init(SDL_Renderer* renderer)
{
    renderer_ = renderer;
}

void AssetManager::Shutdown()
{
    UnloadAll();
    renderer_ = nullptr;
}

bool AssetManager::LoadTextureBMP(const std::string& id, const std::string& path, Uint8 colorKeyR, Uint8 colorKeyG, Uint8 colorKeyB)
{
    if (id.empty() || path.empty() || renderer_ == nullptr)
        return false;

    // Already loaded
    if (textures_.find(id) != textures_.end())
        return true;

    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (!surface)
        return false;

    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, colorKeyR, colorKeyG, colorKeyB));

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_FreeSurface(surface);

    if (!texture)
        return false;

    textures_[id] = texture;
    return true;
}

SDL_Texture* AssetManager::GetTexture(const std::string& id) const
{
    auto it = textures_.find(id);
    if (it == textures_.end())
        return nullptr;

    return it->second;
}

bool AssetManager::HasTexture(const std::string& id) const
{
    return textures_.find(id) != textures_.end();
}

void AssetManager::UnloadTexture(const std::string& id)
{
    auto it = textures_.find(id);
    if (it == textures_.end())
        return;

    if (it->second)
        SDL_DestroyTexture(it->second);

    textures_.erase(it);
}

void AssetManager::UnloadAll()
{
    for (auto& kv : textures_)
    {
        if (kv.second)
            SDL_DestroyTexture(kv.second);
    }
    textures_.clear();
}