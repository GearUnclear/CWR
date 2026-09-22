#include <catch2/catch_test_macros.hpp>
#include <Poseidon/UI/Map/MapLabelLayout.hpp>

// Map label collision rules (UI/Map/MapLabelLayout.hpp), the geometry behind
// CStaticMap::ReserveMarkerLabels / DrawName:
//   * a Guerrilla zone marker sits on the exact Names anchor of the town it
//     seeds, so before this the island name and the marker label ("Houdan"
//     under "Houdan - support 20/60") overprinted each other on one baseline;
//   * a bare flag (unrevealed zone) covered the first letters of the name;
//   * marker labels had no thinning at all, so neighbouring zone labels ran
//     into each other one zoom step out.

using Poseidon::MapLabelLayout;
using Poseidon::Rect2DFloat;

TEST_CASE("MapLabelLayout: rectangles collide on overlap, not on touch", "[ui][map][labels]")
{
    // binary-exact edges so "touching" really is equality, not float noise
    Rect2DFloat a(0.125f, 0.125f, 0.25f, 0.03125f);
    CHECK(MapLabelLayout::Intersects(a, Rect2DFloat(0.25f, 0.140625f, 0.125f, 0.03125f)));
    CHECK(MapLabelLayout::Intersects(a, Rect2DFloat(0.0625f, 0.0625f, 0.5f, 0.125f))); // contains
    CHECK_FALSE(MapLabelLayout::Intersects(a, Rect2DFloat(0.375f, 0.125f, 0.125f, 0.03125f))); // touches right edge
    CHECK_FALSE(MapLabelLayout::Intersects(a, Rect2DFloat(0.125f, 0.15625f, 0.25f, 0.03125f))); // touches bottom
    CHECK_FALSE(MapLabelLayout::Intersects(a, Rect2DFloat(0.125f, 0.25f, 0.25f, 0.03125f)));
}

TEST_CASE("MapLabelLayout: first label claims, a colliding later one is thinned", "[ui][map][labels]")
{
    MapLabelLayout layout;
    // "Camp" then "Litani Checkpoint - securing 40%" 500 m east on the same
    // northing: Camp's label is short, so both fit...
    Rect2DFloat camp(0.500f, 0.490f, 0.030f, 0.020f);
    Rect2DFloat checkpoint(0.560f, 0.490f, 0.240f, 0.020f);
    REQUIRE(layout.ClaimLabel(camp));
    REQUIRE(layout.ClaimLabel(checkpoint));
    CHECK(layout.LabelCount() == 2);

    // ...but a third zone label starting under the Checkpoint's text does not
    // get drawn; nothing is recorded for it, so a fourth one further along
    // is judged against the two real labels only
    Rect2DFloat barracks(0.700f, 0.495f, 0.200f, 0.020f);
    CHECK_FALSE(layout.ClaimLabel(barracks));
    CHECK(layout.LabelCount() == 2);
    Rect2DFloat farTown(0.820f, 0.495f, 0.100f, 0.020f);
    CHECK(layout.ClaimLabel(farTown));
    CHECK(layout.LabelCount() == 3);
}

TEST_CASE("MapLabelLayout: an island name under a marker label is blocked", "[ui][map][labels]")
{
    MapLabelLayout layout;
    // marker on the Houdan anchor (0.5, 0.5): flag 32*0.6 px wide, label to
    // its right, vertically centred on the anchor
    float flagW = 32 * 0.6f / 640;
    Rect2DFloat markerLabel(0.5f + 0.5f * flagW, 0.5f - 0.008f, 0.120f, 0.016f);
    REQUIRE(layout.ClaimLabel(markerLabel));

    // the island's own "Houdan" starts at the anchor on the same baseline
    Rect2DFloat name(0.5f, 0.5f - 0.010f, 0.040f, 0.020f);
    CHECK(layout.Blocked(name));
    CHECK_FALSE(layout.ClaimLabel(name));

    // a name a screen-tenth further south is untouched
    Rect2DFloat other(0.5f, 0.6f - 0.010f, 0.040f, 0.020f);
    CHECK(layout.ClaimLabel(other));
}

TEST_CASE("MapLabelLayout: a bare icon on the anchor slides the name to its right edge", "[ui][map][labels]")
{
    MapLabelLayout layout;
    float flagW = 32 * 0.6f / 640;
    float flagH = 32 * 0.6f / 480;
    Rect2DFloat flag(0.5f - 0.5f * flagW, 0.5f - 0.5f * flagH, flagW, flagH);
    layout.AddIcon(flag);
    CHECK(layout.IconCount() == 1);

    Rect2DFloat name(0.5f, 0.5f - 0.010f, 0.040f, 0.020f);
    float right = -1;
    REQUIRE(layout.IconRightEdge(name, right));
    CHECK(right == 0.5f + 0.5f * flagW);

    // no icon under it: untouched, and the out-parameter is left alone
    Rect2DFloat away(0.7f, 0.5f - 0.010f, 0.040f, 0.020f);
    right = -1;
    CHECK_FALSE(layout.IconRightEdge(away, right));
    CHECK(right == -1);

    // two icons overlapping the name: the right-most edge wins
    Rect2DFloat second(0.51f, 0.5f - 0.5f * flagH, flagW, flagH);
    layout.AddIcon(second);
    REQUIRE(layout.IconRightEdge(name, right));
    CHECK(right == 0.51f + flagW);

    // icons never block a label on their own - only labels do
    CHECK_FALSE(layout.Blocked(name));
    CHECK(layout.ClaimLabel(name));
}

TEST_CASE("MapLabelLayout: the blocking label and its text are recoverable", "[ui][map][labels]")
{
    // DrawName's yield rule needs to know WHICH label covers the town name:
    // "Houdan - support 20/60" begins with "Houdan" (the name is dropped),
    // "Village - support 25/60" does not (the name moves below it)
    MapLabelLayout layout;
    Rect2DFloat zoneLabel(0.52f, 0.49f, 0.20f, 0.02f);
    REQUIRE(layout.ClaimLabel(zoneLabel, RString("Village - support 25/60")));
    REQUIRE(layout.ClaimLabel(Rect2DFloat(0.52f, 0.60f, 0.20f, 0.02f), RString("Camp")));

    Rect2DFloat name(0.50f, 0.485f, 0.05f, 0.03f);
    int blocker = layout.Blocker(name);
    REQUIRE(blocker == 0);
    CHECK(strcmp(layout.LabelText(blocker), "Village - support 25/60") == 0);
    const Rect2DFloat& b = layout.LabelRect(blocker);
    CHECK(b.x == 0.52f);
    CHECK(b.y == 0.49f);

    // the row below the blocker is free and can be claimed for the name
    Rect2DFloat below(b.x, b.y + b.h, 0.05f, 0.03f);
    CHECK(layout.Blocker(below) == -1);
    CHECK(layout.ClaimLabel(below, RString("Houdan")));
    CHECK(layout.LabelCount() == 3);
    CHECK(strcmp(layout.LabelText(2), "Houdan") == 0);

    // a label claimed without a text answers with an empty one
    REQUIRE(layout.ClaimLabel(Rect2DFloat(0.9f, 0.9f, 0.05f, 0.02f)));
    CHECK(layout.LabelText(3).GetLength() == 0);
}

TEST_CASE("MapLabelLayout: Clear resets both lists for the next frame", "[ui][map][labels]")
{
    MapLabelLayout layout;
    REQUIRE(layout.ClaimLabel(Rect2DFloat(0, 0, 1, 1)));
    layout.AddIcon(Rect2DFloat(0, 0, 1, 1));
    CHECK_FALSE(layout.ClaimLabel(Rect2DFloat(0.5f, 0.5f, 0.1f, 0.1f)));
    layout.Clear();
    CHECK(layout.LabelCount() == 0);
    CHECK(layout.IconCount() == 0);
    CHECK(layout.ClaimLabel(Rect2DFloat(0.5f, 0.5f, 0.1f, 0.1f)));
}
