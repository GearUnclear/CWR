"""Sun position for a capture date, ported from LightSun::Recalculate.

Mirrors engine/Poseidon/Graphics/Rendering/Lighting/Lights.cpp (Recalculate)
and World::SetDate (engine/Poseidon/World/WorldSetup.cpp): the same rotation
matrices, the same column-vector convention (Matrix3P stores columns aside,
up, dir) and the same latitude sign (-40 = 40 degrees north, CfgWorlds
DefaultWorld >> latitude). Use it to pick a date/time for a target sun
elevation and to aim the camera relative to the sun.

    python sunplan.py 2026 7 5 8 0            # one moment
    python sunplan.py --scan 2026 10 15       # the day in 15-minute steps

Output: elevation and azimuth in degrees (sun, and the moon with its
LightSun moonIntensity). Azimuth uses the getDir/setDir
convention (0 = north, clockwise, 90 = east).
"""

import math
import sys

DAYS_IN_MONTH = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]


def rot_x(a):
    s, c = math.sin(a), math.cos(a)
    return [[1, 0, 0], [0, c, -s], [0, s, c]]


def rot_y(a):
    s, c = math.sin(a), math.cos(a)
    return [[c, 0, -s], [0, 1, 0], [s, 0, c]]


def mul(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(3)) for j in range(3)] for i in range(3)]


def mulv(a, v):
    return [sum(a[i][k] * v[k] for k in range(3)) for i in range(3)]


def transpose(a):
    return [[a[j][i] for j in range(3)] for i in range(3)]


def days_in_month(year, m):
    leap = (year % 4 == 0 and year % 100 != 0) or year % 400 == 0
    return 29 if (m == 1 and leap) else DAYS_IN_MONTH[m]


def _bodies(year, month, day, hour, minute, latitude_deg):
    """Light directions of the sun and the moon, exactly as Recalculate builds them."""
    one_day = 1.0 / 365
    one_hour = one_day / 24
    one_minute = one_hour / 60
    one_second = one_minute / 60
    d = day - 1
    for m in range(month - 1):
        d += days_in_month(year, m)
    time_in_year = d * one_day + hour * one_hour + minute * one_minute + 0.5 * one_second
    time_of_day = (hour * 3600 + minute * 60) / 86400.0

    earth_axis = rot_x(math.radians(23))
    latitude = rot_x(math.radians(latitude_deg))
    earth_on_orbit = rot_y(time_in_year * 2 * math.pi)
    midnight_to_current = rot_y(time_of_day * 2 * math.pi)
    camera_to_cosmos = mul(mul(mul(earth_axis, midnight_to_current), earth_on_orbit), latitude)
    cosmos_to_camera = transpose(camera_to_cosmos)
    convert = mul(rot_x(-math.pi / 2), cosmos_to_camera)
    earth_dir = [earth_on_orbit[0][2], earth_on_orbit[1][2], earth_on_orbit[2][2]]  # Direction() = column 2
    lunar_month = 28 * one_day
    moon_pos = 0.5 + time_in_year * (1.0 / lunar_month)
    rot_z5 = [[math.cos(math.radians(5)), -math.sin(math.radians(5)), 0],
              [math.sin(math.radians(5)), math.cos(math.radians(5)), 0], [0, 0, 1]]
    moon_on_orbit = mul(rot_z5, rot_y(moon_pos * 2 * math.pi))
    moon_dir = [moon_on_orbit[0][2], moon_on_orbit[1][2], moon_on_orbit[2][2]]
    light = mulv(convert, earth_dir)
    moon = mulv(convert, moon_dir)
    for v in (light, moon):
        v[0] = -v[0]
        v[2] = -v[2]
    return light, moon


def _elev_az(light):
    toward = [-light[0], -light[1], -light[2]]  # light travels from the body; flip to point at it
    elev = math.degrees(math.asin(max(-1.0, min(1.0, toward[1]))))
    az = math.degrees(math.atan2(toward[0], toward[2])) % 360.0
    return elev, az, toward


def sun(year, month, day, hour, minute, latitude_deg=-40.0):
    """Return (elevation_deg, azimuth_deg, toward_sun_xyz) for an engine date."""
    light, _ = _bodies(year, month, day, hour, minute, latitude_deg)
    return _elev_az(light)


def moon(year, month, day, hour, minute, latitude_deg=-40.0):
    """Return (elevation_deg, azimuth_deg, intensity) of the moon.

    intensity is LightSun's moonIntensity: min(cosMoonPhase + 1, 1), where
    cosMoonPhase = -(sunDir . moonDir); 0 = new moon, 1 = half to full.
    """
    light, moonl = _bodies(year, month, day, hour, minute, latitude_deg)
    cos_phase = -sum(a * b for a, b in zip(light, moonl))
    e, a, _ = _elev_az(moonl)
    return e, a, min(cos_phase + 1.0, 1.0)


def main(argv):
    if argv and argv[0] == "--scan":
        y, mo, d = (int(v) for v in argv[1:4])
        lat = float(argv[4]) if len(argv) > 4 else -40.0
        for q in range(0, 24 * 4):
            h, mi = divmod(q * 15, 60)
            e, a, _ = sun(y, mo, d, h, mi, lat)
            if e > -8:
                print(f"{h:02d}:{mi:02d}  elev {e:6.2f}  az {a:6.1f}")
        return 0
    y, mo, d, h, mi = (int(v) for v in argv[:5])
    lat = float(argv[5]) if len(argv) > 5 else -40.0
    e, a, t = sun(y, mo, d, h, mi, lat)
    me, ma, mi_ = moon(y, mo, d, h, mi, lat)
    print(f"{y}-{mo:02d}-{d:02d} {h:02d}:{mi:02d} lat {lat}: sun elev {e:.2f} az {a:.1f}; "
          f"moon elev {me:.1f} az {ma:.1f} intensity {mi_:.2f}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
