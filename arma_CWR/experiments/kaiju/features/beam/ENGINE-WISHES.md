# Beam: what native help would improve it most

Written by the beam feature for the engine agent. Everything in `kj\beam\` works on the stock binary; each item below says what the script does today, why that falls short, and the smallest engine change that would fix it. Ordered by how much the player would notice. If one of these lands, name it in `features\engine\API.md` and the beam will use it behind a check and keep the script path.

## 1. A light the script can set (highest value)

Today: the furnace and impact lights are `Fire`-class objects from the `kjbeam` addon, one config class per brightness (`KJBeamGlowA` to `D`, `KJBeamGlowHit`, `KJBeamGlowRed`), switched with `inflame`. So the charge brightens in four steps, not smoothly, the colour is fixed per class, and every such light flickers by plus or minus 20 percent because `Fireplace::Simulate` does that (`World/Scene/Fireplace.cpp:38-46`). The flicker suits a furnace but cannot be turned off for the steady white of the beam.

Wish: a script light with settable colour, ambient and brightness, attachable to an object or a memory point. For example `light = kjLightCreate [pos]`, `light kjLightSet [[r,g,b], [ar,ag,ab], brightness]`, `light kjLightAttach [obj, "memoryPoint"]`, `kjLightDelete light`. Underneath it is `LightPointOnVehicle` plus `Scene::AddLight` (`World/Scene/Scene.cpp:294`), the same thing the campfire uses. Small.

Also wanted: a line light, or simply several point lights spaced along the beam, so the beam lights the ground and the house fronts under it. The script could place point lights itself if lights were cheap to create and move; today each would be another Fire object. Note the caps: 16 lights a scene, 7 an object (`Core/Config/EngineConfig.hpp:71`, `World/Scene/Scene.cpp:366`).

## 2. A beam primitive (a line drawn by the engine)

Today: the beam is up to 16 separate 50 m models (`KJBeamSeg`) that the script moves and turns every frame with `setPosASL` and `setVectorDirAndUp`. Limits:

- The pieces must be short because an object is filed under the grid cell of its origin and the draw pass and every query assume nothing reaches more than 25 m from it (`World/Simulation/Collisions.cpp:51-55`). So a long beam is many objects and 32 script commands a frame.
- It stops drawing past the object distance, two thirds of the view distance (`World/Scene/SceneDraw.cpp:404-407`). At the default 900 m view a beam is invisible from 600 m. The test raises the view distance to 3500 m to show it at 2 km.
- It is alpha blended, not additive, so it cannot bloom or brighten what is behind it. Where the glow planes cross other alpha surfaces (smoke, water) the sort order is per object.
- The width is fixed in the model. From 2 km the 0.8 m core is under a pixel and only the 7 m halo reads. A native line could keep a minimum width in pixels.

Wish: `kjBeamSet [from, to, coreWidth, haloWidth, [r,g,b], intensity]` and `kjBeamClear`, drawn in the transparent pass as two camera-facing quads (core and halo), additive, unfogged or lightly fogged, exempt from the object distance cut, with a minimum on-screen width of about two pixels. Medium. An even smaller first step that helps a lot: an "always draw, never cull by distance" flag for a vehicle type (the same flag the far-draw work for the creature needs), which the beam pieces would set in config.

## 3. Camera shake

Today: nothing. The engine has no camera shake at all (survey, section 6), and a script can only move a scripted camera, not the player's view. The moment the beam fires and the moments buildings fall are the ones that most need it.

Wish: `kjShake [pos, strength, seconds, radius]`, a decaying random offset on the camera transform that falls off with distance from `pos`. The beam would call it once at ignition (strong, long radius) and weakly while it burns for players near the line. Medium, and roar, crush and the footfalls want the same call.

## 4. Far draw

Same root as item 2. The furnace glow, the beam and the fires it leaves should be seen from across the island, at least at night. Particles (`drop`) are not cut at the object distance by `Scene.cpp:925-960`, but the smoke sources of the fires are (`EnableVisualEffects`, `World/Simulation/Simul.cpp:1179-1189`), and so are the beam pieces. Wish: a per-type exemption from the `objectsZ` cut for the beam pieces and the fire objects (config flag), drawn fogged.

## 5. Reading a memory point from script

Today: the script cannot ask where the creature's chest is. It drops an invisible particle on the memory point `zamerny` (the `drop` command accepts a point name) and lets the particle's "before destroy" script report where it died (`Graphics/Rendering/Effects/Smokes.cpp:2523-2535` and `:2118-2133`). That works but takes a frame or two, gives the rest pose and not the animated pose, and needs the fallback numbers `KJ_beamT_chestFwd` and `KJ_beamT_chestUp` when the point is missing.

Wish: `obj selectionPosition "name"` (model space) and `obj modelToWorld [x,y,z]`, or one call `obj kjPointASL "name"` that returns the animated world position. Small. With it the beam would leave from the real furnace even while he leans.

## 6. Damage with an owner, and a line query

Today: every death is `setDammage 1`. Nobody is credited, no `Hit` or `Killed` event names the creature, AI friends of the dead learn nothing through the damage path, and each call on a map object logs "SetDammage applied on non-EntityAI" (`World/Entities/Weapons/Dammage.cpp:443-447`). To find what the line touches, the script walks back along the last 144 m of the beam in 9 m steps and calls `nearestObjects` at each, ten times a second: at most 16 queries a tick, each a walk over a few 50 m grid cells in C++, plus a script loop over the hits.

Wish: one call, `kjBeamDamage [owner, from, to, radius]`, that walks the object grid along the segment, returns what it touched (or applies damage itself with `LocalDammage` and the creature as owner), and tells buildings from trees from fences (the script cannot: `typeOf` is empty for map objects). Small to medium. It would also let the fires go where buildings were, not just wherever the spacing rule puts them.

## 7. Burning objects natively

Today: a "fire" is a `Fire`-class object at the place (`KJBeamFlame`, `KJBeamFlameBig`, `KJBeamSmoke`), with the engine's own smoke source doing the particles. Trees and houses do not themselves burn or blacken; a destroyed house is the same model squashed.

Wish (optional, larger): a burnt tint on destroyed objects (a constant colour multiply set on the object would do), and a smoke source that can be attached to a map object so the flames sit on the roofline.

## 8. Heat haze

Today: pale low-alpha smoke puffs. There is no refraction effect.

Wish: a screen-space distortion sprite. Low priority; only worth it if the renderer already has a refraction pass.

## Handing the line of fire to the native fire spread

A native fire-spread simulation exists as another session's uncommitted work (worktree `sq-fire-spread`, on the graphics-remaster line; read only, nothing was copied or built from it). When it is merged into the kaiju engine the beam hands its fire over in one place and keeps the scripted after-fire as the fallback.

What that system offers, as read from `World/Simulation/FireSpreadWorld.hpp` and `.cpp` there (not run by me):

- `fireSpreadIgnite [x, y]` lights the fuel cell at a point; `fireSpreadIgnite [x, y, radius]` lights every cell in the radius. It returns the number of cells lit, so 0 means nothing there burns (sand, a road, water).
- The fire then spreads by itself over the terrain's fuel with the weather's wind and rain, burns out, chars the ground, trees and house walls, does burn damage, makes soldiers run from the flames, and its smoke blocks AI sight. It draws its own flames, smoke and lights from capped budgets.
- `fireSpreadSet ["m1h", fraction]` sets how dry the fine fuel is (dry spreads, damp does not); `["wind", [x, y]]`, `["flames", n]`, `["smoke", n]` too. `fireSpreadState [x, y]` answers "none", "unburnt", "burning" or "burnt". `fireSpreadStats`, `fireSpreadClear`.
- It also lights itself from ground explosions and from burning wrecks (`NotifyExplosion`, `NotifyBurning`), so the vehicles the beam wrecks would seed fires with no call from the beam.
- All of it is off, and none of the commands exist, unless the environment variable `POSEIDON_XP_FIRESPREAD` is set.

How the beam calls it. Every ignition point of the beam is created in one function, `KJ_beam_fnIgnite` in `kj\beam\fns.sqf` (marked "THE ONE PLACE A FIRE IS STARTED"), as `[[x, y], strength 0..1, why]` with why one of "trail", "wreck", "razed"; each is recorded in `KJ_beamIgnitions` as `[x, y, strength, time]`. On the trail there is one point per 8 m the impact travels (`KJ_beamT_igniteGap`).

- The mission (or the brain) sets `KJ_fireSpreadNative = true` when the engine has the commands. A script cannot ask whether a command exists, and naming an unknown command is a script error, so the flag has to come from outside and the call is compiled from a string: `call compile format ["fireSpreadIgnite [%1, %2, %3]", x, y, 2 + 6 * strength]`.
- If it returns 0 the scripted fire (`KJ_beam_fnSite`) is lit at that point as today, so a cut across sand or tarmac still burns.
- Nothing else in the beam changes. The scripted `KJBeamFlame` and `KJBeamSmoke` objects simply stop being created wherever the native fire takes the point.

Three things the fire-spread system would need for the beam to use it well, none of which I could check by running it:

1. A strength or heat argument on ignition. Today a point is lit or not, and whether it then spreads depends on fuel moisture. A beam should set light to damp grass too. Either `fireSpreadIgnite [x, y, radius, force]` that ignores moisture for the cells it lights, or the mission lowers `m1h` for the scene.
2. A line form, `fireSpreadIgniteLine [[x0, y0], [x1, y1], width]`, so the engine walks the cut and the script does not send one call per 8 m.
3. A registered marker the script can read, for example a nular `fireSpreadAvailable`, so the beam can find the system without the mission setting a flag.

## Small faults found on the way (cheap to fix)

- The game cannot load DXT5 or DXT3 PAAs: `Graphics/Rendering/Font/Pactext.cpp:1354-1359` has no case for DXT5, and the DXT3/DXT4 case has no `break` and falls into the failure ("Texture format"). A face with such a texture draws opaque.
- `PoseidonTools image convert -f ARGB4444` (and `ARGB1555`, `AI88`) writes a PAA the engine's loader rejects (`Pactext.cpp:478` "Checksum", `:910` "LZW Decode error"). So the only soft-alpha format that works end to end today is ARGB8888.
- `drop` with an array position: the source reads the height as above sea level, the game binary the kaiju tests run on (cryptid-rwdi, 2026-10-17) takes it as above the ground (measured: a particle dropped at 100 reported 100 m above ground that is 14.66 m above the sea). The beam measures this at run time; an engine build for the kaiju should settle which it is.
