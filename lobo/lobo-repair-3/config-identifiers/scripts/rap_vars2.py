import struct, sys
d = open(sys.argv[1], 'rb').read()
# Walk forward from candidate starts: find all positions where an asciiz identifier + int32 chain reaches EOF exactly.
# Backward approach: collect record boundaries by scanning from the end.
end = len(d)
recs = []
q = end
while True:
    # previous record ends at q: last 4 bytes value, before that a NUL-terminated name
    if q - 5 < 0:
        break
    val = struct.unpack_from('<i', d, q - 4)[0]
    if d[q - 5] != 0:
        break
    s = q - 5
    b = s - 1
    while b >= 0 and (48 <= d[b] <= 57 or 65 <= d[b] <= 90 or 97 <= d[b] <= 122 or d[b] == 95):
        b -= 1
    name = d[b + 1:s]
    if not name:
        break
    # The byte before the name must be the previous record's int32 tail or the count.
    recs.append((name.decode(), val, b + 1))
    q = b + 1
    if len(recs) > 3000:
        break
print('records walked back', len(recs))
start = recs[-1][2]
print('first name at', start, 'preceding bytes', d[start - 8:start].hex())
cnt = struct.unpack_from('<i', d, start - 4)[0]
print('int before first:', cnt)
for name, v, _ in sorted(recs):
    if True:
        print(f'  {name} = {v}')
