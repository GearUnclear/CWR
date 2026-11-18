"""Sun and moon heights in this engine's sky model, for choosing a shoot's dates and times.

    python sun.py [--lat 33.55] [--date 6 16] [--times 17:10,18:30,20:00]   sun, moon and camera mode per time
    python sun.py [--lat 33.55] --month 6                                    per day: the moon when the sun is 18 under
    python sun.py [--lat 33.55] --sunset 6 21                                sunrise and sunset of one day

A replica of LightSun::Recalculate (the engine's sun and moon) with the camera's night and dusk weights. Use
it before any night take: true night is a date with the moon below the horizon (or a phase light under 0.25)
and the sun at least 12 degrees under (camNight 1.00). Latitude is the island config's value as the engine
reads it: positive is SOUTH in this engine, so Lebanon80's latitude=33.55 puts June in midwinter (issue #220).
"""
import argparse
import math

import numpy as np

PI = math.pi
DIM = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]


def RX(a):
    s, c = math.sin(a), math.cos(a)
    m = np.eye(3)
    m[1, 1] = c; m[1, 2] = -s; m[2, 1] = s; m[2, 2] = c
    return m


def RY(a):
    s, c = math.sin(a), math.cos(a)
    m = np.eye(3)
    m[0, 0] = c; m[0, 2] = -s; m[2, 0] = s; m[2, 2] = c
    return m


def RZ(a):
    s, c = math.sin(a), math.cos(a)
    m = np.eye(3)
    m[0, 0] = c; m[0, 1] = -s; m[1, 0] = s; m[1, 1] = c
    return m


def calc(month, day, hour, minute, lat=33.55):
    """(sun elevation, moon elevation, moon phase light 0..1, cos phase), degrees"""
    d = day - 1 + sum(DIM[:month - 1])
    one_day = 1 / 365
    tiy = d * one_day + hour * one_day / 24 + minute * one_day / 1440
    tod = (hour * 3600 + minute * 60) / 86400
    latm = RX(lat * PI / 180)
    moon_orbit_angle = RZ(5 * PI / 180)
    earth_axis = RX(23 * PI / 180)
    lunar_month = 28 * one_day
    mpos = 0.5 + tiy / lunar_month
    moon_on_orbit = moon_orbit_angle @ RY(mpos * 2 * PI)
    earth_on_orbit = RY(tiy * 2 * PI)
    m2c = RY(tod * 2 * PI)
    cam_to_cosmos = earth_axis @ m2c @ earth_on_orbit @ latm
    conv = RX(-PI / 2) @ cam_to_cosmos.T
    sun = conv @ earth_on_orbit[:, 2]
    moon = conv @ moon_on_orbit[:, 2]
    cos_phase = -float(sun @ moon)
    sun_el = math.degrees(math.asin(max(-1, min(1, -sun[1]))))
    moon_el = math.degrees(math.asin(max(-1, min(1, -moon[1]))))
    return sun_el, moon_el, min(cos_phase + 1, 1), cos_phase


def cam_night(el):
    t = max(0, min(1, (el + 12) / 7.5))
    return 1 - t * t * (3 - 2 * t)


def cam_dusk(el):
    t = max(0, min(1, (el + 4) / 8))
    return 1 - t * t * (3 - 2 * t)


def sun_crossings(month, day, lat):
    prev, out = None, []
    for mi in range(1440):
        s = calc(month, day, mi // 60, mi % 60, lat)[0]
        if prev is not None and (prev > 0) != (s > 0):
            out.append('%02d:%02d' % (mi // 60, mi % 60))
        prev = s
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--lat', type=float, default=33.55)
    ap.add_argument('--date', nargs=2, type=int, metavar=('MONTH', 'DAY'), default=[6, 16])
    ap.add_argument('--times', default='07:00,12:00,16:45,17:10,17:25,17:40,17:55,18:30,19:00,20:00,22:00,00:00,03:00')
    ap.add_argument('--month', type=int)
    ap.add_argument('--sunset', nargs=2, type=int, metavar=('MONTH', 'DAY'))
    a = ap.parse_args()
    if a.sunset:
        m, d = a.sunset
        print('%d/%d sunrise/sunset %s, noon sun %.1f' % (m, d, sun_crossings(m, d, a.lat), calc(m, d, 12, 0, a.lat)[0]))
        return
    if a.month:
        print('moon at the first evening minute with the sun 18 degrees under, by day of month %d:' % a.month)
        for day in range(1, DIM[a.month - 1] + 1):
            for mi in range(12 * 60, 24 * 60):
                s, mo, ph, cp = calc(a.month, day, mi // 60, mi % 60, a.lat)
                if s < -18:
                    late = calc(a.month, day, 23, 59, a.lat)[1]
                    print('%2d/%02d %02d:%02d moon %6.1f phase light %.2f | 23:59 moon %6.1f%s' % (
                        day, a.month, mi // 60, mi % 60, mo, ph, late, '  dark' if (mo < 3 or ph < 0.25) else ''))
                    break
        return
    m, d = a.date
    print('%d/%d, latitude %+.2f (positive = south in this engine)' % (m, d, a.lat))
    for hm in a.times.split(','):
        h, mi = (int(x) for x in hm.split(':'))
        s, mo, ph, cp = calc(m, d, h, mi, a.lat)
        print('%02d:%02d sun %6.1f  moon %6.1f  phase light %.2f  camNight %.2f  camDusk %.2f%s' % (
            h, mi, s, mo, ph, cam_night(s), cam_dusk(s), '  TRUE NIGHT' if (s <= -12 and (mo < 3 or ph < 0.25)) else ''))


if __name__ == '__main__':
    main()
