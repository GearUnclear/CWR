#pragma once

// Experiment (perf squad lane EX5-SEA, plan item R-2): one wave field for the
// sea's shading and its mirror. Off by default; POSEIDON_XP_SEAMIRROR turns it
// on for a clean A/B with one build.
//
// Today the sea program (s_psWaterRemasterMain) shades its waves from the sea
// texture's own brightness, while the screen-space reflections break the
// mirrored hills and boats up with ripples of their own (four octaves of
// drifting noise, rippleNormal in the trace): the mirror image does not move
// with the waves the player sees, and only the sea's mean sky share is
// swapped for what is mirrored. With the switch on:
//   - the sea program also writes its wave slope (the gradient that built its
//     normal, after the shore's calming) into a sea-only multisampled target,
//     attachment 2 of the frame target while the sea draws;
//   - the trace reads that slope over its half-resolution texel (the mean
//     moves the mirror image, the spread of the four slopes blurs it, in place
//     of the ripples' lost detail), and swaps the sea's own sky weight on that
//     normal (Fresnel, F0 0.02, as the sea program weighs its sky) for the
//     mirrored image, so a facet that shows sky shows the hill in its place.
// Off (unset or 0) the sea program and the trace are built from their stock
// sources, byte for byte. The experiment needs a multisampled frame target
// (the default 4x MSAA); without one the trace keeps its ripples.
//
//   POSEIDON_XP_SEAMIRROR   unset or 0: off; 1 (or any number above 0): on,
//                           the number scales the slope the mirror image is
//                           moved by (1 = the sea program's own waves)

namespace Poseidon::remaster
{

// The switch's value from the text of the variable: 0 off, else the mirror's
// slope scale. Unset, empty, non-numeric or not above 0 is off; capped at 4.
float ParseSeaMirrorXp(const char* value);

// The process's value (POSEIDON_XP_SEAMIRROR, read once).
float SeaMirrorXpScale();

// POSEIDON_XP_SEAMIRROR_WEIGHT, 0 to 1 (unset: 1): the share of the sea
// program's own sky weight over its waves in what the mirror image replaces;
// 0 keeps the stock trace's mean share (the mirror as strong as today, its
// image moved by the sea's own waves).
float ParseSeaMirrorWeight(const char* value);
float SeaMirrorXpWeight();

inline bool SeaMirrorXpOn()
{
    return SeaMirrorXpScale() > 0.0f;
}

} // namespace Poseidon::remaster
