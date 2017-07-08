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
	void setBlurRadius(float radius) { _blurRadius = radius; }
	void setBlurSampleNum(float num) { _blurSampleNum = num; }

	virtual void setTarget(cocos2d::Sprite* target);

protected:
	bool init(float blurRadius = 10.0f, float sampleNum = 5.0f);

	float _blurRadius;
	float _blurSampleNum;
};


