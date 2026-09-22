#include <Poseidon/Game/Guerrilla/LegendAppearance.hpp>
#include <Poseidon/Game/Guerrilla/LegendRegistry.hpp>
#include <Poseidon/IO/ParamFile/ParamFile.hpp>

namespace Poseidon::Guerrilla
{
bool LegendFaceUsable(const ParamEntry* faces, const char* token, bool bodyIsWoman)
{
    if (!faces)
        return true;
    const ParamEntry* face = token ? faces->FindEntry(token) : nullptr;
    if (!face || face->FindEntry("disabled"))
        return false;
    return (face->ReadValue("woman", 0.0f) > 0.5f) == bodyIsWoman;
}

RString ResolveLegendFace(const ParamEntry* faces, const RString& recordedFace, bool bodyIsWoman)
{
    if (recordedFace.GetLength() > 0 && LegendFaceUsable(faces, recordedFace, bodyIsWoman))
        return recordedFace;
    for (int i = 0; i < LegendRegistry::NPortraitFaces; ++i)
        if (LegendFaceUsable(faces, LegendRegistry::kPortraitFaces[i], bodyIsWoman))
            return RString(LegendRegistry::kPortraitFaces[i]);
    return RString("Default");
}
} // namespace Poseidon::Guerrilla
