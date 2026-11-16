// Pins the Surf row of the Graphics page: it is there right under Shore
// Water, it starts at Painted, one step right gives Ribbons (the shore
// ribbons instead of the painted waves, applied live), and what it was left at
// reaches graphics.cfg on unmount (surfStyle=1). Each change is applied live
// (GraphicsProvider::SetRowValue), which the game log shows as a
// "GL33 look: ... surf painted|ribbons" line. Two screenshots of the page.
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

// Page focus starts on Quality Preset; Surf is fifteen focus steps down (the
// Advanced header is skipped rather than focused; Shore Water is fourteen).
for "_i" from 1 to 15 do {
    triSendKey 81;
    triSimFrames 1;
};
triSimFrames 3;
triAssertIncludes [(triVisibleTexts), "Surf"];
triAssertIncludes [(triVisibleTexts), "Painted"];
triScreenshot "surf_row_1_painted";

// Painted -> Ribbons.
triSendKey 79;
triSimFrames 5;
triAssertIncludes [(triVisibleTexts), "Ribbons"];
triScreenshot "surf_row_2_ribbons";

// Esc closes Graphics: GraphicsPage::Unmount saves graphics.cfg.
triSendKey 41;
triSimFrames 3;

triEndTest;
