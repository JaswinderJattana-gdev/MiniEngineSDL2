#include "LevelIO.h"

#include <fstream>
#include <sstream>
#include <string>

namespace LevelIO
{
    bool SaveToFile(const LevelData& level, const std::string& path)
    {
        std::ofstream out(path);
        if (!out.is_open())
            return false;

        out << "WORLD " << level.worldW << " " << level.worldH << "\n";
        out << "PLAYER_SPAWN " << level.playerSpawn.x << " " << level.playerSpawn.y << "\n";

        for (const auto& o : level.obstacles)
        {
            out << "OBSTACLE "
                << o.x << " "
                << o.y << " "
                << o.w << " "
                << o.h << "\n";
        }

        return true;
    }

    bool LoadFromFile(const std::string& path, LevelData& outLevel)
    {
        std::ifstream in(path);
        if (!in.is_open())
            return false;

        LevelData loaded{};

        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty())
                continue;

            std::istringstream iss(line);
            std::string tag;
            iss >> tag;

            if (tag == "WORLD")
            {
                iss >> loaded.worldW >> loaded.worldH;
            }
            else if (tag == "PLAYER_SPAWN")
            {
                iss >> loaded.playerSpawn.x >> loaded.playerSpawn.y;
            }
            else if (tag == "OBSTACLE")
            {
                SDL_Rect r{};
                iss >> r.x >> r.y >> r.w >> r.h;
                loaded.obstacles.push_back(r);
            }
            else
            {
                // Unknown line type: ignore for forward compatibility
            }
        }

        outLevel = loaded;
        return true;
    }
}