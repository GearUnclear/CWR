#pragma once

// A gutted house's shell (the fire spread's buildings; the shell geometry
// agent). When a house's roof burns and falls in, what stands is what a real
// masonry house leaves: its walls at full height, the gables over them, the
// chimney as a brick pier from the ground floor up, and nothing of timber
// above the eaves or between the floors. The house models (Data3D) know none
// of this: their chimney stacks start at the roof's surface, their dormers,
// barge boards, rafters and attic floors hang off the roof, and their walls
// are single-sided. So each house shape's faces are sorted once by where they
// sit (PlanBurntShell), and two copies of the shape are made from that
// (BurntShellShape): the first adds each chimney's pier (copies of its sides
// stepped down to the ground floor) and the walls' inner sides (so a roofless
// house shows its far walls from above and through its gables), the second
// also hides everything the fire took. A burning house wears the first while
// its roof falls in and the second once it is down; the shadow pass draws the
// same faces, so the gone roof casts no shadow.
//
// Nothing here runs while the fire spread is off (FireSpreadWorld calls it).

#include <Poseidon/Foundation/Types/Pointers.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace Poseidon
{
class LODShapeWithShadow;
}

namespace Poseidon::firespread
{

// One face of a house model's visual level, in model space (y up): its
// corners (a triangle or a quad), its outward normal, its texture's file
// name (lower case, no folder), and whether the model already hides it (a
// proxy's placeholder: left as it is).
struct ShellFace
{
    float p[4][3] = {};
    int n = 3;
    float nrm[3] = {0.0f, 1.0f, 0.0f};
    std::string tex;
    bool hidden = false;
};

// Where a face sits in the burnt shell. Kept: Wall (an upright face reaching
// below its eaves, or a flat one under them: sills, the ground floor),
// Carried (an upright face above the eaves that continues a wall below it in
// the same plane: a gable, a chimney built into a wall), Stack (a chimney
// stack standing on the roof: kept and carried down by its pier). Gone:
// Roof (what is seen from straight above), Attic (anything flat over the
// eaves: the roof's underside, rafters, ceilings and floors under it),
// Loose (an upright part above the eaves with nothing under it: dormers,
// barge boards, trusses, a bell turret), Floor (an upper floor, a ceiling,
// stairs), Timber (by its texture's name: glass, frames, doors, shutters,
// railings, boards and beams above the ground floor).
enum class ShellPart : std::uint8_t
{
    Wall,
    Carried,
    Stack,
    Roof,
    Attic,
    Loose,
    Floor,
    Timber,
};

constexpr bool ShellKeeps(ShellPart p)
{
    return p == ShellPart::Wall || p == ShellPart::Carried || p == ShellPart::Stack;
}

// A chimney stack's pier: the faces of the stack's lowest band of sides
// (indices into the faces), copied `copies` times, each `step` metres lower
// than the last, down to the ground floor.
struct ShellPier
{
    std::vector<int> band;
    float step = 0.0f;
    int copies = 0;
};

struct ShellPlan
{
    bool ok = false;             // a roof and walls were found (else the shape keeps its stock look)
    std::vector<ShellPart> part; // one for each face
    float footY = 0.0f, eavesY = 0.0f, ridgeY = 0.0f, groundY = 0.0f;
    std::vector<ShellPier> piers;
    std::vector<int> inner;                      // kept outward faces whose inner side is added
    float wallBox[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // the walls' plan: x0, x1, z0, z1
};

// Sorts a house level's faces. A church (`church`) has no chimney: anything
// small standing on its roof is a timber turret and goes.
ShellPlan PlanBurntShell(const std::vector<ShellFace>& faces, bool church);

// ---- v2: the broken tops ----
// A burnt gable or wall top is not a clean card: its masonry stands about
// 0.45 m thick, its edges broken (0.1 to 0.3 m along a wall top with one or
// two deeper bites where a tie beam tore out). The owner chose (option 3)
// that every gable is cut down to a short ragged stump, no tall triangle
// left, and that now and then a house keeps most of one gable as a broken
// piece, its purlins' pockets left as square holes (the eleventh answer).
// Each outer wall's plane (its kept upright faces, the gable over it
// included) is cut under its lowest top (kCapDepthM, or lower under a deep
// bite); what stood above the cut is rebuilt as masonry as thick as the wall,
// standing to its broken line (ShellTopsStyle) and keeping the wall's
// openings.
struct ShellQuad
{
    float p[4][3] = {};  // its corners (model space), round it
    float tp[4][3] = {}; // the point on its source face's plane whose texture each corner takes
    float nrm[3] = {};   // its outward normal
    int src = -1;        // the kept face whose texture (and texture plane) it takes
    int n = 4;           // its corners: 4, or 3 (a triangle: p[3] unused)
};
struct ShellTops
{
    std::vector<int> hide;                                        // kept faces wholly above their wall's cut: rebuilt
    std::vector<std::pair<int, float>> clip;                      // kept faces across it: their part under y stays
    std::vector<ShellQuad> quads;                                 // the rebuilt masonry
    int walls = 0, gables = 0, stumps = 0, tall = 0, pockets = 0; // gables: stumps + tall
};
// How the tops break (the tops agent). The owner saw the stepped columns
// (Columns: v2, merge 16) and wrote "it looks like minecraft voxels": steps
// of one width read as voxels and battlements. Real broken masonry breaks
// along an irregular, mostly sloping line, so the tops now follow a polyline,
// with the wall's thickness laid over it as a cap, a little uneven across it:
// - Ragged: stones lost in clusters, short slopes and shallow notches of
//   0.1 to 0.3 m at varying lengths, a rare short drop, one or two deeper
//   bites of 0.5 to 1 m on a long wall, the corners holding best; gable
//   stumps broken along one or two diagonal fracture lines, now and then one
//   gable kept tall with an irregular stepped slope and its purlin pockets.
// - Restrained: a near-straight cut a little under the eaves with a gentle
//   undulation (no steps, no bites); gables cut low on a gentle diagonal; the
//   soot band at the wall top does most of the work.
// POSEIDON_XP_FIRESPREAD_SHELLTOPS (0 Columns, 1 Ragged, 2 Restrained; read
// once) picks one, else Ragged.
enum class ShellTopsStyle : std::uint8_t
{
    Columns = 0,
    Ragged = 1,
    Restrained = 2,
};
ShellTopsStyle ShellTopsStyleNow();
// The broken tops of a level the plan has sorted. `seed` draws the breaks
// (a building's own, so no two burnt houses match); the same seed breaks a
// wall the same way in every level. `coarse` (a house's coarser levels): the
// line's smallest bends and the cap's middle bend left out.
ShellTops PlanBurntTops(const std::vector<ShellFace>& faces, const ShellPlan& plan, std::uint32_t seed,
                        ShellTopsStyle style = ShellTopsStyleNow(), bool coarse = false);

// Edits a private copy of a house shape (a LODShapeWithShadow copy no object
// wears yet) in place: on each visual level the rule finds a roof on, adds
// each chimney's pier and the kept walls' inner sides and, with `hideGone`,
// hides what the fire took (and the contents' proxies that burn or would hang
// where the upper floor was). Plans on the level's faces as they are, so call
// it before adding faces of another kind (rubble). Leaves the geometry,
// fire and view levels alone.
struct ShellStats
{
    int levels = 0, piers = 0, pierFaces = 0, gone = 0, inner = 0, contents = 0;
    int walls = 0, gables = 0, stumps = 0, pockets = 0, masonryFaces = 0; // v2: the broken tops
    float footY = 0.0f, eavesY = 0.0f, ridgeY = 0.0f;                     // the first shell level's (model space)
};
ShellStats AddBurntShellVisual(LODShapeWithShadow* shape, bool church, bool hideGone, std::uint32_t seed = 0);

// The burnt shell of a house shape (built once for each shape and kept):
// stage 0 with its piers and inner walls added, stage 1 with what the fire
// took hidden as well. nullptr when no level of it has a roof the rule finds
// (the building keeps the stock rule). Stage 1 is built for each `seed` (a
// building's own: its broken tops); stage 0 is shared. A shell given as `src` returns its
// own family's stage. Only weak links are kept: the caller holds the shell
// (an object wearing it).
Ref<LODShapeWithShadow> BurntShellShape(LODShapeWithShadow* src, bool church, int stage, std::uint32_t seed = 0);

// Whether `shape` is one of the burnt shells.
bool IsBurntShell(const LODShapeWithShadow* shape);

} // namespace Poseidon::firespread
