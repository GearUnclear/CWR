#include <Poseidon/UI/Guerrilla/GuerrillaBodyPreview.hpp>
#include <Poseidon/UI/Guerrilla/GuerrillaNewGame.hpp> // the pure proxy predicates (flag + weapon filters)
#include <Poseidon/Core/Global.hpp>          // Glob.uiTime (turntable)
#include <Poseidon/Graphics/Core/Engine.hpp> // Point2DFloat (slot corners for Convert2DTo3D)
#include <Poseidon/IO/ParamFile/ParamFile.hpp>
#include <Poseidon/IO/ParamFileExt.hpp>                      // Pars (per-frame proxy classification)
#include <Poseidon/Graphics/Rendering/Shape/Shape.hpp>       // Shapes bank + ProxyObject
#include <Poseidon/World/Scene/Scene.hpp>                    // GScene (proxy draw: LOD pick)
#include <Poseidon/World/Scene/Camera/Camera.hpp>            // camera direction for the proxy LOD pick
#include <Poseidon/World/Simulation/FrameInv.hpp>            // FrameWithInverse (proxy draw)
#include <Poseidon/World/Simulation/Animation/Animation.hpp> // AnimationSection (zasleh hide)
#include <Poseidon/Foundation/Math/MathDefs.hpp>             // H_PI (turntable)
#include <Poseidon/Foundation/platform.hpp>                  // stricmp

#include <math.h> // fmodf/fabsf (turntable + fit)

// 3D-UI camera-space depth scale, defined in UIControlsBase.cpp next to
// ControlObject (whose constructor multiplies the config z by it).
extern const float CameraZoom;

namespace Poseidon
{
// The shape-path resolver ControlObject's constructor runs on its `model`
// config value (OptionsUI.cpp) - declared the way UIControlsBase.cpp does.
RString FindShape(RString name);

GuerrillaBodyPreview::GuerrillaBodyPreview(ControlsContainer* parent, int idc, const ParamEntry& cls)
    : ControlObject(parent, idc, cls)
{
}

void GuerrillaBodyPreview::SetModel(RString modelName)
{
    if (stricmp(modelName, _modelName) == 0)
    {
        return;
    }
    _modelName = modelName;
    Ref<LODShapeWithShadow> shape = Shapes.New(FindShape(modelName), false, false);
    if (shape && shape->NLevels() > 0)
    {
        // the same UI-space treatment ControlObject's constructor applies
        shape->LevelOpaque(0)->MakeCockpit();
        shape->OrSpecial(BestMipmap | NoDropdown | DisableSun);
    }
    HideMuzzleFlashSections(shape);
    SetShape(shape);
}

void GuerrillaBodyPreview::HideMuzzleFlashSections(LODShapeWithShadow* body)
{
    auto hideOnShape = [](LODShapeWithShadow* shape)
    {
        if (!shape)
        {
            return;
        }
        AnimationSection zasleh;
        zasleh.Init(shape, "zasleh", nullptr);
        for (int level = 0; level < shape->NLevels(); level++)
        {
            zasleh.Hide(shape, level);
        }
    };
    if (!body)
    {
        return;
    }
    hideOnShape(body);
    for (int level = 0; level < body->NLevels(); level++)
    {
        Shape* lShape = body->Level(level);
        if (!lShape)
        {
            continue;
        }
        for (int i = 0; i < lShape->NProxies(); i++)
        {
            const ProxyObject& proxy = lShape->Proxy(i);
            if (proxy.obj)
            {
                hideOnShape(proxy.obj->GetShape());
            }
        }
    }
}

void GuerrillaBodyPreview::DrawProxies(int level, ClipFlags clipFlags, const Matrix4& transform,
                                       const Matrix4& invTransform, float dist2, float z2, const LightList& lights)
{
    // Pars lives for the process; the entry lookup is one hash probe,
    // cheap enough per frame and always current should a mod reload
    // ever rebuild the merged config.
    const ParamEntry* nonAI = Pars.FindEntry("CfgNonAIVehicles");
    Shape* sShape = _shape->LevelOpaque(level);
    for (int i = 0; i < sShape->NProxies(); i++)
    {
        const ProxyObject& proxy = sShape->Proxy(i);
        if (!proxy.obj || GuerrillaPreviewIsFlagProxy(proxy.name) ||
            GuerrillaPreviewHideWeaponProxy(nonAI, proxy.name, _civilian))
        {
            continue;
        }
        // from here the base Object::DrawProxies body, unchanged
        Matrix4Val pTransform = transform * proxy.obj->Transform();
        Matrix4Val invPTransform = proxy.invTransform * invTransform;
        LODShapeWithShadow* pshape = proxy.obj->GetShapeOnPos(pTransform.Position());
        if (!pshape)
        {
            continue;
        }
        int pLevel = GScene->LevelFromDistance2(pshape, dist2, pTransform.Scale(), pTransform.Direction(),
                                                GScene->GetCamera()->Direction());
        if (pLevel == LOD_INVISIBLE)
        {
            continue;
        }
        FrameWithInverse pFrame(pTransform, invPTransform);
        proxy.obj->Draw(pLevel, ClipAll, pFrame);
    }
}

void GuerrillaBodyPreview::PlaceInSlot(float cx, float topY, float bottomY)
{
    LODShapeWithShadow* shape = GetShape();
    if (!shape)
    {
        return;
    }
    const float depth = kGuerrillaPreviewDepthCfg * CameraZoom;
    Vector3 top = Convert2DTo3D(Point2DFloat(cx, topY), depth);
    Vector3 bottom = Convert2DTo3D(Point2DFloat(cx, bottomY), depth);
    float slotHeight = fabsf(top[1] - bottom[1]);
    float modelHeight = shape->Max()[1] - shape->Min()[1];
    _fitScale = modelHeight > 0.01f ? slotHeight / modelHeight : 1.0f;
    Vector3 pos = (top + bottom) * 0.5f;
    pos[1] -= 0.5f * (shape->Min()[1] + shape->Max()[1]) * _fitScale;
    _position = pos;
    SetPosition(pos);
    SetScale(_fitScale);
}

void GuerrillaBodyPreview::SimulateTurntable()
{
    float t = fmodf(Glob.uiTime.toFloat(), kGuerrillaPreviewTurnPeriod);
    Matrix3 orient(MRotationY, (H_PI * 2.0f / kGuerrillaPreviewTurnPeriod) * t);
    SetOrientation(orient);
    SetScale(_fitScale);
}

void GuerrillaBodyPreview::OnDraw(float alpha)
{
    if (!GetShape())
    {
        return; // a body that failed to load draws nothing, never crashes
    }
    ControlObject::OnDraw(alpha);
}

} // namespace Poseidon
