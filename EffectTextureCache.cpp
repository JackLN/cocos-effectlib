#include "EffectTextureCache.h"


EffectTextureCache* EffectTextureCache::s_instance = nullptr;

EffectTextureCache * EffectTextureCache::getInstance(void)
{
    if (s_instance == nullptr)
        s_instance = new EffectTextureCache();
    
    return s_instance;
}

void EffectTextureCache::createEffectTextureAsync(IEffectSink* pSink, std::string fileName)
{

}
