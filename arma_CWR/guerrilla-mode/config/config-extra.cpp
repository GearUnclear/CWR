// <GameDir>\bin\config-extra.cpp - the data dir's own config supplement.
//
// The engine merges this file into the global config LAST, after every addon
// pbo config and every mod's bin\config.cpp (ParseConfig,
// engine/Poseidon/Asset/Addon/ConfigParsers.cpp), so it is the right place to
// pull in UD's global faction library. The #include resolves against THIS
// file's directory (IO/PreprocC OnEnterInclude), so guerrilla-factions.hpp
// must sit beside it in bin\ - install-missions.ps1 puts it there.
//
// THIS FILE IS A SEED, NOT A TEMPLATE THE INSTALLER OWNS. The full Remaster
// package is expected to ship its own bin\config-extra.cpp (CfgLanguages and
// friends), so the installer writes this copy only when no config-extra.cpp
// exists at all; when one is already there it appends nothing but the include
// line. Never overwrite a package's own file.
#include "guerrilla-factions.hpp"
