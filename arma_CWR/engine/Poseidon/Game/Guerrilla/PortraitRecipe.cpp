#include <Poseidon/Game/Guerrilla/PortraitRecipe.hpp>

#include <algorithm>
#include <array>
#include <cmath>

namespace Poseidon::Guerrilla
{
namespace
{
double Lanczos(double x)
{
    x = std::abs(x);
    if (x < 1e-12)
        return 1;
    if (x >= 3)
        return 0;
    x *= 3.14159265358979323846;
    return std::sin(x) * std::sin(x / 3) / (x * x / 3);
}

struct Sample
{
    int start;
    std::vector<double> weights;
};
} // namespace

std::vector<uint8_t> StylePortrait(const std::vector<uint8_t>& rgb)
{
    constexpr int source = PortraitCaptureSize, inner = PortraitPhotoSize, card = PortraitCardSize;
    if (rgb.size() != source * source * 3)
        return {};

    std::vector<uint8_t> toned(rgb.size());
    constexpr std::array<double, 3> warm = {1.06, 1.00, 0.92};
    for (size_t p = 0; p < rgb.size(); p += 3)
    {
        const double lum = 0.299 * rgb[p] + 0.587 * rgb[p + 1] + 0.114 * rgb[p + 2];
        for (int c = 0; c < 3; ++c)
        {
            double value = std::clamp((lum + (rgb[p + c] - lum) * 0.4) * warm[c], 0.0, 255.0);
            value = 18 + value * (227.0 / 255.0);
            toned[p + c] = static_cast<uint8_t>(std::clamp(128 + (value - 128) * 1.08, 0.0, 255.0));
        }
    }

    // Separable, antialiased Lanczos-3; coefficients depend only on the recipe.
    std::array<Sample, inner> samples;
    constexpr double scale = double(source) / inner;
    for (int dst = 0; dst < inner; ++dst)
    {
        const double center = (dst + 0.5) * scale - 0.5;
        auto& sample = samples[dst];
        sample.start = std::max(0, int(std::ceil(center - 3 * scale)));
        const int end = std::min(source - 1, int(std::floor(center + 3 * scale)));
        double sum = 0;
        for (int src = sample.start; src <= end; ++src)
        {
            const double weight = Lanczos((src - center) / scale);
            sample.weights.push_back(weight);
            sum += weight;
        }
        for (auto& weight : sample.weights)
            weight /= sum;
    }
    std::vector<double> horizontal(source * inner * 3);
    for (int y = 0; y < source; ++y)
        for (int x = 0; x < inner; ++x)
            for (int c = 0; c < 3; ++c)
                for (size_t k = 0; k < samples[x].weights.size(); ++k)
                    horizontal[(y * inner + x) * 3 + c] +=
                        toned[(y * source + samples[x].start + k) * 3 + c] * samples[x].weights[k];

    std::vector<uint8_t> pixels(card * card * 4);
    constexpr std::array<uint8_t, 3> paper = {218, 214, 201}, rule = {194, 190, 177};
    for (int y = 0; y < card; ++y)
        for (int x = 0; x < card; ++x)
        {
            auto* dst = pixels.data() + (y * card + x) * 4;
            const bool edge = x >= 9 && x <= 246 && y >= 9 && y <= 246 && (x == 9 || x == 246 || y == 9 || y == 246);
            for (int c = 0; c < 3; ++c)
                dst[c] = edge ? rule[c] : paper[c];
            dst[3] = 255;
            if (x < 10 || x >= 246 || y < 10 || y >= 246)
                continue;
            const auto& sample = samples[y - 10];
            for (int c = 0; c < 3; ++c)
            {
                double value = 0;
                for (size_t k = 0; k < sample.weights.size(); ++k)
                    value += horizontal[((sample.start + k) * inner + x - 10) * 3 + c] * sample.weights[k];
                dst[c] = static_cast<uint8_t>(std::clamp(std::round(value), 0.0, 255.0));
            }
        }
    return pixels;
}
} // namespace Poseidon::Guerrilla
