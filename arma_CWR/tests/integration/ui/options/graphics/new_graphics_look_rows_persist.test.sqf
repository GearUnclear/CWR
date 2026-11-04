// Pins that the Remastered lighting's look rows (Shadow Detail, Shore Water,
// Lamp Shadows) exist on the Graphics page, change from the keyboard and reach
// graphics.cfg on unmount. The Pester wrapper (tests/smoke/
// graphics_apply_persist.tests.ps1) asserts shadowDetail=3, shoreWater=0 and
// lampShadows=0 in the file.
// Scancodes: 81 Down, 79 Right, 40 Enter, 41 Esc.

triSimFrames 5;
triSetLanguage "English";
triSimFrames 5;
triClickText "OPTIONS";
triSimFrames 5;

// Index focus starts on Audio; two rows down is Graphics.
triSendKey 81;
triSimFrames 1;
triSendKey 81;
triSimFrames 1;
triSendKey 40;
triSimFrames 5;

// Page focus starts on Quality Preset; Shadow Detail is thirteen focus steps
// down (the Advanced header is skipped rather than focused, Lighting is
// twelve).
for "_i" from 1 to 13 do {
    triSendKey 81;
    triSimFrames 1;
};
// Normal -> High.
triSendKey 79;
triSimFrames 2;

// Shore Water: on -> off.
triSendKey 81;
triSimFrames 1;
triSendKey 79;
triSimFrames 2;

// Lamp Shadows: on -> off.
triSendKey 81;
triSimFrames 1;
triSendKey 79;
triSimFrames 2;

// Esc closes Graphics: GraphicsPage::Unmount saves graphics.cfg.
triSendKey 41;
triSimFrames 3;

triEndTest;
