from pathlib import Path
import json, re

root = Path(r"D:\Arma_CWA")
p = root / "arma_CWR/tests/unit/engine/Poseidon/Game/Guerrilla/test_faction_history.cpp"
s = p.read_text()
backup = root / "tmp/narrative-rewrite/test_faction_history.original.cpp"
if not backup.exists():
    backup.write_text(s)

a = s.index("std::string LastWord(")
b = s.index("// Mechanical damage a substitution can do:", a)
s = s[:a] + """// Faction names modify plural people/formation nouns. Checking the complete
// rendered group covers both acronyms and multiword names without forcing an
// artificial introducer such as "the standard now raised as".
void CheckFactionGroup(const std::string& text, const std::string& faction)
{
    const std::string low = Lower(text);
    const std::string name = Lower(faction);
    for (size_t at = low.find(name); at != std::string::npos; at = low.find(name, at + 1))
    {
        const size_t end = at + name.size();
        if ((at > 0 && IsWordChar(low[at - 1])) || (end < low.size() && IsWordChar(low[end])))
        {
            continue;
        }
        REQUIRE(end < low.size());
        REQUIRE(low[end] == ' ');
        size_t stop = end + 1;
        while (stop < low.size() && IsWordChar(low[stop]))
        {
            stop++;
        }
        const std::string noun = low.substr(end + 1, stop - end - 1);
        INFO(text << " / " << faction << " / " << noun);
        CHECK((noun == "troops" || noun == "units" || noun == "soldiers" || noun == "forces" || noun == "fighters"));
    }
}

// Count complete labels rather than common nouns used elsewhere in the story.
std::string FactionLabel(int index, const char* faction)
{
    return std::string(faction) + " " + HistoryFactionPhrase(index);
}

""" + s[b:]
s = s.replace('// no third-person pronoun anywhere: a template can then never mis-gender a',
              '// no gendered third-person pronoun: singular they can refer to a')
s = s.replace('// companion whose script name and body class disagree',
              '// companion whose script name and body class disagree')
s = s.replace('// A faction display name must never be preceded by an article nor followed by a\n// possessive: that is the whole point of the neutral constructions.',
              '// A bare faction modifier takes no article or possessive of its own.')
a = s.index('TEST_CASE("Faction history - the five faction constructions carry no article or possessive",')
b = s.index('TEST_CASE("Faction history - the rendered prose reads correctly', a)
s = s[:a] + """TEST_CASE("Faction history - faction group labels read naturally for every shipped name",
          "[game][guerrilla][legends][history]")
{
    REQUIRE(HistoryFactionPhraseCount() == 5);
    for (int i = 0; i < HistoryFactionPhraseCount(); i++)
    {
        for (int n = 0; n < kShippedNameCount; n++)
        {
            const std::string label = FactionLabel(i, kShippedNames[n]);
            CheckFactionGroup(label, kShippedNames[n]);
            CheckFactionInsertion(label, kShippedNames[n]);
        }
    }
}

""" + s[b:]
s = s.replace("CheckNameIntroducer(", "CheckFactionGroup(")
a = s.index('                    // every faction name is introduced by a construction')
b = s.index('                    CheckFactionGroup', a)
s = s[:a] + '                    // Names are modifiers in complete plural group labels.\n' + s[b:]
a = s.index('                    // the readings the two retired constructions produced,')
b = s.index('\n                }', a)
s = s[:a] + s[b:]
old = 'const int uses = CountSubstring(whole, HistoryFactionPhrase(i));'
assert s.count(old) == 1
s = s.replace(old, 'const int uses = CountSubstring(whole, FactionLabel(i, kPairs[p].occupier)) +\n'
                  '                                     CountSubstring(whole, FactionLabel(i, kPairs[p].resistance));')
s = s.replace('if (CountSubstring(Str(bio), HistoryFactionPhrase(i)) > 0)',
              'if (CountSubstring(Str(bio), FactionLabel(i, "FIA")) > 0)')
s = s.replace('CHECK(CountSubstring(whole, HistoryFactionPhrase(i)) == 0);',
              'CHECK(CountSubstring(whole, FactionLabel(i, "FIA")) == 0);')

# Check all authored biographies, not only variants the budget walk happened to choose.
at = s.index('TEST_CASE("Faction history - a biography takes the one construction')
s = s[:at] + """TEST_CASE("Faction history - every biography variant fits beside its portrait",
          "[game][guerrilla][legends][history]")
{
    const HistoryRecord rec = GenerateHistory(MakeInputs(kPairs[0], "Malden", 7));
    const RString names[] = {RString("Petra"),
        RString("Heartless Jean-Baptiste \\"The Fence-Builder\\" Vakalalabure the Collaborator")};
    const char* places[] = {"Larche", "Ras Nasrani Point", "the old crossing"};
    for (int event = 0; event < kHistoryEvents; event++)
    {
        for (int hostile = 0; hostile <= 1; hostile++)
        {
            for (int variant = 0; variant < HistoryVariantCount(); variant++)
            {
                for (const RString& name : names)
                {
                    for (const char* place : places)
                    {
                        HistoryRecord placed = rec;
                        placed.eventPlace[event] = place;
                        const RString bio = GenerateBioForced(placed, event, name, RString("FIA"),
                                                              hostile != 0, 1, variant);
                        INFO("event " << event << " hostile " << hostile << " variant " << variant << ": " << Str(bio));
                        CHECK(Words(bio) >= kHistoryBioMinWords);
                        CHECK(Words(bio) <= kHistoryBioPageWords);
                        CHECK(CountSubstring(Str(bio), Str(name)) == 1);
                        CHECK(CountSubstring(Str(bio), place) == 1);
                        CheckSharedLints(Str(bio), "forced bio");
                        CheckSentenceShape(Str(bio));
                        CheckRenderedProse(Str(bio));
                    }
                }
            }
        }
    }
}

""" + s[at:]
p.write_text(s)
print("Updated grammar and portrait-budget contracts; golden passages will update after editorial approval.")

