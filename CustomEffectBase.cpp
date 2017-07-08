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
//------------------ Effect Blur End----------------------