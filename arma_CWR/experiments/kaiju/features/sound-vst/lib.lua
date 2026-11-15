-- Helpers for headless REAPER jobs (ReaScript Lua). A job file sets KJ_LOG,
-- does dofile on this file, builds tracks, renders and calls kj_quit().
-- Every helper logs what it did, because nobody is looking at the window.

local logf = io.open(KJ_LOG, "w")
function kj_log(s)
  logf:write(tostring(s) .. "\n")
  logf:flush()
end

kj_log("reaper " .. reaper.GetAppVersion() .. "  resource " .. reaper.GetResourcePath())
do
  local _, mode = reaper.GetAudioDeviceInfo("MODE", "")
  kj_log("audio device: " .. tostring(mode))
end

function kj_track(name)
  local n = reaper.CountTracks(0)
  reaper.InsertTrackAtIndex(n, false)
  local tr = reaper.GetTrack(0, n)
  reaper.GetSetMediaTrackInfo_String(tr, "P_NAME", name or ("t" .. n), true)
  return tr
end

-- Adds an FX by the name REAPER lists it under (for example
-- "VST3i: Augmented VOICES (Arturia)"). Returns the FX index or -1.
function kj_fx(tr, name)
  local t0 = reaper.time_precise()
  local idx = reaper.TrackFX_AddByName(tr, name, false, -1)
  local _, got = reaper.TrackFX_GetFXName(tr, idx, "")
  kj_log(string.format("fx '%s' -> %d (%s) in %.1f s, %d params", name, idx, tostring(got),
    reaper.time_precise() - t0, idx >= 0 and reaper.TrackFX_GetNumParams(tr, idx) or 0))
  return idx
end

-- Writes every parameter (index, name, normalized value, shown value) and the
-- preset list state to a text file.
function kj_dump(tr, fx, path)
  local f = io.open(path, "w")
  local _, nm = reaper.TrackFX_GetFXName(tr, fx, "")
  f:write("# " .. nm .. "\n")
  local pi, np = reaper.TrackFX_GetPresetIndex(tr, fx)
  local _, pn = reaper.TrackFX_GetPreset(tr, fx, "")
  f:write(string.format("# presets: %d, current %d '%s'\n", np, pi, tostring(pn)))
  local n = reaper.TrackFX_GetNumParams(tr, fx)
  for i = 0, n - 1 do
    local _, pname = reaper.TrackFX_GetParamName(tr, fx, i, "")
    local v = reaper.TrackFX_GetParamNormalized(tr, fx, i)
    local _, shown = reaper.TrackFX_GetFormattedParamValue(tr, fx, i, "")
    f:write(string.format("%d\t%s\t%.4f\t%s\n", i, pname, v, shown))
  end
  f:close()
  kj_log("dumped " .. n .. " params of " .. nm .. " to " .. path)
end

-- Lists the host-visible presets (programs) of an FX.
function kj_dump_presets(tr, fx, path, max)
  local f = io.open(path, "w")
  local _, np = reaper.TrackFX_GetPresetIndex(tr, fx)
  for i = 0, math.min(np, max or 4000) - 1 do
    reaper.TrackFX_SetPresetByIndex(tr, fx, i)
    local _, pn = reaper.TrackFX_GetPreset(tr, fx, "")
    f:write(i .. "\t" .. tostring(pn) .. "\n")
  end
  f:close()
  kj_log("dumped " .. np .. " preset names to " .. path)
end

local function find_param(tr, fx, p)
  if type(p) == "number" then return p end
  local n = reaper.TrackFX_GetNumParams(tr, fx)
  for i = 0, n - 1 do
    local _, pname = reaper.TrackFX_GetParamName(tr, fx, i, "")
    if pname == p then return i end
  end
  kj_log("PARAM NOT FOUND: " .. tostring(p))
  return -1
end

-- Sets a parameter (index or exact name) to a normalized value 0..1.
function kj_set(tr, fx, p, v)
  local i = find_param(tr, fx, p)
  if i < 0 then return end
  reaper.TrackFX_SetParamNormalized(tr, fx, i, v)
  local _, shown = reaper.TrackFX_GetFormattedParamValue(tr, fx, i, "")
  kj_log(string.format("  set %s = %.4f (%s)", tostring(p), v, tostring(shown)))
end

function kj_preset(tr, fx, name_or_index)
  local ok
  if type(name_or_index) == "number" then ok = reaper.TrackFX_SetPresetByIndex(tr, fx, name_or_index)
  else ok = reaper.TrackFX_SetPreset(tr, fx, name_or_index) end
  local _, pn = reaper.TrackFX_GetPreset(tr, fx, "")
  kj_log("  preset " .. tostring(name_or_index) .. " -> " .. tostring(ok) .. " now '" .. tostring(pn) .. "'")
end

-- Plugin state as REAPER stores it (base64), to a file and back.
function kj_save_chunk(tr, fx, path)
  local ok, s = reaper.TrackFX_GetNamedConfigParm(tr, fx, "vst_chunk")
  local f = io.open(path, "w")
  f:write(ok and s or "")
  f:close()
  kj_log("  chunk saved: " .. tostring(ok) .. " " .. (ok and #s or 0) .. " chars -> " .. path)
end

function kj_load_chunk(tr, fx, path)
  local f = io.open(path, "r")
  if not f then kj_log("  NO CHUNK FILE " .. path) return end
  local s = f:read("a")
  f:close()
  local ok = reaper.TrackFX_SetNamedConfigParm(tr, fx, "vst_chunk", s)
  kj_log("  chunk loaded: " .. tostring(ok) .. " " .. #s .. " chars from " .. path)
end

-- Automation of one parameter: pts = {{time, normalized value}, ...}.
function kj_env(tr, fx, p, pts, shape)
  local i = find_param(tr, fx, p)
  if i < 0 then return end
  local e = reaper.GetFXEnvelope(tr, fx, i, true)
  for _, pt in ipairs(pts) do
    reaper.InsertEnvelopePoint(e, pt[1], pt[2], shape or 0, 0, false, true)
  end
  reaper.Envelope_SortPoints(e)
  kj_log(string.format("  env %s: %d points", tostring(p), #pts))
end

-- MIDI: notes = {{start_s, end_s, pitch, velocity, [channel]}, ...};
-- events = {{time_s, status(0xB0 cc, 0xE0 bend, 0xD0 pressure, 0xC0 program), data1, data2, [channel]}, ...}
function kj_midi(tr, len, notes, events)
  local item = reaper.CreateNewMIDIItemInProj(tr, 0, len, false)
  local take = reaper.GetActiveTake(item)
  for _, ev in ipairs(events or {}) do
    local ppq = reaper.MIDI_GetPPQPosFromProjTime(take, ev[1])
    reaper.MIDI_InsertCC(take, false, false, ppq, ev[2], ev[5] or 0, ev[3], ev[4])
  end
  for _, nt in ipairs(notes or {}) do
    local a = reaper.MIDI_GetPPQPosFromProjTime(take, nt[1])
    local b = reaper.MIDI_GetPPQPosFromProjTime(take, nt[2])
    reaper.MIDI_InsertNote(take, false, false, a, b, nt[5] or 0, nt[3], nt[4], true)
  end
  reaper.MIDI_Sort(take)
  kj_log(string.format("  midi: %d notes, %d events", #(notes or {}), #(events or {})))
  return take
end

-- Puts an audio file on a track at a time (to run a render through effects).
function kj_audio(tr, path, at)
  reaper.SetOnlyTrackSelected(tr)
  reaper.SetEditCurPos(at or 0, false, false)
  local ok = reaper.InsertMedia(path, 0)
  kj_log("  audio " .. path .. " -> " .. tostring(ok))
end

function kj_vol(tr, gain) reaper.SetMediaTrackInfo_Value(tr, "D_VOL", gain) end

-- Offline render of the master mix, 24 bit wav, stereo unless ch = 1.
function kj_render(path, t0, t1, sr, ch)
  local dir, file = path:match("^(.*)[\\/]([^\\/]+)%.wav$")
  reaper.GetSetProjectInfo(0, "RENDER_SETTINGS", 0, true)
  reaper.GetSetProjectInfo(0, "RENDER_BOUNDSFLAG", 0, true)
  reaper.GetSetProjectInfo(0, "RENDER_STARTPOS", t0, true)
  reaper.GetSetProjectInfo(0, "RENDER_ENDPOS", t1, true)
  reaper.GetSetProjectInfo(0, "RENDER_SRATE", sr or 44100, true)
  reaper.GetSetProjectInfo(0, "RENDER_CHANNELS", ch or 2, true)
  reaper.GetSetProjectInfo(0, "RENDER_ADDTOPROJ", 0, true)
  reaper.GetSetProjectInfo(0, "RENDER_DITHER", 0, true)
  reaper.GetSetProjectInfo(0, "RENDER_TAILFLAG", 0, true)
  reaper.GetSetProjectInfo_String(0, "RENDER_FILE", dir, true)
  reaper.GetSetProjectInfo_String(0, "RENDER_PATTERN", file, true)
  reaper.GetSetProjectInfo_String(0, "RENDER_FORMAT", "ZXZhdxgAAA==", true) -- wav, 24 bit
  os.remove(path)
  local t = reaper.time_precise()
  reaper.Main_OnCommand(42230, 0) -- render with these settings, close the render window when done
  local f = io.open(path, "rb")
  local size = -1
  if f then size = f:seek("end") f:close() end
  kj_log(string.format("render %s: %.1f s of audio in %.1f s, %d bytes", path, t1 - t0, reaper.time_precise() - t, size))
end

-- Saves the project for the record, then quits REAPER. A changed project
-- makes "quit" ask whether to save, which would block forever on a hidden
-- desktop, and saving by script does not clear the changed flag (seen in
-- 7.20). Opening an empty project with "noprompt:" does, so that is done
-- first. KJ_EMPTY is the path of that empty project (the job sets it).
function kj_quit(rpp)
  if rpp then reaper.Main_SaveProjectEx(0, rpp, 0) end
  local f = io.open(KJ_EMPTY, "w")
  f:write('<REAPER_PROJECT 0.1 "7.20/win64" 0\n>\n')
  f:close()
  reaper.Main_openProject("noprompt:" .. KJ_EMPTY)
  kj_log("quit (dirty " .. tostring(reaper.IsProjectDirty(0)) .. ")")
  logf:close()
  reaper.Main_OnCommand(40004, 0)
end

