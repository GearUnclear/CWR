// Group id churn: two overlapping groups at a time, created and released
// 300 times. NGroups never passes 2, so every createGroup must succeed and
// the id handed out must stay inside the name register (1..MaxGroups).
triSimUntil { alive player }
udFirst = format ["%1", group player]
udPrev = createGroup west
udSecond = format ["%1", udPrev]
udI = 0
udNull = 0
udLast = ""
while {udI < 300} do {udNext = createGroup west; if (isNull udNext) then {udNull = udNull + 1}; udLast = format ["%1", udNext]; deleteGroup udPrev; udPrev = udNext; udI = udI + 1}
triSimFrames 5
triAssertEq [udNull, 0]
// 300 is even, so the last group sits in the second slot again (lowest free id)
triAssertEq [udLast, udSecond]
triEndTest
