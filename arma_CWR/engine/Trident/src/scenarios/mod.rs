//! Test scenario definitions and runners.

use std::time::Duration;

/// Result of a test scenario.
#[derive(Debug, Clone, Default)]
pub struct ScenarioResult {
    pub passed: bool,
    pub message: String,
    /// Wall-clock time the test took, stamped by the runner that
    /// invokes the scenario (defaults to zero if not stamped).
    pub duration: Duration,
}

pub mod integration;
pub mod multi;
pub mod stress;

/// The game's `POSEIDON_CACHE_DIR` for a launch: `TRI_POSEIDON_CACHE_DIR` from
/// tri's own environment when it is set and not empty, else the launch's user
/// dir (the default: every launch starts with an empty cache).
///
/// The cache dir holds the GL program binary cache (`gl33_shaders.bin`), so a
/// shared dir gives a warm program-cache lane across launches. It also holds
/// the portrait cache, the MP mission cache and the land cache, which are then
/// shared too.
pub fn poseidon_cache_dir(user_dir: &str) -> String {
    cache_dir_from(std::env::var("TRI_POSEIDON_CACHE_DIR").ok(), user_dir)
}

fn cache_dir_from(shared: Option<String>, user_dir: &str) -> String {
    shared
        .filter(|d| !d.is_empty())
        .unwrap_or_else(|| user_dir.to_string())
}

#[cfg(test)]
mod tests {
    use super::cache_dir_from;

    #[test]
    fn cache_dir_defaults_to_the_user_dir() {
        assert_eq!(cache_dir_from(None, "u"), "u");
        assert_eq!(cache_dir_from(Some(String::new()), "u"), "u");
    }

    #[test]
    fn cache_dir_follows_tri_poseidon_cache_dir() {
        assert_eq!(cache_dir_from(Some("D:/warm".to_string()), "u"), "D:/warm");
    }
}
