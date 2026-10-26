import os
os.chdir(r'D:\Arma_CWA_worktrees\look-fog\arma_CWR')


def edit(p, pairs):
    s = open(p, encoding='utf-8', newline='').read()
    for a, b in pairs:
        assert s.count(a) == 1, (p, a[:80], s.count(a))
        s = s.replace(a, b)
    open(p, 'w', encoding='utf-8', newline='').write(s)


edit('engine/PoseidonGL33/EngineGL33.hpp', [
    ('''        unsigned int nearTex = 0, nearFbo = 0;
        int nearW = 0, nearH = 0;''', '''        unsigned int nearTex = 0, nearFbo = 0;
        int nearW = 0, nearH = 0;
        // The horizon ring (RGBA16F, kFogRingBins x 2, ping-pong): the sky's
        // colour above the land in every azimuth, measured from the frame.
        unsigned int ringTex[2] = {}, ringFbo[2] = {};
        int ringCur = 0;'''),
    ('''        unsigned int progInject = 0, progIntegrate = 0, progNear = 0, progApply = 0;''',
     '''        unsigned int progInject = 0, progIntegrate = 0, progNear = 0, progApply = 0, progRing = 0;'''),
])

p = 'engine/PoseidonGL33/EngineGL33_PostFog.cpp'
edit(p, [
    ('''constexpr float kFogNightAirShare = 0.18f;''', '''constexpr float kFogNightAirShare = 0.18f;
// The horizon ring: azimuth bins, the share of a new measurement in a bin
// (the rest is the last frame's), and the decay of what a bin knows per frame
// out of view.
constexpr int kFogRingBins = 256;
constexpr float kFogRingShare = 0.3f;
constexpr float kFogRingDecay = 0.995f;'''),
    # destroy
    ('''    DeleteFbo(_fog.nearFbo);
    DeleteTex(_fog.nearTex);
    DeleteTex(_fog.floorTex);
    for (unsigned int* prog : {&_fog.progInject, &_fog.progIntegrate, &_fog.progNear, &_fog.progApply})''',
     '''    DeleteFbo(_fog.nearFbo);
    DeleteTex(_fog.nearTex);
    for (int i = 0; i < 2; ++i)
    {
        DeleteFbo(_fog.ringFbo[i]);
        DeleteTex(_fog.ringTex[i]);
    }
    DeleteTex(_fog.floorTex);
    for (unsigned int* prog :
         {&_fog.progInject, &_fog.progIntegrate, &_fog.progNear, &_fog.progApply, &_fog.progRing})'''),
    # programs
    ('''        _fog.progApply = LinkFogProgram(vs, PostShader::FogApply);
        if (vs)
            glDeleteShader(vs);
        if (!_fog.progInject || !_fog.progIntegrate || !_fog.progNear || !_fog.progApply)''',
     '''        _fog.progApply = LinkFogProgram(vs, PostShader::FogApply);
        _fog.progRing = LinkFogProgram(vs, PostShader::FogRing);
        if (vs)
            glDeleteShader(vs);
        if (!_fog.progInject || !_fog.progIntegrate || !_fog.progNear || !_fog.progApply || !_fog.progRing)'''),
    # targets
    ('''        complete = MakeFbo(_fog.outTex, _fog.outFbo) && complete;
        complete = MakeFbo(_fog.nearTex, _fog.nearFbo) && complete;''',
     '''        complete = MakeFbo(_fog.outTex, _fog.outFbo) && complete;
        complete = MakeFbo(_fog.nearTex, _fog.nearFbo) && complete;
        if (!_fog.ringTex[0])
        {
            GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
            const std::vector<float> zero(static_cast<size_t>(kFogRingBins) * 2 * 4, 0.0f);
            for (int i = 0; i < 2; ++i)
            {
                _fog.ringTex[i] = Make2D(GL_RGBA16F, GL_RGBA, GL_FLOAT, kFogRingBins, 2, zero.data());
                glBindTexture(GL_TEXTURE_2D, _fog.ringTex[i]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glBindTexture(GL_TEXTURE_2D, 0);
                complete = MakeFbo(_fog.ringTex[i], _fog.ringFbo[i]) && complete;
            }
            GL33Bind::Invalidate();
        }'''),
    # shared: ring sampler
    ('''        SetInt(prog, "canopySun", kCanopyVisUnit);''', '''        SetInt(prog, "canopySun", kCanopyVisUnit);
        SetInt(prog, "fogRing", kFogRingUnit);'''),
    ('''        BindTex(kFogFloorUnit, _fog.floorTex ? _fog.floorTex : p.blackTex);
        BindSkyTable(); // unit 8: the table the world read, or its stand-in''',
     '''        BindTex(kFogFloorUnit, _fog.floorTex ? _fog.floorTex : p.blackTex);
        BindTex(kFogRingUnit, _fog.ringTex[_fog.ringCur]);
        BindSkyTable(); // unit 8: the table the world read, or its stand-in'''),
    # ring pass before injection
    ('''    // ---- 1. Injection --------------------------------------------------------''',
     '''    // ---- 0. The horizon ring ----------------------------------------------------
    // The sky's colour just above the land in every azimuth in view, from the
    // resolved frame (before the fog); out of view the last frame's, fading.
    // A camera jump starts it again.
    {
        const int next = 1 - _fog.ringCur;
        const GLuint ring = _fog.progRing;
        glUseProgram(ring);
        SetInt(ring, "colorTex", 0);
        SetInt(ring, "ringPrev", 3);
        SetInt(ring, "depthTex", 5);
        BindTex(0, p.colorTex);
        BindTex(3, _fog.ringTex[_fog.ringCur]);
        BindTex(5, p.depthTex);
        Set4(ring, "ringView0", rot[0], rot[1], rot[2], 0.0f);
        Set4(ring, "ringView1", rot[3], rot[4], rot[5], 0.0f);
        Set4(ring, "ringView2", rot[6], rot[7], rot[8], 0.0f);
        Set4(ring, "ringProj", p11, p22, history ? 0.0f : 1.0f, kFogRingShare);
        Set4(ring, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
        Set4(ring, "ringCtl", static_cast<float>(kFogRingBins), kFogRingDecay, 0.0f, 0.0f);
        DrawTo(_fog.ringFbo[next], kFogRingBins, 2);
        _fog.ringCur = next;
    }

    // ---- 1. Injection --------------------------------------------------------'''),
])
print('ok')
