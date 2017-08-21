#pragma once
#include "cocos2d.h"

USING_NS_CC;

struct IEffectSink;


struct PretrentData
{
    PretrentData()
    {
        pAddress = nullptr;
        iLen = 0;
        iWidth = 0;
        iHeight = 0;
    }

    unsigned char* pAddress;
    int iLen;
    int iWidth;
    int iHeight;
    Size mSize;
};

struct IEffectSink
{
    virtual void OnPretrent(const PretrentData &data) = 0;
    virtual void OnPretrent(){};
    virtual void OnPretrentSuccess() {};
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

class EffectEntity : public Sprite
{
public:
    enum ORIGIN_TYPE
    {
        FILE,
        FRAME,

        INVALID,
    };
public:
    virtual bool initWithFile(const std::string& filename) override;
    virtual bool initWithFrameName(const std::string& framename);
    virtual bool initWithTexture(Texture2D *texture) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect, bool rotated) override;
	virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
    virtual void setUniformInfo(){} //设置uniform值
protected:
	EffectCommond _effectCommond;
    std::string _effectName;
    std::string _fragShader;
    ORIGIN_TYPE _originType;
};

class EffectTextureEntity : public EffectEntity , public IEffectSink
{
public:
    virtual bool initWithFile(const std::string& filename) override;
    virtual bool initWithFrameName(const std::string& framename) override;
    virtual bool initWithTexture(Texture2D *texture) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect, bool rotated) override;
    virtual void setUniformInfo(){} //设置uniform值
public:
    virtual void OnPretrent(const PretrentData &data) = 0;
    virtual void OnPretrent();
    virtual void OnPretrentSuccess();
    virtual void OnPretrent(std::string filename);
    virtual void OnPretrent(SpriteFrame* frame);
protected:
    EffectTextureEntity();
    ~EffectTextureEntity();
    Texture2D* _effectTexture;
    std::string _originName;
    PretrentData _pretrentData;
};

class GrayEntity : public EffectEntity
{
public:
    static GrayEntity* create(const std::string& filename);
protected:
    GrayEntity();
};

class OutGlowEntity : public EffectTextureEntity
{
public:
    static OutGlowEntity* create(const std::string& filename,Color4F glowColor,int rangeMin,int rangeMax);
    static OutGlowEntity* createWithFrameName(const std::string& frameName, Color4F glowColor, int rangeMin, int rangeMax);
    virtual bool init(const std::string& filename, Color4F glowColor, int rangeMin, int rangeMax);
    virtual bool initWithFrameName(const std::string& filename, Color4F glowColor, int rangeMin, int rangeMax);
    virtual void setUniformInfo() override;
public:
    virtual void OnPretrent(const PretrentData &data);
    //virtual void OnPretrent(std::string filename);
    //virtual void OnPretrentSuccess(std::string filename);
protected:
    OutGlowEntity();

    Color4F _glowColor;
    int _rangeMin;
    int _rangeMax;
};