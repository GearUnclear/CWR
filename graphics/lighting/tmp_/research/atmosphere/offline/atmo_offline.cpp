#include <Poseidon/Graphics/Remaster/Atmosphere.hpp>
#include <cstdio>
#include <cmath>
#include <chrono>
using namespace Poseidon::remaster;
int main(int argc, char** argv) {
    Atmosphere atmo;
    float elevs[] = {72.0f, 30.0f, 12.0f, 8.4f, 5.1f, 0.0f, -1.7f, -4.0f};
    // panorama: 360 x 90 (azimuth from sun -180..180, elevation 0..45) raw floats per elevation
    for (float e : elevs) {
        SkyViewLut lut; AtmosphereSettings s;
        auto t0 = std::chrono::steady_clock::now();
        atmo.BuildSkyView(e * 3.14159265f / 180.0f, s, lut);
        double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        printf("elev %.1f built %.1f ms sunT %.3f %.3f %.3f low %.5f %.5f %.5f up %.4f %.4f %.4f sun %.4f %.4f %.4f anti %.4f %.4f %.4f\n", e, ms,
            lut.sunTransmittance.r, lut.sunTransmittance.g, lut.sunTransmittance.b, lut.meanLowBand.r, lut.meanLowBand.g, lut.meanLowBand.b,
            lut.irrUp.r, lut.irrUp.g, lut.irrUp.b, lut.irrSunward.r, lut.irrSunward.g, lut.irrSunward.b, lut.irrAntiSun.r, lut.irrAntiSun.g, lut.irrAntiSun.b);
        char name[256]; snprintf(name, sizeof name, "%s/pano_%+05.1f.pfm", argv[1], e);
        FILE* f = fopen(name, "wb");
        const int W = 360, H = 120; // elevation -10..80
        fprintf(f, "PF\n%d %d\n-1.0\n", W, H);
        const float sinE = std::sin(e * 3.14159265f / 180.0f), cosE = std::cos(e * 3.14159265f / 180.0f);
        for (int y = 0; y < H; ++y) { // PFM bottom-to-top
            float el = (-10.0f + 90.0f * (y + 0.5f) / H) * 3.14159265f / 180.0f;
            for (int x = 0; x < W; ++x) {
                float az = (-180.0f + 360.0f * (x + 0.5f) / W) * 3.14159265f / 180.0f;
                float dx = std::cos(el) * std::cos(az), dy = std::sin(el), dz = std::cos(el) * std::sin(az);
                float c = dx * cosE + dy * sinE;
                Rgb L = Atmosphere::Sample(lut, el, std::fabs(az), c);
                float px[3] = {L.r, L.g, L.b}; fwrite(px, 4, 3, f);
            }
        }
        fclose(f);
    }
}
