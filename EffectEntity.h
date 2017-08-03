#pragma once
#include "cocos2d.h"

USING_NS_CC;

struct IEffectSink;


struct IEffectSink
{
    virtual void OnTextureSuccess(Texture2D* texture) {};
};

class EffectCommond : public TrianglesCommand
{
public:
	void init(float globalOrder, GLuint textureID, GLProgramState* glProgramState, BlendFunc blendType, const Triangles& triangles, const Mat4& mv, uint32_t flags);
protected:
    void generateMaterialID();
};

//需要注意的问题：
//1.glProgramState的生存周期
//2.android环境下从backGround切换回来后是否有问题
//3.如果需要处理Texture.是否要支持异步

class EffectEntity : public Sprite , public IEffectSink
{
public:
    virtual bool initWithFile(const std::string& filename) override;
    virtual bool initWithTexture(Texture2D *texture) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect, bool rotated) override;
	virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
protected:
	EffectCommond _effectCommond;
    static std::string _effectName;
    static std::string _fragShader;
};

class EffectTextureEntity : public EffectEntity
{
public:
    virtual bool init(const std::string& filename);
    virtual bool initWithTexture(Texture2D* texture);
protected:
    EffectTextureEntity();
    Texture2D* _effectTexture;
};

class GrayEntity : public EffectEntity
{
public:
    static GrayEntity* create(const std::string& filename);
};
