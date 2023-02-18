#pragma once

#include <entt/fwd.hpp>
#include <entt/entity/entity.hpp>

/// generic system definition
struct System {
    enum Stage : uint32_t {
        input       = 500,
        update      = 1000,
        draw        = 2000,
        draw_debug  = 2500,
        render      = 3000,
        imgui_draw  = 4000,
        flush_frame = 6000,
        cleanup     = 0xffffffff,
    };

    /// helper type so we don't have to type this out everywhere
    template <typename... Args>
    using View = entt::view<entt::get_t<Args...>, entt::exclude_t<>>;

    /// config for a system without a view
    template <typename... Args>
    struct Config {
        const char* name{ nullptr };
        bool enabled{ true };
        bool always_run{ false };
        unsigned stage;
        std::function<void(entt::registry&, float)> handler;
    };

    /// config for a system with a view
    template <typename T, typename... Args>
    struct Config<T, Args...> {
        const char* name{ nullptr };
        bool enabled{ true };
        bool always_run{ false };
        unsigned stage;
        std::function<void(entt::registry&, View<T, Args...>&, float)> handler;
    };

    System(){};
    System(const System&) = default;

    /// constructor with view config
    System(const Config<>& cfg)
        : name{ cfg.name }
        , stage{ cfg.stage }
        , enabled{ cfg.enabled }
        , always_run{ cfg.always_run }
    {
        run = [handler = cfg.handler](
                  System& system, entt::registry& reg, float delta) {
            auto start = std::chrono::high_resolution_clock::now();
            handler(reg, delta);
            auto elapsed = std::chrono::high_resolution_clock::now() - start;

            if (system.perf.max < elapsed) {
                system.perf.max = elapsed;
            }
            system.perf.last = elapsed;
        };
    }

    /// constructor with view config
    template <typename... Args>
    System(const Config<Args...>& cfg)
        : name{ cfg.name }
        , stage{ cfg.stage }
        , enabled{ cfg.enabled }
        , always_run{ cfg.always_run }
    {
        run = [handler = cfg.handler](
                  System& system, entt::registry& reg, float delta) {
            auto view = reg.view<Args...>();
            // system.perf.entities = reg.storage<Args...>().size();
            system.perf.entities = std::distance(view.begin(), view.end());

            auto start = std::chrono::high_resolution_clock::now();
            handler(reg, view, delta);
            auto elapsed = std::chrono::high_resolution_clock::now() - start;

            if (system.perf.max < elapsed) {
                system.perf.max = elapsed;
            }
            system.perf.last = elapsed;
        };
    }

    /// human-readable name of the system
    const char* name{ nullptr };

    /// systems are ordered by stage before execution
    unsigned stage{ update };

    /// set to false to disable the system
    bool enabled{ true };

    /// set to true to ensure system runs even during step-mode
    bool always_run{ false };

    /// callback to run the function with the given registry and time delta
    std::function<void(System&, entt::registry&, float)> run;

    /// performance tracking information
    struct {
        size_t entities{ 0 };
        std::chrono::nanoseconds last{ 0 };
        std::chrono::nanoseconds max{ 0 };
    } perf;
};

/// used to maintain step-mode state
struct system_step_state {
    bool enabled{false};    // set to true to enable step mode
    bool frame{false};      // set to true to run all systems until next frame
    bool step{false};       // set to true to run the next system
    entt::entity next_system{entt::null};   // next system to be run
};
