#include <Poseidon/Game/Guerrilla/PortraitRenderer.hpp>
#include <Poseidon/Game/Guerrilla/LegendAppearance.hpp>
#include <Poseidon/World/Entities/Infantry/SoldierOld.hpp>
#include <Poseidon/World/World.hpp>
#include <Poseidon/World/Simulation/Animation/RtAnimation.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/Graphics/Core/Engine.hpp>
#include <Poseidon/Graphics/Textures/TextureBank.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/Lights.hpp>
#include <Poseidon/Graphics/Rendering/Lighting/Material.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>
#include <Poseidon/IO/Streams/QStream.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaNewGame.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <map>

namespace Poseidon::Guerrilla
{
namespace
{
// Preserve proxy classification while owning private geometry. Keeping the source
// alive also keeps its type alive; no draw or animation is sent to that source.
class PortraitProxy final : public Object
{
    Ref<Object> _source;

  public:
    PortraitProxy(LODShapeWithShadow* shape, Object* source) : Object(shape, -1), _source(source) {}
    const EntityType* GetVehicleType() const override { return _source->GetVehicleType(); }
    void Draw(int, ClipFlags flags, const FrameBase& frame) override { Object::Draw(0, flags, frame); }
};
void EffectiveEntries(const ParamEntry& entry, std::map<std::string, const ParamEntry*>& entries)
{
    if (const auto* cls = entry.GetClassInterface())
        if (const auto* base = cls->GetBase())
            EffectiveEntries(*base, entries);
    for (int i = 0; i < entry.GetEntryCount(); ++i)
    {
        const auto& child = entry.GetEntry(i);
        RString name = child.GetName();
        name.Lower();
        entries[(const char*)name] = &child;
    }
}
void SaveEffective(const ParamEntry& entry, QOStream& stream)
{
    if (!entry.IsClass())
    {
        entry.Save(stream, 0);
        return;
    }
    std::map<std::string, const ParamEntry*> entries;
    EffectiveEntries(entry, entries);
    stream << "class " << (const char*)entry.GetName() << " {\n";
    for (const auto& [name, child] : entries)
        SaveEffective(*child, stream);
    stream << "};\n";
}
} // namespace
struct PortraitRenderer::State
{
    Ref<Man> person;
    std::vector<std::string> dependencies;
    std::string configuration;
    std::string materialConfiguration;

    void PrivateShape(LODShapeWithShadow* shape, int depth = 0)
    {
        if (!shape || depth > 8)
            throw std::runtime_error("invalid portrait proxy graph");

        shape->OrSpecial(DisableSun | FogDisabled | NoShadow | NoDropdown);
        for (int level = 0; level < shape->NLevels(); ++level)
        {
            auto* lod = shape->Level(level);
            if (!lod)
                continue;

            for (int p = 0; p < lod->NProxies(); ++p)
            {
                auto& proxy = lod->Proxy(p);
                if (!proxy.obj || !proxy.obj->GetShape())
                    continue;
                const auto transform = proxy.obj->Transform();
                auto* original = proxy.obj->GetShape();
                Ref<LODShapeWithShadow> copy = new LODShapeWithShadow(*original);
                PrivateShape(copy, depth + 1);
                Ref<Object> object = new PortraitProxy(copy, proxy.obj);
                object->SetTransform(transform);
                proxy.obj = object;
            }
        }
        shape->OptimizeRendering();
    }
    void DependenciesOf(LODShapeWithShadow* shape, bool body)
    {
        dependencies.emplace_back(shape->Name());
        const auto* lod = shape->Level(0);
        for (int i = 0; i < lod->NSections(); ++i)
        {
            const auto& section = lod->GetSection(i);
            if (section.properties.Special() & (IsHidden | IsHiddenProxy))
                continue;
            if (auto* texture = section.properties.GetTexture())
                dependencies.emplace_back(texture->GetName());
            if (const auto* material = section.surfMat.GetRef())
            {
                const char* name = material->GetName();
                if (*name == '#')
                {
                    const auto* materials = Pars.FindEntry("CfgMaterials");
                    if (const auto* cfg = materials ? materials->FindEntry(name + 1) : nullptr)
                    {
                        QOStream stream;
                        SaveEffective(*cfg, stream);
                        materialConfiguration.append(stream.str(), stream.pcount());
                    }
                }
                else if (*name)
                    dependencies.emplace_back(name);
                for (auto* texture :
                     {material->_tex.GetRef(), material->_bumpmap.GetRef(), material->_detailmap.GetRef()})
                    if (texture)
                        dependencies.emplace_back(texture->GetName());
            }
        }
        for (int i = 0; i < lod->NProxies(); ++i)
        {
            auto* object = lod->Proxy(i).obj.GetRef();
            if (!object || !object->GetShape())
                continue;
            const auto* type = object->GetVehicleType();
            // An unequipped Man draws only alwaysshow attachments. Nested plain
            // objects use Object's normal proxy path, which draws all proxies.
            if (!body || (type && type->_simName == RString("alwaysshow")))
                DependenciesOf(object->GetShape(), false);
        }
    }
};

PortraitRenderer::PortraitRenderer() : _state(std::make_unique<State>()) {}
PortraitRenderer::~PortraitRenderer() = default;
const std::vector<std::string>& PortraitRenderer::Dependencies() const
{
    return _state->dependencies;
}
const std::string& PortraitRenderer::Configuration() const
{
    return _state->configuration;
}

bool PortraitRenderer::Prepare(const PortraitAppearance& appearance, std::string& error)
{
    _state = std::make_unique<State>();
    if (!GEngine || !GScene || !GWorld)
    {
        error = "graphics scene unavailable";
        return false;
    }
    const auto* vehicles = Pars.FindEntry("CfgVehicles");
    const auto* cfg = vehicles ? vehicles->FindEntry(appearance.body) : nullptr;
    const auto* faces = Pars.FindEntry("CfgFaces");
    const auto* face = faces ? faces->FindEntry(appearance.face) : nullptr;
    if (!cfg || !face || stricmp(appearance.face, "Default") == 0 || stricmp(appearance.face, "Custom") == 0)
    {
        error = "appearance configuration unavailable";
        return false;
    }
    try
    {
        if (stricmp(cfg->ReadValue("simulation", RString()), "soldier") != 0 &&
            stricmp(cfg->ReadValue("simulation", RString()), "soldierold") != 0)
        {
            error = "appearance is not a character";
            return false;
        }
        const auto* reflectors = cfg->FindEntry("Reflectors");
        if (reflectors && reflectors->GetEntryCount())
        {
            error = "character has simulation-owned reflectors";
            return false;
        }
        // No World::AddVehicle, AIGroup, identity selection, Init event or script.
        auto* type = dynamic_cast<ManType*>(VehicleTypes.New(appearance.body));
        if (!type || type->IsAbstract())
        {
            error = "character type unavailable";
            return false;
        }
        // NewVehicle constructs Soldier, whose AIUnit constructor consumes RNG.
        // Man owns the identical model/animation/head machinery without a brain.
        type->VehicleAddRef();
        struct ReleaseType
        {
            ManType* type;
            ~ReleaseType() { type->VehicleRelease(); }
        } release{type};
        if (!type->GetShape())
        {
            error = "body model unavailable";
            return false;
        }
        _state->person = new Man(type, false);
        Man* man = _state->person;
        if (!LegendFaceUsable(faces, appearance.face, man->IsWoman()))
        {
            error = "unsupported face binding";
            return false;
        }
        Ref<LODShapeWithShadow> body = new LODShapeWithShadow(*man->GetShape());
        if (!body || body->NLevels() == 0)
        {
            error = "body model unavailable";
            return false;
        }
        _state->PrivateShape(body);
        man->SetShape(body);
        man->SetFace(appearance.face);
        const RString texture = man->GetFaceTextureName();
        const RString expectedTexture = GetPictureName(face->ReadValue("texture", RString()));
        if (texture.GetLength() == 0 || stricmp(texture, expectedTexture) != 0)
        {
            error = "face texture unavailable";
            return false;
        }
        _state->dependencies.emplace_back(texture);
        man->SetMimic("Default");
        const MoveId pose = man->Type()->GetMoveId("EffectStandStill");
        if (pose == MoveIdNone)
        {
            error = "idle pose unavailable";
            return false;
        }
        man->SwitchMove(RStringB("EffectStandStill"));
        man->SetPosition(VZero);
        QOStream config;
        SaveEffective(*cfg, config);
        SaveEffective(*face, config);
        if (const auto* moves = Pars.FindEntry(cfg->ReadValue("moves", RString("CfgMovesMC"))))
            SaveEffective(*moves, config);
        if (const auto* mimics = Pars.FindEntry("CfgMimics"))
            SaveEffective(*mimics, config);
        man->Animate(0);
        _state->DependenciesOf(body, true);
        bool bound = false;
        for (int i = 0; i < body->Level(0)->NSections(); ++i)
            if (auto* actual = body->Level(0)->GetSection(i).properties.GetTexture())
                bound = bound || stricmp(actual->GetName(), texture) == 0;
        man->Deanimate(0);
        if (!bound)
        {
            error = "model has no personality binding";
            return false;
        }
        if (const auto* mapping = Pars.FindEntry("CfgTextureToMaterial"))
            SaveEffective(*mapping, config);
        _state->configuration.assign(config.str(), config.pcount());
        _state->configuration += _state->materialConfiguration;
        const auto* animation = man->Type()->GetAnimation(pose);
        if (!animation)
        {
            error = "idle animation unavailable";
            return false;
        }
        _state->dependencies.emplace_back(animation->Name());
        auto& deps = _state->dependencies;
        std::sort(deps.begin(), deps.end());
        deps.erase(std::unique(deps.begin(), deps.end()), deps.end());
        return true;
    }
    catch (const std::exception& e)
    {
        error = e.what();
        return false;
    }
}

bool PortraitRenderer::Capture(std::vector<uint8_t>& rgb, std::string& error)
{
    if (!_state->person)
    {
        error = "mannequin not prepared";
        return false;
    }
    const bool ok = GEngine->CapturePortrait(
        [&]
        {
            LightSun fixedSun;
            struct Restore
            {
                LightSun* sun = GScene->MainLight();
                Camera camera = *GScene->GetCamera();
                LightList lights = GScene->ActiveLights();
                Color colour = GScene->GetConstantColor();
                float fog = GScene->GetConstantFog();
                SectionClassFilter filter = GSectionFilter;
                render::PassKindHint pass = GEngine->GetPassKindHint();
                ~Restore()
                {
                    GEngine->FlushQueues();
                    GSectionFilter = filter;
                    GEngine->SetPassKindHint(pass);
                    GScene->SetMainLight(sun);
                    GScene->SetCamera(camera);
                    GScene->SetActiveLights(lights);
                    GScene->SetConstantColor(colour);
                    GScene->SetConstantFog(fog);
                    GEngine->UpdateFrameCamera();
                }
            } restore;
            GSectionFilter = SectionClassFilter::All;
            GEngine->SetPassKindHint(render::PassKindHint::None);
            GScene->SetMainLight(&fixedSun);
            auto* man = _state->person.GetRef();
            Vector3 bounds[2];
            man->Animate(0);
            bool first = true;
            const auto includeGeometry = [&](const auto& self, LODShapeWithShadow* shape, const Matrix4& transform,
                                             bool body) -> void
            {
                const auto* geometry = shape->Level(0);
                for (Offset f = geometry->BeginFaces(); f < geometry->EndFaces(); geometry->NextFace(f))
                {
                    const auto& face = geometry->Face(f);
                    if (face.Special() & (IsHidden | IsHiddenProxy))
                        continue;
                    for (int v = 0; v < face.N(); ++v)
                    {
                        const Vector3 point = transform * geometry->Pos(face.GetVertex(v));
                        if (first)
                        {
                            bounds[0] = bounds[1] = point;
                            first = false;
                        }
                        for (int axis = 0; axis < 3; ++axis)
                        {
                            bounds[0][axis] = std::min(bounds[0][axis], point[axis]);
                            bounds[1][axis] = std::max(bounds[1][axis], point[axis]);
                        }
                    }
                }
                // Match EntityAI/Object's visible attachment transforms, so a
                // helmet stored in a proxy contributes to the head clearance.
                for (int i = 0; i < geometry->NProxies(); ++i)
                {
                    const auto* object = geometry->Proxy(i).obj.GetRef();
                    if (!object || !object->GetShape())
                        continue;
                    const auto* type = object->GetVehicleType();
                    if (!body || (type && type->_simName == RString("alwaysshow")))
                        self(self, object->GetShape(), transform * object->Transform(), false);
                }
            };
            includeGeometry(includeGeometry, man->GetShape(), MIdentity, true);
            man->Deanimate(0);
            if (first)
                throw std::runtime_error("character has no visible geometry");
            const float height = bounds[1].Y() - bounds[0].Y();
            const float extent = std::max(0.45f, height * 0.34f);
            const float top = bounds[1].Y() + extent * 0.08f;
            Camera camera;
            camera.SetPerspective(0.05f, 100.0f, 0.35f, 0.35f);
            camera.SetPosition(Vector3(0, top - extent * 0.5f, extent / 0.7f));
            camera.SetOrient(Vector3(0, 0, -1), VUp);
            camera.Adjust(GEngine);
            GScene->SetCamera(camera);
            LightList lights;
            Ref<LightPoint> light = new LightPoint(Color(0.8f, 0.8f, 0.8f), Color(0.45f, 0.45f, 0.45f));
            light->SetPosition(Vector3(-2, top + 1, 3));
            light->SetBrightness(18);
            lights.Add(Ref<Light>(light.GetRef()));
            GScene->SetActiveLights(lights);
            GScene->SetConstantFog(0);
            GEngine->UpdateFrameCamera();
            man->Draw(0, ClipAll, *man);
        },
        rgb);
    if (!ok)
        error = "offscreen capture unavailable";
    return ok;
}
} // namespace Poseidon::Guerrilla
