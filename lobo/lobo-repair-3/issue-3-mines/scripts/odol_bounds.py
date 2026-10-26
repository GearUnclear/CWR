import struct, sys
data = open(sys.argv[1],'rb').read()
res = [float(x) for x in sys.argv[2].split(",")]; n = int(sys.argv[3])
pat = struct.pack('<%df' % len(res), *res)
i = data.rfind(pat)
print('resolutions at', i, 'of', len(data))
p = i + 4*n; print("all res", struct.unpack_from("<%df" % n, data, i))
special, bsphere, gsphere, remarks, andh, orh = struct.unpack_from('<iffiII', data, p); p += 24
aim = struct.unpack_from('<3f', data, p); p += 12
color, colortop = struct.unpack_from('<II', data, p); p += 8
vd, = struct.unpack_from('<f', data, p); p += 4
mn = struct.unpack_from('<3f', data, p); p += 12
mx = struct.unpack_from('<3f', data, p); p += 12
bc = struct.unpack_from('<3f', data, p); p += 12
print('boundingSphere', bsphere, 'geometrySphere', gsphere)
print('aimingCenter', aim)
print('min', mn, 'max', mx, 'boundingCenter', bc)
print('original-frame minY', mn[1]+bc[1], 'maxY', mx[1]+bc[1])
