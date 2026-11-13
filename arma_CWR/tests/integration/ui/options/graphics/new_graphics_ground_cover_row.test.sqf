// Pins the Ground Cover row of the Graphics page: it is there, it starts Off,
// it flips On and back Off from the keyboard, and what it was left at (On)
// reaches graphics.cfg on unmount. Each flip is applied live
// (GraphicsProvider::SetRowValue), which the game log shows as a
// "GL33 look: ... ground cover on|off" line. Three screenshots of the page.
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

// Page focus starts on Quality Preset; Ground Cover is seventeen focus steps
// down (the Advanced header is skipped rather than focused).
for "_i" from 1 to 17 do {
    triSendKey 81;
    triSimFrames 1;
};
triSimFrames 3;
triAssertIncludes [(triVisibleTexts), "Ground Cover"];
triScreenshot "ground_cover_row_1_off";

// Off -> On.
triSendKey 79;
triSimFrames 3;
triScreenshot "ground_cover_row_2_on";

// On -> Off.
triSendKey 79;
triSimFrames 3;
triScreenshot "ground_cover_row_3_off";

// Off -> On, and leave it there.
triSendKey 79;
triSimFrames 3;

// Esc closes Graphics: GraphicsPage::Unmount saves graphics.cfg.
triSendKey 41;
triSimFrames 3;

triEndTest;
