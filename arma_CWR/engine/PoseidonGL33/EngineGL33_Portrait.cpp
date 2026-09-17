#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Core/GLPipelineState.hpp>
#include <Poseidon/Graphics/Core/GLClear.hpp>
#include <array>

bool EngineGL33::CapturePortrait(const std::function<void()>& draw, std::vector<uint8_t>& rgb)
{
    if (!_glContext || _portraitTargetSize)
        return false;
    FlushQueues();
    constexpr int size = 512;
    GLint readFbo, drawFbo, viewport[4], pack, renderbuffer, packBuffer, packRow, packSkipRows, packSkipPixels;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFbo);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_PACK_ALIGNMENT, &pack);
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &packBuffer);
    glGetIntegerv(GL_PACK_ROW_LENGTH, &packRow);
    glGetIntegerv(GL_PACK_SKIP_ROWS, &packSkipRows);
    glGetIntegerv(GL_PACK_SKIP_PIXELS, &packSkipPixels);
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &renderbuffer);
    const bool scissor = glIsEnabled(GL_SCISSOR_TEST), srgb = glIsEnabled(GL_FRAMEBUFFER_SRGB);
    const auto pass = _activePassId;
    const bool sun = _sunEnabled, shadows = _shadowMapActive;
    const auto exposure = _accomodateEye;
    const auto frame = _frameState;
    const auto constants = _psConstants;
    const auto aspect = _aspectSettings;
    const int width = _w, height = _h;
    const bool night = _nightVision;
    GLuint fbo = 0, colour = 0, depth = 0;
    glGenFramebuffers(1, &fbo);
    glGenRenderbuffers(1, &colour);
    glGenRenderbuffers(1, &depth);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, colour);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, size, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colour);
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth);
    bool ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    if (ok)
    {
        _portraitTargetSize = _w = _h = size;
        _nightVision = false;
        _accomodateEye = Color(1, 1, 1);
        _shadowMapActive = false;
        _sunEnabled = false;
        BeginScreenPass();
        _materialSetSpec = -1;
        _aspectSettings.worldLeft = _aspectSettings.worldTop = 0;
        _aspectSettings.worldRight = _aspectSettings.worldBottom = 1;
        glViewport(0, 0, size, size);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_FRAMEBUFFER_SRGB);
        Clear(true, true, PackedColor(0xff777a78));
        try
        {
            draw();
            FlushQueues();
            std::vector<uint8_t> bottomUp(size * size * 3);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_ROW_LENGTH, 0);
            glPixelStorei(GL_PACK_SKIP_ROWS, 0);
            glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
            glReadPixels(0, 0, size, size, GL_RGB, GL_UNSIGNED_BYTE, bottomUp.data());
            rgb.resize(bottomUp.size());
            for (int y = 0; y < size; ++y)
                std::copy_n(bottomUp.data() + (size - y - 1) * size * 3, size * 3, rgb.data() + y * size * 3);
        }
        catch (...)
        {
            ok = false;
            rgb.clear();
        }
    }
    BeginScreenPass();
    _portraitTargetSize = 0;
    _sunEnabled = sun;
    _shadowMapActive = shadows;
    _w = width;
    _h = height;
    _nightVision = night;
    _accomodateEye = exposure;
    _aspectSettings = aspect;
    _frameState = frame;
    _psConstants = constants;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFbo);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, packBuffer);
    glPixelStorei(GL_PACK_ALIGNMENT, pack);
    glPixelStorei(GL_PACK_ROW_LENGTH, packRow);
    glPixelStorei(GL_PACK_SKIP_ROWS, packSkipRows);
    glPixelStorei(GL_PACK_SKIP_PIXELS, packSkipPixels);
    if (scissor)
        glEnable(GL_SCISSOR_TEST);
    else
        glDisable(GL_SCISSOR_TEST);
    if (srgb)
        glEnable(GL_FRAMEBUFFER_SRGB);
    else
        glDisable(GL_FRAMEBUFFER_SRGB);
    glDeleteRenderbuffers(1, &depth);
    glDeleteRenderbuffers(1, &colour);
    glDeleteFramebuffers(1, &fbo);
    GL33Bind::Invalidate();
    InvalidatePipelineCache();
    if (pass != Poseidon::PassId::ScreenSpace)
        BeginPass(pass);
    else
        UploadVSScreenConstants();
    _materialSetSpec = -1;
    UpdateShadowMapLitState();
    UploadFrameConstants(frame);
    FlushPSConstants();
    return ok;
}
