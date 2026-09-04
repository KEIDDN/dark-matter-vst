#include "BlackHoleOrb.h"
#include "../dsp/Parameters.h"
#include <cmath>

namespace dm::ui
{

namespace
{
    const char* vertexShaderSrc = R"(
        attribute vec2 p;
        void main() { gl_Position = vec4(p, 0.0, 1.0); }
    )";

    // Ported near-verbatim from the Dark Matter UI prototype (Dark Matter v4.dc.html).
    const char* fragmentShaderSrc = R"(
        #ifdef GL_ES
        precision highp float;
        #endif
        uniform vec2 uRes;
        uniform float uTime,uSize,uMix,uDecay,uMod,uPulse,uDamp,uDiff,uPower;

        vec3 mod289(vec3 x){return x-floor(x*(1.0/289.0))*289.0;}
        vec2 mod289(vec2 x){return x-floor(x*(1.0/289.0))*289.0;}
        vec3 permute(vec3 x){return mod289(((x*34.0)+1.0)*x);}
        float snoise(vec2 v){
          const vec4 C=vec4(0.211324865405187,0.366025403784439,-0.577350269189626,0.024390243902439);
          vec2 i=floor(v+dot(v,C.yy)); vec2 x0=v-i+dot(i,C.xx);
          vec2 i1=(x0.x>x0.y)?vec2(1.0,0.0):vec2(0.0,1.0);
          vec4 x12=x0.xyxy+C.xxzz; x12.xy-=i1; i=mod289(i);
          vec3 pp=permute(permute(i.y+vec3(0.0,i1.y,1.0))+i.x+vec3(0.0,i1.x,1.0));
          vec3 m=max(0.5-vec3(dot(x0,x0),dot(x12.xy,x12.xy),dot(x12.zw,x12.zw)),0.0); m=m*m; m=m*m;
          vec3 x=2.0*fract(pp*C.www)-1.0; vec3 h=abs(x)-0.5; vec3 ox=floor(x+0.5); vec3 a0=x-ox;
          m*=1.79284291400159-0.85373472095314*(a0*a0+h*h);
          vec3 g; g.x=a0.x*x0.x+h.x*x0.y; g.yz=a0.yz*x12.xz+h.yz*x12.yw;
          return 130.0*dot(m,g);
        }
        float fbm(vec2 p){ float a=0.5,s=0.0; for(int i=0;i<5;i++){ s+=a*snoise(p); p=p*2.03+vec2(1.7,9.2); a*=0.5; } return s; }
        void main(){
          vec2 uv=(gl_FragCoord.xy-0.5*uRes)/uRes.y;
          float r=length(uv); float ang=atan(uv.y,uv.x); float t=uTime;
          float sw=(0.35+uMod*1.1)*t*0.085/(r+0.16) + 0.06*sin(t*0.13);
          float c=cos(sw),s=sin(sw); vec2 q=mat2(c,-s,s,c)*uv;
          vec2 drift=0.045*vec2(sin(t*0.21),cos(t*0.17));
          vec2 w=q*3.2+drift; w+=0.55*vec2(fbm(w+t*0.03),fbm(w-t*0.024+5.1));
          float n1=fbm(w*(1.4+uDiff*1.0)+t*0.02);
          float n2=fbm(w*3.6-t*0.014+2.0);
          float fil=pow(1.0-abs(n1),3.0)*0.95+pow(1.0-abs(n2),6.0)*0.55;
          float breathe=1.0+0.012*sin(t*0.47)+0.006*sin(t*1.13);
          float coreR=(0.085+uSize*0.15)*breathe;
          float wob=fbm(vec2(ang*1.4+t*0.055,t*0.04))*0.22*coreR+fbm(vec2(ang*3.8-t*0.035,3.0))*0.09*coreR;
          float cr=coreR+wob;
          float core=smoothstep(cr-0.003,cr+0.018,r);
          float ext=0.06+uDecay*0.32;
          float env=exp(-max(r-cr,0.0)/ext);
          float dens=fil*env*core;
          float rim=exp(-abs(r-cr)*(70.0-uMod*25.0))*core*(0.92+0.08*sin(t*0.6+ang*2.0));
          float pr=cr+(1.0-uPulse)*0.5;
          float pulse=exp(-pow((r-pr)*30.0,2.0))*uPulse*core;
          float warm=1.0-uDamp;
          vec3 deep=vec3(0.20,0.09,0.52), lav=vec3(0.66,0.55,1.0), hot=vec3(0.96,0.93,1.0);
          vec3 col=deep*dens*1.7+lav*pow(dens,2.0)*(1.1+warm*1.2)+hot*pow(dens,5.0)*(0.4+warm*1.6);
          col+=lav*rim*(0.45+uMix*0.85)+hot*rim*rim*warm*0.5;
          col+=hot*pulse*0.5;
          col*=(0.45+uMix*1.0);
          vec2 suv=uv+vec2(t*0.0016,-t*0.0011);
          vec2 g=floor(suv*140.0); float h=fract(sin(dot(g,vec2(127.1,311.7)))*43758.5453);
          vec2 cell=fract(suv*140.0)-0.5; float sp=smoothstep(0.35,0.05,length(cell));
          float star=step(0.994,h)*sp*core*smoothstep(cr,cr+0.15,r)*(0.45+0.55*sin(t*0.9+h*60.0));
          col+=vec3(0.8,0.76,1.0)*star*0.5;
          col=col*core+vec3(0.004,0.003,0.008)*(1.0-core);
          float m=1.0-smoothstep(0.44,0.5,r);
          col*=mix(0.22,1.0,uPower);
          gl_FragColor=vec4(col*m,m);
        }
    )";

    static const float quadVerts[8] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
}

BlackHoleOrb::BlackHoleOrb(juce::AudioProcessorValueTreeState& apvts) : state(apvts)
{
    glContext.setOpenGLVersionRequired(juce::OpenGLContext::defaultGLVersion);
    glContext.setContinuousRepainting(true);
    glContext.setRenderer(this);
    glContext.attachTo(*this);
}

BlackHoleOrb::~BlackHoleOrb()
{
    glContext.detach();
}

void BlackHoleOrb::paint(juce::Graphics& g)
{
    // JUCE draws this 2D layer on top of renderOpenGL()'s output, so only
    // paint the fallback backdrop if the shader actually failed — otherwise
    // this would opaquely cover the real shader render every frame.
    if (! shaderOk.load())
    {
        g.setColour(juce::Colour(0xff030304));
        g.fillEllipse(getLocalBounds().toFloat());
    }
}

void BlackHoleOrb::newOpenGLContextCreated()
{
    using namespace juce::gl;

    shader = std::make_unique<juce::OpenGLShaderProgram>(glContext);
    const bool ok = shader->addVertexShader(vertexShaderSrc)
                  && shader->addFragmentShader(fragmentShaderSrc)
                  && shader->link();
    if (! ok)
    {
        DBG("BlackHoleOrb shader failed: " << shader->getLastError());
        shader.reset();
        return;
    }
    shaderOk.store(true);

    positionAttribute = new juce::OpenGLShaderProgram::Attribute(*shader, "p");

    glContext.extensions.glGenBuffers(1, &vertexBuffer);
    glContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
}

void BlackHoleOrb::openGLContextClosing()
{
    shaderOk.store(false);
    delete positionAttribute;
    positionAttribute = nullptr;
    if (vertexBuffer != 0)
        glContext.extensions.glDeleteBuffers(1, &vertexBuffer);
    vertexBuffer = 0;
    shader.reset();
}

void BlackHoleOrb::renderOpenGL()
{
    using namespace juce::gl;

    const float scale = (float) glContext.getRenderingScale();
    glViewport(0, 0, juce::roundToInt(getWidth() * scale), juce::roundToInt(getHeight() * scale));
    juce::OpenGLHelpers::clear(juce::Colours::transparentBlack);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if (shader == nullptr || positionAttribute == nullptr)
        return;

    shader->use();

    const double now = juce::Time::getMillisecondCounterHiRes();
    double dt = lastRenderTimeMs > 0.0 ? (now - lastRenderTimeMs) / 1000.0 : 0.0;
    dt = juce::jlimit(0.0, 0.05, dt);
    lastRenderTimeMs = now;

    t += (float) dt * 5.0f * 0.2f;

    const float req = pulseRequest.exchange(0.0f);
    if (req > pulseEnv)
        pulseEnv = req;
    pulseEnv *= std::pow(0.08f, (float) dt);

    const bool bypassed = state.getRawParameterValue(ParamID::bypass)->load() > 0.5f;
    const float targets[7] = {
        (float) state.getParameter(ParamID::size)->getValue(),
        (float) state.getParameter(ParamID::mix)->getValue(),
        (float) state.getParameter(ParamID::decay)->getValue(),
        (float) state.getParameter(ParamID::modulation)->getValue(),
        (float) state.getParameter(ParamID::damping)->getValue(),
        (float) state.getParameter(ParamID::diffusion)->getValue(),
        bypassed ? 0.0f : 1.0f
    };
    const float k = 1.0f - std::pow(0.015f, (float) dt);
    for (int i = 0; i < 7; ++i)
        uSmooth[i] += (targets[i] - uSmooth[i]) * k;

    shader->setUniform("uRes", (float) getWidth() * scale, (float) getHeight() * scale);
    shader->setUniform("uTime", t);
    shader->setUniform("uSize", uSmooth[0]);
    shader->setUniform("uMix", uSmooth[1]);
    shader->setUniform("uDecay", uSmooth[2]);
    shader->setUniform("uMod", uSmooth[3]);
    shader->setUniform("uPulse", pulseEnv);
    shader->setUniform("uDamp", uSmooth[4]);
    shader->setUniform("uDiff", uSmooth[5]);
    shader->setUniform("uPower", uSmooth[6]);

    glContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glContext.extensions.glEnableVertexAttribArray((GLuint) positionAttribute->attributeID);
    glContext.extensions.glVertexAttribPointer((GLuint) positionAttribute->attributeID, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glContext.extensions.glDisableVertexAttribArray((GLuint) positionAttribute->attributeID);
}

} // namespace dm::ui
