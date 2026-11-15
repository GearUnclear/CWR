"""THE CUT of the Dedushka film record (first cut, a demonstration; more iterations are to follow).

Edit this file, then run build_all.py. Times are seconds; clip_in is a frame of the clip (30 a second).

Framing: a Soviet military film record ("Film record No. 0417", the Everon garrison's film unit, June
1985): a cameraman on the ground with a tripod and long lenses, a sound recorder that is not locked to
the camera, typed annotations added by the archive. It serves the horror because it has no author:
nobody explains, nobody is brave, the camera only stands where a man could stand and looks up.

Music (owner, 2026-11-12): "Dream Steppin'" by Two People, a demonstration bed; no tight sync asked.
The track is 100 beats a minute (a bar is 2.4 s). The film takes it from 122.49 s (a bar line inside
the passage without bass), so the bass comes in at film 7.2 s, and the music ends 76.8 s later on the
downbeat at 199.29 s, in the loudest stretch of the track, where it is cut dead under the last hit.
Two picture cuts sit on those bar lines (the second night shot at 7.2, the brand card at 72.0). The
shots between them must add up to 68.8 s: change one length, give or take the same from another.
"""
from make_edl import CARD, DUSK_FAR, DUSK_MID, DUSK_NEAR, FAR_NIGHT, cap, card, shot

MASTER_GAIN = 0.85

# a dark take lifted before the grade (the far furnace by night is a few dim pixels as shot)
LIFT = "curves=all='0/0 0.2/0.3 0.6/0.8 1/1'"
# the last of the dusk, lifted a little so the men and the tanks still read
LATE = "curves=all='0/0 0.25/0.33 0.6/0.68 1/1'"

SLOTS = [
    card('leader', 'leader', 3.2, what='The reel card: film record No. 0417, reel 2 of 3.',
         snd=dict(motor=-28, extra=[dict(t=0.35, snd='synth:beep', gain=-22, args=dict(dur=0.12, f=1000))])),
    # ---- 11 June, night: the furnace before the body
    shot('n1_fire', 'night1', 'nfar', 'boat5', 0, 4.0, look='night', move=dict(z0=1.04, z1=1.1, weave=1.2), snd=FAR_NIGHT, pre=LIFT,
         what='Night, 5 km off shore, long lens: a small fire with a shape. He is dormant.'),
    shot('n2_ribs', 'night1', 'nidle', 'boat2', 0, 3.8, look='night', move=dict(z0=1.5, z1=1.62, weave=1.5, sway=2), pre=LIFT,
         snd=dict(FAR_NIGHT, furnace=-24, extra=[dict(t=0.5, snd='roar_bear2_far', gain=-17, ring=1.0, fade_out=1.0)]),
         what='Night, 2.3 km, long lens: the fire is a ribcage.'),
    # ---- 13 June, dusk
    shot('d1_wake', 'main1', 'wake', 'street', 166, 4.8, move=dict(z0=1.04, z1=1.1), snd=dict(DUSK_MID, furnace=-26),
         what='From the street, 195 m, long lens, three soldiers small at the bottom: he straightens and turns his head to the town.'),
    shot('w1_low', 'main1', 'walkA', 'low', 360, 6.0, move=dict(z0=1.03, sway=2), snd=dict(DUSK_NEAR),
         what='Human height behind a house at the west end: he comes up over the roof, a footfall, the house starts to go.'),
    shot('c1_church', 'church1', 'church', 'se', 402, 6.2, move=dict(z0=1.03, sway=2),
         snd=dict(DUSK_NEAR, extra=[dict(t=2.1, snd='synth:collapse', gain=-4, args=dict(seed=77, dur=5.0))]),
         what='Human height, south-east of the church of Morton: he comes up behind it; the tower breaks and the nave goes down in dust; he comes on over it.'),
    shot('r1_roar', 'main1', 'roar', 'square', 100, 5.2, move=dict(z0=1.03, sway=3), snd=dict(DUSK_NEAR, ev_gain=1, roar_ring=3.2, pre=2.5, jcut=True),
         what='From among the garrison, 55 m: he rears and roars; the men break and run past the camera.'),
    shot('r2_ribs', 'main1', 'roar', 'east', 178, 2.6, move=dict(z0=1.08, z1=1.14), snd=dict(DUSK_FAR, events=False),
         what='From the east road, 300 m, long lens: the ribs stand open, the furnace throws rays.'),
    shot('g1_guns', 'main2', 'guns', 'west', 98, 3.8, move=dict(z0=1.03, sway=2), snd=dict(DUSK_FAR, guns=-5, furnace=-30),
         what='From beside the tank platoon on the west road, behind him: the guns flash, shells burst on his back, he does not turn. The town burns beyond him.'),
    shot('b1_charge', 'main2', 'beam', 'square', 82, 5.0, move=dict(z0=1.03, z1=1.07), snd=dict(DUSK_NEAR, furnace=-16, furnace_end=-6, pre=1.5),
         what='From the square, 55 m, looking up: he straightens, smoke pours off him, the furnace climbs to white, and the beam leaves the chest over the camera.'),
    shot('b2_beam', 'main2', 'beam', 'north', 232, 2.8, move=dict(z0=1.12, z1=1.16, px0=-60), snd=dict(DUSK_MID, events=False, furnace=-12,
         extra=[dict(t=0.0, snd='beam_loop', gain=-8, ring=0.1), dict(t=0.0, snd='beam_loop', gain=-12, ring=0.1, rate=0.5)]),
         what='From the open ground north of the row, side on: the thin line of the beam across the dusk, his furnace white.'),
    shot('b3_cut', 'main2', 'beam', 'armour', 462, 4.6, move=dict(z0=1.2, z1=1.24, py0=-150, sway=2),
         snd=dict(DUSK_NEAR, furnace=None, pre=3.2, extra=[dict(t=0.75, snd='synth:burst', gain=-3), dict(t=1.2, snd='synth:collapse', gain=-5, args=dict(seed=31)),
                                                            dict(t=1.7, snd='synth:burst', gain=-5, args=dict(seed=32))]),
         what='Behind the armour it was aimed at: the beam arrives; the house, the street and a T-80 go up in white, then fire.'),
    shot('l1_stand', 'main2', 'bay', 'shelf', 176, 3.8, move=dict(z0=1.03, sway=2), snd=dict(DUSK_MID, guns=-6, gun_far=True, furnace=-26), pre=LATE,
         what='The shelf above the shore, 100 m: badly hurt and smoking, he stops walking for the sea and turns on the guns; shells burst on him.'),
    shot('x1_flare', 'main2', 'death', 'shelf', 250, 4.2, move=dict(z0=1.13, z1=1.16, py0=-110), snd=dict(DUSK_MID, death=True, furnace=-22, furnace_end=-60, extra=[dict(t=-3.4, snd='beam_charge', gain=-5), dict(t=1.25, snd='roar_bear4', gain=-3, ring=2.0, fade_out=1.5)]), pre=LATE,
         what='The same place: the furnace flares white and goes out; he stands black against the sky, head back, and begins to tip.'),
    shot('x2_fall', 'main2', 'death', 'front', 376, 4.4, move=dict(z0=1.36, z1=1.4, py0=-210, px0=40), snd=dict(DUSK_MID, death=True, furnace=None), pre=LATE,
         what='From behind a tank on the shore: he pitches forward toward the guns and goes down behind the bank; the dust rolls over the tank.'),
    # ---- 14 June, morning
    shot('m1_body', 'main2', 'morning', 'head', 0, 3.8, look='day', speed=0.75, move=dict(z0=1.2, z1=1.26, px0=40, py0=-140), snd=dict(wind=-24, motor=-33, events=False),
         what='05:40 the next morning, human height, 80 m: the body lies dark on the grass, the ribs out, a little smoke still rising.'),
    # ---- the next night: the other creature (class UDDedushkaHound, not named), take hound1. Frames 340 to 448 of
    # hwalk-shelf are the clean stretch after its turn (the whole-ground light pops at 10.27 s and 11.27 s are before it),
    # slowed to 0.95 to fill the 3.8 s. The stand-in was night1 nwalk-boat2 from frame 60, look night, z 1.5 to 1.6, LIFT.
    shot('h1_light', 'hound1', 'hwalk', 'shelf', 340, 3.8, look='night', speed=0.95, move=dict(z0=1.24, z1=1.32, weave=1.5, sway=2),
         snd=dict(FAR_NIGHT, furnace=-26, events=False, extra=[dict(t=1.0, snd='roar_dog1_far', gain=-15, ring=0.0, fade_out=0.05)]),
         what='The next night, from the shelf above the shore, 220 m, long lens: a second furnace stands in the bay, a dog\'s head on it, and it has turned to face the shore.'),
    card('brand', 'brand', 6.2, move=dict(z0=1.02, z1=1.06), flash=False, what='The brand card, with the music credit.',
         shakes=[[0.0, 26, 0.35, 9.0], [4.8, 34, 0.45, 8.0]],
         snd=dict(motor=-120, extra=[
             # the card comes on with a hit; the music is at full level under it
             dict(t=0.0, snd='synth:subhit', gain=-1, bus='end'), dict(t=0.0, snd='step_2', gain=-3, bus='end'),
             dict(t=0.05, snd='roar_bear4', gain=-3, bus='end'),
             # the last hit, on the downbeat where the music is cut: the loudest thing in the film; it rings out, unfaded
             dict(t=4.8, snd='synth:subhit', gain=0, bus='end', args=dict(seed=9, dur=1.4)), dict(t=4.8, snd='step_3', gain=0, bus='end'),
             dict(t=4.8, snd='beam', gain=-3, bus='end'),
         ])),
]

CAPTIONS = [
    cap('c01', 'n1_fire', 0.5, 3.3, ['A fire on the shore.', 'The town reports no fire.'], head='11 JUNE 1985  02:10  PATROL BOAT 311'),
    cap('c02', 'n2_ribs', 0.8, 2.7, ['It is not a fire.']),
    cap('c03', 'd1_wake', 0.4, 4.0, ['It has stood there two days.', 'The men call it Dedushka.', 'Grandfather.'], head='13 JUNE  19:10  MORTON'),
    cap('c04', 'w1_low', 1.0, 3.8, ['Hold. Do not fire.'], head='ORDER TO THE GARRISON'),
    cap('c05', 'c1_church', 0.5, 3.2, ['The church of Morton.', 'Built 1764.'], head='19:24'),
    cap('c06', 'g1_guns', 0.4, 3.0, ['The tanks open fire.', 'No order was given.'], head='19:31'),
    cap('c07', 'b3_cut', 1.9, 2.5, ['2nd Tank Platoon: no reply.']),
    cap('c08', 'l1_stand', 0.5, 2.9, ['At the shore it turned', 'on the guns.'], head='19:52'),
    cap('c09', 'm1_body', 0.5, 3.0, ['The furnace is cold.'], head='14 JUNE  05:40'),
    cap('c10', 'h1_light', 0.4, 3.2, ['A light on the water.', 'It is not the same one.'], head='14 JUNE  23:50  SHORE POST'),
]

VO = [
    dict(id='l01', slot='leader', at=0.7, room=2.4, text='Film record four one seven. Everon, June nineteen eighty five.', note='flat, a file number being read'),
    dict(id='l02', slot='n1_fire', at=0.4, room=3.5, text='Ten past two. A patrol boat reports a fire on the shore. The town reports no fire.', note='quiet'),
    dict(id='l03', slot='n2_ribs', at=0.9, room=2.7, text='It is not a fire.', note='after a breath; no weight on it'),
    dict(id='l04', slot='d1_wake', at=0.4, room=4.2, text='By the second evening it has not moved. The soldiers have a name for it. Dedushka. Grandfather.', note='the name said plainly: DYEH-doosh-ka'),
    dict(id='l05', slot='w1_low', at=1.0, room=4.6, text='The garrison is ordered to hold, and not to fire.', note=''),
    dict(id='l06', slot='c1_church', at=0.5, room=1.5, text='The church of Morton.', note='ends before the tower breaks'),
    dict(id='l07', slot='g1_guns', at=0.4, room=3.2, text='At nineteen thirty one the tanks open fire. No order was given.', note=''),
    dict(id='l08', slot='b3_cut', at=1.9, room=2.6, text='Second tank platoon does not answer.', note='over the fire'),
    dict(id='l09', slot='l1_stand', at=0.5, room=3.2, text='At the shore it turned on the guns. They needed one more minute.', note='the only line with any feeling in it, and that held back'),
    dict(id='l10', slot='m1_body', at=0.5, room=3.0, text='By morning the furnace is cold.', note=''),
    dict(id='l11', slot='h1_light', at=0.4, room=3.2, text='Ten to midnight. The shore post reports a light on the water. It is not the same one.', note='the same voice as line two, no change at all'),
    dict(id='l12', slot='brand', at=5.25, room=0.9, text='Uslu dur.', note='OOS-loo DOOR, two words, after the last hit, not shouted', no_duck=True),
]

MUSIC = dict(file='dream_steppin_48k.wav', **{'in': 122.49}, **{'from': 'leader'}, at=0.0, to='brand', to_at=4.8,
             gain_at=[['leader', 0.0, -13], ['n2_ribs', 0.0, -13], ['n2_ribs', 0.4, -10], ['brand', -0.02, -10], ['brand', 0.0, -1.5], ['brand', 4.8, -1.5]])
