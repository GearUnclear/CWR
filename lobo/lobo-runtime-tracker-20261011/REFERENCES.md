# References and recording policy

## Sources

- [CWR implementation repository](https://github.com/GearUnclear/CWR)
- [Audit issue and acceptance checklist](https://github.com/GearUnclear/lobo/issues/1)
- [Repair source PR #70](https://github.com/GearUnclear/CWR/pull/70) and [reproduction/policy documentation](https://github.com/GearUnclear/CWR/blob/ba21ff49370766f02f4d29ca6c65c5fb5e7e3eb3/arma_CWR/tools/lobo/CONTENT-REPAIRS.md).
- Original audited engine revision: `9f3bc9970f2ea8462149bfd24a775f475de93cd8`.
- CWR paths: `arma_CWR/tools/lobo/`, `arma_CWR/tests/ci/`, and `roadmap/roadmap.yaml`.
- Local-only audit artifacts: `tmp/lobo-audit-20261011/` in the CWR checkout. These are not hosted here and are not required to understand the issue.

## Small, useful records

For each repair batch, record the source revision or patch fingerprint, affected archive/member counts, before/after hashes or a compact manifest fingerprint, exact offline validation commands and results, compatibility decisions, and known limitations. Link implementation changes rather than duplicating source trees. Never upload third-party game data or machine-specific secrets.

Balance numbers describe a game policy unless validated as a physical model. Preserve stable class identifiers for old missions. Keep backups local and repair generation reproducible; record corrections if new evidence invalidates an earlier conclusion.
