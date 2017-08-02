#include "CustomEffectBase.h"

#define STRINGIFY(A)  #A

CustomEffectBase::CustomEffectBase()
: _GLState(nullptr)
, _pTarget(nullptr)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	_backgroundListener = EventListenerCustom::create(EVENT_RENDERER_RECREATED,
		[this](EventCustom*)
	{
		auto glProgram = _GLState->getGLProgram();
		glProgram->reset();
		glProgram->initWithByteArrays(ccPositionTextureColor_noMVP_vert, _fragSource.c_str());
		glProgram->link();
		glProgram->updateUniforms();
	}
	);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_backgroundListener, -1);
#endif
}

CustomEffectBase::~CustomEffectBase()
{
	CC_SAFE_RELEASE_NULL(_GLState);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	Director::getInstance()->getEventDispatcher()->removeEventListener(_backgroundListener);
#endif
}

bool CustomEffectBase::initGLProgramState(const std::string &fragSource)
{
	auto glprogram = GLProgram::createWithByteArrays(ccPositionTextureColor_noMVP_vert, fragSource.c_str());

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	_fragSource = fragSource;
#endif

	_GLState = (glprogram == nullptr ? nullptr : GLProgramState::getOrCreateWithGLProgram(glprogram));
	CC_SAFE_RETAIN(_GLState);

	return _GLState != nullptr;
}

void CustomEffectBase::setTarget(cocos2d::Sprite* target)
{
	if (nullptr == target) return;

	//release current sprite glProgramState
	/*auto pTmpState = target->getGLProgramState();
	CC_SAFE_RELEASE(pTmpState);*/

	target->setGLProgramState(_GLState);
	_pTarget = target;

}


//------------------ Effect Blur ----------------------
const char* effectblur_frag = STRINGIFY(
 \n#ifdef GL_ES\n
 precision lowp float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform vec2 resolution;
uniform float blurRadius;
uniform float sampleNum;

vec4 blur(vec2);

void main(void)
{
    vec4 col = blur(v_texCoord); //* v_fragmentColor.rgb;
    gl_FragColor = vec4(col) * v_fragmentColor;
}

vec4 blur(vec2 p)
{
    if (blurRadius > 0.0 && sampleNum > 1.0)
    {
        vec4 col = vec4(0.0);
        vec2 unit = 1.0 / resolution.xy;
        
        float r = blurRadius;
        float sampleStep = r / sampleNum;
        
        float count = 0.0;
        
        for(float x = -r; x < r; x += sampleStep)
        {
            for(float y = -r; y < r; y += sampleStep)
            {
                float weight = (r - abs(x)) * (r - abs(y));
                col += texture2D(CC_Texture0, p + vec2(x * unit.x, y * unit.y)) * weight;
                count += weight;
            }
        }
        
        return col / count;
    }
    
    return texture2D(CC_Texture0, p);
}
);

void BlurEffect::setTarget(cocos2d::Sprite* target)
{
	CustomEffectBase::setTarget(target);

	auto size = _pTarget->getTexture()->getContentSizeInPixels();
	getGLProgramState()->setUniformVec2("resolution", size);
	getGLProgramState()->setUniformFloat("blurRadius", _blurRadius);
	getGLProgramState()->setUniformFloat("sampleNum", _blurSampleNum);
}

bool BlurEffect::init(float blurRadius /*= 10.0f*/, float sampleNum /*= 5.0f*/)
{
	initGLProgramState(effectblur_frag);

	_blurRadius = blurRadius;
	_blurSampleNum = sampleNum;

	return true;
}

//------------------ Effect Gray -----------------

const char* effectgray_frag = STRINGIFY(
\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

void main(void)
{
    //float foo = xx;

	vec4 c = texture2D(CC_Texture0, v_texCoord);
	gl_FragColor.xyz = vec3(0.2126*c.r + 0.7152*c.g + 0.0722*c.b);
	gl_FragColor.w = c.w;
}
);

bool GrayEffect::init()
{
	initGLProgramState(effectgray_frag);

	return true;
}

//------------------ Effect color offset -----------------
const char* coloroffset_frag = STRINGIFY(
\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

void main(void)
{
	vec4 c = texture2D(CC_Texture0, v_texCoord);
	c = vec4(c.r / c.a, c.g / c.a, c.b / c.a, c.a);
	c = clamp(c * v_fragmentColor, 0.0, 1.0);
	c.rgb *= c.a;
	gl_FragColor = c;
}
);

bool ColorOffsetEffect::init()
{
	initGLProgramState(coloroffset_frag);
	return true;
}

//------------------ Effect Outer Glow -----------------

const char* outerglow_frag = STRINGIFY(
\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform vec2 resolution;
uniform int iRange;
uniform vec4 glowColor;

void main(void)
{
    float outter = 0.0f;

    int range = iRange;
    int iStep = 1;
    //range = fract(abs(sin(CC_Time[2])*10.0)) + range;

    float inner = 0.0f;
    int count = 0;
    vec2 unit = 1.0 / resolution.xy;

    for (int k = -range; k < range; k+=iStep)
    {
        for (int j = -range; j < range; j+=iStep)
        {
            vec4 c = texture2D(CC_Texture0, v_texCoord + vec2(unit.x * k , unit.y * j));
            outter += (1.0 - c.a);
            inner += c.a;
			count ++;
        }
    }

    inner /= count;
    outter /= count;
    
    vec4 col = texture2D(CC_Texture0, v_texCoord) * v_fragmentColor;
	float out_alpha = max(col.a,inner);
	col.rgb = col.rgb + (1.0 - col.a) * glowColor.a * glowColor.rgb;

	//gl_FragColor.rgb = col.rgb;
    //gl_FragColor.a = col.a *out_alpha;
    gl_FragColor = col * out_alpha;
}
);

bool OuterGlowEffect::init()
{
    initGLProgramState(outerglow_frag);

    _range = 20;
    _glowColor = Color4F::ORANGE;

    return true;
}

void OuterGlowEffect::setTarget(cocos2d::Sprite* target)
{
    CustomEffectBase::setTarget(target);

    auto size = _pTarget->getTexture()->getContentSizeInPixels();
    getGLProgramState()->setUniformVec2("resolution", size);
    getGLProgramState()->setUniformInt("iRange", _range);
    getGLProgramState()->setUniformVec4("glowColor", Vec4(_glowColor.r, _glowColor.g, _glowColor.b, _glowColor.a));
}

//========================= Effect Glow=========

const char* glow_frag = STRINGIFY(
\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

vec4 outColor = vec4(1.0, 0.0, 0.0, 1.0);

uniform float minRange;
uniform float maxRange;

void main(void)
{
    //current scale
    //float offset = sin(CC_Time[3]) * (maxRange - minRange) * 0.5 + (maxRange - minRange);
    float offset = 0.1f;

	vec4 normal = texture2D(CC_Texture0, v_texCoord) * v_fragmentColor;
	float fTmpX = v_texCoord.x - (v_texCoord.x - 0.5f) * offset;
	float fTmpY = v_texCoord.y - (v_texCoord.y - 0.5f) * offset;
	vec4 col = texture2D(CC_Texture0, vec2(fTmpX, fTmpY)) * v_fragmentColor;

    float dis = 1.0f - abs(v_texCoord.x - 0.5) * 2.0;
    dis = dis * (1.0f - abs(v_texCoord.y - 0.5) * 2.0);

	if (col.a > 0)
	{
		normal.rgb = normal.rgb + (1.0 - normal.a) * outColor.a * outColor.rgb;
	}

    if (normal.a > 0)
    {
        dis = 1.0f;
    }
	
    
	//col.rgb = col.rgb + (1.0 - col.a) * normal.a * normal.rgb;
	gl_FragColor.rgb = normal.rgb;
	gl_FragColor.a = col.a + normal.a;
    //gl_FragColor *= dis;
}
);
bool GlowEffect::init()
{
	initGLProgramState(glow_frag);

    /* getGLProgramState()->setUniformFloat("minRange", 0.5f);
     getGLProgramState()->setUniformFloat("maxRange", 0.23f);*/

	return true;
}


//========================= Effect OutGlow2=========

const char* outglow2_frag = STRINGIFY(
\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

vec4 outColor = vec4(1.0, 0.5, 0.0, 1.0);
float offsetx = 0.95f;
float offsety = 0.7f;

void main(void)
{
    float fTime = sin(CC_Time[3])*0.1;

    offsetx += fTime;
    offsety += fTime;

    vec4 normal =  texture2D(CC_Texture0, v_texCoord) * v_fragmentColor; 

    vec4 col = outColor;
    float x = (v_texCoord.x - 0.5) / offsetx;
    float y = (v_texCoord.y - 0.5) / offsety;

    float dissq = x*x + y*y;
    float alpha = 1.0- dissq * 4;
    col.a = alpha;

    float out_alpha = max(normal.a, alpha);
    normal.rgb = normal.rgb + (1.0 - normal.a) * col.a * col.rgb;

    gl_FragColor = normal * out_alpha;
}
);
bool OutGlow2Effect::init()
{
    initGLProgramState(outglow2_frag);
	return true;
}

//===================Outer Glow Tex =========================

const char* outerglowtex_frag = STRINGIFY(
\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform int iRange;
uniform vec4 glowColor;

void main(void)
{
    float fTime = sin(CC_Time[2]);
    int iMinRange = 10;

    float fCurRange = (float(iRange) - float(iMinRange))*fTime*0.5 + 15.0f;
    vec4 col = texture2D(CC_Texture0, v_texCoord) * v_fragmentColor;
    gl_FragColor.rgb = glowColor.rgb;

    float out_alpha = smoothstep(0.0f,20.0f,fCurRange);
    gl_FragColor.a = out_alpha*col.a;
}
);

bool OuterGlowTex::init()
{
    initGLProgramState(outerglowtex_frag);

    _range = 20;
    _glowColor = Color4F::ORANGE;

    return true;
}

void OuterGlowTex::setTarget(cocos2d::Sprite* target)
{
    CustomEffectBase::setTarget(target);

    getGLProgramState()->setUniformInt("iRange", _range);
    getGLProgramState()->setUniformVec4("glowColor", Vec4(_glowColor.r, _glowColor.g, _glowColor.b, _glowColor.a));
}
