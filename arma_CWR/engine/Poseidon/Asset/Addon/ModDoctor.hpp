#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Generic repair pass for third-party addon archives.
//
// Old OFP-era mods carry content defects this engine is stricter about than the
// 1.96 one was, and the repair is always the same shape: a same-length byte
// rewrite inside an uncompressed pbo entry, so the archive's header table does
// not move and no repack is involved. This unit is the pure logic (scan, plan,
// apply over a byte buffer); PoseidonTools' `mod doctor` command wraps it with
// file I/O, backups and reporting, so the whole thing is unit-testable against
// in-memory buffers.
namespace Poseidon::ModDoctor
{

enum class DefectClass
{
    //! A config that uses a scope keyword the file never #defines. Reported once
    //! per config entry as the cause, with the individual sites as children.
    MissingDefineHeader,
    //! One `scope = public;` site under a MissingDefineHeader cause.
    UndefinedScopeKeyword,
    //! An unquoted numeric token with two or more dots, e.g. `0.0.1`.
    MalformedFloat,
    //! An ODOL model whose whole mesh sits at or below its own origin.
    BuriedModelOrigin,
    //! Not a defect: an entry that could hold one but cannot be read or patched.
    NotPatchable
};

const char* ToString(DefectClass defect);

//! A same-length in-place byte rewrite. `offset` is absolute in the pbo buffer.
struct Patch
{
    int64_t offset = 0;
    std::string original;
    std::string replacement;
};

struct Finding
{
    DefectClass defect = DefectClass::NotPatchable;
    //! Entry name inside the pbo, e.g. "config.cpp" or "LoBo_M60A1_wreck.p3d".
    std::string entry;
    //! Human-readable site: class path, model name plus the numbers, or a reason.
    std::string detail;
    //! 1-based source line for config findings, 0 when it does not apply.
    int line = 0;
    bool patchable = true;
    std::vector<Patch> patches;
    //! MissingDefineHeader lists its UndefinedScopeKeyword sites here.
    std::vector<Finding> children;
};

//! One entry of a pbo header table.
struct PboEntry
{
    std::string name;
    uint32_t packingMethod = 0;
    uint32_t originalSize = 0;
    uint32_t length = 0;
    int64_t dataOffset = 0; //!< absolute offset of the entry data in the buffer
    bool compressed = false;
};

//! Walk a pbo header table. Returns false (with `error` set) on a table that
//! runs off the end of the buffer or does not terminate.
bool ReadPboEntries(const char* data, size_t size, std::vector<PboEntry>& entries, std::string& error);

//! Scan one text config for both config defect classes. `entryOffset` is the
//! entry's absolute offset in the pbo buffer, so the emitted patch offsets are
//! absolute too; pass 0 to scan a bare config text.
std::vector<Finding> ScanConfigText(const char* text, size_t length, int64_t entryOffset, const std::string& entryName);

//! Located model-origin fields of an ODOL v7 p3d.
struct ModelOrigin
{
    int trailerOffset = 0; //!< offset of `special`, the first model-wide field
    float minY = 0.0f;
    float maxY = 0.0f;
    float boundingCenterY = 0.0f;
};

//! Byte offsets of the model-wide fields, relative to the trailer. The field
//! order is readModel()'s (P3DStructures.hpp), which is LODShape::SerializeBin's.
constexpr int kMinMaxOffset = 48;
constexpr int kMinMaxMaxOffset = 60;
constexpr int kBoundingCenterOffset = 72;

//! Locate the model trailer by replaying the ODOL v7 reader over `data`.
//! Returns false (with `error` set) when the model cannot be read.
bool ReadModelOrigin(const char* data, size_t size, ModelOrigin& origin, std::string& error);

//! Scan one p3d entry for the buried-origin defect. Empty when the model seats.
std::vector<Finding> ScanModelEntry(const char* data, size_t size, int64_t entryOffset, const std::string& entryName);

//! Scan a whole pbo image: every config.cpp and every *.p3d entry.
std::vector<Finding> ScanPbo(const char* data, size_t size, std::string& error);

//! True when the finding or any of its children carries a patch.
bool HasPatches(const Finding& finding);

//! Apply every patch of `findings` to `data` in place. A patch whose original
//! bytes no longer match is skipped, so applying twice is a no-op. Returns the
//! number of patches applied.
int ApplyPatches(char* data, size_t size, const std::vector<Finding>& findings);

//! Case-insensitive glob over `*` and `?`, used by the --pbo filter.
bool WildcardMatch(const char* pattern, const char* text);

} // namespace Poseidon::ModDoctor
