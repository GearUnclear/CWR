# Editorial audit memo: AI-writing tells and narrative quality

**For:** the 36 game bios and campaign-history strings in `arma_CWR/engine/Poseidon/Game/Guerrilla/FactionHistory.cpp`  
**Use:** a quality review, not a test of whether prose “looks AI-written.”

## What detection research supports

Authorship detectors infer patterns from text distributions; they do not establish who wrote a passage. OpenAI reported its own classifier caught only 26% of AI-written challenge-set text and falsely flagged 9% of human text, was unreliable below 1,000 characters, and was retired for low accuracy. That length caveat is especially relevant to short bios. In one study, seven detectors misclassified many human TOEFL essays by non-native writers, while simple editing prompts sharply reduced detection of generated essays. A test of 14 tools found every tool below 80% accuracy, with false positives and false negatives; a separate robustness study showed paraphrasing could cut detection while preserving text quality. These are bounded, mostly 2023 evaluations—not a claim that every current detector has the same score—but they are enough to reject a detector score or a style “tell” as a reliable editorial verdict. [OpenAI’s classifier evaluation](https://openai.com/index/new-ai-classifier-for-indicating-ai-written-text/); [Liang et al., *Patterns*](https://doi.org/10.1016/j.patter.2023.100779); [Weber-Wulff et al., *International Journal for Educational Integrity*](https://doi.org/10.1007/s40979-023-00146-z); [Sadasivan et al., *Can AI-Generated Text be Reliably Detected?*](https://arxiv.org/abs/2303.11156).

Quirky punctuation and banned-word lists are weak controls for the same reason: they check surface form, not whether a passage has a clear actor, a causal reason, believable world detail, or a distinct character motive. Punctuation and familiar words are ordinary parts of human style; stripping them can flatten voice, while a model can avoid a blacklist without fixing generic or incoherent writing. Treat any repeated stylistic feature as a prompt to inspect the prose, never as a score or prohibition by itself.

## What to audit in this source

The source defines 36 bios: three event kinds × friendly/hostile stance × six variants. Its history is also modular: six options each for four opening slots, three event titles, and three event passages. The campaign narrative’s order is ancient grievance → broken settlement → remembered catastrophe/stand, with the opening slots bridging those beats. Review the individual strings and the rendered combinations: a passage can read well by itself and still fail when a place, faction name, or neighboring beat changes.

A quick sample of the existing bank has strong local images—pumps, seals, checkpoints, a folded name list—but clusters heavily around land claims, ledgers, family memory, archives, and paperwork. Repeated “The X…” openings and aphoristic last lines are a useful review signal here, not a reason to ban those devices. The code also sets real continuity constraints: bios are pre-campaign and must not claim rank, XP, kills, or captured zones; it intentionally repeats `{Name}` instead of using third-person pronouns. Keep those as explicit factual/format gates, separate from the prose score.

## Scoring rubric

Score each bio variant and each history passage from 0 to 2 in every dimension. Score a rendered multi-beat history separately for combination fit and overall rhythm. Add the scores (maximum 12 per unit); target **10 or more**, with **no zero in actor/action, causality, or combination fit**. A hard lore or generator constraint failure blocks the passage regardless of total.

| Dimension | 0 — revise | 1 — workable | 2 — strong |
|---|---|---|---|
| Actor and action | Actor is vague/hidden; no concrete action or useful consequence. | Actor/action is recoverable but mostly summary or passive reporting. | Reader can name who did what to whom/what, and what changed. |
| Causality | Theme or grievance is stated without showing why it matters now. | A reason or consequence is present, but the link is thin or implied. | A legible chain connects an event to a consequence and the person’s present choice/belief. |
| Credible detail | Detail is generic, contradictory, or feels like unsupported precision. | One relevant local detail, with some grounding in the setting. | A specific, plausible object, procedure, route, or remembered action carries the beat; claims fit established lore. |
| Motive distinction | Repeats another entry’s motive in different wording. | Has a recognizable motive but shares the same narrative engine with several neighbors. | Adds a distinct reason for allegiance/action (e.g., duty, livelihood, grief, kinship, conviction, coercion, ambition, local knowledge, practical opportunity). |
| Rhythm and voice | Same opening/cadence/turn/closing as adjacent entries; prose feels mechanically parallel. | Some variation, with one obvious repeated pattern. | Sentence lengths, openings, emphasis, and endings vary naturally while retaining the project’s voice. |
| Combination fit | Substitution breaks grammar, reference, chronology, or transition. | Usually fits; one awkward name/transition or repeated beat needs repair. | Reads cleanly with supported names/places/factions and neighboring pieces; chronology and referents remain clear. |

## Practical review pass

1. **Keep a ledger.** One row per bio variant and history option: ID, actor, concrete action, causal chain, detail/lore anchor, dominant motive, opening/ending pattern, six scores, and one sentence naming the revision reason. Track motive tags across all 36 bios so new variants expand the palette instead of merely changing the nouns.
2. **Run the actor/cause check.** For every passage, fill in: “`[actor] did [action] to/at [object] because [motive or pressure]; this caused [consequence]`.” If the sentence cannot be filled from the prose, revise for agency or causality. Some entries may intentionally center a remembered event; then make clear who remembers it and why it shapes the present.
3. **Ground details.** Compare named places, dates, counts, offices, agreements, and faction actions with the project’s lore. Keep precise quantities only when they belong to canon; otherwise use a concrete observation rather than invented numerical certainty. Require each passage to contribute at least one detail that could not be swapped into any conflict setting without changing its meaning.
4. **Compare neighbors, not isolated prose.** Read all six variants in each table side by side, then all 36 bios grouped by beat and stance. Mark repeated motive, image, sentence-entry pattern, three-step build, and final aphorism. The aim is a varied bank, not a quota for odd punctuation or “human-sounding” mistakes.
5. **Render combinations.** Inspect representative seeded full histories across the supported faction-name shapes and world/place inputs. Check title-to-paragraph agreement, timeline, transition, pronoun/reference clarity, repeated details, and grammar at every placeholder boundary. Include edge shapes such as acronyms and multiword faction names; the file already treats those as a compatibility concern. Keep the existing no-XP/no-deed and pre-campaign constraints as hard gates.
6. **Use detectors only as non-binding curiosity.** If one is tried, do not use its label to accept/reject prose. Record only an editorial defect that can be pointed to in the passage and fixed directly.

The durable measure is whether a reader can picture who acted, understand why it matters, remember a grounded detail, and encounter a different human motive on the next roll.
