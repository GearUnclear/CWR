#include <Poseidon/Core/BuildProvenance.hpp>

#include <Poseidon/Core/BuildId.hpp>

namespace Poseidon::BuildProvenance
{

const char* BuildId()
{
    return BuildInfo::BuildId;
}

} // namespace Poseidon::BuildProvenance
