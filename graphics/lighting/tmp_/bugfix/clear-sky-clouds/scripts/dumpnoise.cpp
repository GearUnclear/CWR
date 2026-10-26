#include <Poseidon/Graphics/Remaster/CloudNoise.hpp>
#include <cstdio>
int main(int argc, char** argv)
{
    const auto v = Poseidon::remaster::BuildCloudNoise(256, 1);
    FILE* f = std::fopen(argv[1], "wb");
    std::fwrite(v.data(), 1, v.size(), f);
    std::fclose(f);
    return 0;
}
