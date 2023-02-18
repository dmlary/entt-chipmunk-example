## Sample integration of EnTT + Chipmunk2d + ImGui + sokol
This repo contains a rough integration between EnTT & Chipmunk2d for collision
detection, along with an ImGui-based entity editor for Chipmunk2d components.

### Features
* Chipmunk2D collision detection
* ImGui-based entity editing
* System/frame stepping

https://user-images.githubusercontent.com/857742/219883858-b0f1ce9c-9979-481a-8add-35476376c6fa.mov

### Assets
Graphics assets from [kenney.nl](https://www.kenney.nl/)

## Dependencies
* [entt](https://github.com/skypjack/entt)
* [Chipmunk2d](https://github.com/slembcke/Chipmunk2D)
* [Dear ImGui](https://github.com/ocornut/imgui)
* [sokol](https://github.com/floooh/sokol)
* [sokol-tools](https://github.com/floooh/sokol-tools)

## Build instructions
Note: This project builds in my environment, but I make no guarantees that
it'll work anywhere else.  This project is really just intended as a code
reference.

Note 2: Oh yea, definitely a pain in the butt because the shaders require
`sokol-shdc` from sokol-tools to build

```txt
cmake -S . -B ./build -G Ninja
ninja -C ./build
```

## Code overview
What follows are rough descriptions of how the different pieces are integrated
with EnTT.

### Systems
There's no generic system implementation in EnTT.  Coming from
[flecs](https://github.com/SanderMertens/flecs) I implemented something similar
to their System interface.  Each system has a set of components to match
(exclusion not currently implemented), and calls a function with the view of
those components and the EnTT registry.

Implementation: [src/system.hpp](src/system.hpp)

Example usage: [src/render/plugin.cpp](src/render/plugin.cpp#L177,L193)

### Chipmunk2d
Chipmunk2d makes extensive use of pointers between individual structures
(`cpBody` points at `cpSpace`, and `cpShape`).  Instead of manually managing
the memory for each structure, we make use of EnTT's
[stable pointer support](src/physics/body.hpp#L17).

Each Chipmunk2d structure used in this example is wrapped in a component class
that is added to EnTT.  The physics shapes are the ugliest right now, with
`Box` wrapping a `cpPolyShape` very unsafely.

Generally, as `Body` and `Shape` are added to entities, we have observers that
add them to the `Space` (wrapping `cpSpace`) singleton stored in the EnTT
registry context.  We also have another set of observers to remove them from
the `cpSpace` on destruction.

* [space creation](src/physics/plugin.cpp#L100)
* [observer registration](src/physics/plugin.cpp#L74)
* [observer implementation](src/physics/plugin.cpp#L25:L33)

#### Movement
Movement is handled poorly in this example, the combination of `Movable` and
`Accelerate` components result in `cpBodySetVelocity()` being regularly called
on a `Body` until it reaches a specific velocity.
There's also a `Destination` component to stop `Body`s once they reach
the proper coords.

I would not recommend using this implementation as a reference for how to
implement movement.  Chipmunk2D does offer a pivot-based movement system, but
it's too "physics-realistic" for top-down tile-based movement.

Overall, the movement system works for this demonstration, so I'll leave it as
it is.

#### Collision handling
We register a [wildcard collision handler](src/physics/plugin.cpp#L103:L126)
for the player entity that sets a `Collision` component on both entities
involved with the collision.  This handler is called anytime a collision occurs
while we're [stepping the physics space](src/physics/plugin.cpp#L143:L153).

After the physics space has been stepped, we do collision handling on any
entity that collided, and has `Destination` & `Body` components
([system here](src/physics/plugin.cpp#L155:L171)).

#### Multiple shapes for a single Body
EnTT doesn't natively support relationships, so the `Shape<T>` component has a
`parent` entity relationship.  I'm honestly not a huge fan of this arrangement,
but I don't want to clean it up right now.

The result of this architecture is that I had to create additional systems to
prune any orphaned `Shape<T>` components once their parent entity is destroyed
([here](src/physics/plugin.cpp#L173:L28)).

### ImGui
The sokol imgui integration is one of the soothest I've seen.

Building on top of that, there is a system that runs the `Draw::draw` function
for any entity with the `Draw` component.

* [imgui system](src/imgui.cpp#L72:L109)
* [`Draw` component example](src/imgui.cpp#L135:L167)

### Entity Editor
The entity editor is based off
[imgui_entt_entity_editor](https://github.com/Green-Sky/imgui_entt_entity_editor).
I didn't go with this library because:
* drag-n-drop selection mechanism not intuitive
* no support for per-entity editor windows

That said, imgui_entt_entity_editor has component-based filtering of the entity
list, and the ability to add/remove components, and destroy entities entirely.
These are very necessary functions that are absent in this project.

Components are registered with the entity editor, along with their individual
draw functions.  When an entity is opened in the editor, the editor checks for
the presence of the component on the entity, and if present, calls the
component-specific draw function ([source](src/entity_editor.cpp#L19:L26)).

Most component draw functions are [simple](src/entity_editor.cpp#L92:L116), but
some of the Chipmunk2D components require
[more complex implementations](src/physics.cpp#L115:L127).  This happens
because the `cpBody` set functions modify more than a single value in the
structure.

### Source files with descriptions
```txt
src
├── CMakeLists.txt
├── asset_loader.cpp    # basic asset loader
├── asset_loader.hpp
├── components.hpp      # has only HumanDescription component
├── entity_editor.cpp   # entity-editor
├── entity_editor.hpp
├── fmt                 # formatting helpers for libfmt
│   ├── chipmunk.hpp
│   ├── entt.hpp
│   ├── glm.hpp
│   ├── sokol.hpp
│   └── std.hpp
├── image.cpp           # loader for images
├── image.hpp
├── imgui.cpp           # ImGui systems & components
├── imgui.hpp
├── input.cpp           # input system
├── input.hpp
├── log.cpp             # basic macros & helpers for spdlog
├── log.hpp
├── main.cpp            # main, uses sokol_app
├── physics
│   ├── body.cpp        # chipmunk2d cpBody wrapper
│   ├── body.hpp
│   ├── collision_type.cpp  # collision types
│   ├── collision_type.hpp
│   ├── debug_draw.cpp  # system for drawing debug shapes for chipmunk2d
│   ├── debug_draw.hpp
│   ├── plugin.cpp      # physics systems
│   ├── plugin.hpp
│   ├── shape.hpp       # chipmunk2d cpSegmentShape, cpPolyShape wrapper
│   ├── space.cpp       # chipmunk2d cpSpace wrapper
│   └── space.hpp
├── physics.cpp         # cardinal movement helpers & snap-to-grid
├── physics.hpp
├── render              # most of this is PoC code, only look at plugin
│   ├── buffer.hpp
│   ├── line_renderer.cpp   # basic line renderer
│   ├── line_renderer.hpp
│   ├── plugin.cpp      # render systems & camera management
│   ├── plugin.hpp
│   ├── quad_renderer.cpp   # basic textured quad renderer
│   ├── quad_renderer.hpp
│   └── rgba.hpp
├── render.hpp          # render components
├── shaders             # shaders, sokol-shdc builds these
│   ├── CMakeLists.txt
│   ├── line.glsl
│   └── quad.glsl
├── system.hpp          # generic system implementation for entt
└── tags.hpp            # tag components
```

## Why is this a public repo?
When I started looking at ECS libraries, and Chipmunk2d, I was unable to find
any deep examples integrating the two.  So when I finally got a basic
implementation working I decided to share it.

## Contribution
Honestly, fork this repo, create a better version, and leave it public.
I think there's more value in seeing multiple implementations versus a single
"correct" one.

