#pragma once

#include <filesystem>
#include <entt/fwd.hpp>
#include "image.hpp"

struct Scene {};

class asset_loader {
public:
    asset_loader(std::filesystem::path asset_dir)
        : m_asset_dir{asset_dir}
    {}

    void cleanup();
    bool load_scene(entt::registry&);
    void clear_scene(entt::registry&);

private:
    std::filesystem::path m_asset_dir;
    Image m_map_tileset{};
    Image m_mob_tileset{};
};
