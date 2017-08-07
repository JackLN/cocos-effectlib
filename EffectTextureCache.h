#ifndef _EFFECT_TEX_CACHE_H
#define _EFFECT_TEX_CACHE_H

#include "cocos2d.h"
#include "EffectEntity.h"

class EffectTextureCache : public Ref
{
public:
    static EffectTextureCache *getInstance(void);
    void               pretrentTextureAsync(const std::string& name,IEffectSink* sink);
    void               pretrentTexture(const std::string& name, const std::string& effectname, IEffectSink* sink);
    void               addTextureWithName(const std::string& name,const std::string& effectname,Texture2D* texture);
    Texture2D*         getTextureWithName(const std::string& name,const std::string& effectname);
    GLProgramState*    getOrCreateProgramStateWithShader(std::string name,const GLchar* vShaderByteArray, const GLchar* fShaderByteArray);

    void               init();
    void               update(float dt);
private:
    static EffectTextureCache *s_instance;
    std::vector<std::string> _loadingTextures;
    std::deque<IEffectSink*> _sinks;
    std::unordered_map<std::string, Texture2D*> _textures;//ÒÑ¾­okµÄÍ¼
    std::mutex _mutex;
};


#endif