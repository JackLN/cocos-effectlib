#pragma once
#include "cocos2d.h"

USING_NS_CC;

struct TexData
{
    void allocM(int width,int height,int len)
    {
        pAddress = (unsigned char*)(malloc(len * sizeof(unsigned char)));
        memset(pAddress,0,len * sizeof(unsigned char));
        iDataLen = len;
        iWidth = width;
        iHeight = height;
    }
    void freeM()
    {
        CC_SAFE_FREE(pAddress);
    }
    void copyM(unsigned char* src,int len)
    {
        memcpy(pAddress, src, len);
    }

    unsigned char*          getData()          { return pAddress; }
    int                     getWidth()         { return iWidth; }
    int                     getHeight()        { return iHeight; }
    int                     getDataLen()       { return iDataLen; }
    bool                    getIsRotate()      { return bRotate; }

    unsigned char*          pAddress           = nullptr;
    int                     iDataLen           = 0;
    int                     iWidth             = 0;
    int                     iHeight            = 0;
    bool                    bRotate            = false;
};

struct IEffectSink;
struct IEffectSink
{
    enum ORIGIN_TYPE
    {
        FILE,
        FRAME,

        INVALID,
    };
    ORIGIN_TYPE getOriginType() { return _originType; }
    ORIGIN_TYPE _originType;

    std::string getOriginName() { return _originName; }
    std::string _originName;

    virtual void OnPretrent() = 0;
    virtual void OnPretrentWithFile(std::string fileName) = 0;
    virtual void OnPretrentWithFrame(std::string frameName) = 0;
    virtual void OnPretrentSuccess() {};
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

class EffectEntity : public Sprite
{
public:
    virtual bool initWithFile(const std::string& filename) override;
    virtual bool initWithFrameName(const std::string& framename);
    virtual bool initWithTexture(Texture2D *texture) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect, bool rotated) override;
	virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
    virtual void setUniformInfo(){} //����uniformֵ
protected:
	EffectCommond _effectCommond;
    std::string _effectName;
    std::string _fragShader;
};

class EffectTextureEntity : public EffectEntity , public IEffectSink
{
public:
    virtual bool initWithFile(const std::string& filename) override;
    virtual bool initWithFrameName(const std::string& framename) override;
    virtual bool initWithTexture(Texture2D *texture) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect) override;
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect, bool rotated) override;
    virtual void setUniformInfo(){} //����uniformֵ
public:
    virtual void OnPretrentSuccess();
    virtual void OnPretrentWithFile(std::string fileName);
    virtual void OnPretrentWithFrame(std::string frameName);
protected:
    EffectTextureEntity();
    ~EffectTextureEntity();
    TexData _texData;
    int _addWidth;
    int _addHeight;
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
    virtual void OnPretrent();
    //virtual void OnPretrent(std::string filename);
    //virtual void OnPretrentSuccess(std::string filename);
protected:
    OutGlowEntity();

    Color4F _glowColor;
    int _rangeMin;
    int _rangeMax;
};