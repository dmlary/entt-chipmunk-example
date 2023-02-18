#include <cstdlib>
#include <entt/entt.hpp>

#include "asset_loader.hpp"
#include "components.hpp"
#include "image.hpp"
#include "log.hpp"
#include "physics.hpp"
#include "physics/body.hpp"
#include "physics/collision_type.hpp"
#include "physics/shape.hpp"
#include "render.hpp"
#include "tags.hpp"

using namespace entt::literals;

bool
asset_loader::load_scene(entt::registry& ecs)
{
    m_map_tileset = Image{ m_asset_dir / "map.png"};
    assert(m_map_tileset.valid());
    m_mob_tileset = Image{ m_asset_dir / "mob.png"};
    assert(m_mob_tileset.valid());

    // create the player
    entt::entity p = ecs.create();
    ecs.emplace<Scene>(p);
    ecs.emplace<HumanDescription>(p, "test player", "");
    ecs.emplace<render::Translate>(p, 0, 0, 1);
    ecs.emplace<render::Sprite>(p,
        render::Sprite{
            {16, 16},
            m_mob_tileset.sg_image(),
            m_mob_tileset.crop_transform(17, 5 * 16 + 5, 16, 16)
    });
    auto& body = ecs.emplace<physics::Body>(p);
    cpBodySetPosition(body, { 160, 96 });
    ecs.emplace<physics::Movable>(p, 1000.0f, 100.0f);
    auto& shape = ecs.emplace<physics::Box>(p, 15.0, 15.0, 0, p);
    cpShapeSetCollisionType(shape, physics::CT_Player);
    ecs.ctx().emplace_as<entt::entity>("player"_hs, p);

    // create an immovable wall
    entt::entity w = ecs.create();
    ecs.emplace<Scene>(w);
    ecs.emplace<HumanDescription>(w, "wall", "immovable wall");
    ecs.emplace<render::Translate>(w, 0, 0, 1); // will be updated from body
    ecs.emplace<render::Sprite>(w,
        render::Sprite{
            {16, 16},
            m_map_tileset.sg_image(),
            m_map_tileset.crop_transform(17 * 15, 17 * 8, 16, 16)
    });
    {
        auto& body = ecs.emplace<physics::Body>(w);
        cpBodySetType(body, CP_BODY_TYPE_STATIC);
        cpBodySetPosition(body, { 208, 96 });
        auto& shape = ecs.emplace<physics::Box>(w, 15, 15, 0, w);
        cpShapeSetCollisionType(shape, physics::CT_Object);
    }

    // create an openable door

    // create a single room of grass
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 21; x++) {
            entt::entity tile = ecs.create();
            ecs.emplace<Scene>(tile);
            ecs.emplace<render::Translate>(tile, x * 16 - 8, y * 16 - 8, 0);
            ecs.emplace<render::Sprite>(tile, glm::vec2{ 16, 16 },
                m_map_tileset.sg_image(),
                m_map_tileset.crop_transform(
                    17 * 5, (rand() % 2) * 17, 16, 16));
        }
    }

    // y-z sort all of the things we're gonna render
    ecs.sort<render::Translate>([](const auto& lhs, const auto& rhs) {
        return lhs.z == rhs.z ? lhs.v.y < rhs.v.y : lhs.z < rhs.z;
    });

    return true;
}

void
asset_loader::clear_scene(entt::registry& ecs)
{
    for (auto entity : ecs.view<Scene>()) {
        ecs.emplace<tags::Destroy>(entity);
    }
    ecs.ctx().erase<entt::entity>("player"_hs);
}

void
asset_loader::cleanup()
{
    m_map_tileset.reset();
    m_mob_tileset.reset();
}
