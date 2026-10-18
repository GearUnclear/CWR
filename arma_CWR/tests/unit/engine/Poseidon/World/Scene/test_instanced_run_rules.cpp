#include <catch2/catch_test_macros.hpp>

#include <Poseidon/World/Scene/InstancedRunRules.hpp>

#include <vector>

// The opaque pass draws a run of identical static shapes as one instanced draw:
// the head is drawn once and repeated at every member's world matrix, from the
// head's vertex buffer.  A land-following bush (ClipLandKeep vertices) has its
// vertices refitted to the terrain under it on every draw (Object::Animate), so
// a run of bushes drew every member with the head's terrain fit: the members
// floated over or sank into their own slopes and changed outline.  These tests
// pin the grouping rules so an object that rewrites its own vertices never
// shares the head's buffer, and keep the older rules from drifting.

using Poseidon::InstancedRun::CanHead;
using Poseidon::InstancedRun::CanJoin;
using Poseidon::InstancedRun::RunEntry;

namespace
{
int shapeA = 0;
int shapeB = 0;
const void* kShapeA = &shapeA;
const void* kShapeB = &shapeB;

// A static, undamaged object of shape A at 100 m (distance2 10000).
RunEntry Plain(float distance2 = 10000.0f)
{
    return RunEntry{kShapeA,
                    /*drawLOD=*/1,
                    /*passNum=*/0,
                    /*special=*/0,
                    distance2,
                    /*isStatic=*/true,
                    /*ownVertices=*/false};
}

// The same shape, but its draw refits its vertices for itself: a bush whose
// level has ClipLandKeep points, or a damaged object (Object::IsAnimated).
RunEntry OwnVertices(float distance2 = 10000.0f)
{
    RunEntry e = Plain(distance2);
    e.ownVertices = true;
    return e;
}

// The pass-1 grouping loop over an already shape-sorted queue, without the
// engine's scene gates and batch capacity: returns the length of each run.
std::vector<int> RunLengths(const std::vector<RunEntry>& queue)
{
    std::vector<int> runs;
    for (size_t i = 0; i < queue.size();)
    {
        size_t end = i + 1;
        if (CanHead(queue[i]))
        {
            while (end < queue.size() && CanJoin(queue[i], queue[end]))
            {
                end++;
            }
        }
        runs.push_back(static_cast<int>(end - i));
        i = end;
    }
    return runs;
}
} // namespace

TEST_CASE("InstancedRun: a land-following bush never heads a run", "[scene][instancing]")
{
    // Every bush of a ClipLandKeep shape refits its own vertices, so none of
    // them may lead an instanced draw.
    CHECK_FALSE(CanHead(OwnVertices()));
    CHECK(CanHead(Plain()));

    // Five bushes of one shape at almost the same distance: each is drawn on
    // its own, with its own terrain fit, never as one run of five.
    const std::vector<RunEntry> bushes{OwnVertices(10000.0f), OwnVertices(10050.0f), OwnVertices(10100.0f),
                                       OwnVertices(10150.0f), OwnVertices(10200.0f)};
    CHECK(RunLengths(bushes) == std::vector<int>{1, 1, 1, 1, 1});
}

TEST_CASE("InstancedRun: an object that rewrites its vertices never joins a run", "[scene][instancing]")
{
    // A damaged copy among undamaged twins: it would otherwise be drawn with the
    // head's undamaged vertices.  It ends the run and is drawn alone; the twins
    // after it start a run of their own.
    CHECK_FALSE(CanJoin(Plain(), OwnVertices()));
    CHECK(CanJoin(Plain(), Plain(10100.0f)));

    const std::vector<RunEntry> queue{Plain(10000.0f), Plain(10010.0f), Plain(10020.0f), OwnVertices(10030.0f),
                                      Plain(10040.0f), Plain(10050.0f), Plain(10060.0f), Plain(10070.0f)};
    CHECK(RunLengths(queue) == std::vector<int>{3, 1, 4});
}

TEST_CASE("InstancedRun: the older grouping rules are unchanged", "[scene][instancing]")
{
    const RunEntry head = Plain();
    REQUIRE(CanJoin(head, Plain()));

    RunEntry other = Plain();
    other.shape = kShapeB;
    CHECK_FALSE(CanJoin(head, other));

    other = Plain();
    other.drawLOD = 2;
    CHECK_FALSE(CanJoin(head, other));

    other = Plain();
    other.passNum = 1;
    CHECK_FALSE(CanJoin(head, other));

    other = Plain();
    other.special = 0x100;
    CHECK_FALSE(CanJoin(head, other));

    other = Plain();
    other.isStatic = false;
    CHECK_FALSE(CanJoin(head, other));
    CHECK_FALSE(CanHead(other));
}

TEST_CASE("InstancedRun: members stay inside the fog band", "[scene][instancing]")
{
    // 0.90x .. 1.10x of the head's squared distance.
    const RunEntry head = Plain(10000.0f);
    CHECK(CanJoin(head, Plain(9001.0f)));
    CHECK(CanJoin(head, Plain(10999.0f)));
    CHECK_FALSE(CanJoin(head, Plain(8999.0f)));
    CHECK_FALSE(CanJoin(head, Plain(11001.0f)));
}

TEST_CASE("InstancedRun: an object lit by a local light neither heads nor joins a run", "[scene][instancing]")
{
    // drawGrouping (plan section 5 A4): with light slots in the scene, an
    // object takes part in runs when the light list its own draw would select
    // is empty. The run is drawn with the head's list, so a lit object would
    // either light its unlit twins (as head) or lose its own light (as member).
    RunEntry lit = Plain(10010.0f);
    lit.localLit = true;
    CHECK_FALSE(CanHead(lit));
    CHECK_FALSE(CanJoin(Plain(), lit));
    CHECK(CanJoin(Plain(), Plain(10010.0f)));

    // Five twins along a street, the third under a lamp: it is drawn alone,
    // the two before it and the two after it are each too short for a run.
    const std::vector<RunEntry> street{Plain(10000.0f), Plain(10010.0f), lit, Plain(10030.0f), Plain(10040.0f)};
    CHECK(RunLengths(street) == std::vector<int>{2, 1, 2});

    // A wood far from any lamp keeps its runs.
    const std::vector<RunEntry> wood{Plain(10000.0f), Plain(10010.0f), Plain(10020.0f), Plain(10030.0f),
                                     Plain(10040.0f)};
    CHECK(RunLengths(wood) == std::vector<int>{5});
}
