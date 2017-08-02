#pragma once
#include "cocos2d.h"

USING_NS_CC;

class CustomEffectBase : public cocos2d::Ref
{
public:

	cocos2d::GLProgramState* getGLProgramState() const { return _GLState; }
	virtual void setTarget(cocos2d::Sprite* target);

protected:
	CustomEffectBase();
	virtual ~CustomEffectBase();

	bool initGLProgramState(const std::string &fragSource);

	cocos2d::GLProgramState* _GLState; //cocos opengl state
	cocos2d::Sprite* _pTarget;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	std::string _fragSource;
	cocos2d::EventListenerCustom* _backgroundListener;
#endif
};


class BlurEffect: public CustomEffectBase
{
public:
	CREATE_FUNC(BlurEffect);
	void setBlurRadius(float radius) {
		_blurRadius = radius; 
		getGLProgramState()->setUniformFloat("blurRadius", _blurRadius);		
	}
	void setBlurSampleNum(float num) { 
		_blurSampleNum = num; 
		getGLProgramState()->setUniformFloat("sampleNum", _blurSampleNum);
	}

	virtual void setTarget(cocos2d::Sprite* target);

protected:
	bool init(float blurRadius = 10.0f, float sampleNum = 5.0f);

	float _blurRadius;
	float _blurSampleNum;
};

class GrayEffect : public CustomEffectBase
{
public:
	CREATE_FUNC(GrayEffect);
protected:
	bool init();
};

class ColorOffsetEffect : public CustomEffectBase
{
public:
	CREATE_FUNC(ColorOffsetEffect);
protected:
	bool init();
};

class OuterGlowEffect : public CustomEffectBase
{
public:
    CREATE_FUNC(OuterGlowEffect);
    virtual void setTarget(cocos2d::Sprite* target);

    void setRange(int range){
        _range = range;
        getGLProgramState()->setUniformInt("iRange", _range);
    }
    void setGlowColor(Color4F glowColor){
        _glowColor = glowColor;
        getGLProgramState()->setUniformVec4("glowColor", Vec4(_glowColor.r, _glowColor.g, _glowColor.b, _glowColor.a));
    }

protected:
    bool init();

    int _range;
    Color4F _glowColor;
};

class OutGlow2Effect : public CustomEffectBase
{
public:
    CREATE_FUNC(OutGlow2Effect);
    bool init();
};



class GlowEffect : public CustomEffectBase
{
public:
	CREATE_FUNC(GlowEffect);
	bool init();
};

class OuterGlowTex : public CustomEffectBase
{
public:
    CREATE_FUNC(OuterGlowTex);
    virtual void setTarget(cocos2d::Sprite* target);

    void setRange(int range){
        _range = range;
        getGLProgramState()->setUniformInt("iRange", _range);
    }
    void setGlowColor(Color4F glowColor){
        _glowColor = glowColor;
        getGLProgramState()->setUniformVec4("glowColor", Vec4(_glowColor.r, _glowColor.g, _glowColor.b, _glowColor.a));
    }
protected:
    bool init();

    int _range;
    Color4F _glowColor;
};
