import os
import re

os.chdir(r"D:\Arma_CWA_worktrees\look-light\arma_CWR\engine\Poseidon\Graphics\Remaster")

p = "README.md"
s = open(p, encoding="utf-8").read()
start = s.index("| Photoreal light and camera (switch `photoLight`")
end = s.index("\n", start)
row = (
    "| Photoreal light and camera (switch `photoLight`, look lane \"light\") | `PhotoCamera.cpp`, `LightRig.cpp` "
    "`ComputePhysicalRig`, `PoseidonGL33/EngineGL33_Camera.cpp`, shaders `camFold`, `camDecode` (`kCameraCodecGLSL`), "
    "`camSunDisc`, `camSkyLook`, `camCloudTexel`, `finishFragment`, `skyClear`, `psPostComposite`, `psPostMeter`, "
    "`psPostAirApply` | The rig becomes physical: the sun is the sky table's transmittance toward it (with a summer "
    "aerosol on its own path, `kSunAerosolTau`, so a sun under 20 degrees is golden on the ground) and the sky fill is "
    "the table's clear sky integrated over the hemisphere for an up-facing surface and for walls toward and away from "
    "the sun; under the camera the table's air holds a summer haze (`camHaze`, 6 x the reference aerosol). The clear "
    "dome, the fog colour and the sky the sea reflects are the table's own radiance at the rig's scale, with the "
    "camera's look on top (`camSkyLook`): a pale bright band within about 6 degrees of the horizon by day, the upper "
    "sky turned toward the zenith's blue at a low sun (away from the sun's glow), a twilight belt brighter and warmer "
    "toward the sun's bearing, and under broken cloud (`CameraWeatherVeil`) a pale grey veil; the painted clouds are "
    "sampled through a cubic B-spline (no stair-stepped edges). The sun's sprite is replaced by a disc of 0.45 degrees "
    "(`camSunDisc`); light flares keep none of their alpha by day (the sun's starburst) and 0.45 at night (a lamp's "
    "halo); the screen-space shafts do not run under the camera. The world's passes write scene light at the frame's "
    "pre-exposure on a near-logarithmic fold (`camFold`, `camDecode`); the air pass and the reflections decode and "
    "encode through the same functions. The air pass's haze is 6 x denser (`camAerial`: aerial perspective, ridges 1 "
    "to 3 km out go pale and blue), its light toward a low sun is capped for near surfaces and its sun beams fade in "
    "with distance (no washed backlit silhouettes). The post composite is a camera: a 64 x 36 log-luminance meter "
    "read back asynchronously (a ring of four pixel-pack buffers in the target's own half floats, a slot whose fence "
    "has not passed is never reused, only the newest finished one is read, a histogram of 1/32 EV bins on the CPU, "
    "about 0.1 ms in all), the darkest 5 % and brightest 3 % ignored, centre weighted; a scene darker than a sunlit "
    "plain keeps 0.4 of its darkness, a brighter one 0.45 of its brightness; guards on the highlights, the sky's "
    "bright end and the sky by a low sun; at night 3.8 EV lower and at most -1 EV of lift; eye adaptation over about a "
    "second; a daylight white balance that follows the light by 8 % under 8 degrees of sun up to 55 % at 40; the cool "
    "shade of a low sun on what the sky lights more than the sun; less colour in the dark tones by day and on a grey "
    "day; the night's rod shift (warm light keeps its colour); a vignette; a filmic hue-keeping curve (Lottes 2016) "
    "with glare only from light past white. At night lamps, fires and headlights take `camLocal` (2.5 x) and a street "
    "lamp 2 x more, faded in over the first metres from its bulb (its own pole stays dark). Sub-knobs `camEv`, "
    "`camAdapt`, `camCurve`, `camContrast`, `camShoulder`, `camWhite`, `camSat`, `camWb`, `camSky`, `camSkyHue`, "
    "`camNight`, `camVignette`, `camBloom`, `camHaze`, `camScotopic`, `camAdditive`, `camAerial`, `camLocal`; "
    "`triFeatureState` appends the camera's state (`| cam ev= target= pre= mean= high= wb= sky= sun= skyup= reads= "
    "cpu=collect/queue skip=`). |"
)
s = s[:start] + row + s[end:]
s = s.replace("the `Camera` slot of `LitConstants` (79). |", "the `Camera` and `CameraLook` slots of `LitConstants` (79, 80). |")
open(p, "w", encoding="utf-8", newline="").write(s)

p = "research/INTEGRATION-PLAN.md"
s = open(p, encoding="utf-8").read()
s = s.replace("| 80 | `Count` | | |",
              "| 80 | `CameraLook` | photoreal camera's look (weather veil, lamp ambient kept, lamp gain, low-sun sky blue) | frame |\n| 81 | `Count` | | |")
s = s.replace("w the frame's pre-exposure. Any pass that reads or writes",
              "w the frame's pre-exposure; critic round 2 added `CameraLook` at 80 (`Count` 81). Any pass that reads or writes")
open(p, "w", encoding="utf-8", newline="").write(s)
print("ok")
