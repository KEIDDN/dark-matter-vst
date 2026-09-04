#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_opengl/juce_opengl.h>
#include <atomic>

namespace dm::ui
{

/** The reactive black-hole visualisation — a GLSL fragment shader rendering
    swirling plasma whose motion follows the plugin's parameters. */
class BlackHoleOrb : public juce::Component, private juce::OpenGLRenderer
{
public:
    explicit BlackHoleOrb(juce::AudioProcessorValueTreeState& apvts);
    ~BlackHoleOrb() override;

    void paint(juce::Graphics&) override;
    void resized() override {}

    /** Call when a preset is loaded — makes the orb pulse, like the prototype. */
    void triggerPulse() { pulseRequest.store(0.75f); }

private:
    juce::AudioProcessorValueTreeState& state;
    juce::OpenGLContext glContext;
    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    juce::OpenGLShaderProgram::Attribute* positionAttribute = nullptr;
    unsigned int vertexBuffer = 0;

    std::atomic<float> pulseRequest { 0.0f };
    std::atomic<bool> shaderOk { false };

    // GL-thread-only animation state.
    double lastRenderTimeMs = 0.0;
    float t = 0.0f;
    float pulseEnv = 0.0f;
    float uSmooth[7] {}; // size, mix, decay, mod, damp, diff, power (smoothed)

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlackHoleOrb)
};

} // namespace dm::ui
