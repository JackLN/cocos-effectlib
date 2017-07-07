#include "BlurSprite.h"


#define STRINGIFY(A)  #A

const char* blurSprite_frag = STRINGIFY(
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
        vec4 col = vec4(0);
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

BlurSprite::~BlurSprite()
{
}

bool BlurSprite::initWithTexture(Texture2D* texture, const Rect& rect)
{
	_blurRadius = 0;
	if (Sprite::initWithTexture(texture, rect))
	{
		initGLProgram();

		return true;
	}

	return false;
}

void BlurSprite::initGLProgram()
{
	auto program = GLProgram::createWithByteArrays(ccPositionTextureColor_noMVP_vert, blurSprite_frag);
	auto glProgramState = GLProgramState::getOrCreateWithGLProgram(program);
	setGLProgramState(glProgramState);

	auto size = getTexture()->getContentSizeInPixels();
	getGLProgramState()->setUniformVec2("resolution", size);
	getGLProgramState()->setUniformFloat("blurRadius", _blurRadius);
	getGLProgramState()->setUniformFloat("sampleNum", 7.0f);
}

BlurSprite* BlurSprite::create(const char *pszFileName)
{
	BlurSprite* pRet = new (std::nothrow) BlurSprite();
	if (pRet)
	{
		bool result = pRet->initWithFile("");
		CCLOG("Test call Sprite::initWithFile with bad file name result is : %s", result ? "true" : "false");
	}

	if (pRet && pRet->initWithFile(pszFileName))
	{
		pRet->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(pRet);
	}

	return pRet;
}

void BlurSprite::setBlurRadius(float radius)
{
	_blurRadius = radius;
	getGLProgramState()->setUniformFloat("blurRadius", _blurRadius);
}

void BlurSprite::setBlurSampleNum(float num)
{
	_blurSampleNum = num;
	getGLProgramState()->setUniformFloat("sampleNum", _blurSampleNum);
}
