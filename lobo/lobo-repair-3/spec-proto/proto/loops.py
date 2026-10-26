"""SPEC PROTOTYPE: LoBo issue 2 plus audit-gap loop/hang defects (AG-1, AG-3,
AG-4, AG-10). Adds waits and exits to vehicle-lifetime script loops, and
retargets a self-jump that spins without a wait. Profile-neutral (OFP 1.x SQS).
"""
from .common import _explicit_exit, apply_rules, p

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


def _maxwind(var):
    line = f"?{var} == {var}:Goto "
    return (("line", line + '"PrecisionSet"', line + '"WindSet"', 1),)


PRISTROJE = '_plane setobjecttexture [16,"' + p("csjairlines", "pristroje.pac") + '"]'
RULES = {
    FLOAT: (("sub", '#loop\n_floater setpos [getpos _Hfloat select 0, getpos _Hfloat select 1, 1]\ngoto "loop"',
             '#loop\n?(not alive _Hfloat): deleteVehicle _floater; exit\n'
             '_floater setpos [getpos _Hfloat select 0, getpos _Hfloat select 1, 1]\n~0.1\ngoto "loop"', 1),),
    DOLPHIN: (("sub", '#loop\n?(Behaviour _sub == "STEALTH")',
               '#loop\n?(not alive _sub): goto "exit"\n?(Behaviour _sub == "STEALTH")', 1),
              ("sub", '?(Behaviour _sub == "SAFE"): _sub animate ["arm1", 0]\n\ngoto "loop"',
               '?(Behaviour _sub == "SAFE"): _sub animate ["arm1", 0]\n~1\ngoto "loop"', 1)),
    MIG25: (("sub", "#CheckPosLoop\n\n;********** check pilot status",
             "#CheckPosLoop\n~0.5\n\n;********** check pilot status", 1),
            ("sub", '\ngoto "CheckPosition"\n', '\n;goto "CheckPosition"\n', 1)),
    BMP: (("sub", '?(not alive _unit ): goto "exit"\n\ngoto "GO"\t',
           '?(not alive _unit ): goto "exit"\n~0.5\ngoto "GO"\t', 1),),
    F5: (("sub", 'goto "CanopyDown"\ngoto "PollLoop"', 'goto "CanopyDown"\n~0.5\ngoto "PollLoop"', 1),),
    A4: (("sub", "#Mainloop\n_pos = GetPos _plane1;", "#Mainloop\n?(not alive _plane1): exit\n_pos = GetPos _plane1;", 1),
         ("sub", "~0.0001\n", "~0.5\n", 1)),
    "loboart/scripts/m252start.sqs": _maxwind("CoCM252MaxWind"),
    "loboart/scripts/m109start.sqs": _maxwind("CoCM109MaxWind"),
    "loboart/scripts/m1064a3start.sqs": _maxwind("CoCm252MaxWind"),
    "coc_arty/scripts/m252start.sqs": _maxwind("CoCM252MaxWind"),
    "coc_arty/scripts/m109start.sqs": _maxwind("CoCM109MaxWind"),
    AIDLIGHTS: (("sub", PRISTROJE + '\ngoto "ckL"\n',
                 PRISTROJE + '\n?(alive _plane) && (isengineon _plane): goto "ckL"\n', 1),),
    ILSACT: (("line", "_wait = .02", "_wait = 1", 1),),
}
RULES.update({path: (("sub", "#repeat\n~0.001\n", "#repeat\n~1\n", 1),) for path in SETFLAGS})
# Pre-existing, harmless static findings that must stay exactly as they are.
EXPECTED_FINDINGS = {
    "loboart/scripts/m252start.sqs": ("duplicate label: precisionset",),
    "coc_arty/scripts/m252start.sqs": ("duplicate label: precisionset",),
}
ADDONS = tuple(sorted({path.split("/")[0] for path in RULES}))


def repair(documents):
    return apply_rules(documents, RULES, expected_findings=EXPECTED_FINDINGS, post={BMP: _explicit_exit})
