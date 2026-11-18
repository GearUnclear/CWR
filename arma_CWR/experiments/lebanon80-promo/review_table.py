# Review of the reshoot clips (thirds sheets: first, middle and last delivered frame with the grid and the
# logged subject). Read by make_shotlist.py.  name: (rating 1-5, camera move, faults, composition note)
HEAD = ('Every clip: object detail pinned at its finest (lod 0.0078125 at 2560 wide, pin 1), triSettleScene [300, 30, 2] '
        'on its first pose with the world paused, 15 head frames cut, no camera tremor. Composition: the gate result '
        'is from the per-frame log (subject screen position); "symmetric, deliberate" marks the two kinds of shot '
        'allowed off the thirds. Pop scan: the largest number of 20 px blocks that changed in a one-frame step '
        '(40 or more is listed in Faults with what it is).')
REVIEW = {
    'c01_dawn_tyre_harbour_orbit': (5, 'slow aerial orbit from the sea, 155 to 170 m up, long lens', 'none seen', 'harbour on the lower right crossing, horizon on the upper third'),
    'c02_dawn_olive_plain_push': (3, 'low push 12 to 14 m over the plain, peak about 24 m/s', 'flat dawn light, rocks in the foreground; settle oscillated (see notes)', 'Tyre skyline on the upper right crossing'),
    'c03_day_ghajar_village_dolly': (4, 'descending aerial dolly', 'settle oscillated (see notes)', 'village on the right third, mountains behind, horizon upper third'),
    'c04_day_tyre_shore_track': (4, 'lateral track off the shore, 118 to 125 m up', 'none seen', 'town on the left third, sea in the foreground, horizon upper third'),
    'c05_golden_tyre_sunset_over_sea': (4, 'high slow dolly toward the setting sun', 'none seen', 'Tyre on the left third, sun over the sea above it'),
    'c06_dusk_ghajar_lamps_orbit': (4, 'aerial orbit at the end of blue hour', 'dark hills, the lit streets read', 'village on the right third'),
    'c07_golden_litani_valley_pan': (3, 'high slow dolly into the low sun', 'heavy haze, low contrast', 'valley floor on the lower right crossing'),
    'c08_dawn_ghajar_road_misty_descent': (2, 'descending dolly down the north road', 'the village sits small on the far ridge; little mist reads', 'village on the left third'),
    'c09_golden_ghajar_orbit': (5, 'aerial orbit at sunset', 'settle oscillated (see notes)', 'village on the left third, glowing sky above'),
    'c10_day_idf_patrol_ghajar_front': (4, 'static long lens, the patrol walks toward it', 'none seen', 'second man on the right third, eyes on the upper third'),
    'c11_day_idf_patrol_ghajar_behind': (3, 'follow over the shoulder of the last man', 'the shoulder fills the left; dark foreground', 'third man on the left third'),
    'c12_day_idf_patrol_ghajar_side_dolly': (3, 'side dolly with the walking patrol, 6.5 m', 'men medium size', 'man on the right third, open space ahead'),
    'c13_day_idf_checkpoint_van_arrives': (4, 'slow push down the paved road onto the checkpoint, 3 to 3.6 m up', 'the van passes close under the lens and stops in front of the closed bar at 8.9 s', 'barrier soldier on the upper left crossing'),
    'c14_day_idf_checkpoint_low_behind_soldier': (4, 'static behind the soldiers at the bar, slight drift', 'settle oscillated (see notes)', 'stopped van on the right third, soldier in the foreground'),
    'c15_day_idf_checkpoint_telephoto': (4, 'long lens down the village street onto the checkpoint, slow lateral drift', 'a lamp post and a wall in the right foreground', 'soldier on the upper left crossing, the bar across the road, the van beyond'),
    'c16_day_idf_checkpoint_orbit': (4, 'low arc (9.5 to 10 m, 3.6 to 4.2 m up) round the checkpoint from the approach side', 'none seen', 'soldier on the right third, the bar across the road'),
    'c17_day_idf_garrison_post_push': (2, 'slow push from 3.5 m', 'a bush fills the lower left; soldiers small', 'machine gunner on the left third'),
    'c20_day_idf_armour_roadside': (3, 'static roadside, long lens, pan with the lead tank', 'exhaust and dust render blue-grey; the tank passes close at mid-clip', 'lead tank on the right third'),
    'c21_day_idf_armour_aerial_follow': (3, 'aerial follow, 9 to 12 m up', 'blue-grey exhaust; ends behind a tree', 'tank on the lower left crossing'),
    'c22_day_idf_merkava_front_dolly': (3, 'low dolly ahead of the lead tank through the village edge', 'houses pass close to the lens (the pop scan frames are those passes)', 'tank on the right third'),
    'c23c_dusk_idf_convoy_aerial_headlights': (2, 'aerial follow from the side', 'shot 17:35 (dusk, formerly labelled night); dark; the convoy drops out of sight in a dip in the last third', 'truck on the right third while in view'),
    'c23b_golden_idf_convoy_aerial_side': (2, 'aerial follow from the side', 'the convoy is hidden by a dip in the ground for most of the clip; village and low sun read', 'logged truck point on the right third'),
    'c24_golden_idf_convoy_roadside_pass': (3, 'static roadside, long lens', 'a tree crosses the middle of the clip; settle oscillated (see notes)', 'lead Humvee on the right third'),
    'c25_golden_idf_helicopters_flyover': (4, 'static low camera, pan up with a Bell 212 as it passes overhead', 'pan speeds up as it passes (pop scan frame 174 is the Bell crossing)', 'Bell on the right third'),
    'c27_dusk_tyre_lights_coming_on': (4, 'slow aerial dolly', 'none seen', 'town on the right third, horizon upper third'),
    'c30_dawn_hezb_squad_olive_grove_pass': (3, 'static at 2.2 m, long lens', 'squad medium size', 'leader on the upper left crossing'),
    'c31_dawn_hezb_squad_behind': (4, 'follow behind the squad', 'none seen', 'leader on the right third, Tyre far behind'),
    'c32_dawn_hezb_squad_aerial_reveal': (3, 'rising pull-back to 28 m', 'fighters small by the end', 'squad on the left third'),
    'c33_day_hezb_technical_side': (4, 'tracking alongside the vehicle, framed in its own frame (no aim)', 'the first 0.3 s passes under a roof edge (pop scan frame 5): cut it', 'technical fills the lower frame; checked by eye'),
    'c34_day_hezb_technical_chase': (3, 'chase behind the vehicle through the village', 'walls pass close at mid-clip', 'technical on the lower centre; checked by eye'),
    'c35b_golden_hezb_cache_orbit': (4, 'orbit at 9 to 10 m, 3 m up', 'shop sign on a map building carries Arabic lettering (map texture)', 'crates on the right third'),
    'c36_golden_hezb_cache_push_in': (3, 'slow push', 'map building signage', 'crates on the lower left crossing'),
    'c37_golden_hezb_file_side_dolly': (4, 'side dolly with the walking file', 'none seen', 'fighter on the upper right crossing, open space ahead'),
    'c38_golden_hezb_file_front_backpedal': (4, 'backpedal in front of the leader, the file behind him', 'none seen', 'leader on the upper left crossing'),
    'c39_golden_hezb_file_over_shoulder': (4, 'follow over the shoulder into the sun', 'none seen', 'fighter on the left third'),
    'c40_golden_ambush_wide_over_fighters': (4, 'wide over two fighters in the grass, slow drift', 'the road dips behind a rise; fighters small', 'truck on the left third'),
    'c41_golden_ambush_roadside_truck_hit': (5, 'low across the road, the truck passes 10 m away', 'none seen: the ambush opens at 3.2 s, the truck blows at 5.8 s, fireball and black smoke close', 'truck on the right third'),
    'c42_golden_ambush_rpg_gunner_close': (4, 'over the RPG gunner as he rises and fires', 'RPG back-blast dust in the lower left late in the clip', 'truck on the right third'),
    'c43_dusk_ambush_tracers_close': (2, 'roadside, static', 'dark; fighters do not read; muzzle smoke', 'ambush line on the left third'),
    'c43b_dusk_ambush_behind_fighters_tracers': (3, 'close behind the fighters, static', 'fighters dark; fire opens at 3.2 s, the truck blows at 6.0 s', 'truck on the right third'),
    'c44_dusk_ambush_low_road': (3, 'low on the road ahead of the convoy', 'dark until the hit; the lead Humvee blows near the end', 'Humvee on the right third'),
    'c45_golden_ambush_burning_wreck_smoke': (4, 'slow push on the burning truck', 'none seen', 'wreck on the right third, smoke column above'),
    'c46_golden_ambush_idf_dismount_return_fire': (3, 'static roadside behind the troop truck', 'the dismount happens partly behind the truck', 'troop truck on the right third'),
    'c47_golden_ambush_fighters_rise_from_cover': (3, 'static along the line from its flank', 'fighters go down again in the grass after the first volley; the truck blows top right', 'fighter on the left third'),
    'c48_golden_ambush_truck_hit_high_angle': (3, 'high from across the road, slow drift', 'steep angle, fighters hidden in the grass', 'truck on the lower right crossing'),
    'c49_dusk_ambush_burning_truck_return_fire': (2, 'behind the dismounted soldiers', 'dark, smoke fills the frame', 'truck on the left third'),
    'c50_kaiju_golden_advance_with_armour_low_front': (4, 'static low, pan up with him', 'the escort tanks stay hidden behind the sand bank', 'symmetric, deliberate: he walks straight at the lens'),
    'c51_kaiju_golden_escort_aerial_scale': (4, 'aerial follow from the side', 'settle oscillated (see notes)', 'his chest on the left third, open space ahead'),
    'c52_kaiju_golden_idf_soldiers_watch_him_pass': (5, 'static behind the soldiers, pan with him', 'none seen', 'his chest on the upper right crossing, soldiers in the foreground'),
    'c53_kaiju_golden_legs_footfall_low': (4, 'ground level, tracking his legs', 'dust', 'feet on the left third'),
    'c54_kaiju_golden_reveal_behind_idf_roadblock': (5, 'long lens behind the roadblock, the M113 in the left foreground', 'none seen: the soldiers swing the bar open at 4.1 s (open at 7.1 s) and step off to the shoulders as he comes on', 'his chest on the right third, horizon low'),
    'c55_kaiju_golden_roar_over_roadblock': (5, 'static behind the soldiers stood aside at the open roadblock', 'he walks up the coast road toward the open bar, stops at 9.5 s about 56 m short of it and roars from 10.5 s', 'his chest on the upper left crossing'),
    'c61_kaiju_night_walks_through_burning_position': (4, 'follow from the side at 42 to 45 m', 'lit by the fires and his chest only', 'his chest on the left third'),
    'c62_kaiju_night_glow_over_ghajar': (3, 'aerial dolly from the village', 'dark; settle oscillated (see notes)', 'his chest on the right third above the roofs'),
    'c63_kaiju_night_silhouette_from_village_street': (4, 'street level, static, long lens', 'shop sign in Arabic (map texture)', 'his chest on the upper right crossing'),
    'c64_kaiju_beam_wide_over_shoulder_valley': (5, 'high behind him, slow drift', 'the first second is lit orange, then the ground light changes (furnace light)', 'he is on the left third, the beam lands on the right third'),
    'c65_kaiju_beam_hero_under_chest': (5, 'low under him, tilt from the chest down the beam', 'none seen', 'symmetric, deliberate: the beam down the centre'),
    'c66_kaiju_beam_reverse_up_the_beam': (3, 'static from behind the fighters, long lens', 'smoke from the earlier passes hides part of him', 'his chest on the upper right crossing'),
    'c67_kaiju_beam_night_hero': (5, 'beside his legs at night, static', 'settle oscillated (see notes); the chest and the beam are the only light', 'his chest on the upper right crossing, the beam crosses to the lower right'),
    'c68_kaiju_dusk_fighters_fire_on_him': (2, 'static behind the fighters', 'dark, smoke; he is small', 'his chest on the upper right crossing'),
    'c70_day_fp_advance_with_squad': (2, 'first person', 'the player stands still while the squad walks off; action menu and HUD (game UI)', 'not gated (first person)'),
    'c71_day_fp_firefight_olive_grove': (2, 'first person, firing bursts', 'the first 5 s are the weapon-raise animation; usable from about 6 s', 'not gated (first person)'),
    'c72_fieldjournal_pages': (4, 'static map screen, pages turn every 3.2 s', 'the title now reads "Guerrilla" (the template\'s own); the pop scan frames are the page turns', 'not gated (map screen)'),
    'c80_dusk_idf_patrol_side_lamplit_village': (3, 'side dolly with the walking patrol at blue hour', 'dark foreground; building lit behind', 'man on the upper left crossing'),
    'c81_dusk_idf_patrol_behind_into_village': (3, 'follow behind the last man', 'dark foreground; the lit village and the pink sky read', 'man on the left third'),
    'c82_dusk_hezb_squad_crosses_road': (2, 'static at 1.9 m', 'fighters dark against the village', 'leader on the left third'),
    'c83_dusk_firefight_behind_idf_machine_gun': (3, 'static behind a machine gunner', 'tracers faint; muzzle flash reads', 'gunner on the left third'),
    'c84_dusk_firefight_behind_hezb_machine_gun': (3, 'static behind a machine gunner', 'tracers faint; muzzle flash reads', 'gunner on the left third'),
    'c06n_night_ghajar_street_lamp_orbit': (3, 'slow low orbit (20 to 22 m, 6 to 7 m up) round the street lamp at the village edge', 'tree branches cross the lens in the first second (pop scan frames 18 to 21)', 'lamp pool on the lower right crossing'),
    'c13n_night_idf_checkpoint_in_headlights': (3, 'static behind the soldiers at the bar, slight drift', 'the soldiers read as dark figures against the Humvee headlights', 'soldier on the upper left crossing, the bar across the lit road'),
    'c23n_night_idf_convoy_headlights_low_road': (3, 'static low on the road, long lens', 'the lead Humvee drives over the lens at 6.3 s (pop scan frames 189 to 192): cut before it; its lit cab and headlights fill the frame just before', 'lead Humvee on the right third'),
    'c35n_night_hezb_cache_campfire_orbit': (5, 'low orbit at 7.5 to 8 m round the fire', 'none seen', 'the fire on the lower right crossing, fighters and crates in its light'),
    'c36n_night_hezb_cache_campfire_push': (4, 'slow push on the fire', 'a crate covers the fire late in the clip', 'the fire on the lower left crossing'),
    'c85_night_firefight_behind_idf_machine_gun': (3, 'static 3 m behind the gunner', 'a black frame with the gunner lit by his own muzzle flashes; tracers faint', 'gunner on the left third'),
    'c85b_night_firefight_between_the_lines': (2, 'static low between the lines', 'almost black; a few flashes and faint tracers', 'machine gunner on the right third'),
    'c86_night_firefight_behind_hezb_machine_gun': (2, 'static 3 m behind the gunner', 'almost black; small muzzle flashes', 'gunner on the left third'),
    'c87_night_ambush_headlights_low_road': (2, 'static low on the road ahead of the convoy', 'the headlights are small; the hit reads as a flash and a fire far off', 'lead Humvee on the right third'),
    'c88_night_ambush_burning_truck_return_fire': (4, 'slow push on the burning truck from the roadside', 'the fire is the only light; a second burst at 4.0 s', 'truck on the right third'),
    'c90_kaiju_heli_cobras_flypast_chest_golden': (3, 'static ahead-left of him, pan with him', 'the Cobras cross small at the top right', 'his chest on the right third'),
    'c91_kaiju_heli_escort_pair_at_shoulders': (4, 'follow behind-left and above him', 'none seen', 'his chest on the right third, helicopters at his shoulders'),
    'c92_kaiju_heli_chase_bell_overtakes_him': (4, 'chase at the Bell 212\'s height as it overtakes him', 'none seen', 'Bell on the left third, he fills the right'),
    'c93_kaiju_heli_orbit_cobra_foreground': (5, 'orbit at helicopter height in step with a circling Cobra', 'the orbit starts at full speed on frame 0; the Cobra carries the LoBo model\'s roundel texture', 'his chest on the right third, the Cobra on the left'),
    'c94_kaiju_heli_cobra_past_his_head_closeup': (4, 'long lens on his head, slow drift', 'the Cobra crosses in the first half', 'his head on the upper left crossing'),
    'c95_kaiju_heli_dusk_lookup_circling_glow': (5, 'ground level looking up', 'none seen', 'his chest on the left third, helicopters circling right'),
    'c96_kaiju_heli_dusk_orbit_glow_wide': (4, 'wide orbit at 42 to 45 m', 'none seen', 'his chest on the right third'),
    'c97_dollyzoom_fighter_golden': (4, 'dolly zoom (backs 3.5 to 15 m, lens narrows to match), then static', 'the road cut slopes the ground line', 'fighter on the upper left crossing, Dedushka grows onto the right third'),
    'c98_dollyzoom_fighter_golden_roar': (5, 'dolly zoom while he roars, then static', 'the ground line slopes', 'fighter on the upper left crossing, Dedushka grows onto the right third'),
    'c99_dollyzoom_fighter_dusk_glow': (4, 'dolly zoom at dusk, then static', 'the ground line slopes', 'fighter on the upper left crossing, Dedushka grows onto the right third'),
}
PICKS = '''
## Picks

Best beam take: **c65_kaiju_beam_hero_under_chest.mp4** (symmetric, deliberate: the beam down the centre). The
chest glow builds from frame 0 (the charge starts about 0.3 s before the clip), the beam fires at **4.10 s** and
the first impact lands about 0.6 s later, then the sweep. Night alternative **c67** (true night, fire at 4.40 s, chest on the
upper right crossing); wide **c64** (fire at 4.37 s, he on the left third and the impact on the right third);
reverse **c66** (fire at 4.30 s, impact about 4.8 s).

Dolly zoom: best **c98_dollyzoom_fighter_golden_roar.mp4**: zoom 0.0 to 3.5 s while he roars, the fighter
holds the upper left crossing and Dedushka grows from near the middle onto the right third; the fighter turns
and sprints off at 4.35 s and is out of frame by about 6.5 s. Also c97 (golden) and c99 (dusk glow), same timing.

Helicopters for scale: best three **c93** (orbit with a Cobra in the foreground), **c95** (dusk look-up, three
circling the glowing chest), **c91** (escort pair at his shoulders). Also c92 (chase behind a Bell as it
overtakes him), c94 (Cobra past his head), c96 (dusk wide orbit), c90 (Cobras past his chest).

Ambush: **c41** (the truck blows 10 m from a low roadside camera), **c42** (over the RPG gunner), **c45**
(burning wreck), **c40** (wide over the fighters); third angle on the hit **c48** (high); dusk **c43b**
(behind the fighters, the truck blows) and **c44** (low on the road, the Humvee hit); true night **c88** (the
burning truck is the only light).

True night (16 June 20:00, new moon, see shoot-notes.md): **c35n** (fighters round a camp fire at the
cache), **c67** (night beam hero), **c61** (he walks through the burning position), **c63** (his silhouette
from a village street), **c88** (burning truck), **c36n**, **c62**, **c06n**. The darkest, near-black frames
with one small light: c85, c85b, c86, c87, c13n.

Best ten overall: c65, c98, c55, c52, c93, c41, c09, c01, c95, c64. Next: c67, c50, c10, c42, c45, c37,
c38, c39, c03, c04.
'''
