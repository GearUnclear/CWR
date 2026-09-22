#include <Poseidon/Game/Guerrilla/PortraitService.hpp>
#include <Poseidon/Game/Guerrilla/LegendAppearance.hpp>
#include <Poseidon/Game/Guerrilla/LegendRegistry.hpp>
#include <Poseidon/Game/Guerrilla/LegendPlacement.hpp>
#include <Poseidon/Game/Guerrilla/ZoneRegistry.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/Core/Progress.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Core/Application.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Input/InputSubsystem.hpp>
#include <Poseidon/Network/NetworkConfig.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <SDL3/SDL_scancode.h>
#include <set>
#include <algorithm>
#include <chrono>

namespace Poseidon::Guerrilla
{
std::vector<PortraitAppearance> BuildPortraitRoster(const FactionRecord* resistance, const FactionRecord* occupier,
                                                    const ParamEntry* vehicles, const ParamEntry* faces,
                                                    const std::vector<PortraitAppearance>& recorded)
{
    std::set<std::string> bodies;
    const auto add = [&](const RString& body)
    {
        if (body.GetLength())
            bodies.emplace((const char*)body);
    };
    if (resistance)
        for (int i = 0; i < resistance->values.Size(); ++i)
            if (resistance->values[i].key == RString("companionClass") ||
                resistance->values[i].key == RString("companionClassCiv"))
                add(resistance->values[i].value);
    if (const auto* faction = occupier)
    {
        std::vector<float> levels{0};
        for (int i = 0; i < faction->tierThresholds.Size(); ++i)
            levels.push_back(faction->tierThresholds[i]);
        for (const float level : levels)
        {
            AutoArray<LegendRoleResolution> roles;
            ResolveLegendRoles(ReadLegendCapability(*faction, level), 3, roles);
            for (int i = 0; i < roles.Size(); ++i)
                add(roles[i].bossClass);
        }
    }
    std::vector<PortraitAppearance> roster;

    for (const auto& body : bodies)
    {
        const auto* cfg = vehicles ? vehicles->FindEntry(body.c_str()) : nullptr;
        const bool woman = cfg && cfg->ReadValue("woman", 0.0f) > 0.5f;
        for (const auto* rolled : LegendRegistry::kPortraitFaces)
            roster.push_back({RString(body.c_str()), ResolveLegendFace(faces, RString(rolled), woman)});
    }
    for (const auto& appearance : recorded)
    {
        // A fallen record is never rebound: keep its exact saved appearance
        // so a removed face reaches Unavailable before gameplay, rather than
        // substituting a photograph of the fallback identity in its dossier.
        roster.push_back(appearance);
        const auto* cfg = vehicles ? vehicles->FindEntry(appearance.body) : nullptr;
        const bool woman = cfg && cfg->ReadValue("woman", 0.0f) > 0.5f;
        // Also prepare the shared fallback a living character may bind on load.
        roster.push_back({appearance.body, ResolveLegendFace(faces, appearance.face, woman)});
    }
    std::set<std::string> seen;
    roster.erase(std::remove_if(roster.begin(), roster.end(),
                                [&](const PortraitAppearance& appearance)
                                {
                                    RString body = appearance.body, face = appearance.face;
                                    body.Lower();
                                    face.Lower();
                                    return !seen.emplace(std::string(body) + ":" + std::string(face)).second;
                                }),
                 roster.end());
    return roster;
}

std::vector<PortraitAppearance> CampaignPortraitRoster()
{
    const auto& zones = ZoneRegistry::Instance();
    const auto& legends = LegendRegistry::Instance();
    std::vector<PortraitAppearance> recorded;
    for (int i = 0; i < legends.RowCount(); ++i)
    {
        const auto& row = legends.Row(i);
        if (row.bodyClass.GetLength() && row.face.GetLength())
            recorded.push_back({row.bodyClass, row.face});
    }
    return BuildPortraitRoster(zones.FindFactionForSide(zones.ResistanceSide()),
                               zones.FindFactionForSide(zones.OccupierSide()), Pars.FindEntry("CfgVehicles"),
                               Pars.FindEntry("CfgFaces"), recorded);
}

bool PrepareCampaignPortraits()
{
    if (IsDedicatedServer() || !GEngine || !GEngine->IsAbleToDraw() ||
        stricmp(GEngine->GetRendererName(), "None") == 0 || !ZoneRegistry::Instance().IsActive())
        return true;
    auto& service = PortraitService::Instance();
    service.PrepareRoster(CampaignPortraitRoster());
    const auto start = std::chrono::steady_clock::now();
    auto& progress = GetGProgress();
    const bool wasActive = progress.Active();
    const RString title = progress.Title();
    if (!wasActive)
        progress.Start("Preparing dossier photographs");
    bool cancelled = false;
    while (service.Pending())
    {
        const std::string label = "Preparing dossier photographs\xe2\x80\xa6 " + std::to_string(service.Completed()) +
                                  " / " + std::to_string(service.Total());
        progress.SetTitle(RString(label.c_str()));
        progress.Frame();
        // Progress pumps SDL, but normal frame input processing is suspended
        // during loading. Consume the buffered keyboard edges without running
        // gameplay actions, scripts, voice shortcuts or simulation.
        InputSubsystem::Instance().PollLoadingKeyboard();
        if (Glob.exit || (GApp && GApp->m_closeRequest) ||
            InputSubsystem::Instance().GetKeyToDo(SDL_SCANCODE_ESCAPE, true, false))
        {
            service.Cancel();
            cancelled = true;
            break;
        }
        service.Advance();
    }
    if (!cancelled)
    {
        const auto label = "Preparing dossier photographs\xe2\x80\xa6 " + std::to_string(service.Completed()) + " / " +
                           std::to_string(service.Total());
        progress.SetTitle(RString(label.c_str()));
        progress.Frame();
    }
    progress.SetTitle(title);
    if (!wasActive)
        progress.Finish();
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    LOG_INFO(Core, "Portrait preparation: {} / {}, {} renders, {} cache hits, {} ms{}", service.Completed(),
             service.Total(), service.RenderCount(), service.CacheHits(), elapsed, cancelled ? " (cancelled)" : "");
    return !cancelled;
}
} // namespace Poseidon::Guerrilla
