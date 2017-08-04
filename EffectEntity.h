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

//��Ҫע������⣺
//1.glProgramState����������
//2.android�����´�backGround�л��������Ƿ�������
//3.�����Ҫ����Texture.�Ƿ�Ҫ֧���첽

class EffectEntity : public Sprite , public IEffectSink
{
public:
    virtual bool initWithFile(const std::string& filename) override;
    virtual bool initWithTexture(Texture2D *texture) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect, bool rotated) override;
	virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
    virtual void setUniformInfo(){} //����uniformֵ
protected:
	EffectCommond _effectCommond;
    static std::string _effectName;
    static std::string _fragShader;
};

class EffectTextureEntity : public EffectEntity
{
public:
    virtual bool initWithFile(const std::string& filename) override;
    virtual bool initWithTexture(Texture2D *texture) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect, bool rotated) override;
    virtual bool pretrentTexture(const std::string& filename); //Ԥ����Texture
    virtual void setUniformInfo(){} //����uniformֵ
protected:
    EffectTextureEntity();
    Texture2D* _effectTexture;
};

class GrayEntity : public EffectEntity
{
public:
    static GrayEntity* create(const std::string& filename);
};

class OutGlowEntity : public EffectTextureEntity
{
public:
    static OutGlowEntity* create(const std::string& filename,Color4F glowColor,int rangeMin,int rangeMax);
    virtual bool init(const std::string& filename, Color4F glowColor, int rangeMin, int rangeMax);
    virtual bool pretrentTexture(const std::string& filename);
    virtual void setUniformInfo() override;
protected:
    Color4F _glowColor;
    int _rangeMin;
    int _rangeMax;
};