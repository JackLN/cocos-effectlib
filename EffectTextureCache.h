#ifndef _EFFECT_TEX_CACHE_H
#define _EFFECT_TEX_CACHE_H

#include "cocos2d.h"
#include "EffectEntity.h"


class EffectTextureCache
{
public:
    static EffectTextureCache *getInstance(void);
    void               addTextureWithName(const std::string& name,Texture2D* texture);
    Texture2D*         getTextureWithName(const std::string& name);
    GLProgramState*    getOrCreateProgramStateWithShader(std::string name,const GLchar* vShaderByteArray, const GLchar* fShaderByteArray);
private:
    static EffectTextureCache *s_instance;
    std::unordered_map<std::string, Texture2D*> _textures;//ÒÑ¾­okµÄÍ¼
};


#endif