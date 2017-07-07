#pragma once
#include "cocos2d.h"

USING_NS_CC;

class BlurSprite : public Sprite
{
public:
	~BlurSprite();

	bool initWithTexture(Texture2D* texture, const Rect&  rect);
	void initGLProgram();

	static BlurSprite* create(const char *pszFileName);
	void setBlurRadius(float radius);
	void setBlurSampleNum(float num);

protected:
	float _blurRadius;
	float _blurSampleNum;

};