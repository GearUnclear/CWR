#include <Poseidon/Game/Guerrilla/LegendRegistry.hpp>
#include <Poseidon/Game/Guerrilla/PortraitRecipe.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Random/randomGen.hpp>
#include <Poseidon/Game/Guerrilla/PortraitService.hpp>
#include <Evaluator/express.hpp>
#include <Poseidon/World/World.hpp>
#include <Poseidon/UI/Map/UIMap.hpp>
#include <Poseidon/UI/Controls/UIControls.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>
#include <Poseidon/Foundation/Strings/RString.hpp>
#include <Poseidon/Input/InputSubsystem.hpp>
#include <Poseidon/Foundation/Framework/AppFrame.hpp>
#include <Poseidon/Input/UserActionDesc.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>               // global Pars for triListFaces
#include <Poseidon/Graphics/Textures/TextureBank.hpp> // Texture::Name for triBriefingImages

#include <SDL3/SDL_scancode.h>
#include <Poseidon/AI/AICenter.hpp>
#include <Poseidon/Network/NetworkIface.hpp>
#include <Poseidon/IO/Serialization/ParamArchive.hpp>
#include <Poseidon/IO/Streams/QStream.hpp>
#include <Poseidon/Core/SaveVersion.hpp>
#include <sstream>

#include <cstdio>

using namespace Poseidon;

extern void SDLInput_BufferKeyEvent(SDL_Scancode sc, bool down, DWORD timestamp);

/// triOpenMap -> "OK" or "FAIL:<reason>"
GameValue TriOpenMap(const GameState* /*state*/)
{
    if (!GWorld)
        return GameValue("FAIL:no_world");
    GWorld->CreateMainMap();
    GWorld->ForceMap(true);
    LOG_INFO(Core, "[tri] triOpenMap");
    return GameValue("OK");
}

/// triShowMap <0|1> -> "OK" or "FAIL:<reason>"
GameValue TriShowMap(const GameState* /*state*/, GameValuePar arg)
{
    if (!GWorld)
        return GameValue("FAIL:no_world");
    auto* map = dynamic_cast<DisplayMap*>(GWorld->Map());
    if (!map)
        return GameValue("FAIL:no_map");
    map->ShowMap((float)arg != 0.0f);
    LOG_INFO(Core, "[tri] triShowMap {}", (float)arg != 0.0f);
    return GameValue("OK");
}

/// triMapSetScale <scale> — set the in-mission map zoom (clamped to the map's own min/max).
GameValue TriMapSetScale(const GameState* /*state*/, GameValuePar arg)
{
    if (!GWorld)
        return GameValue("FAIL:no_world");
    auto* map = dynamic_cast<DisplayMap*>(GWorld->Map());
    if (!map || !map->GetMap())
        return GameValue("FAIL:no_map");
    map->GetMap()->SetScale((float)arg);
    LOG_INFO(Core, "[tri] triMapSetScale {}", (float)arg);
    return GameValue("OK");
}

/// triMapGetScale -> the in-mission map zoom scale, or -1 if no map.
GameValue TriMapGetScale(const GameState* /*state*/)
{
    if (!GWorld)
        return GameValue((GameScalarType)-1.0f);
    auto* map = dynamic_cast<DisplayMap*>(GWorld->Map());
    if (!map || !map->GetMap())
        return GameValue((GameScalarType)-1.0f);
    return GameValue((GameScalarType)map->GetMap()->GetScale());
}

/// triBindAction ["<ActionName>", <scancode>] -> "OK" / "FAIL:<reason>". Replaces
/// the action's binding with a single keyboard key in every context; a rebind hook.
GameValue TriBindAction(const GameState* /*state*/, GameValuePar arg)
{
    if (arg.GetType() != GameArray)
        return GameValue("FAIL:expected_array");
    const GameArrayType& a = arg;
    if (a.Size() < 2)
        return GameValue("FAIL:need_action_and_key");
    RString name = (GameStringType)a[0];
    int sc = static_cast<int>(static_cast<GameScalarType>(a[1]));

    const UserActionDesc* descs = InputSubsystem::GetUserActionDesc();
    int action = -1;
    for (int i = 0; i < UAN; ++i)
        if (descs[i].name && stricmp(descs[i].name, (const char*)name) == 0)
        {
            action = i;
            break;
        }
    if (action < 0)
        return GameValue("FAIL:unknown_action");

    auto& input = InputSubsystem::Instance();
    for (int c = 0; c < static_cast<int>(InputContext::Count); ++c)
    {
        InputProfile& p = input.GetProfile(static_cast<InputContext>(c));
        p.ClearBindings(static_cast<UserAction>(action));
        p.Bind(static_cast<UserAction>(action), InputCode::Key(static_cast<SDL_Scancode>(sc)));
    }
    LOG_INFO(Core, "[tri] triBindAction {} sc=0x{:x}", (const char*)name, sc);
    return GameValue("OK");
}

/// triShowVoiceOverlay <0|1> — create / destroy the CapsLock VoIP voice-chat
/// overlay (the cursor-less HUD that normally appears while UAVoiceOverNet is
/// held).  Lets tests reproduce the menu-cursor-vanishes-under-overlay case
/// without driving the live action binding.
GameValue TriShowVoiceOverlay(const GameState* /*state*/, GameValuePar arg)
{
    if (!GWorld)
        return GameValue("FAIL:no_world");
    if ((float)arg != 0.0f)
        GWorld->CreateVoiceChat(false);
    else
        GWorld->DestroyVoiceChat(0);
    LOG_INFO(Core, "[tri] triShowVoiceOverlay {}", (float)arg != 0.0f);
    return GameValue("OK");
}

/// triClickBriefingLink "<href>" -> "OK:section=<name>,img=<n>" / "FAIL:<reason>".
/// Finds the note in-page link with this href in the in-mission map's briefing/
/// notes control and replays its click (the `#X` -> SwitchSection path), then
/// reports the resulting current section and how many image fields it holds.
/// Used to regression-test in-page briefing links.
GameValue TriClickBriefingLink(const GameState* /*state*/, GameValuePar arg)
{
    if (!GWorld)
        return GameValue("FAIL:no_world");
    auto* map = dynamic_cast<DisplayMap*>(GWorld->Map());
    if (!map)
        return GameValue("FAIL:no_map");
    CHTML* html = map->GetBriefingControl();
    if (!html)
        return GameValue("FAIL:no_briefing");
    // Diagnostic: FindControl (the real click router) only delivers to a control
    // that is visible AND enabled; otherwise map foreground controls may receive
    // the click before the notes link does.
    LOG_INFO(Core, "[tri] briefing control: visible={} enabled={} idc={}", html->IsVisible(), html->IsEnabled(),
             html->IDC());
    GameStringType href = static_cast<GameStringType>(arg);
    RString result = html->ActivateHRef((const char*)href);
    LOG_INFO(Core, "[tri] triClickBriefingLink {} -> {}", (const char*)href, (const char*)result);
    return GameValue((const char*)result);
}

/// triProbeClickBriefingLink "<href>" -> "OK:hit=x,y,section=<name>" /
/// "FAIL:findfield_miss" / "FAIL:no_link". Like triClickBriefingLink but goes
/// through the REAL hit-test (FindField) + OnMouseMove/OnLButtonDown, to locate
/// where a real click on the notes link is routed.
GameValue TriProbeClickBriefingLink(const GameState* /*state*/, GameValuePar arg)
{
    if (!GWorld)
        return GameValue("FAIL:no_world");
    auto* map = dynamic_cast<DisplayMap*>(GWorld->Map());
    if (!map)
        return GameValue("FAIL:no_map");
    CHTML* html = map->GetBriefingControl();
    if (!html)
        return GameValue("FAIL:no_briefing");
    GameStringType href = static_cast<GameStringType>(arg);
    RString result = html->ProbeClickHRef((const char*)href);
    LOG_INFO(Core, "[tri] triProbeClickBriefingLink {} -> {}", (const char*)href, (const char*)result);
    return GameValue((const char*)result);
}

/// triBriefingSection -> the current briefing/notes section name (test hook).
GameValue TriBriefingSection(const GameState* /*state*/)
{
    if (!GWorld)
        return GameValue("");
    auto* map = dynamic_cast<DisplayMap*>(GWorld->Map());
    if (!map)
        return GameValue("");
    CHTML* html = map->GetBriefingControl();
    if (!html)
        return GameValue("");
    RString name = html->CurrentSectionName();
    LOG_INFO(Core, "[tri] triBriefingSection -> {}", (const char*)name);
    return GameValue((const char*)name);
}

/// triBriefingSwitch "<section>" -> switch the briefing/notes to a named section;
/// returns the resulting section name.
GameValue TriBriefingSwitch(const GameState* /*state*/, GameValuePar arg)
{
    if (!GWorld)
        return GameValue("FAIL:no_world");
    auto* map = dynamic_cast<DisplayMap*>(GWorld->Map());
    if (!map)
        return GameValue("FAIL:no_map");
    CHTML* html = map->GetBriefingControl();
    if (!html)
        return GameValue("FAIL:no_briefing");
    GameStringType sec = static_cast<GameStringType>(arg);
    html->SwitchSection((const char*)sec);
    return GameValue((const char*)html->CurrentSectionName());
}

/// triBriefingLinkRoute "<href>" -> "OK:linkpos=x,y,route_idc=<n>,briefing_idc=56"
/// / "FAIL:<reason>". Finds the on-screen centre of the notes link, then asks the
/// in-mission map's hit-test (GetCtrl — the same foreground-first order the real
/// click dispatch uses) which control a click there routes to. If route_idc is
/// not the briefing control's idc, another control (e.g. the full-screen map)
/// intercepts the click before the notes.
GameValue TriBriefingLinkRoute(const GameState* /*state*/, GameValuePar arg)
{
    if (!GWorld)
        return GameValue("FAIL:no_world");
    auto* map = dynamic_cast<DisplayMap*>(GWorld->Map());
    if (!map)
        return GameValue("FAIL:no_map");
    CHTML* html = map->GetBriefingControl();
    if (!html)
        return GameValue("FAIL:no_briefing");
    GameStringType href = static_cast<GameStringType>(arg);
    float sx = 0, sy = 0;
    if (!html->FindLinkScreenPos((const char*)href, sx, sy))
        return GameValue("FAIL:no_link");
    IControl* hit = map->GetCtrl(sx, sy);
    int routeIdc = hit ? hit->IDC() : -1;
    char buf[160];
    snprintf(buf, sizeof(buf), "OK:linkpos=%.3f,%.3f,route_idc=%d,briefing_idc=%d", sx, sy, routeIdc, html->IDC());
    LOG_INFO(Core, "[tri] triBriefingLinkRoute {} -> {}", (const char*)href, buf);
    return GameValue(buf);
}

/// triBriefingClickAt ["<href>", "<expectedSection>"] -> "OK" / "FAIL:..". Drives
/// the REAL CHTML::OnLButtonDown at the link's on-screen position with NO latched
/// hover (clears _activeField first), then asserts the notes switched to the
/// expected section.
GameValue TriBriefingClickAt(const GameState* /*state*/, GameValuePar arg)
{
    if (arg.GetType() != GameArray)
        return GameValue("FAIL:expected_array");
    const GameArrayType& a = arg;
    if (a.Size() < 2)
        return GameValue("FAIL:need_href_and_section");
    RString href = (GameStringType)a[0];
    RString want = (GameStringType)a[1];
    if (!GWorld)
        return GameValue("FAIL:no_world");
    auto* map = dynamic_cast<DisplayMap*>(GWorld->Map());
    if (!map)
        return GameValue("FAIL:no_map");
    CHTML* html = map->GetBriefingControl();
    if (!html)
        return GameValue("FAIL:no_briefing");
    float sx = 0, sy = 0;
    if (!html->FindLinkScreenPos((const char*)href, sx, sy))
        return GameValue("FAIL:no_link");
    // Clear any latched hover field so this exercises a click that arrives without
    // a preceding OnMouseMove for this control — the #32 failure mode.
    html->OnMouseMove(sx, sy, false);
    html->OnLButtonDown(sx, sy);
    RString got = html->CurrentSectionName();
    LOG_INFO(Core, "[tri] triBriefingClickAt {} -> section={} (want {})", (const char*)href, (const char*)got,
             (const char*)want);
    if (stricmp((const char*)got, (const char*)want) != 0)
    {
        char buf[160];
        snprintf(buf, sizeof(buf), "FAIL:got=%s,want=%s", (const char*)got, (const char*)want);
        return GameValue(buf);
    }
    return GameValue("OK");
}

/// triBriefingMetrics -> "x,y,w,h,scale,pageW,pageH" (%.4f each) / "FAIL:<reason>".
/// The briefing/notes control's screen rect and scale (a per-frame projection of
/// the 3D notepad's memory-point quad) plus the page width/height the HTML
/// layout wraps and paginates against. Page units, so a Guerrilla journal
/// capture lane can log the budget its pages were laid out for.
GameValue TriBriefingMetrics(const GameState* /*state*/)
{
    if (!GWorld)
        return GameValue("FAIL:no_world");
    auto* map = dynamic_cast<DisplayMap*>(GWorld->Map());
    if (!map)
        return GameValue("FAIL:no_map");
    CHTML* html = map->GetBriefingControl();
    if (!html)
        return GameValue("FAIL:no_briefing");
    char buf[200];
    snprintf(buf, sizeof(buf), "%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f", html->X(), html->Y(), html->W(), html->H(),
             html->GetScale(), html->GetPageWidth(), html->GetPageHeight());
    LOG_INFO(Core, "[tri] triBriefingMetrics -> {}", buf);
    return GameValue(buf);
}

/// triBriefingSlot <n> -> "face,size" / "FAIL:<reason>". The face name and the
/// size (%.4f) bound to HTMLFormat slot n of the briefing/notes control: 0 = P,
/// 1..6 = H1..H6; anything else is "FAIL:bad_slot". The face is "" when the slot
/// has no font. Lets a capture lane assert the Guerrilla journal's slot binding
/// (garamond / couriernewb / cwrpen) against the live CfgFonts table.
GameValue TriBriefingSlot(const GameState* /*state*/, GameValuePar arg)
{
    if (!GWorld)
        return GameValue("FAIL:no_world");
    auto* map = dynamic_cast<DisplayMap*>(GWorld->Map());
    if (!map)
        return GameValue("FAIL:no_map");
    CHTML* html = map->GetBriefingControl();
    if (!html)
        return GameValue("FAIL:no_briefing");
    const int n = static_cast<int>(static_cast<GameScalarType>(arg));
    if (n < (int)HFP || n > (int)HFH6)
        return GameValue("FAIL:bad_slot");
    const HTMLFormat slot = static_cast<HTMLFormat>(n);
    const Font* font = html->GetFormatFont(slot, false);
    const char* face = (font && font->Name()) ? font->Name() : "";
    char buf[200];
    snprintf(buf, sizeof(buf), "%s,%.4f", face, html->GetFormatSize(slot));
    LOG_INFO(Core, "[tri] triBriefingSlot {} -> {}", n, buf);
    return GameValue(buf);
}

/// triBriefingImages -> "name|w|h;name|w|h;..." / "" / "FAIL:<reason>".
/// Every HFImg field of the briefing/notes control's CURRENT section, in layout
/// order: the texture's own name (or "-" when the field never resolved a
/// texture), and the field's stored width and height. Those are PAGE units, not
/// the w640/h480 numbers handed to AddImage: AddImage divides them by 640 and
/// 480 (UIControlsExt.cpp), so compare against the pageW/pageH triBriefingMetrics
/// reports. HTMLField keeps no copy of the source path, only Ref<Texture>
/// texture1 (UIControlsBase.hpp), so "-" is exactly the "the .paa did not
/// resolve" signal a portrait lane needs, and a name that carries the expected
/// portrait key is proof the journal drew the photograph it named.
GameValue TriBriefingImages(const GameState* /*state*/)
{
    if (!GWorld)
        return GameValue("FAIL:no_world");
    auto* map = dynamic_cast<DisplayMap*>(GWorld->Map());
    if (!map)
        return GameValue("FAIL:no_map");
    CHTML* html = map->GetBriefingControl();
    if (!html)
        return GameValue("FAIL:no_briefing");
    const int s = html->CurrentSection();
    if (s < 0 || s >= html->NSections())
        return GameValue("FAIL:no_section");

    const HTMLSection& section = html->GetSection(s);
    char buf[2048];
    int used = 0;
    buf[0] = 0;
    for (int i = 0; i < section.fields.Size(); ++i)
    {
        const HTMLField& field = section.fields[i];
        if (field.format != HFImg)
            continue;
        const char* name = (field.texture1 && field.texture1->Name()) ? field.texture1->Name() : "-";
        const int n = snprintf(buf + used, sizeof(buf) - used, "%s%s|%.4f|%.4f", used > 0 ? ";" : "", name, field.width,
                               field.height);
        if (n < 0 || used + n >= (int)sizeof(buf))
        {
            buf[used] = 0;
            break;
        }
        used += n;
    }
    LOG_INFO(Core, "[tri] triBriefingImages section={} -> {}", s, buf);
    return GameValue(buf);
}

/// triListFaces -> array of CfgFaces class names usable on a MAN body.
/// Walks Pars >> "CfgFaces" the way DisplayUIMenus.cpp's face list box does, but
/// skips the entries that list box deliberately keeps: an entry carrying a
/// `disabled` key, an entry whose `woman` value reads > 0.5 (Head::SetFace
/// silently returns on a woman/man mismatch, so a shoot fed a woman token would
/// photograph an unchanged head), and the "Custom" placeholder. Class names, not
/// display names, because setFace takes the class name; and read through
/// FindEntry("name") rather than the dialog's unconditional `entry >> "name"`,
/// so a nameless entry cannot fault the walk.
GameValue TriListFaces(const GameState* /*state*/)
{
    AutoArray<GameValue> result;
    const ParamEntry* faces = Pars.FindEntry("CfgFaces");
    if (!faces)
        return GameValue(result);

    for (int i = 0; i < faces->GetEntryCount(); ++i)
    {
        const ParamEntry& entry = faces->GetEntry(i);
        if (!entry.IsClass())
            continue;
        if (entry.FindEntry("disabled"))
            continue;
        const ParamEntry* woman = entry.FindEntry("woman");
        if (woman && (float)(*woman) > 0.5f)
            continue;
        RString name = entry.GetName();
        if (name.GetLength() == 0)
            continue;
        if (stricmp(name, "Custom") == 0)
            continue;
        result.Add(GameValue(name));
    }
    LOG_INFO(Core, "[tri] triListFaces -> {} usable faces", result.Size());
    return GameValue(result);
}

GameValue TriPortraitStats(const GameState*)
{
    const auto& service = Guerrilla::PortraitService::Instance();
    AutoArray<GameValue> values;
    values.Add(float(service.Total()));
    values.Add(float(service.Completed()));
    values.Add(float(service.RenderCount()));
    values.Add(float(service.CacheHits()));
    return GameValue(values);
}
GameValue TriPortraitId(const GameState*, GameValuePar arg)
{
    const GameArrayType& args = arg;
    if (args.Size() != 2)
        return GameValue("FAIL:arguments");
    return GameValue(Guerrilla::PortraitService::Instance().Request({RString(args[0]), RString(args[1])}, true));
}

GameValue TriPortraitRecreate(const GameState*)
{
    if (!GEngine)
        return GameValue(false);
    // Exercise both content remount and the full device-resource reset.
    GEngine->ResetForRemount();
    return GameValue(GEngine->ResetHard());
}

GameValue TriPortraitRow(const GameState*, GameValuePar arg)
{
    const int index = int(float(arg));
    const auto& registry = Guerrilla::LegendRegistry::Instance();
    AutoArray<GameValue> result;
    if (index < 0 || index >= registry.RowCount())
        return GameValue(result);
    const auto& row = registry.Row(index);
    auto& service = Guerrilla::PortraitService::Instance();
    const RString id = service.Request({row.bodyClass, row.face});
    result.Add(GameValue(row.bodyClass));
    result.Add(GameValue(row.face));
    result.Add(GameValue(id));
    result.Add(GameValue(float(service.Find(id)->status)));
    return GameValue(result);
}

GameValue TriPortraitPixels(const GameState*, GameValuePar arg)
{
    const GameArrayType& args = arg;
    if (args.Size() != 2)
        return GameValue("FAIL:arguments");
    Guerrilla::PortraitRenderer renderer;
    std::vector<uint8_t> rgb;
    std::string error;
    if (!renderer.Prepare({RString(args[0]), RString(args[1])}, error) || !renderer.Capture(rgb, error))
        return GameValue(RString("FAIL:") + RString(error.c_str()));
    const auto pixels = Guerrilla::StylePortrait(rgb);
    return GameValue(
        Guerrilla::PortraitDigest(std::string(reinterpret_cast<const char*>(pixels.data()), pixels.size())).c_str());
}

GameValue TriPortraitRefresh(const GameState*)
{
    auto* map = GWorld ? dynamic_cast<DisplayMap*>(GWorld->Map()) : nullptr;
    CHTML* html = map ? map->GetBriefingControl() : nullptr;
    if (!html)
        return GameValue(false);
    const RString section = html->CurrentSectionName();
    Guerrilla::PortraitService::Instance().Teardown();
    map->RefreshGuerrillaJournal();
    if (html->CurrentSectionName() != section)
        return GameValue(false);
    const auto& fields = html->GetSection(html->CurrentSection()).fields;
    bool placeholder = false;
    for (int i = 0; i < fields.Size(); ++i)
    {
        if (fields[i].format == HFImg && fields[i].texture1)
            return GameValue(false);
        placeholder = placeholder || fields[i].text == RString("Photograph unavailable");
    }
    return GameValue(placeholder && Guerrilla::PortraitService::Instance().Pending());
}

GameValue TriBriefingFits(const GameState*)
{
    auto* map = GWorld ? dynamic_cast<DisplayMap*>(GWorld->Map()) : nullptr;
    CHTML* html = map ? map->GetBriefingControl() : nullptr;
    if (!html || html->CurrentSection() < 0 || html->CurrentSection() >= html->NSections())
        return GameValue(false);
    const auto& section = html->GetSection(html->CurrentSection());
    float height = 0;
    for (int i = 0; i < section.rows.Size(); ++i)
    {
        const auto& row = section.rows[i];
        height += row.height;
        if (row.width > html->GetPageWidth() + 0.001f)
            return GameValue(false);
    }
    LOG_INFO(Core, "[tri] briefing fit height={} budget={}", height, html->GetPageHeight());
    return GameValue(section.rows.Size() > 0 && height <= html->GetPageHeight() + 0.001f);
}

GameValue TriPortraitCancel(const GameState*)
{
    auto& service = Guerrilla::PortraitService::Instance();
    service.Teardown();
    service.PrepareRoster(Guerrilla::CampaignPortraitRoster());
    service.Advance();
    const size_t completed = service.Completed();
    // This is the raw buffer used by SDL's event pump, before normal frame
    // processing. Loading must consume it even though simulation is suspended.
    const auto tick = Foundation::GlobalTickCount();
    SDLInput_BufferKeyEvent(SDL_SCANCODE_ESCAPE, true, tick);
    SDLInput_BufferKeyEvent(SDL_SCANCODE_ESCAPE, false, tick + 1);
    const bool cancelled = !Guerrilla::PrepareCampaignPortraits();
    const bool retained =
        completed == 1 && service.Total() == completed && service.Completed() == completed && !service.Pending();
    const bool resumed = Guerrilla::PrepareCampaignPortraits();
    return GameValue(cancelled && retained && resumed && service.Completed() == service.Total());
}

// Capture synchronously without advancing simulation, then check the externally
// observable world and random stream. This also exercises the mid-frame path.
GameValue TriPortraitIsolation(const GameState*, GameValuePar arg)
{
    const GameArrayType& args = arg;
    if (args.Size() == 0)
    {
        for (const auto& appearance : Guerrilla::CampaignPortraitRoster())
        {
            AutoArray<GameValue> pair;
            pair.Add(GameValue(appearance.body));
            pair.Add(GameValue(appearance.face));
            if (!static_cast<bool>(TriPortraitIsolation(nullptr, GameValue(pair))))
                return GameValue(false);
        }
        return GameValue(true);
    }
    if (args.Size() != 2 || !GWorld || !GScene)
        return GameValue(false);
    const auto registrySnapshot = []
    {
        ParamArchiveSave archive(WorldSerializeVersion);
        archive.Serialize("GuerrillaLegends", Guerrilla::LegendRegistry::Instance(), 14);
        QOStream stream;
        archive.GetParamFile()->Save(stream, 0);
        return std::string(stream.str(), stream.pcount());
    };
    const auto aiSnapshot = []
    {
        std::ostringstream out;
        for (auto* center : {GWorld->GetWestCenter(), GWorld->GetEastCenter(), GWorld->GetGuerrilaCenter(),
                             GWorld->GetCivilianCenter(), GWorld->GetLogicCenter()})
        {
            out << center << ':';
            if (!center)
                continue;
            out << center->NGroups() << ':';
            for (int i = 0; i < center->NGroups(); ++i)
            {
                const auto* group = center->GetGroup(i);
                out << group << ':';
                if (group)
                {
                    const auto id = group->GetNetworkId();
                    out << group->NUnits() << ':' << id.creator << ':' << id.id << ';';
                }
            }
        }
        return out.str();
    };
    const auto registryBefore = registrySnapshot(), aiBefore = aiSnapshot();
    const auto networkState = GetNetworkManager().GetGameState();
    const auto networkServer = GetNetworkManager().GetServerState();
    const int networkPlayer = GetNetworkManager().GetPlayer(), networkRoles = GetNetworkManager().NPlayerRoles();
    const int nextMagazine = GWorld->NextMagazineID();
    auto random = std::make_unique<RandomGenerator>(GRandGen);
    const auto time = Glob.time;
    const int vehicles = GWorld->NVehicles(), fast = GWorld->NFastVehicles(), out = GWorld->NOutVehicles();
    const int rows = Guerrilla::LegendRegistry::Instance().RowCount(), lights = GScene->NLights();
    const Camera camera = *GScene->GetCamera();
    const auto* sun = GScene->MainLight();
    const float clouds = GLandscape ? GLandscape->CloudsAlpha() : 0;
    const float rain = GLandscape ? GLandscape->GetRainDensity() : 0;
    const Vector3 wind = GLandscape ? GLandscape->GetWind() : VZero;
    std::vector<uint8_t> rgb;
    std::string error;
    {
        Guerrilla::PortraitRenderer renderer;
        if (!renderer.Prepare({RString(args[0]), RString(args[1])}, error) || !renderer.Capture(rgb, error))
        {
            LOG_WARN(Core, "[tri] isolated portrait failed: {}", error);
            return GameValue(false);
        }
    }
    auto after = std::make_unique<RandomGenerator>(GRandGen);
    bool stable = true;
    for (int i = 0; i < 16; ++i)
        stable = stable && random->RandomValue() == after->RandomValue();
    LOG_INFO(Core,
             "[tri] isolation components rng={} time={} "
             "vehicles={}/{},fast={}/{},out={}/{},rows={}/{},lights={}/{},sun={} camera={}/{}",
             stable, Glob.time == time, vehicles, GWorld->NVehicles(), fast, GWorld->NFastVehicles(), out,
             GWorld->NOutVehicles(), rows, Guerrilla::LegendRegistry::Instance().RowCount(), lights, GScene->NLights(),
             GScene->MainLight() == sun, GScene->GetCamera()->Position().Distance2(camera.Position()),
             GScene->GetCamera()->Direction().Distance2(camera.Direction()));
    stable = stable && Glob.time == time && GWorld->NVehicles() == vehicles && GWorld->NFastVehicles() == fast &&
             GWorld->NOutVehicles() == out && Guerrilla::LegendRegistry::Instance().RowCount() == rows &&
             GScene->NLights() == lights && GScene->MainLight() == sun &&
             GScene->GetCamera()->Position().Distance2(camera.Position()) < 1e-10f &&
             GScene->GetCamera()->Direction().Distance2(camera.Direction()) < 1e-10f;
    stable = stable && registrySnapshot() == registryBefore && aiSnapshot() == aiBefore &&
             GetNetworkManager().GetGameState() == networkState &&
             GetNetworkManager().GetServerState() == networkServer &&
             GetNetworkManager().GetPlayer() == networkPlayer && GetNetworkManager().NPlayerRoles() == networkRoles;
    stable = stable && GWorld->NextMagazineID() == nextMagazine;
    if (GLandscape)
        stable = stable && GLandscape->CloudsAlpha() == clouds && GLandscape->GetRainDensity() == rain &&
                 GLandscape->GetWind().Distance2(wind) < 1e-10f;
    // Releasing all service handles models the portrait portion of device loss:
    // the next journal repaint must rebuild from retained CPU pixels.
    Guerrilla::PortraitService::Instance().ReleaseGraphics();
    LOG_INFO(Core, "[tri] portrait isolation {}", stable ? "OK" : "FAILED");
    return GameValue(stable && rgb.size() == 512 * 512 * 3);
}
