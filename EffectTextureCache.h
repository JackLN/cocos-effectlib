#ifndef _EFFECT_TEX_CACHE_H
#define _EFFECT_TEX_CACHE_H

#include "cocos2d.h"
#include "EffectEntity.h"

#define GENERATE_TEX_NAME(_FILENAME_,_EFFECTNAME_) (_FILENAME_+_EFFECTNAME_)

class EffectTextureCache : public Ref
{
public:
    static EffectTextureCache *getInstance(void);

    void               addTextureWithName(const std::string& name,const std::string& effectname,Texture2D* texture);
    void               addTextureWithName(const std::string& name,Texture2D* texture);
    Texture2D*         getTextureWithName(const std::string& name,const std::string& effectname);
    Texture2D*         getTextureWithName(const std::string& name);
    GLProgramState*    getOrCreateProgramStateWithShader(std::string name,const GLchar* vShaderByteArray, const GLchar* fShaderByteArray);

    void               pretrentTextureAsync(IEffectSink* sink);
    void               pretrentTexture(const std::string& name, const std::string& effectname, IEffectSink* sink);
    void               onPretrentSuccess(IEffectSink* sink);

    void               init();
    void               update(float dt);
private:
    static EffectTextureCache *s_instance;
    std::map<std::string,std::vector<IEffectSink*>> _waitingTextures;
    std::deque<IEffectSink*> _sinks;
    std::unordered_map<std::string, Texture2D*> _textures;//ÒÑ¾­okµÄÍ¼
    std::mutex _mutex;
};


#endif