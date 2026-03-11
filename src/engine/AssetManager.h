#pragma once
#include <string>
#include <unordered_map>
#include <SDL.h>

class AssetManager
{
public:
    AssetManager() = default;
    ~AssetManager();

    // Non-copyable
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    void Init(SDL_Renderer* renderer);
    void Shutdown();

    // Loads a BMP into cache under the given id.
    // Returns true if loaded successfully, or if already loaded.
    bool LoadTextureBMP(const std::string& id, const std::string& path, Uint8 colorKeyR = 255, Uint8 colorKeyG = 0, Uint8 colorKeyB = 255);

    // Returns cached texture, or nullptr if not found.
    SDL_Texture* GetTexture(const std::string& id) const;

    bool HasTexture(const std::string& id) const;

    void UnloadTexture(const std::string& id);
    void UnloadAll();

private:
    SDL_Renderer* renderer_ = nullptr;
    std::unordered_map<std::string, SDL_Texture*> textures_;
};