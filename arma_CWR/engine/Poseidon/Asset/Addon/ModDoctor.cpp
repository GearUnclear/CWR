#include <Poseidon/Asset/Addon/ModDoctor.hpp>

#include <Poseidon/Asset/Formats/P3D/P3DStructures.hpp>
#include <Poseidon/IO/Streams/FileInfo.h>
#include <Poseidon/IO/Streams/QStream.hpp>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <exception>

namespace Poseidon::ModDoctor
{
namespace
{

// Config entries whose value is read as a number, i.e. the ones a bare scope
// keyword actually breaks. `access` is in the set because it drives ParamClass's
// access mode the same way.
const char* const kScopeEntries[] = {"scope", "scopeWeapon", "scopeMagazine", "scopeCurator", "access"};

struct ScopeKeyword
{
    const char* name;
    int value;
};

// The values the missing `#define private 0 / protected 1 / public 2` header
// would have given them.
const ScopeKeyword kScopeKeywords[] = {{"private", 0}, {"protected", 1}, {"public", 2}};

bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool IsWordChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || IsDigit(c) || c == '_';
}

// A token character that would make a dotted number part of a longer name, e.g.
// a texture path `\data\tex.0.0.1.paa`.
bool IsTokenNeighbour(char c)
{
    return IsWordChar(c) || c == '.' || c == '\\' || c == '/';
}

//! Match `word` at `p` within [p, end) and require an identifier boundary after it.
bool MatchWord(const char* p, const char* end, const char* word)
{
    size_t len = ::strlen(word);
    if (static_cast<size_t>(end - p) < len)
    {
        return false;
    }
    if (::memcmp(p, word, len) != 0)
    {
        return false;
    }
    return p + len == end || !IsWordChar(p[len]);
}

void SkipBlanks(const char*& p, const char* end)
{
    while (p < end && (*p == ' ' || *p == '\t'))
    {
        ++p;
    }
}

//! `#define <keyword>` anywhere in the file. Case-sensitive because the engine's
//! own preprocessor is: `#define Public 2` does not define `public`.
bool DefinesKeyword(const char* text, const char* end, const char* keyword)
{
    for (const char* line = text; line < end;)
    {
        const char* lineEnd = static_cast<const char*>(::memchr(line, '\n', static_cast<size_t>(end - line)));
        if (!lineEnd)
        {
            lineEnd = end;
        }
        const char* p = line;
        SkipBlanks(p, lineEnd);
        if (p < lineEnd && *p == '#')
        {
            ++p;
            SkipBlanks(p, lineEnd);
            if (MatchWord(p, lineEnd, "define"))
            {
                p += 6;
                SkipBlanks(p, lineEnd);
                size_t len = ::strlen(keyword);
                if (static_cast<size_t>(lineEnd - p) >= len && ::memcmp(p, keyword, len) == 0 &&
                    (p + len == lineEnd || !IsWordChar(p[len])))
                {
                    return true;
                }
            }
        }
        line = (lineEnd == end) ? end : lineEnd + 1;
    }
    return false;
}

//! Pad `replacement` with trailing spaces up to `width`. Returns false when it
//! does not fit, which would move the pbo header table and is never legal.
bool PadTo(std::string& replacement, size_t width)
{
    if (replacement.size() > width)
    {
        return false;
    }
    replacement.append(width - replacement.size(), ' ');
    return true;
}

//! Format the strtod prefix of a malformed float back into exactly `width`
//! characters. Padding with fractional zeros rather than spaces keeps the token
//! a plain number, so the result is valid wherever the original stood.
bool FormatMalformedFloatReplacement(const char* token, size_t width, std::string& out)
{
    char* end = nullptr;
    ::strtod(token, &end);
    if (!end || end == token)
    {
        return false;
    }
    out.assign(token, static_cast<size_t>(end - token));
    if (out.size() >= width)
    {
        return false;
    }
    if (out.find('.') == std::string::npos)
    {
        out.push_back('.');
    }
    out.append(width - out.size(), '0');
    return out.size() == width;
}

int LineOf(const char* text, const char* p)
{
    int line = 1;
    for (const char* q = text; q < p; ++q)
    {
        if (*q == '\n')
        {
            ++line;
        }
    }
    return line;
}

} // namespace

const char* ToString(DefectClass defect)
{
    switch (defect)
    {
        case DefectClass::MissingDefineHeader:
            return "MISSING_DEFINE_HEADER";
        case DefectClass::UndefinedScopeKeyword:
            return "UNDEFINED_SCOPE_KEYWORD";
        case DefectClass::MalformedFloat:
            return "MALFORMED_FLOAT";
        case DefectClass::BuriedModelOrigin:
            return "BURIED_MODEL_ORIGIN";
        case DefectClass::NotPatchable:
            return "NOT_PATCHABLE";
    }
    return "UNKNOWN";
}

bool WildcardMatch(const char* pattern, const char* text)
{
    // Iterative glob with a backtrack point, so a pattern full of stars cannot
    // blow the stack on a long name.
    const char* star = nullptr;
    const char* mark = nullptr;
    while (*text)
    {
        char p = *pattern;
        char t = *text;
        if (p >= 'A' && p <= 'Z')
        {
            p = static_cast<char>(p - 'A' + 'a');
        }
        if (t >= 'A' && t <= 'Z')
        {
            t = static_cast<char>(t - 'A' + 'a');
        }
        if (p == '?' || p == t)
        {
            ++pattern;
            ++text;
        }
        else if (*pattern == '*')
        {
            star = pattern++;
            mark = text;
        }
        else if (star)
        {
            pattern = star + 1;
            text = ++mark;
        }
        else
        {
            return false;
        }
    }
    while (*pattern == '*')
    {
        ++pattern;
    }
    return *pattern == 0;
}

bool ReadPboEntries(const char* data, size_t size, std::vector<PboEntry>& entries, std::string& error)
{
    entries.clear();
    error.clear();
    // name asciiz + 5 uint32 (packingMethod, originalSize, reserved, time, length),
    // terminated by a record with an empty name; data follows in table order.
    size_t p = 0;
    bool terminated = false;
    for (;;)
    {
        if (p >= size)
        {
            error = "header table runs off the end of the file";
            return false;
        }
        const void* zero = ::memchr(data + p, 0, size - p);
        if (!zero)
        {
            error = "unterminated entry name in header table";
            return false;
        }
        size_t nameEnd = static_cast<size_t>(static_cast<const char*>(zero) - data);
        if (nameEnd + 1 + 20 > size)
        {
            error = "header table runs off the end of the file";
            return false;
        }
        std::string name(data + p, nameEnd - p);
        p = nameEnd + 1;
        uint32_t fields[5];
        ::memcpy(fields, data + p, sizeof(fields));
        p += 20;

        if (name.empty())
        {
            // A "new bank" starts with a Vers record carrying asciiz property
            // pairs; the real table follows it.
            if (entries.empty() && fields[0] == static_cast<uint32_t>(VersionMagic))
            {
                for (;;)
                {
                    const void* z = (p < size) ? ::memchr(data + p, 0, size - p) : nullptr;
                    if (!z)
                    {
                        error = "unterminated property in Vers header";
                        return false;
                    }
                    size_t end = static_cast<size_t>(static_cast<const char*>(z) - data);
                    bool empty = (end == p);
                    p = end + 1;
                    if (empty)
                    {
                        break;
                    }
                }
                continue;
            }
            terminated = true;
            break;
        }

        PboEntry entry;
        entry.name = name;
        entry.packingMethod = fields[0];
        entry.originalSize = fields[1];
        entry.length = fields[4];
        entry.compressed =
            (fields[0] == static_cast<uint32_t>(CompMagic)) || (fields[0] == static_cast<uint32_t>(EncrMagic));
        entries.push_back(entry);
    }
    if (!terminated)
    {
        error = "header table did not terminate";
        return false;
    }

    int64_t at = static_cast<int64_t>(p);
    for (PboEntry& entry : entries)
    {
        entry.dataOffset = at;
        at += entry.length;
        if (at > static_cast<int64_t>(size))
        {
            error = "entry data runs off the end of the file";
            return false;
        }
    }
    return true;
}

std::vector<Finding> ScanConfigText(const char* text, size_t length, int64_t entryOffset, const std::string& entryName)
{
    std::vector<Finding> findings;
    const char* end = text + length;

    // --- UNDEFINED_SCOPE_KEYWORD, grouped under its MISSING_DEFINE_HEADER cause
    Finding cause;
    cause.defect = DefectClass::MissingDefineHeader;
    cause.entry = entryName;
    cause.patchable = true;
    for (const char* line = text; line < end;)
    {
        const char* lineEnd = static_cast<const char*>(::memchr(line, '\n', static_cast<size_t>(end - line)));
        if (!lineEnd)
        {
            lineEnd = end;
        }
        const char* p = line;
        SkipBlanks(p, lineEnd);
        const char* indentEnd = p;

        const char* matchedEntry = nullptr;
        for (const char* name : kScopeEntries)
        {
            size_t len = ::strlen(name);
            if (static_cast<size_t>(lineEnd - p) >= len && ::memcmp(p, name, len) == 0 &&
                (p + len == lineEnd || !IsWordChar(p[len])))
            {
                // Longest match wins: `scopeWeapon` must not be read as `scope`.
                if (!matchedEntry || len > ::strlen(matchedEntry))
                {
                    matchedEntry = name;
                }
            }
        }
        if (matchedEntry)
        {
            const char* q = p + ::strlen(matchedEntry);
            SkipBlanks(q, lineEnd);
            if (q < lineEnd && *q == '=')
            {
                ++q;
                SkipBlanks(q, lineEnd);
                for (const ScopeKeyword& kw : kScopeKeywords)
                {
                    size_t len = ::strlen(kw.name);
                    if (static_cast<size_t>(lineEnd - q) < len || ::memcmp(q, kw.name, len) != 0)
                    {
                        continue;
                    }
                    const char* r = q + len;
                    SkipBlanks(r, lineEnd);
                    if (r >= lineEnd || *r != ';')
                    {
                        continue;
                    }
                    if (DefinesKeyword(text, end, kw.name))
                    {
                        break; // the header is there, the preprocessor handles it
                    }
                    size_t width = static_cast<size_t>(r + 1 - line);
                    std::string replacement(line, static_cast<size_t>(indentEnd - line));
                    replacement += matchedEntry;
                    replacement += " = ";
                    replacement += std::to_string(kw.value);
                    replacement += ";";
                    if (!PadTo(replacement, width))
                    {
                        break;
                    }
                    Finding site;
                    site.defect = DefectClass::UndefinedScopeKeyword;
                    site.entry = entryName;
                    site.line = LineOf(text, line);
                    site.detail = std::string(matchedEntry) + " = " + kw.name + ";";
                    Patch patch;
                    patch.offset = entryOffset + (line - text);
                    patch.original.assign(line, width);
                    patch.replacement = replacement;
                    site.patches.push_back(patch);
                    cause.children.push_back(site);
                    break;
                }
            }
        }
        line = (lineEnd == end) ? end : lineEnd + 1;
    }
    if (!cause.children.empty())
    {
        cause.detail =
            "config uses a scope keyword but never #defines it (" + std::to_string(cause.children.size()) + " site(s))";
        findings.push_back(cause);
    }

    // --- MALFORMED_FLOAT: an unquoted numeric token carrying two or more dots.
    // Quoted strings and comments are skipped: a "0.0.1" string literal or a
    // version number in a comment is not a defect.
    for (const char* p = text; p < end;)
    {
        if (*p == '"')
        {
            ++p;
            while (p < end && *p != '"')
            {
                ++p;
            }
            if (p < end)
            {
                ++p;
            }
            continue;
        }
        if (*p == '/' && p + 1 < end && p[1] == '/')
        {
            while (p < end && *p != '\n')
            {
                ++p;
            }
            continue;
        }
        if (*p == '/' && p + 1 < end && p[1] == '*')
        {
            p += 2;
            while (p + 1 < end && !(*p == '*' && p[1] == '/'))
            {
                ++p;
            }
            p = (p + 1 < end) ? p + 2 : end;
            continue;
        }

        const char* tokenStart = p;
        const char* q = p;
        if (*q == '+' || *q == '-')
        {
            ++q;
        }
        if (q >= end || !IsDigit(*q))
        {
            ++p;
            continue;
        }
        if (tokenStart > text && IsTokenNeighbour(tokenStart[-1]))
        {
            // Part of a longer name (a texture path, say); skip the whole run so
            // it is not re-entered. The unconditional ++p guarantees progress
            // even when the run starts on a sign character.
            ++p;
            while (p < end && IsTokenNeighbour(*p))
            {
                ++p;
            }
            continue;
        }
        int dots = 0;
        while (q < end && (IsDigit(*q) || *q == '.'))
        {
            if (*q == '.')
            {
                ++dots;
            }
            ++q;
        }
        bool wholeToken = (q >= end) || !IsTokenNeighbour(*q);
        if (dots >= 2 && wholeToken)
        {
            size_t width = static_cast<size_t>(q - tokenStart);
            std::string token(tokenStart, width);
            std::string replacement;
            if (FormatMalformedFloatReplacement(token.c_str(), width, replacement))
            {
                Finding site;
                site.defect = DefectClass::MalformedFloat;
                site.entry = entryName;
                site.line = LineOf(text, tokenStart);
                site.detail = "'" + token + "' -> '" + replacement + "'";
                Patch patch;
                patch.offset = entryOffset + (tokenStart - text);
                patch.original = token;
                patch.replacement = replacement;
                site.patches.push_back(patch);
                findings.push_back(site);
            }
        }
        p = q;
    }

    return findings;
}

bool ReadModelOrigin(const char* data, size_t size, ModelOrigin& origin, std::string& error)
{
    error.clear();
    if (size < 12 || ::memcmp(data, "ODOL", 4) != 0)
    {
        error = "not an ODOL model";
        return false;
    }
    try
    {
        QIStream in(data, static_cast<int>(size));
        Asset::Formats::BinaryReader reader(in);
        // Replay the reader up to the model-wide fields; tell() then gives the
        // exact trailer offset, so no trailer-anchoring heuristic is needed.
        Asset::Formats::P3D::P3DHeader header = Asset::Formats::P3D::readHeader(reader);
        for (uint32_t i = 0; i < header.lodCount; ++i)
        {
            Asset::Formats::P3D::readCompleteLOD(reader);
        }
        for (uint32_t i = 0; i < header.lodCount; ++i)
        {
            (void)reader.read<float>();
        }
        if (reader.fail())
        {
            error = "truncated model";
            return false;
        }
        int trailer = reader.tell();
        if (trailer < 0 || static_cast<size_t>(trailer) + kBoundingCenterOffset + 12 > size)
        {
            error = "model trailer runs off the end of the entry";
            return false;
        }
        origin.trailerOffset = trailer;
        ::memcpy(&origin.minY, data + trailer + kMinMaxOffset + 4, sizeof(float));
        ::memcpy(&origin.maxY, data + trailer + kMinMaxMaxOffset + 4, sizeof(float));
        ::memcpy(&origin.boundingCenterY, data + trailer + kBoundingCenterOffset + 4, sizeof(float));
        return true;
    }
    catch (const std::exception& e)
    {
        error = e.what();
        return false;
    }
    catch (...)
    {
        error = "unreadable model";
        return false;
    }
}

std::vector<Finding> ScanModelEntry(const char* data, size_t size, int64_t entryOffset, const std::string& entryName)
{
    std::vector<Finding> findings;
    if (size < 12 || ::memcmp(data, "ODOL", 4) != 0)
    {
        return findings; // MLOD and non-models carry no seating information here
    }

    ModelOrigin origin;
    std::string error;
    if (!ReadModelOrigin(data, size, origin, error))
    {
        Finding note;
        note.defect = DefectClass::NotPatchable;
        note.entry = entryName;
        note.patchable = false;
        note.detail = "model not readable (" + error + "), skipped";
        findings.push_back(note);
        return findings;
    }

    // A degenerate bounding box carries no seating information: a flat road or
    // runway plate authored at Y=0 reports minY == maxY == 0, which would read as
    // buried while nothing is wrong. Only a mesh with vertical extent can be.
    if (!(origin.maxY > origin.minY))
    {
        return findings;
    }

    // A static prop is seated at terrainY + boundingCenter.Y (Entity::PlaceOnSurface,
    // Static branch), so a mesh whose top is strictly below its own origin can
    // never seat: it spawns entirely underground. Lift the origin to the lowest
    // vertex. Exactly zero is left alone - a model whose highest vertex lands on
    // its origin seats flush with the ground, which is a deliberate authoring
    // choice (@LoBo's mk2 floater does it).
    if (origin.boundingCenterY + origin.maxY >= 0.0f)
    {
        return findings;
    }

    float newY = -origin.minY;
    char detail[256];
    ::snprintf(detail, sizeof(detail), "boundingCenter.Y %.4f -> %.4f (mesh %.4f..%.4f, buried %.2f m)",
               static_cast<double>(origin.boundingCenterY), static_cast<double>(newY), static_cast<double>(origin.minY),
               static_cast<double>(origin.maxY), static_cast<double>(-(origin.boundingCenterY + origin.maxY)));

    Finding finding;
    finding.defect = DefectClass::BuriedModelOrigin;
    finding.entry = entryName;
    finding.detail = detail;
    Patch patch;
    patch.offset = entryOffset + origin.trailerOffset + kBoundingCenterOffset + 4;
    patch.original.assign(data + origin.trailerOffset + kBoundingCenterOffset + 4, sizeof(float));
    patch.replacement.assign(reinterpret_cast<const char*>(&newY), sizeof(float));
    finding.patches.push_back(patch);
    findings.push_back(finding);
    return findings;
}

namespace
{

bool NameIs(const std::string& name, const char* wanted)
{
    // pbo entry names use backslashes; compare the basename case-insensitively.
    size_t slash = name.find_last_of("\\/");
    const char* base = name.c_str() + (slash == std::string::npos ? 0 : slash + 1);
    size_t len = ::strlen(wanted);
    if (::strlen(base) != len)
    {
        return false;
    }
    for (size_t i = 0; i < len; ++i)
    {
        char a = base[i];
        char b = wanted[i];
        if (a >= 'A' && a <= 'Z')
        {
            a = static_cast<char>(a - 'A' + 'a');
        }
        if (a != b)
        {
            return false;
        }
    }
    return true;
}

bool HasExtension(const std::string& name, const char* ext)
{
    size_t len = ::strlen(ext);
    if (name.size() < len)
    {
        return false;
    }
    const char* tail = name.c_str() + name.size() - len;
    for (size_t i = 0; i < len; ++i)
    {
        char a = tail[i];
        if (a >= 'A' && a <= 'Z')
        {
            a = static_cast<char>(a - 'A' + 'a');
        }
        if (a != ext[i])
        {
            return false;
        }
    }
    return true;
}

} // namespace

std::vector<Finding> ScanPbo(const char* data, size_t size, std::string& error)
{
    std::vector<Finding> findings;
    std::vector<PboEntry> entries;
    if (!ReadPboEntries(data, size, entries, error))
    {
        return findings;
    }

    bool haveTextConfig = false;
    for (const PboEntry& entry : entries)
    {
        if (NameIs(entry.name, "config.cpp"))
        {
            haveTextConfig = true;
        }
    }

    for (const PboEntry& entry : entries)
    {
        const char* body = data + entry.dataOffset;
        if (NameIs(entry.name, "config.cpp"))
        {
            if (entry.compressed)
            {
                Finding note;
                note.defect = DefectClass::NotPatchable;
                note.entry = entry.name;
                note.patchable = false;
                note.detail = "entry is compressed; an in-place patch would move the header table";
                findings.push_back(note);
                continue;
            }
            std::vector<Finding> configFindings = ScanConfigText(body, entry.length, entry.dataOffset, entry.name);
            findings.insert(findings.end(), configFindings.begin(), configFindings.end());
        }
        else if (NameIs(entry.name, "config.bin") && !haveTextConfig)
        {
            Finding note;
            note.defect = DefectClass::NotPatchable;
            note.entry = entry.name;
            note.patchable = false;
            note.detail = "binarised config, not scannable as text";
            findings.push_back(note);
        }
        else if (HasExtension(entry.name, ".p3d"))
        {
            if (entry.compressed)
            {
                continue; // a compressed model cannot be rewritten in place
            }
            std::vector<Finding> modelFindings = ScanModelEntry(body, entry.length, entry.dataOffset, entry.name);
            for (Finding& finding : modelFindings)
            {
                // A model this reader cannot parse is common enough (mlod, other
                // ODOL versions) that reporting each one would drown the output.
                if (finding.defect == DefectClass::NotPatchable)
                {
                    continue;
                }
                findings.push_back(finding);
            }
        }
    }
    return findings;
}

bool HasPatches(const Finding& finding)
{
    if (!finding.patches.empty())
    {
        return true;
    }
    for (const Finding& child : finding.children)
    {
        if (HasPatches(child))
        {
            return true;
        }
    }
    return false;
}

int ApplyPatches(char* data, size_t size, const std::vector<Finding>& findings)
{
    int applied = 0;
    for (const Finding& finding : findings)
    {
        for (const Patch& patch : finding.patches)
        {
            if (patch.original.size() != patch.replacement.size())
            {
                continue; // never move the header table
            }
            if (patch.offset < 0 || static_cast<size_t>(patch.offset) + patch.original.size() > size)
            {
                continue;
            }
            // Verify the bytes still read as planned, so a second run over an
            // already-repaired archive changes nothing.
            if (::memcmp(data + patch.offset, patch.original.data(), patch.original.size()) != 0)
            {
                continue;
            }
            ::memcpy(data + patch.offset, patch.replacement.data(), patch.replacement.size());
            ++applied;
        }
        applied += ApplyPatches(data, size, finding.children);
    }
    return applied;
}

} // namespace Poseidon::ModDoctor
