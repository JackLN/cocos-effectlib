#ifndef _EFFECT_TEX_CACHE_H
#define _EFFECT_TEX_CACHE_H

#include "cocos2d.h"
#include "EffectEntity.h"


class EffectTextureCache
{
public:
    static EffectTextureCache *getInstance(void);
    void               createEffectTextureAsync(IEffectSink* pSink,std::string fileName);//�첽����ͼƬ��Դ
    GLProgramState*    getOrCreateProgramStateWithShader(std::string name,const GLchar* vShaderByteArray, const GLchar* fShaderByteArray);
private:
    static EffectTextureCache *s_instance;
    std::unordered_map<std::string, Texture2D*> _textures;//�Ѿ�ok��ͼ
};


#endif