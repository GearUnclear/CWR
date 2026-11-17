#pragma once

namespace Poseidon::BuildProvenance
{

// Which source this binary was built from, for perf runs and captures
// (build_id=, triBuildId): `git describe --always --dirty --abbrev=8` of the
// tree at build time, then "+<12 hex>" when engine/ or apps/ held uncommitted
// changes or untracked files, a hash of exactly those (cmake/
// GenerateBuildInfo.cmake). Two builds of one commit with different local
// changes therefore report different ids; a clean build reports the commit.
const char* BuildId();

} // namespace Poseidon::BuildProvenance
