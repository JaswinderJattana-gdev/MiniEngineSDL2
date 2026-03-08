#pragma once
#include <string>
#include "LevelData.h"

namespace LevelIO
{
    bool SaveToFile(const LevelData& level, const std::string& path);
    bool LoadFromFile(const std::string& path, LevelData& outLevel);
}