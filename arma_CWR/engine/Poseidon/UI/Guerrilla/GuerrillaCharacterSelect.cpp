#include <Poseidon/UI/Guerrilla/GuerrillaCharacterSelect.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaBodyPreview.hpp>
#include <Poseidon/Core/resincl.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp> // Pars + Res
#include <Poseidon/IO/Streams/QBStream.hpp>
#include <Poseidon/Foundation/platform.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>

#include <stdio.h>

namespace Poseidon
{
namespace
{
// Display-local idcs. The parent's injected controls stop at 155 (152
// RESERVED for issue #16, next free idc there: 156); this display starts at
// 160 to keep the two ranges visibly disjoint even though idcs are scoped
// per display. IDC_OK (1) / IDC_CANCEL (2) are the CONFIRM / BACK buttons.
constexpr int kIdcCharList = 160; // the roster CListBox (rows carry roster indices as values)
// 161 is the mannequin: a GuerrillaBodyPreview CT_OBJECT injected lazily on
// the first body model that resolves (the UpdateOutfitPreview pattern) -
// hidden, never substituted, when the highlighted class/model is missing.
constexpr int kIdcCharPreview = 161;
constexpr int kIdcCharInfo = 162;     // side/class/source info line under the list
constexpr int kIdcCharTitle = 163;    // "SELECT CHARACTER" title
constexpr int kIdcCharBackdrop = 164; // full-screen dimmer behind everything (background list)

// List row `value` markers. Body rows carry their roster index (>= 0).
constexpr int kRowMatchOutfit = -1; // row 0: the untouched-flow default, no pick
constexpr int kRowSideHeader = -2;  // per-side header rows: never a pick, skipped over

// Normalized-screen layout: roster list on the left, mannequin on the
// right, CONFIRM/BACK bottom right, info line under the list.
constexpr float kListX = 0.05f;
constexpr float kListY = 0.16f;
constexpr float kListW = 0.40f;
constexpr float kListH = 0.62f;
constexpr float kListRowH = 0.04f;
constexpr float kListSizeEx = 0.035f;
constexpr float kInfoY = 0.80f;
constexpr float kInfoW = 0.55f;
constexpr float kInfoH = 0.04f;
constexpr float kInfoSizeEx = 0.03f;
constexpr float kTitleX = 0.20f;
constexpr float kTitleY = 0.06f;
constexpr float kTitleW = 0.60f;
constexpr float kTitleH = 0.08f;
constexpr float kTitleSizeEx = 0.06f;
constexpr float kButtonX = 0.62f;
constexpr float kButtonW = 0.32f;
constexpr float kButtonH = 0.05f;
constexpr float kConfirmY = 0.80f;
constexpr float kBackY = 0.87f;
// The mannequin slot (PlaceInSlot parameters): the free right-hand column.
constexpr float kCharPreviewCX = 0.72f;
constexpr float kCharPreviewTopY = 0.18f;
constexpr float kCharPreviewBottomY = 0.72f;

// The row-0 wording is a contract: it names the same "(match outfit)"
// default the parent's CHARACTER label shows for the untouched flow.
constexpr const char* kMatchOutfitRow = "(match outfit)";

// The styled main-menu Quit class the synthesized buttons clone - the same
// lookup GuerrillaNewGame::InjectFactionCyclers performs.
ParamEntry* FindMenuQuitClass()
{
    const char* mainRsc = Res.FindEntry("RscDisplayMainRemaster") ? "RscDisplayMainRemaster" : "RscDisplayMain";
    ParamEntry* quitCls = Res.FindEntry(mainRsc) ? (Res >> mainRsc).FindEntry("Quit") : nullptr;
    if (quitCls == nullptr || !quitCls->IsClass())
    {
        return nullptr;
    }
    return quitCls;
}

// Author the color array config key GetPackedColor reads ({r,g,b,a} floats).
void AddColor(ParamClass* cls, const char* name, float r, float g, float b, float a)
{
    ParamEntry* color = cls->AddArray(name);
    color->AddValue(r);
    color->AddValue(g);
    color->AddValue(b);
    color->AddValue(a);
}
} // namespace

bool GuerrillaCharacterSelect::CanBuild()
{
    if (Res.FindEntry("RscDisplayGuerrillaCharacterSelect"))
    {
        return true;
    }
    return FindMenuQuitClass() != nullptr;
}

GuerrillaCharacterSelect::GuerrillaCharacterSelect(ControlsContainer* parent,
                                                   const std::vector<GuerrillaBodyChoice>& roster, int currentSel)
    : Display(parent), _roster(roster)
{
    _sel = (currentSel >= 0 && currentSel < (int)_roster.size()) ? currentSel : -1;
    // One shared builder for parent and child (GuerrillaBodyRowLabels), so
    // the CHARACTER label and the list rows can never disagree on a name.
    _rowLabels = GuerrillaBodyRowLabels(_roster);

    // Prefer a dedicated resource when the game data ships one (the engine
    // repo ships no UI resource data); otherwise synthesize everything,
    // gated on the styled menu Quit class - see the class comment.
    if (Res.FindEntry("RscDisplayGuerrillaCharacterSelect"))
    {
        Load("RscDisplayGuerrillaCharacterSelect");
    }
    else
    {
        BuildControls();
    }
    // Never keep a resource's idd - GuerrillaNewGame::OnChildDestroyed keys
    // the result on IDD_GUERRILLA_CHARACTER_SELECT (the parent's own
    // IDD_GUERRILLA_NEW_GAME forcing, one level up).
    _idd = IDD_GUERRILLA_CHARACTER_SELECT;

    // Sync the info line and the mannequin to the opening row (the ctor's
    // SetCurSel deliberately did not fire OnLBSelChanged mid-construction).
    if (CListBox* list = dynamic_cast<CListBox*>(GetCtrl(kIdcCharList)))
    {
        ApplySelection(list->GetCurSel());
    }
}

void GuerrillaCharacterSelect::BuildControls()
{
    ParamEntry* quitCls = FindMenuQuitClass();
    if (quitCls == nullptr)
    {
        return; // no styled menu resource (headless run): own no controls
    }

    // Outlives the injected controls - like the parent's
    // s_guerrillaCyclerCfg, each control back-references its ParamClass for
    // the display's lifetime. Only one GuerrillaCharacterSelect exists at a
    // time (it is the deepest child), so a function-static is safe.
    static ParamFile s_charSelectCfg;
    s_charSelectCfg.Clear();

    // Full-screen dimmer in the BACKGROUND list: the parent's island
    // notebook stays on screen behind a child display, and a plain CStatic
    // with an alpha colorBackground draws exactly the solid quad wanted
    // (CStatic::OnDraw's default branch). Background controls draw before
    // the 3D objects and the foreground, so everything else sits on top.
    {
        ParamClass* cls = s_charSelectCfg.AddClass("GuerrillaCharBackdrop");
        cls->Add("type", CT_STATIC);
        cls->Add("idc", kIdcCharBackdrop);
        cls->Add("style", ST_LEFT);
        cls->Add("x", 0.0f);
        cls->Add("y", 0.0f);
        cls->Add("w", 1.0f);
        cls->Add("h", 1.0f);
        cls->Add("text", RString(""));
        cls->Add("font", RString("tahomaB24"));
        cls->Add("sizeEx", 0.03f);
        AddColor(cls, "colorBackground", 0.0f, 0.0f, 0.0f, 0.75f);
        AddColor(cls, "colorText", 0.0f, 0.0f, 0.0f, 0.0f);
        LoadControlBackground(*cls);
    }

    // Title: the menu's title face (SteelfishB128, the main-menu button
    // font), centered across the top.
    {
        ParamClass* cls = s_charSelectCfg.AddClass("GuerrillaCharTitle");
        cls->Add("type", CT_STATIC);
        cls->Add("idc", kIdcCharTitle);
        cls->Add("style", ST_CENTER);
        cls->Add("x", kTitleX);
        cls->Add("y", kTitleY);
        cls->Add("w", kTitleW);
        cls->Add("h", kTitleH);
        cls->Add("text", RString("SELECT CHARACTER"));
        cls->Add("font", RString("SteelfishB128"));
        cls->Add("sizeEx", kTitleSizeEx);
        AddColor(cls, "colorBackground", 0.0f, 0.0f, 0.0f, 0.0f);
        AddColor(cls, "colorText", 1.0f, 1.0f, 1.0f, 1.0f);
        LoadControl(*cls);
    }

    // The roster list. Body font (tahomaB24: denser rows and wider glyph
    // coverage than the title face); selection in the menu's active red.
    // OnCreateCtrl populates the rows.
    {
        ParamClass* cls = s_charSelectCfg.AddClass("GuerrillaCharList");
        cls->Add("type", CT_LISTBOX);
        cls->Add("idc", kIdcCharList);
        cls->Add("style", ST_LEFT);
        cls->Add("x", kListX);
        cls->Add("y", kListY);
        cls->Add("w", kListW);
        cls->Add("h", kListH);
        cls->Add("font", RString("tahomaB24"));
        cls->Add("sizeEx", kListSizeEx);
        cls->Add("rowHeight", kListRowH);
        AddColor(cls, "colorText", 1.0f, 1.0f, 1.0f, 1.0f);
        AddColor(cls, "colorSelect", 1.0f, 0.0f, 0.0f, 1.0f);
        LoadControl(*cls);
    }

    // Info line under the list: side / class / source of the highlighted row.
    {
        ParamClass* cls = s_charSelectCfg.AddClass("GuerrillaCharInfo");
        cls->Add("type", CT_STATIC);
        cls->Add("idc", kIdcCharInfo);
        cls->Add("style", ST_LEFT);
        cls->Add("x", kListX);
        cls->Add("y", kInfoY);
        cls->Add("w", kInfoW);
        cls->Add("h", kInfoH);
        cls->Add("text", RString(""));
        cls->Add("font", RString("tahomaB24"));
        cls->Add("sizeEx", kInfoSizeEx);
        AddColor(cls, "colorBackground", 0.0f, 0.0f, 0.0f, 0.0f);
        AddColor(cls, "colorText", 1.0f, 1.0f, 1.0f, 1.0f);
        LoadControl(*cls);
    }

    // CONFIRM / BACK: clone the menu Quit class so they pick up the menu's
    // font/colors/sounds - local idc/text/style entries added BEFORE SetBase
    // (ParamClass::Add resolves FindEntry through the base chain and would
    // mutate the shared Quit class otherwise; see the DisplayMain MODS
    // injection comment).
    const struct
    {
        const char* cls;
        int idc;
        const char* text;
        float y;
    } buttons[] = {
        {"GuerrillaCharConfirm", IDC_OK, "CONFIRM", kConfirmY},
        {"GuerrillaCharBack", IDC_CANCEL, "BACK", kBackY},
    };
    for (const auto& button : buttons)
    {
        ParamClass* cls = s_charSelectCfg.AddClass(button.cls);
        cls->Add("idc", button.idc);
        cls->Add("text", RString(button.text));
        cls->Add("style", ST_LEFT);
        cls->SetBase(quitCls->GetClassInterface());
        LoadControl(*cls);
        if (Control* ctrl = dynamic_cast<Control*>(GetCtrl(button.idc)))
        {
            ctrl->SetPos(kButtonX, button.y, kButtonW, kButtonH);
        }
    }
}

Control* GuerrillaCharacterSelect::OnCreateCtrl(int type, int idc, const ParamEntry& cls)
{
    if (idc == kIdcCharList)
    {
        // One factory for both the synthesized and the resource-provided
        // list (the DisplayGame::OnCreateCtrl precedent): rows are row 0
        // "(match outfit)" (value -1), then per side present one dimmed
        // header row "--- <SIDE> ---" (value -2, never a pick) followed by
        // that side's body rows (value = roster index, text = the shared
        // GuerrillaBodyRowLabel). The roster is already emitted WEST, EAST,
        // GUER, CIV with config scan order inside a side, so one linear walk
        // groups it.
        CListBox* list = new CListBox(this, idc, cls);
        int openRow = list->AddString(kMatchOutfitRow);
        list->SetValue(openRow, kRowMatchOutfit);
        const PackedColor headerColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
        RString lastSide;
        for (int i = 0; i < (int)_roster.size(); i++)
        {
            if (i == 0 || stricmp(_roster[i].side, lastSide) != 0)
            {
                lastSide = _roster[i].side;
                char header[64];
                snprintf(header, sizeof(header), "--- %s ---", (const char*)_roster[i].side);
                int headerRow = list->AddString(header);
                list->SetValue(headerRow, kRowSideHeader);
                list->SetFtColor(headerRow, headerColor);
            }
            int row = list->AddString(_rowLabels[i]);
            list->SetValue(row, i);
            // Row data carries the exact classname: the display never reads
            // it back (value -> roster index is the pick channel), but it
            // gives the tri harness a stable selector (triSelectListByData)
            // that survives label wording and row-position changes.
            list->SetData(row, _roster[i].className);
            if (i == _sel)
            {
                openRow = row;
            }
        }
        // No update: the display is mid-construction; the ctor syncs the
        // dependent readouts once every control exists.
        list->SetCurSel(openRow, false);
        _lastListRow = openRow;
        return list;
    }
    return Display::OnCreateCtrl(type, idc, cls);
}

ControlObject* GuerrillaCharacterSelect::OnCreateObject(int type, int idc, const ParamEntry& cls)
{
    if (idc == kIdcCharPreview)
    {
        // the DisplayNewUser::OnCreateObject -> CHead hook, for the mannequin
        return new GuerrillaBodyPreview(this, idc, cls);
    }
    return Display::OnCreateObject(type, idc, cls);
}

void GuerrillaCharacterSelect::OnSimulate(EntityAI* vehicle)
{
    if (auto* preview = dynamic_cast<GuerrillaBodyPreview*>(GetCtrl(kIdcCharPreview)))
    {
        if (preview->IsVisible())
        {
            preview->SimulateTurntable();
        }
    }
    Display::OnSimulate(vehicle);
}

void GuerrillaCharacterSelect::OnLBSelChanged(int idc, int curSel)
{
    if (idc != kIdcCharList)
    {
        Display::OnLBSelChanged(idc, curSel);
        return;
    }
    CListBox* list = dynamic_cast<CListBox*>(GetCtrl(kIdcCharList));
    if (!list || curSel < 0)
    {
        return;
    }
    // Header rows are not picks: keep moving in the travel direction
    // (inferred from the last landed row, forward by default) until a
    // selectable row turns up; bounce back the other way from the landing
    // row when the walk runs off the list. Row 0 is always the selectable
    // "(match outfit)" row, so upward walks always land.
    int dir = curSel >= _lastListRow ? 1 : -1;
    int row = curSel;
    while (row >= 0 && row < list->GetSize() && list->GetValue(row) == kRowSideHeader)
    {
        row += dir;
    }
    if (row < 0 || row >= list->GetSize())
    {
        row = curSel;
        while (row >= 0 && row < list->GetSize() && list->GetValue(row) == kRowSideHeader)
        {
            row -= dir;
        }
    }
    if (row < 0 || row >= list->GetSize())
    {
        row = curSel; // every row is a header: nothing selectable, keep it
    }
    if (row != curSel)
    {
        list->SetCurSel(row, false); // no re-fire: this handler already runs
    }
    ApplySelection(row);
}

void GuerrillaCharacterSelect::OnLBDblClick(int idc, int curSel)
{
    if (idc == kIdcCharList)
    {
        CListBox* list = dynamic_cast<CListBox*>(GetCtrl(kIdcCharList));
        if (list && list->GetValue(curSel) != kRowSideHeader)
        {
            // double-click on a pickable row confirms, like the parent's
            // island list double-click launches
            OnButtonClicked(IDC_OK);
        }
        return;
    }
    Display::OnLBDblClick(idc, curSel);
}

void GuerrillaCharacterSelect::ApplySelection(int row)
{
    CListBox* list = dynamic_cast<CListBox*>(GetCtrl(kIdcCharList));
    if (!list || row < 0 || row >= list->GetSize())
    {
        return;
    }
    _lastListRow = row;
    int value = list->GetValue(row);
    if (value >= kRowMatchOutfit)
    {
        _sel = (value >= 0 && value < (int)_roster.size()) ? value : -1;
    }

    // The info line: side / sanitized class / source addon for a body row,
    // the outfit hint for the default row. Never an underscore anywhere -
    // the menu fonts drop that glyph (GuerrillaSanitizeLabel).
    char buffer[512];
    if (value >= 0 && value < (int)_roster.size())
    {
        const GuerrillaBodyChoice& body = _roster[value];
        RString className = GuerrillaSanitizeLabel(body.className);
        RString source = body.addon.GetLength() > 0 ? GuerrillaSanitizeLabel(body.addon) : RString("Base game");
        snprintf(buffer, sizeof(buffer), "SIDE: %s  CLASS: %s  SOURCE: %s", (const char*)body.side,
                 (const char*)className, (const char*)source);
    }
    else if (value == kRowMatchOutfit)
    {
        snprintf(buffer, sizeof(buffer), "Matches the OUTFIT selection");
    }
    else
    {
        buffer[0] = 0; // header rows (unreachable via ApplySelection callers)
    }
    if (IControl* info = GetCtrl(kIdcCharInfo))
    {
        if (CStatic* text = dynamic_cast<CStatic*>(info))
        {
            text->SetText(buffer);
        }
        else if (CActiveText* text = dynamic_cast<CActiveText*>(info))
        {
            text->SetText(buffer);
        }
    }

    UpdatePreview();
}

void GuerrillaCharacterSelect::UpdatePreview()
{
    RString previewClass;
    if (_sel >= 0 && _sel < (int)_roster.size())
    {
        previewClass = _roster[_sel].className;
    }
    // "(match outfit)" previews nothing: without the parent's outfit context
    // there is no honest body to show, so the mannequin hides (the same
    // plan-15 shaped degrade as a missing model).
    RString model;
    if (previewClass.GetLength() > 0)
    {
        model = GuerrillaOutfitPreviewModel(Pars.FindEntry("CfgVehicles"), previewClass,
                                            [](RString path) { return QIFStreamB::FileExist(path); });
    }
    GuerrillaBodyPreview* preview = dynamic_cast<GuerrillaBodyPreview*>(GetCtrl(kIdcCharPreview));
    if (model.GetLength() == 0)
    {
        if (preview)
        {
            preview->ShowCtrl(false);
        }
        return;
    }
    if (!preview)
    {
        // Keep the headless invariant: never inject the mannequin into a
        // display that otherwise owns no controls.
        if (!GetCtrl(kIdcCharList))
        {
            return;
        }
        // First model that resolved: inject the CT_OBJECT now. Function-
        // static lifetime for the same reason as s_charSelectCfg: the
        // control back-references its ParamClass for the display's lifetime.
        static ParamFile s_charPreviewCfg;
        s_charPreviewCfg.Clear();
        ParamClass* cls = s_charPreviewCfg.AddClass("GuerrillaCharacterPreview");
        cls->Add("type", CT_OBJECT);
        cls->Add("idc", kIdcCharPreview);
        cls->Add("model", model);
        ParamEntry* position = cls->AddArray("position");
        position->AddValue(0.0f);
        position->AddValue(0.0f);
        position->AddValue(kGuerrillaPreviewDepthCfg); // PlaceInSlot refines below
        ParamEntry* direction = cls->AddArray("direction");
        direction->AddValue(0.0f);
        direction->AddValue(0.0f);
        direction->AddValue(1.0f);
        ParamEntry* up = cls->AddArray("up");
        up->AddValue(0.0f);
        up->AddValue(1.0f);
        up->AddValue(0.0f);
        cls->Add("scale", 1.0f);
        LoadObject(*cls);
        preview = dynamic_cast<GuerrillaBodyPreview*>(GetCtrl(kIdcCharPreview));
        if (!preview)
        {
            return;
        }
    }
    // On the injection pass the constructor already loaded this model and
    // SetModel just records the name (the shape bank caches the reload); on
    // later passes it swaps the shape when the body actually changed.
    preview->SetModel(model);
    preview->SetCivilian(GuerrillaClassIsCivilian(Pars.FindEntry("CfgVehicles"), previewClass));
    preview->PlaceInSlot(kCharPreviewCX, kCharPreviewTopY, kCharPreviewBottomY);
    preview->ShowCtrl(true);
}

} // namespace Poseidon
