#ifndef _EFFECT_TEX_CACHE_H
#define _EFFECT_TEX_CACHE_H

#include "cocos2d.h"
#include "EffectEntity.h"

class EffectTextureCache
{
public:
    static EffectTextureCache *getInstance(void);
    void     createEffectTextureAsync(IEffectSink* pSink,std::string fileName);//异步处理图片资源
    //void     createEffectTexture(IEffectSink* pSink,std::string fileName);//处理图片资源
private:
    static EffectTextureCache *s_instance;
    std::unordered_map<std::string, Texture2D*> _textures;//已经ok的图
    std::vector<IEffectSink*> _sinks; //正在处理的队列
};


#endif