// Pins the Render Scale row of the Graphics page: it is there, it starts at
// 100%, one step left gives 85% (the world drawn at 85% of the window and
// upsampled, applied live), and what it was left at reaches graphics.cfg on
// unmount (renderScale=0.85). Two screenshots of the page, at 100% and 85%.
// Scancodes: 81 Down, 80 Left, 40 Enter, 41 Esc.

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

// Page focus starts on Quality Preset; Render Scale is ten focus steps down
// (the Advanced header is skipped rather than focused).
for "_i" from 1 to 10 do {
    triSendKey 81;
    triSimFrames 1;
};
triSimFrames 3;
triAssertIncludes [(triVisibleTexts), "Render Scale"];
triAssertIncludes [(triVisibleTexts), "100%"];
triScreenshot "render_scale_row_1_100";

// 100% -> 85%.
triSendKey 80;
triSimFrames 5;
triAssertIncludes [(triVisibleTexts), "85%"];
triScreenshot "render_scale_row_2_85";

// Esc closes Graphics: GraphicsPage::Unmount saves graphics.cfg.
triSendKey 41;
triSimFrames 3;

triEndTest;
