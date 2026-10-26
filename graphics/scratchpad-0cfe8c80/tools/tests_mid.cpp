TEST_CASE("Volumetric fog: clear noon is thin haze and blue air; grey sky, rain and fog thicken it", "[remaster][fog]")
{
    const LightingTuning t{};
    const FogMedium noon = ComputeFogMedium(ClearNoon(), t, true);
    CHECK(noon.hazeSigma == Catch::Approx(kHazeClear));
    CHECK(noon.rayleigh == Catch::Approx(kRayleighScale));
    CHECK(noon.weatherSigma == 0.0f);
    CHECK(noon.mistSigma == 0.0f);   // no morning, no night, a clear sky
    CHECK(noon.groundSigma == 0.0f); // and no humid ground air
    // Far hills 3 km out at noon sit in visible air, not in a wash.
    const float noonLeft = std::exp(-FogExtinctionAt(noon, 50.0f, 0.0f) * 3000.0f);
    CHECK(noonLeft > 0.15f);
    CHECK(noonLeft < 0.5f);

    // A broken sky (the blue sky with its sunlit cumulus) keeps nearly the
    // clear air; a grey deck thickens the haze and brings the mist.
    FogInputs broken = ClearNoon();
    broken.skyThrough = 0.5f;
    CHECK(ComputeFogMedium(broken, t, true).hazeSigma < 1.25f * noon.hazeSigma);
    FogInputs grey = ClearNoon();
    grey.skyThrough = 0.0f;
    const FogMedium overcast = ComputeFogMedium(grey, t, true);
    CHECK(overcast.hazeSigma > 2.0f * noon.hazeSigma);
    CHECK(overcast.mistSigma > 0.0f);
    CHECK(overcast.groundSigma > 0.0f);

    FogInputs wet = grey;
    wet.rain = 1.0f;
    const FogMedium rain = ComputeFogMedium(wet, t, true);
    CHECK(rain.weatherSigma > 0.0f);
    CHECK(rain.hazeSigma > overcast.hazeSigma);
    CHECK(rain.mistSigma > overcast.mistSigma);
    CHECK(rain.weatherHeight == Catch::Approx(kRainHazeHeight));

    // The knobs: density scales every part, 0 clears the air.
    LightingTuning thick;
    thick.fogDensity = 2.0f;
    CHECK(ComputeFogMedium(wet, thick, true).hazeSigma == Catch::Approx(2.0f * rain.hazeSigma));
    LightingTuning none;
    none.fogDensity = 0.0f;
    const FogMedium clear = ComputeFogMedium(wet, none, true);
    CHECK(FogExtinctionAt(clear, 0.0f, 0.0f) == 0.0f);
    LightingTuning noMist;
    noMist.fogMist = 0.0f;
    CHECK(ComputeFogMedium(wet, noMist, true).mistSigma == 0.0f);

    // A dry evening toward a low sun: no humid air, no mist, nothing to veil
    // the near field.
    FogInputs evening = ClearNoon();
    evening.sunElevationDeg = 9.0f;
    evening.sunAzimuthDeg = 280.0f;
    const FogMedium low = ComputeFogMedium(evening, t, true);
    CHECK(low.groundSigma == 0.0f);
    CHECK(low.mistSigma == 0.0f);
}

TEST_CASE("Volumetric fog: a misty morning is a weather event the date seeds", "[remaster][fog]")
{
    CHECK(FogDayOfYear(1985, 1, 1) == 0);
    CHECK(FogDayOfYear(1985, 7, 5) == 185);
    CHECK(FogDayOfYear(1984, 3, 1) == 60); // a leap year
    int misty = 0;
    for (int d = 0; d < 365; ++d)
    {
        const float e = FogMistEvent(1985, d, 1.0f, 0.0f);
        CHECK(e == FogMistEvent(1985, d, 1.0f, 0.0f)); // the same day every run
        if (e > 0.0f)
        {
            ++misty;
            CHECK(e >= 0.6f);
            CHECK(e <= 1.0f);
            // A grey sky and rain only add misty days.
            CHECK(FogMistEvent(1985, d, 0.0f, 1.0f) > 0.0f);
        }
    }
    // About kMistChanceClear of clear days.
    CHECK(misty > 110);
    CHECK(misty < 180);
}

TEST_CASE("Volumetric fog: the valley mist fills on a misty morning, pools under its top and drops off above",
          "[remaster][fog]")
{
    // An eastern sun from 6 degrees under the horizon, full around sunrise,
    // gone by 24 degrees; a western sun gives no morning.
    CHECK(FogMorningWeight(-10.0f, 70.0f) == 0.0f);
    CHECK(FogMorningWeight(2.0f, 70.0f) == Catch::Approx(1.0f));
    CHECK(FogMorningWeight(16.0f, 70.0f) > 0.0f);
    CHECK(FogMorningWeight(16.0f, 70.0f) < 1.0f);
    CHECK(FogMorningWeight(25.0f, 70.0f) == 0.0f);
    CHECK(FogMorningWeight(2.0f, 290.0f) == 0.0f);

    // A misty and a clear day of the same summer.
    int mistyDay = -1, clearDay = -1;
    for (int d = 150; d < 250 && (mistyDay < 0 || clearDay < 0); ++d)
    {
        if (FogMistEvent(1985, d, 1.0f, 0.0f) > 0.0f)
            mistyDay = mistyDay < 0 ? d : mistyDay;
        else
            clearDay = clearDay < 0 ? d : clearDay;
    }
    REQUIRE(mistyDay >= 0);
    REQUIRE(clearDay >= 0);
    FogInputs dawn = ClearNoon();
    dawn.sunElevationDeg = 1.0f;
    dawn.sunAzimuthDeg = 65.0f;
    dawn.dayOfYear = clearDay;
    const FogMedium clearDawn = ComputeFogMedium(dawn, LightingTuning{}, true);
    CHECK(clearDawn.mistSigma == 0.0f);
    CHECK(clearDawn.groundSigma == 0.0f);
    dawn.dayOfYear = mistyDay;
    const FogMedium m = ComputeFogMedium(dawn, LightingTuning{}, true);
    CHECK(m.mistEvent > 0.0f);
    CHECK(m.mistSigma == Catch::Approx(kMistSigma * m.mistEvent));
    CHECK(m.morning == Catch::Approx(1.0f));
    CHECK(m.groundSigma > 0.0f);
    // Inside the layer the full density; above its top it falls off.
    const float floorY = 40.0f;
    const float layer = 30.0f;
    const float inside = FogExtinctionAt(m, floorY + 10.0f, floorY, 1e4f, layer);
    const float ridge = FogExtinctionAt(m, floorY + 200.0f, floorY, 1e4f, layer);
    CHECK(inside > 8.0f * ridge);
    CHECK(inside ==
          Catch::Approx(FogExtinctionAt(m, floorY + layer * m.mistLayer - 1.0f, floorY, 1e4f, layer)).epsilon(0.05));
    // No layer (a plain, the open sea): no mist.
    CHECK(FogExtinctionAt(m, floorY + 5.0f, floorY, 1e4f, 0.0f) < 0.2f * inside);
    // The same hour in the evening light of that day keeps under a third.
    FogInputs dusk = dawn;
    dusk.sunAzimuthDeg = 295.0f;
    const FogMedium e = ComputeFogMedium(dusk, LightingTuning{}, true);
    CHECK(e.mistSigma > 0.15f * m.mistSigma);
    CHECK(e.mistSigma < 0.35f * m.mistSigma);
}

TEST_CASE("Volumetric fog: the mist layer follows the local relief: valleys hold one, the open sea none",
          "[remaster][fog]")
{
    // A valley along z at x = 32 with slopes rising 8 m per 50 m cell, a flat
    // plain at 30 m on the right and the sea on the left edge.
    const int n = 64;
    const float cell = 50.0f;
    std::vector<float> h(static_cast<size_t>(n) * n);
    for (int z = 0; z < n; ++z)
        for (int x = 0; x < n; ++x)
            h[static_cast<size_t>(z) * n + x] = x < 6     ? -20.0f
                                                : x >= 50 ? 30.0f
                                                          : 30.0f + 8.0f * std::fabs(static_cast<float>(x - 32));
    const std::vector<float> floorY = BuildFogFloor(h.data(), n, cell);
    const std::vector<float> layer = BuildFogLayer(h.data(), floorY, n, cell);
    REQUIRE(layer.size() == h.size());
    auto at = [&](int x, int z) { return layer[static_cast<size_t>(z) * n + x]; };
    CHECK(at(32, 32) > 10.0f);               // the valley holds a layer
    CHECK(at(32, 32) <= kMistLayerMax);      // never deeper than the cap
    CHECK(at(2, 32) == Catch::Approx(0.0f)); // the open sea none
    // Nothing for a floor of another size.
    CHECK(BuildFogLayer(h.data(), std::vector<float>(3), n, cell).empty());
}

TEST_CASE("Volumetric fog: AI sight takes the mist and the humid air the player sees", "[remaster][fog]")
{
    const float a[3] = {500.0f, 45.0f, 500.0f};
    const float b[3] = {1100.0f, 45.0f, 500.0f};
    SetFogSight(nullptr, nullptr);
    CHECK_FALSE(FogSightActive());
    CHECK(FogSightTransmittance(a, b) == 1.0f);
    // A flat valley floor at 40 m under a 30 m layer, on a misty morning.
    auto map = std::make_shared<FogSightMap>();
    map->size = 64;
    map->cellSize = 50.0f;
    map->ground.assign(64 * 64, 40.0f);
    map->floorY.assign(64 * 64, 40.0f);
    map->layer.assign(64 * 64, 30.0f);
    FogMedium m;
    m.mistSigma = kMistSigma;
    m.groundSigma = 0.0f;
    SetFogSight(&m, map);
    CHECK(FogSightActive());
    // 600 m through the layer: most of it gone; the same line 300 m up, clear.
    CHECK(FogSightTransmittance(a, b) < 0.3f);
    const float up[2][3] = {{500.0f, 340.0f, 500.0f}, {1100.0f, 340.0f, 500.0f}};
    CHECK(FogSightTransmittance(up[0], up[1]) > 0.95f);
    // A clear medium hands nothing over.
    FogMedium dry;
    SetFogSight(&dry, map);
    CHECK_FALSE(FogSightActive());
    CHECK(FogSightTransmittance(a, b) == 1.0f);
    SetFogSight(nullptr, nullptr);
}

