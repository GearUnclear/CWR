"""LoBo issue 2 plus the audit-gap loop defects AG-1, AG-3, AG-4 and AG-10.

Adds waits and alive exits to vehicle-lifetime SQS loops that otherwise run
every frame for the whole session, and retargets the CoC artillery MaxWind
self-jump that spins without a wait. Profile-neutral: OFP 1.x SQS only.
"""
from .rules import apply_rules, p
from .scripts import _explicit_exit

FLOAT = "loboeghelo/mk2/float.sqs"
DOLPHIN = "lobo_fab/dolp/init.sqs"
MIG25 = "lobomig/mig25/sqs/mig25gear.sqs"
BMP = "lobosyrapc/scripts/bmp11anim.sqs"
F5 = "lobof5/scripts/autocanopy.sqs"
A4 = "lobo_a4/scripts/canopy.sqs"
SETFLAGS = ("lobo_fab/lcm/scripts/setflags.sqs", "lobo_fab/lcm/scripts/setflags2.sqs",
            "lobo_fab/osa/scripts/setflags.sqs", "lobo_fab/saar5/scripts/setflags.sqs")
AIDLIGHTS = "csjairlines/scripts/aidlights.sqs"
ILSACT = "csjils/ilsact.sqs"
M252 = ("loboart/scripts/m252start.sqs", "coc_arty/scripts/m252start.sqs")


def _maxwind(var: str) -> tuple:
    # AG-1: the guard sits under #PrecisionSet and jumped back to it; the five
    # sibling scripts jump to #WindSet, three lines below in every copy.
    line = f"?{var} == {var}:Goto "
    return (("line", line + '"PrecisionSet"', line + '"WindSet"', 1),)


PRISTROJE = '_plane setobjecttexture [16,"' + p("csjairlines", "pristroje.pac") + '"]'
RULES = {
    # Sea King float plate: one pass per 0.1 s; a dead or deleted helicopter
    # deletes the local plate and ends the script.
    FLOAT: (("sub", '#loop\n_floater setpos [getpos _Hfloat select 0, getpos _Hfloat select 1, 1]\ngoto "loop"',
             '#loop\n?(not alive _Hfloat): deleteVehicle _floater; exit\n'
             '_floater setpos [getpos _Hfloat select 0, getpos _Hfloat select 1, 1]\n~0.1\ngoto "loop"', 1),),
    # Dolphin arm: follow behaviour within 1 s, stop with the submarine.
    DOLPHIN: (("sub", '#loop\n?(Behaviour _sub == "STEALTH")',
               '#loop\n?(not alive _sub): goto "exit"\n?(Behaviour _sub == "STEALTH")', 1),
              ("sub", '?(Behaviour _sub == "SAFE"): _sub animate ["arm1", 0]\n\ngoto "loop"',
               '?(Behaviour _sub == "SAFE"): _sub animate ["arm1", 0]\n~1\ngoto "loop"', 1)),
    # MiG-25 gear: one wait covers all four back-edges; the unreachable jump
    # to a missing label is commented out.
    MIG25: (("sub", "#CheckPosLoop\n\n;********** check pilot status",
             "#CheckPosLoop\n~0.5\n\n;********** check pilot status", 1),
            ("sub", '\ngoto "CheckPosition"\n', '\n;goto "CheckPosition"\n', 1)),
    # Syrian BMP-1 doors. The trailing TAB keeps the anchor distinct from the
    # second goto "GO".
    BMP: (("sub", '?(not alive _unit ): goto "exit"\n\ngoto "GO"\t',
           '?(not alive _unit ): goto "exit"\n~0.5\ngoto "GO"\t', 1),),
    # F-5 canopy: the loop keeps running, the Open Canopy action can reopen.
    F5: (("sub", 'goto "CanopyDown"\ngoto "PollLoop"', 'goto "CanopyDown"\n~0.5\ngoto "PollLoop"', 1),),
    # A-4 canopy: wrecks no longer get the canopy forced open every frame.
    A4: (("sub", "#Mainloop\n_pos = GetPos _plane1;", "#Mainloop\n?(not alive _plane1): exit\n_pos = GetPos _plane1;", 1),
         ("sub", "~0.0001\n", "~0.5\n", 1)),
    M252[0]: _maxwind("CoCM252MaxWind"),
    "loboart/scripts/m109start.sqs": _maxwind("CoCM109MaxWind"),
    "loboart/scripts/m1064a3start.sqs": _maxwind("CoCm252MaxWind"),
    M252[1]: _maxwind("CoCM252MaxWind"),
    "coc_arty/scripts/m109start.sqs": _maxwind("CoCM109MaxWind"),
    # AG-4: at night a dead or shut-down airliner falls into #dlightsOff,
    # which clears the textures and exits.
    AIDLIGHTS: (("sub", PRISTROJE + '\ngoto "ckL"\n',
                 PRISTROJE + '\n?(alive _plane) && (isengineon _plane): goto "ckL"\n', 1),),
    # AG-10: the ILS helper polls knowsAbout/reveal once per second.
    ILSACT: (("line", "_wait = .02", "_wait = 1", 1),),
}
# AG-3: flag ownership is released up to 1 s after the driver leaves.
RULES.update({path: (("sub", "#repeat\n~0.001\n", "#repeat\n~1\n", 1),) for path in SETFLAGS})
# Pre-existing, harmless duplicate label (#precisionset and #PrecisionSet);
# it must stay exactly the one static finding of these two scripts.
EXPECTED_FINDINGS = {path: ("duplicate label: precisionset",) for path in M252}
POST = {BMP: _explicit_exit}
ADDONS = tuple(sorted({path.split("/")[0] for path in RULES}))


def repair(documents: dict[str, str]) -> dict[str, str]:
    return apply_rules(documents, RULES, expected_findings=EXPECTED_FINDINGS, post=POST)
