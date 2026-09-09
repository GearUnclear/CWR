#pragma once

// Guerrilla Mode journal: the handbook (Reference) chapters.
//
// A static in-universe text table, typed onto the notepad by the journal's
// Compose stage (JournalComposePeoplePlaces.cpp).  Ten chapters, each an
// anchor ("GM_MAN_MODE" ...), a title, a subtitle and a null-terminated list
// of lines in the mini markup below.  The table itself lives in
// JournalManual.cpp; these accessors are the whole public surface (exposed
// for the tests and the docs as well as the composer).
//
// Mini markup per line:
//   "#Heading"        section head        "!A|B|C"   table header
//   "|a|b|c"          table row; a cell starting with ~r is in red ink, ~w
//                     is bold type, ~g / ~y are plain type (kept so the
//                     text reads the same on paper as it did on the mock)
//   "- text"          bullet              "@standing" the live cover row
//   anything else     paragraph

namespace Poseidon::Guerrilla
{
struct ManualTopic
{
    const char* anchor; // "GM_MAN_MODE" ...
    const char* title;
    const char* subtitle;
    const char* lines[24]; // null-terminated; mini markup per GuerrillaJournalPages.cpp:78-85
};
int GuerrillaManualTopicCount();
const char* GuerrillaManualTopicTitle(int i);   // "" out of range
const char* GuerrillaManualTopicAnchor(int i);  // "" out of range
const ManualTopic& GuerrillaManualTopic(int i); // index clamped into range
} // namespace Poseidon::Guerrilla
