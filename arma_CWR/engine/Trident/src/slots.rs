//! Machine-wide game-launch budget shared by every `tri` process.
//!
//! `-j N` only caps one `tri` invocation. Several sessions often run their own
//! `tri` at once, and too many simultaneous `PoseidonGame` instances stall the
//! startup path. This module adds a second, machine-wide gate:
//!
//! * **Slot locks.** The budget is `N` lock files (`slot-0.lock` ..
//!   `slot-{N-1}.lock`) in a per-user directory that does not depend on the
//!   cwd (`TRI_GLOBAL_SLOT_DIR`, else `%LOCALAPPDATA%\Trident\game-slots` on
//!   Windows, else `<temp>/trident-game-slots`). A permit is an exclusive
//!   advisory lock (`LockFileEx` / `flock`, via `fd-lock`) on one file. The OS
//!   drops the lock when the holding process exits, so a crashed or killed
//!   `tri` never leaks a permit. (A named kernel semaphore would leak: its
//!   count is not restored when a holder dies while other processes keep the
//!   object open.)
//! * **Ledger.** While a permit is held, `<ledger>/<tri-pid>-<seq>.json` says
//!   who holds it. `tri slots` prints the ledger and the lock state.
//!
//! The limit is per process: each `tri` only uses slots `0..N` of its own `N`.
//! When two processes disagree, the smaller `N` is respected by the process
//! that asked for it, while the larger one can push the machine total up to
//! its own `N`. A warning is logged when a live ledger entry shows a different
//! limit.

use anyhow::{bail, Context, Result};
use serde::{Deserialize, Serialize};
use std::collections::HashSet;
use std::fs::File;
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::OnceLock;
use std::time::{Duration, Instant, SystemTime};

/// Default machine-wide limit on simultaneous game instances.
pub const DEFAULT_GLOBAL_SLOTS: u32 = 4;
/// Environment override for the limit (`0` = unlimited).
pub const ENV_GLOBAL_SLOTS: &str = "TRI_GLOBAL_SLOTS";
/// Environment override for the ledger directory.
pub const ENV_LEDGER_DIR: &str = "TRI_SLOT_LEDGER_DIR";
/// Environment override for the slot-lock directory.
pub const ENV_LOCK_DIR: &str = "TRI_GLOBAL_SLOT_DIR";

const POLL_INTERVAL: Duration = Duration::from_millis(250);
const WAIT_REMINDER: Duration = Duration::from_secs(30);

/// Resolve the limit: CLI flag > `TRI_GLOBAL_SLOTS` > default.
pub fn resolve_global_slots(cli: Option<u32>, env: Option<&str>) -> Result<u32> {
    if let Some(n) = cli {
        return Ok(n);
    }
    match env.map(str::trim) {
        Some(raw) if !raw.is_empty() => raw
            .parse::<u32>()
            .with_context(|| format!("{ENV_GLOBAL_SLOTS}='{raw}' is not a non-negative integer")),
        _ => Ok(DEFAULT_GLOBAL_SLOTS),
    }
}

/// Process-wide settings for the global gate.
#[derive(Debug, Clone)]
pub struct GlobalSlots {
    /// Machine-wide limit; `0` = unlimited (no locks taken).
    pub limit: u32,
    pub lock_dir: PathBuf,
    pub ledger_dir: PathBuf,
}

static GLOBAL: OnceLock<GlobalSlots> = OnceLock::new();
static SEQ: AtomicU64 = AtomicU64::new(0);

impl GlobalSlots {
    /// Build from the environment and cwd.
    pub fn from_env(limit: u32) -> Self {
        Self {
            limit,
            lock_dir: default_lock_dir(),
            ledger_dir: default_ledger_dir(),
        }
    }
}

/// Install the process settings, sweep stale ledger entries, and warn when a
/// live `tri` runs with a different limit. Call once at startup.
pub fn init(settings: GlobalSlots) {
    let swept = sweep_stale(&settings.ledger_dir);
    if swept > 0 {
        tracing::debug!("swept {swept} stale engine-slot ledger entries");
    }
    let me = std::process::id();
    let mut others: Vec<u32> = read_ledger(&settings.ledger_dir)
        .into_iter()
        .filter(|(_, e)| e.tri_pid != me && e.global_limit != settings.limit)
        .map(|(_, e)| e.global_limit)
        .collect();
    others.sort_unstable();
    others.dedup();
    if !others.is_empty() {
        tracing::warn!(
            "global game-slot limit mismatch: this tri uses {}, other running tri use {:?}; \
             each process caps the machine at its own limit",
            fmt_limit(settings.limit),
            others
        );
    }
    let _ = GLOBAL.set(settings);
}

/// The installed settings (default limit and dirs if `init` was never called).
pub fn settings() -> &'static GlobalSlots {
    GLOBAL.get_or_init(|| GlobalSlots::from_env(DEFAULT_GLOBAL_SLOTS))
}

fn fmt_limit(limit: u32) -> String {
    if limit == 0 {
        "unlimited".into()
    } else {
        limit.to_string()
    }
}

fn default_lock_dir() -> PathBuf {
    if let Some(dir) = std::env::var_os(ENV_LOCK_DIR).filter(|v| !v.is_empty()) {
        return PathBuf::from(dir);
    }
    #[cfg(windows)]
    if let Some(local) = std::env::var_os("LOCALAPPDATA").filter(|v| !v.is_empty()) {
        return PathBuf::from(local).join("Trident").join("game-slots");
    }
    std::env::temp_dir().join("trident-game-slots")
}

fn default_ledger_dir() -> PathBuf {
    if let Some(dir) = std::env::var_os(ENV_LEDGER_DIR).filter(|v| !v.is_empty()) {
        return PathBuf::from(dir);
    }
    std::env::current_dir()
        .ok()
        .and_then(|cwd| find_repo_root(&cwd))
        .map_or_else(
            || std::env::temp_dir().join("trident-engine-slots"),
            |root| root.join("tmp").join("engine-slots"),
        )
}

/// Walk up from `start` to the git root. A linked worktree (`.git` is a file
/// pointing at `<main>/.git/worktrees/<name>`) resolves to the main checkout,
/// so every worktree shares one ledger.
pub fn find_repo_root(start: &Path) -> Option<PathBuf> {
    for dir in start.ancestors() {
        let dot_git = dir.join(".git");
        if dot_git.is_dir() {
            return Some(dir.to_path_buf());
        }
        if dot_git.is_file() {
            let text = std::fs::read_to_string(&dot_git).ok()?;
            let gitdir = text.lines().find_map(|l| l.strip_prefix("gitdir:"))?.trim();
            let gitdir = PathBuf::from(gitdir);
            let main = gitdir
                .ancestors()
                .find(|p| p.file_name().is_some_and(|n| n == ".git"))
                .and_then(Path::parent);
            return Some(main.map_or_else(|| dir.to_path_buf(), Path::to_path_buf));
        }
    }
    None
}

// ---------------------------------------------------------------------------
// Ledger
// ---------------------------------------------------------------------------

/// One ledger record: a `tri` process holding global permits for one test.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq)]
pub struct LedgerEntry {
    pub tri_pid: u32,
    pub test: String,
    pub data_dir: Option<String>,
    pub slots: u32,
    pub global_limit: u32,
    pub started: String,
}

/// Write a ledger entry; returns its path.
pub fn write_ledger(dir: &Path, entry: &LedgerEntry) -> Result<PathBuf> {
    std::fs::create_dir_all(dir)
        .with_context(|| format!("failed to create ledger dir '{}'", dir.display()))?;
    let seq = SEQ.fetch_add(1, Ordering::Relaxed);
    let path = dir.join(format!("{}-{seq}.json", entry.tri_pid));
    let body = serde_json::to_string_pretty(entry)?;
    std::fs::write(&path, body)
        .with_context(|| format!("failed to write ledger entry '{}'", path.display()))?;
    Ok(path)
}

/// Read every parseable ledger entry, sorted by start time.
pub fn read_ledger(dir: &Path) -> Vec<(PathBuf, LedgerEntry)> {
    let Ok(rd) = std::fs::read_dir(dir) else {
        return Vec::new();
    };
    let mut out: Vec<_> = rd
        .filter_map(Result::ok)
        .map(|e| e.path())
        .filter(|p| p.extension().is_some_and(|x| x == "json"))
        .filter_map(|p| {
            let text = std::fs::read_to_string(&p).ok()?;
            let entry = serde_json::from_str::<LedgerEntry>(&text).ok()?;
            Some((p, entry))
        })
        .collect();
    out.sort_by(|a, b| a.1.started.cmp(&b.1.started));
    out
}

fn ledger_pid(path: &Path) -> Option<u32> {
    let stem = path.file_stem()?.to_str()?;
    stem.split('-').next()?.parse().ok()
}

/// Remove entries whose `tri` pid is dead (or is this process: a fresh
/// process has written nothing yet, so a same-pid entry is a reused pid).
/// Returns how many were removed. Does nothing if liveness is unknowable.
pub fn sweep_stale(dir: &Path) -> usize {
    let Some(live) = live_pids() else {
        return 0;
    };
    sweep_stale_with(dir, &live, std::process::id())
}

fn sweep_stale_with(dir: &Path, live: &HashSet<u32>, me: u32) -> usize {
    let Ok(rd) = std::fs::read_dir(dir) else {
        return 0;
    };
    let mut removed = 0;
    for path in rd.filter_map(Result::ok).map(|e| e.path()) {
        if !path.extension().is_some_and(|x| x == "json") {
            continue;
        }
        let Some(pid) = ledger_pid(&path) else {
            continue;
        };
        if (pid == me || !live.contains(&pid)) && std::fs::remove_file(&path).is_ok() {
            removed += 1;
        }
    }
    removed
}

/// Set of live pids on this machine, or `None` if it cannot be determined.
fn live_pids() -> Option<HashSet<u32>> {
    #[cfg(windows)]
    {
        let out = std::process::Command::new("tasklist")
            .args(["/FO", "CSV", "/NH"])
            .output()
            .ok()?;
        if !out.status.success() {
            return None;
        }
        let text = String::from_utf8_lossy(&out.stdout);
        let pids: HashSet<u32> = text
            .lines()
            .filter_map(|l| l.split("\",\"").nth(1)?.trim_matches('"').parse().ok())
            .collect();
        (!pids.is_empty()).then_some(pids)
    }
    #[cfg(not(windows))]
    {
        if let Ok(rd) = std::fs::read_dir("/proc") {
            let pids: HashSet<u32> = rd
                .filter_map(Result::ok)
                .filter_map(|e| e.file_name().to_str()?.parse().ok())
                .collect();
            if !pids.is_empty() {
                return Some(pids);
            }
        }
        let out = std::process::Command::new("ps")
            .args(["-A", "-o", "pid="])
            .output()
            .ok()?;
        let pids: HashSet<u32> = String::from_utf8_lossy(&out.stdout)
            .lines()
            .filter_map(|l| l.trim().parse().ok())
            .collect();
        (!pids.is_empty()).then_some(pids)
    }
}

/// ISO 8601 UTC timestamp (`YYYY-MM-DDTHH:MM:SSZ`) without a date crate.
pub fn iso8601_utc(t: SystemTime) -> String {
    let secs = t
        .duration_since(SystemTime::UNIX_EPOCH)
        .map_or(0, |d| d.as_secs());
    let days = i64::try_from(secs / 86_400).unwrap_or(0);
    let rem = secs % 86_400;
    // Howard Hinnant's civil_from_days.
    let z = days + 719_468;
    let era = z.div_euclid(146_097);
    let doe = z.rem_euclid(146_097);
    let yoe = (doe - doe / 1460 + doe / 36_524 - doe / 146_096) / 365;
    let doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    let mp = (5 * doy + 2) / 153;
    let day = doy - (153 * mp + 2) / 5 + 1;
    let month = if mp < 10 { mp + 3 } else { mp - 9 };
    let year = yoe + era * 400 + i64::from(month <= 2);
    format!(
        "{year:04}-{month:02}-{day:02}T{:02}:{:02}:{:02}Z",
        rem / 3600,
        (rem % 3600) / 60,
        rem % 60
    )
}

// ---------------------------------------------------------------------------
// Slot locks
// ---------------------------------------------------------------------------

fn slot_path(dir: &Path, index: u32) -> PathBuf {
    dir.join(format!("slot-{index}.lock"))
}

fn open_slot(dir: &Path, index: u32) -> Result<fd_lock::RwLock<File>> {
    let path = slot_path(dir, index);
    let file = std::fs::OpenOptions::new()
        .create(true)
        .truncate(false)
        .read(true)
        .write(true)
        .open(&path)
        .with_context(|| format!("failed to open slot lock '{}'", path.display()))?;
    Ok(fd_lock::RwLock::new(file))
}

/// Try to lock one slot file. `Ok(None)` when another holder has it.
fn try_lock_slot(dir: &Path, index: u32) -> Result<Option<fd_lock::RwLock<File>>> {
    let mut lock = open_slot(dir, index)?;
    let acquired = match lock.try_write() {
        Ok(guard) => {
            // Keep the lock past the guard: it is released when the file
            // handle closes (dropping the RwLock, or process exit).
            std::mem::forget(guard);
            true
        }
        Err(e) if e.kind() == std::io::ErrorKind::WouldBlock => false,
        Err(e) => {
            return Err(e)
                .with_context(|| format!("failed to lock '{}'", slot_path(dir, index).display()))
        }
    };
    Ok(acquired.then_some(lock))
}

/// All-or-nothing: lock `want` distinct slots among `0..limit`, or none.
fn try_lock_many(dir: &Path, limit: u32, want: u32) -> Result<Option<Vec<fd_lock::RwLock<File>>>> {
    std::fs::create_dir_all(dir)
        .with_context(|| format!("failed to create slot dir '{}'", dir.display()))?;
    let mut held = Vec::new();
    for index in 0..limit {
        if let Some(lock) = try_lock_slot(dir, index)? {
            held.push(lock);
            if held.len() >= want as usize {
                return Ok(Some(held));
            }
        }
    }
    Ok(None)
}

/// Which of slots `0..count` are currently held by someone (probe only).
pub fn probe_held(dir: &Path, count: u32) -> Vec<bool> {
    (0..count)
        .map(|i| slot_path(dir, i).exists() && matches!(try_lock_slot(dir, i), Ok(None) | Err(_)))
        .collect()
}

/// Held global permits plus their ledger entry; both released on drop.
#[derive(Debug)]
pub struct GlobalPermit {
    locks: Vec<fd_lock::RwLock<File>>,
    ledger: Option<PathBuf>,
}

impl GlobalPermit {
    pub fn count(&self) -> usize {
        self.locks.len()
    }
}

impl Drop for GlobalPermit {
    fn drop(&mut self) {
        if let Some(path) = self.ledger.take() {
            let _ = std::fs::remove_file(path);
        }
        self.locks.clear();
    }
}

/// Acquire `slots` machine-wide permits for one game launch, using the
/// process settings. Waits (polling) until they are free.
pub async fn acquire(slots: u32, test: &str, data_dir: Option<&str>) -> Result<GlobalPermit> {
    acquire_with(settings(), slots, test, data_dir, None).await
}

/// Like [`acquire`] with explicit settings and an optional wait timeout.
pub async fn acquire_with(
    cfg: &GlobalSlots,
    slots: u32,
    test: &str,
    data_dir: Option<&str>,
    timeout: Option<Duration>,
) -> Result<GlobalPermit> {
    let want = if cfg.limit == 0 {
        0
    } else {
        slots.clamp(1, cfg.limit)
    };
    let start = Instant::now();
    let mut last_note: Option<Instant> = None;
    let locks = if want == 0 {
        Vec::new()
    } else {
        loop {
            if let Some(locks) = try_lock_many(&cfg.lock_dir, cfg.limit, want)? {
                break locks;
            }
            if last_note.map_or(true, |t| t.elapsed() >= WAIT_REMINDER) {
                let holders: Vec<String> = read_ledger(&cfg.ledger_dir)
                    .into_iter()
                    .map(|(_, e)| format!("{} (pid {}, {})", e.test, e.tri_pid, e.slots))
                    .collect();
                tracing::info!(
                    "{test}: waiting for {want} global game slot(s) (machine limit {}, waited {:.0}s); \
                     holders: {}",
                    cfg.limit,
                    start.elapsed().as_secs_f64(),
                    if holders.is_empty() {
                        "unknown (see `tri slots`)".to_string()
                    } else {
                        holders.join(", ")
                    }
                );
                last_note = Some(Instant::now());
            }
            if timeout.is_some_and(|t| start.elapsed() >= t) {
                bail!(
                    "timed out after {:.1}s waiting for {want} global game slot(s)",
                    start.elapsed().as_secs_f64()
                );
            }
            tokio::time::sleep(POLL_INTERVAL).await;
        }
    };
    if last_note.is_some() {
        tracing::info!(
            "{test}: got {want} global game slot(s) after waiting {:.1}s",
            start.elapsed().as_secs_f64()
        );
    }
    let entry = LedgerEntry {
        tri_pid: std::process::id(),
        test: test.to_string(),
        data_dir: data_dir.map(String::from),
        slots: want,
        global_limit: cfg.limit,
        started: iso8601_utc(SystemTime::now()),
    };
    let ledger = match write_ledger(&cfg.ledger_dir, &entry) {
        Ok(p) => Some(p),
        Err(e) => {
            tracing::warn!("engine-slot ledger write failed: {e:#}");
            None
        }
    };
    Ok(GlobalPermit { locks, ledger })
}

/// `tri slots`: print the limit, the lock state, and the ledger.
pub fn print_status(cfg: &GlobalSlots) {
    println!("global game-slot limit: {}", fmt_limit(cfg.limit));
    println!("lock dir:   {}", cfg.lock_dir.display());
    println!("ledger dir: {}", cfg.ledger_dir.display());
    let existing = std::fs::read_dir(&cfg.lock_dir).map_or(0, |rd| {
        rd.filter_map(Result::ok)
            .filter_map(|e| {
                let name = e.file_name().into_string().ok()?;
                name.strip_prefix("slot-")?
                    .strip_suffix(".lock")?
                    .parse::<u32>()
                    .ok()
            })
            .max()
            .map_or(0, |m| m + 1)
    });
    let count = existing.max(cfg.limit);
    if count > 0 {
        let held = probe_held(&cfg.lock_dir, count);
        let busy = held.iter().filter(|h| **h).count();
        println!("slots held: {busy}/{}", fmt_limit(cfg.limit));
    }
    let entries = read_ledger(&cfg.ledger_dir);
    if entries.is_empty() {
        println!("no games running under tri");
        return;
    }
    println!();
    println!(
        "{:>7}  {:>5}  {:<20}  {:<40}  data dir",
        "pid", "slots", "started", "test"
    );
    for (_, e) in entries {
        println!(
            "{:>7}  {:>5}  {:<20}  {:<40}  {}",
            e.tri_pid,
            e.slots,
            e.started,
            e.test,
            e.data_dir.as_deref().unwrap_or("-")
        );
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn cfg_in(dir: &Path, limit: u32) -> GlobalSlots {
        GlobalSlots {
            limit,
            lock_dir: dir.join("locks"),
            ledger_dir: dir.join("ledger"),
        }
    }

    #[test]
    fn precedence_cli_over_env_over_default() {
        assert_eq!(resolve_global_slots(Some(2), Some("7")).unwrap(), 2);
        assert_eq!(resolve_global_slots(Some(0), Some("7")).unwrap(), 0);
        assert_eq!(resolve_global_slots(None, Some("7")).unwrap(), 7);
        assert_eq!(resolve_global_slots(None, Some(" 0 ")).unwrap(), 0);
        assert_eq!(
            resolve_global_slots(None, None).unwrap(),
            DEFAULT_GLOBAL_SLOTS
        );
        assert_eq!(
            resolve_global_slots(None, Some("")).unwrap(),
            DEFAULT_GLOBAL_SLOTS
        );
        assert!(resolve_global_slots(None, Some("four")).is_err());
        assert!(resolve_global_slots(None, Some("-1")).is_err());
    }

    #[test]
    fn iso8601_known_values() {
        assert_eq!(iso8601_utc(SystemTime::UNIX_EPOCH), "1970-01-01T00:00:00Z");
        let t = SystemTime::UNIX_EPOCH + Duration::from_secs(1_709_210_096);
        assert_eq!(iso8601_utc(t), "2024-02-29T12:34:56Z");
    }

    #[test]
    fn ledger_write_read_remove() {
        let dir = tempfile::tempdir().unwrap();
        let entry = LedgerEntry {
            tri_pid: 4242,
            test: "a/b.test.sqf".into(),
            data_dir: Some("D:/data".into()),
            slots: 2,
            global_limit: 4,
            started: "2026-10-13T00:00:00Z".into(),
        };
        let path = write_ledger(dir.path(), &entry).unwrap();
        assert!(path
            .file_name()
            .unwrap()
            .to_str()
            .unwrap()
            .starts_with("4242-"));
        let read = read_ledger(dir.path());
        assert_eq!(read.len(), 1);
        assert_eq!(read[0].1, entry);
        std::fs::remove_file(&path).unwrap();
        assert!(read_ledger(dir.path()).is_empty());
    }

    #[test]
    fn sweep_removes_dead_and_own_pid_entries() {
        let dir = tempfile::tempdir().unwrap();
        for name in ["100-0.json", "200-1.json", "300-2.json", "notes.txt"] {
            std::fs::write(dir.path().join(name), "{}").unwrap();
        }
        let live: HashSet<u32> = [100, 300].into_iter().collect();
        // 200 is dead, 300 is "me" (reused pid), 100 is a live sibling.
        assert_eq!(sweep_stale_with(dir.path(), &live, 300), 2);
        assert!(dir.path().join("100-0.json").exists());
        assert!(!dir.path().join("200-1.json").exists());
        assert!(!dir.path().join("300-2.json").exists());
        assert!(dir.path().join("notes.txt").exists());
    }

    #[test]
    fn live_pids_contains_self() {
        if let Some(live) = live_pids() {
            assert!(live.contains(&std::process::id()));
        }
    }

    #[test]
    fn repo_root_follows_worktree_gitfile() {
        let dir = tempfile::tempdir().unwrap();
        let main = dir.path().join("main");
        std::fs::create_dir_all(main.join(".git").join("worktrees").join("wt")).unwrap();
        let wt = dir.path().join("wt");
        std::fs::create_dir_all(wt.join("sub")).unwrap();
        let gitdir = main.join(".git").join("worktrees").join("wt");
        std::fs::write(wt.join(".git"), format!("gitdir: {}\n", gitdir.display())).unwrap();
        assert_eq!(find_repo_root(&wt.join("sub")).unwrap(), main);
        assert_eq!(find_repo_root(&main).unwrap(), main);
    }

    #[tokio::test]
    async fn permit_writes_and_removes_ledger() {
        let dir = tempfile::tempdir().unwrap();
        let cfg = cfg_in(dir.path(), 2);
        let permit = acquire_with(&cfg, 1, "t", Some("dd"), None).await.unwrap();
        assert_eq!(permit.count(), 1);
        let entries = read_ledger(&cfg.ledger_dir);
        assert_eq!(entries.len(), 1);
        assert_eq!(entries[0].1.test, "t");
        assert_eq!(entries[0].1.global_limit, 2);
        drop(permit);
        assert!(read_ledger(&cfg.ledger_dir).is_empty());
    }

    #[tokio::test]
    async fn slots_clamp_to_limit_and_unlimited_takes_no_locks() {
        let dir = tempfile::tempdir().unwrap();
        let cfg = cfg_in(dir.path(), 2);
        let permit = acquire_with(&cfg, 6, "big", None, None).await.unwrap();
        assert_eq!(permit.count(), 2);
        drop(permit);
        let unlimited = cfg_in(dir.path(), 0);
        let p = acquire_with(&unlimited, 3, "u", None, None).await.unwrap();
        assert_eq!(p.count(), 0);
    }

    #[tokio::test]
    async fn limit_one_serialises_and_times_out() {
        let dir = tempfile::tempdir().unwrap();
        let cfg = cfg_in(dir.path(), 1);
        let first = acquire_with(&cfg, 1, "first", None, None).await.unwrap();
        let short = Some(Duration::from_millis(600));
        let err = acquire_with(&cfg, 1, "second", None, short)
            .await
            .unwrap_err();
        assert!(err.to_string().contains("timed out"), "{err:#}");
        assert_eq!(probe_held(&cfg.lock_dir, 1), vec![true]);
        drop(first);
        assert_eq!(probe_held(&cfg.lock_dir, 1), vec![false]);
        let second = acquire_with(&cfg, 1, "second", None, short).await.unwrap();
        assert_eq!(second.count(), 1);
    }

    #[test]
    fn limit_one_serialises_across_threads() {
        let dir = tempfile::tempdir().unwrap();
        let cfg = std::sync::Arc::new(cfg_in(dir.path(), 1));
        let inside = std::sync::Arc::new(std::sync::atomic::AtomicU32::new(0));
        let max_seen = std::sync::Arc::new(std::sync::atomic::AtomicU32::new(0));
        let handles: Vec<_> = (0..3)
            .map(|i| {
                let cfg = cfg.clone();
                let inside = inside.clone();
                let max_seen = max_seen.clone();
                std::thread::spawn(move || {
                    let rt = tokio::runtime::Builder::new_current_thread()
                        .enable_time()
                        .build()
                        .unwrap();
                    rt.block_on(async {
                        let name = format!("t{i}");
                        let timeout = Some(Duration::from_secs(10));
                        let permit = acquire_with(&cfg, 1, &name, None, timeout).await.unwrap();
                        let now = inside.fetch_add(1, Ordering::SeqCst) + 1;
                        max_seen.fetch_max(now, Ordering::SeqCst);
                        tokio::time::sleep(Duration::from_millis(150)).await;
                        inside.fetch_sub(1, Ordering::SeqCst);
                        drop(permit);
                    });
                })
            })
            .collect();
        for h in handles {
            h.join().unwrap();
        }
        assert_eq!(max_seen.load(Ordering::SeqCst), 1);
    }
}
