#pragma once
#include "cocos2d.h"

USING_NS_CC;

class EffectCommond : public TrianglesCommand
{
public:
	void init(float globalOrder, GLuint textureID, GLProgramState* glProgramState, BlendFunc blendType, const Triangles& triangles, const Mat4& mv, uint32_t flags);
protected:
    void generateMaterialID();
};

class EffectEntity : public Sprite
{
public:
	static EffectEntity* create(const std::string& filename);
    static EffectEntity* createWithTexture(Texture2D *texture);
public:
	virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
protected:
	EffectCommond _effectCommond;
};
