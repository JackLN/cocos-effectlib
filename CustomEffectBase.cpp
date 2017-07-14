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
    //float outter = 0.0f;

    float range = iRange;
    range = abs(sin(CC_Time[3])*3.0) + range;

    float inner = 0.0f;
    int count = 0;
    vec2 unit = 1.0 / resolution.xy;

    for (float k = -range; k < range; k+=1.0f)
    {
        for (float j = -range; j < range; j+=1.0f)
        {
            vec4 c = texture2D(CC_Texture0, v_texCoord + vec2(unit.x * k , unit.y * j));
            //outter += (1.0 - c.a);
            inner += c.a;
			count += 1;
        }
    }

    inner /= count;
    //outter /= count;
    
    vec4 col = texture2D(CC_Texture0, v_texCoord) * v_fragmentColor;
    vec4 tmp = col;

	float out_alpha = inner;

	col.rgb = col.rgb + (1.0 - col.a) * glowColor.a * glowColor.rgb;
	col.a = out_alpha;
	//col.a = 0.0f;
    if (out_alpha < 0.0001f)
    {
        gl_FragColor = col * tmp;
        return;
    }

	gl_FragColor = col;
    
	/*gl_FragColor.rgb = col.rgb;
	gl_FragColor.w = 0.0f;*/
}
);

bool OuterGlowEffect::init()
{
    initGLProgramState(outerglow_frag);

    _range = 5;
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