# Lobo — internal change tracking

Private reference repository for Lost Brothers mod maintenance in Uslu dur!.

This repository holds issues, decisions, validation summaries and a concise change log. It intentionally does **not** contain PBOs, models, textures, audio, extracted addon sources, full logs, credentials or game installers.

- [Active repair audit](https://github.com/GearUnclear/lobo/issues/1) — transferred from CWR #69, with the audit and original history preserved.
- [Implementation repository](https://github.com/GearUnclear/CWR) — repair tooling stays under `arma_CWR/tools/lobo/`; engine/tests stay with their source.
- [Change log](CHANGELOG.md)
- [References and recording policy](REFERENCES.md)

When recording work, distinguish **proposed**, **implemented**, **applied**, **offline verified**, and **runtime verified**. Never infer runtime verification from a static config check. Record reproducible commands, relevant source revisions, a small hash/manifest summary, and unresolved limitations. Asset backups belong beside the local installation, not in GitHub.
