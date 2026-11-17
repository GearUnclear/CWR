"""Which frames go on the owner's sheets (read by make_picks.py). A frame is "<run tag>/<label>".

Runs: a_photo1 (first pass, A on its mark, five cameras spoiled), a_photo2 (second pass; its first six
night frames are the near-black ones, so A's night frames at 60 m and 300 m at the normal lens are
taken from a_photo1 and from the later frame nx06 of a_photo2, same cameras), d_photo2 (second pass,
D on its mark, complete). d_photo1 is NOT used: in it the creature stood 10.6 m off its mark.
"""
SOURCES = {"a": ["a_photo1", "a_photo2"], "d": ["d_photo2"]}

PICKS = {
    "a": [
        ("a_photo2/x09_day_town_tank_whole_45m", "Noon, from 45 m: a T-80, a soldier (left of the gun barrel) and a town house under him."),
        ("a_photo1/d09_day_right_side_75m", "His right side from 75 m, a man's eye height, with Morton's church and houses."),
        ("a_photo1/f01_right_foot_and_soldier_9m", "A man's eye height, 9 m from the right foot: a soldier at the toes, the slipped hide, the pole."),
        ("a_photo1/f06_up_whole_body_26m", "From the feet, looking up from 26 m in front of him."),
        ("a_photo2/nx06_night_60m", "Night, 60 m: the furnace sheet is fullbright, the hide is moonlit."),
        ("a_photo2/nx01_night_town_tank_whole_45m", "Night, 45 m, over the tank."),
    ],
    "d": [
        ("d_photo2/x09_day_town_tank_whole_45m", "Noon, from 45 m: a T-80, a soldier (left of the gun barrel) and a town house under him."),
        ("d_photo2/d09_day_right_side_75m", "His right side (the hound's) from 75 m, a man's eye height, with Morton's church and houses."),
        ("d_photo2/f01_right_foot_and_soldier_9m", "A man's eye height, 9 m from the hound's paw, a soldier beside it; the lizard leg at the right edge."),
        ("d_photo2/f04_left_foot_from_4m", "A man's eye height, 4 m from the lizard foot."),
        ("d_photo2/nx06_night_60m", "Night, 60 m: the furnace sheet is fullbright, the hide is moonlit."),
        ("d_photo2/nx01_night_town_tank_whole_45m", "Night, 45 m, over the tank."),
    ],
}

# (label, caption, A's run, D's run): one row per camera, A left, D right.
PAIRS = [
    ("d01_day_60m", "Noon, 60 m in front, a man's eye height (tank at the right, soldier at the feet, the town behind)", "a_photo2", "d_photo2"),
    ("x09_day_town_tank_whole_45m", "Noon, 45 m, front left quarter: tank, soldier, house", "a_photo2", "d_photo2"),
    ("d09_day_right_side_75m", "Noon, his right side from 75 m, with the church and houses", "a_photo2", "d_photo2"),
    ("x02_day_back_from_30m_up", "Noon, from behind, 90 m back and 30 m up (over the church roof)", "a_photo2", "d_photo2"),
    ("f01_right_foot_and_soldier_9m", "A man's eye height, 9 m from the right foot, a soldier beside it", "a_photo2", "d_photo2"),
    ("f02_right_shin_from_4m", "A man's eye height, 4 m in front of the right toes, looking at the shin", "a_photo2", "d_photo2"),
    ("f04_left_foot_from_4m", "A man's eye height, 4 m in front of the left toes", "a_photo2", "d_photo2"),
    ("f06_up_whole_body_26m", "From the feet looking up, 26 m in front", "a_photo2", "d_photo2"),
    ("z02_zoom_chest_60m", "The chest from 60 m through the hold-zoom (4 times)", "a_photo2", "d_photo2"),
    ("d02_day_300m", "Noon, 300 m, from the water", "a_photo2", "d_photo2"),
    ("d03_day_1km", "Noon, 1 km, from the water", "a_photo2", "d_photo2"),
    ("z05_zoom_1km", "Noon, 1 km through the hold-zoom", "a_photo2", "d_photo2"),
    ("nx06_night_60m", "Night (23:30), 60 m: furnace fullbright", "a_photo2", "d_photo2"),
    ("n02_night_300m", "Night, 300 m, from the water", "a_photo1", "d_photo2"),
    ("nz04_night_zoom_300m", "Night, 300 m through the hold-zoom", "a_photo2", "d_photo2"),
    ("nx01_night_town_tank_whole_45m", "Night, 45 m, over the tank", "a_photo2", "d_photo2"),
]
