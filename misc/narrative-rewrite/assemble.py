from pathlib import Path
import json, re, textwrap, itertools

root = Path(r"D:\Arma_CWA")
scratch = root / "tmp/narrative-rewrite"
source = root / "arma_CWR/engine/Poseidon/Game/Guerrilla/FactionHistory.cpp"
tests = root / "arma_CWR/tests/unit/engine/Poseidon/Game/Guerrilla/test_faction_history.cpp"
cpp = source.read_text()
cards = json.loads((scratch / "grounded-cards.json").read_text())
campaign = json.loads((scratch / "campaign.json").read_text())
banks = ["kOpenA", "kOpenB", "kOpenC", "kOpenD", "kTitle0", "kTitle1", "kTitle2",
         "kText0", "kText1", "kText2", "kBio0Friendly", "kBio0Hostile",
         "kBio1Friendly", "kBio1Hostile", "kBio2Friendly", "kBio2Hostile"]
patterns = {bank: re.compile(r"static const char\* const " + bank +
             r"\[kVariants\] = \{.*?\};", re.S) for bank in banks}
original_file = scratch / "original-narratives.json"
if not original_file.exists():
    originals = {}
    for bank in banks:
        block = patterns[bank].search(cpp).group()
        # Commas outside literals separate entries, including compact title arrays.
        body = block[block.index("{")+1:block.rindex("}")]
        originals[bank] = []
        entry = ""
        for token in re.findall(r'"(?:\\.|[^"\\])*"|,', body):
            if token == ',':
                originals[bank].append(entry)
                entry = ""
            else:
                entry += json.loads(token)
        if entry:
            originals[bank].append(entry)
        assert len(originals[bank]) == 6, (bank, len(originals[bank]))
    original_file.write_text(json.dumps(originals, indent=2))

new = {bank: campaign[bank] for bank in banks[:10]}
for bank in banks[10:]:
    new[bank] = [None]*6
drafts = []
for card in cards:
    file = scratch / (card["id"] + ".json")
    draft = json.loads(file.read_text())
    assert (draft["id"], draft["bank"], draft["variant"]) == (card["id"], card["bank"], card["variant"])
    text = draft["text"]
    previous = scratch / "creative-pass" / file.name
    assert not previous.exists() or text != json.loads(previous.read_text())["text"], (card["id"], "old draft")
    words = len(text.split())
    assert 20 <= words <= 36, (card["id"], words)
    assert text.count("{Name}") == 1 and text.count("{Place}") == 1, card["id"]
    assert re.findall(r"\{[^}]+\}", text) in ([ "{Name}", "{Place}" ], ["{Place}", "{Name}"]), card["id"]
    assert not re.search(r"\b(he|she|his|her|him|hers)\b", text, re.I), card["id"]
    assert text.isascii() and not re.search(r"[0-9\n\r\t]", text), card["id"]
    assert re.search(r'[.!?]"?$', text), (card["id"], text)
    new[card["bank"]][card["variant"]] = text
    drafts.append({**draft, "word_count": words, "direction": card["direction"],
                   "source_pattern": card["source_pattern"], "source_url": card["source_url"],
                   "fictional_changes": card["fictional_changes"]})
for bank in banks:
    assert len(new[bank]) == 6 and all(isinstance(x, str) for x in new[bank]), bank

def literals(text, indent="    ", width=99):
    chunks = textwrap.wrap(text, width=width, break_long_words=False, break_on_hyphens=False,
                           replace_whitespace=False, drop_whitespace=True)
    return "\n".join(indent + json.dumps(chunk + (" " if i+1<len(chunks) else ""), ensure_ascii=True)
                     for i,chunk in enumerate(chunks))

for bank in banks:
    replacement = "static const char* const " + bank + "[kVariants] = {\n" + ",\n".join(
        literals(text) for text in new[bank]) + "};"
    cpp, count = patterns[bank].subn(lambda m: replacement, cpp)
    assert count == 1, bank
source.write_text(cpp)

nouns = ["troops", "units", "soldiers", "forces", "fighters"]
def render(text, slots):
    for token, value in slots.items():
        text = text.replace("{" + token + "}", value)
    assert "{" not in text, text
    return text

cases = [
 dict(letter="A", island="Malden", resistance="FIA", occupier="Soviet Army",
      places=["Larche","Houdan","Chapoi"], opening=[0,0,4,0], events=[0,0,0], phrases=[0,2,1,4], bio_event=0,bio_variant=0),
 dict(letter="B", island="Lebanon", resistance="IDF", occupier="Hizballah",
      places=["Ghajar","Tyre","Saida"], opening=[1,1,1,1], events=[1,1,1], phrases=[1,4,0,2], bio_event=1,bio_variant=1),
 dict(letter="C", island="Sinai", resistance="PLO East", occupier="IDF",
      places=["Nuweiba","El Tor","Ras Nasrani"], opening=[2,2,2,2], events=[2,2,2], phrases=[3,0,2,1], bio_event=2,bio_variant=2)]
test_text = tests.read_text()
examples = []
for case in cases:
    def group(faction, index):
        return faction + " " + nouns[index]
    slots = {"Island": case["island"], "Place":case["places"][0],
             "Occupier":group(case["occupier"],case["phrases"][0]),
             "OccupierCap":group(case["occupier"],case["phrases"][0]),
             "Resistance":group(case["resistance"],case["phrases"][1]),
             "ResistanceCap":group(case["resistance"],case["phrases"][1])}
    page1 = render(new["kOpenA"][case["opening"][0]],slots)+" "+render(new["kOpenB"][case["opening"][1]],slots)
    page2 = render(new["kOpenC"][case["opening"][2]], {**slots,"Place":case["places"][1]})+" "+render(new["kOpenD"][case["opening"][3]],slots)
    expected = {"rec.openingPage1":page1, "rec.openingPage2":page2}
    for beat in range(3):
        event_slots = {"Place":case["places"][beat],
             "Occupier":group(case["occupier"],case["phrases"][2]),
             "OccupierCap":group(case["occupier"],case["phrases"][2]),
             "Resistance":group(case["resistance"],case["phrases"][3]),
             "ResistanceCap":group(case["resistance"],case["phrases"][3])}
        expected["rec.eventTitle["+str(beat)+"]"] = render(new["kTitle"+str(beat)][case["events"][beat]],event_slots)
        expected["rec.eventText["+str(beat)+"]"] = render(new["kText"+str(beat)][case["events"][beat]],event_slots)
    expected["bio"] = render(new["kBio"+str(case["bio_event"])+"Friendly"][case["bio_variant"]],
                             {"Name":"Petra","Place":case["places"][case["bio_event"]]})
    a = test_text.index('TEST_CASE("Faction history - worked example '+case["letter"]+',')
    b = test_text.find('\nTEST_CASE(',a+1)
    if b == -1:
        b = len(test_text)
    block = test_text[a:b]
    for expr,text in expected.items():
        pattern = re.compile(r"CHECK\(Str\(" + re.escape(expr) + r"\) ==\s*(?:\"(?:\\.|[^\"\\])*\"\s*)+\);",re.S)
        block,count = pattern.subn(lambda m:"CHECK(Str("+expr+") ==\n"+literals(text,"          ")+");",block)
        if expr.startswith("rec.opening") or expr=="bio":
            assert count == 1, (case["letter"],expr)
    test_text = test_text[:a]+block+test_text[b:]
    examples.append({"campaign":case["letter"],"opening":page1+" "+page2,
                     "events":[expected["rec.eventText["+str(i)+"]"] for i in range(3)],
                     "bio":expected["bio"]})
tests.write_text(test_text)
(scratch/"final-narratives.json").write_text(json.dumps(new,indent=2))
(scratch/"review.json").write_text(json.dumps({"biographies":drafts,"campaign_examples":examples},indent=2))
catalog = ["CHARACTER BACKGROUNDS", "Fiction informed by historical research. Names, places and specific game events are invented.",
           "Name and place placeholders are shown as written.", ""]
for d in drafts:
    catalog += [d["id"]+" - "+d["bank"]+" / "+str(d["variant"]+1), d["text"], ""]
catalog += ["CAMPAIGN HISTORY EXAMPLES",""]
for ex in examples:
    catalog += ["Campaign "+ex["campaign"],ex["opening"],"",*sum(([t,""] for t in ex["events"]),[])]
(scratch/"stories.txt").write_text("\n".join(catalog))
print("Installed 36 biographies and 60 campaign/history strings; updated three pinned examples.")

