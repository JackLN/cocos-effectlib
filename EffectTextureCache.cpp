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
    auto it = _textures.find(fileName);
    if (it != _textures.end())
    {
        pSink->OnTextureSuccess(it->second);
        return;
    }
    


}

 GLProgramState* EffectTextureCache::getOrCreateProgramStateWithShader(std::string name, const GLchar* vShaderByteArray, const GLchar* fShaderByteArray)
 {
     auto glprogramcache = GLProgramCache::getInstance();
     const std::string key = name;
     auto glprogram = glprogramcache->getGLProgram(key);

     if (!glprogram) {
         glprogram = GLProgram::createWithByteArrays(vShaderByteArray, fShaderByteArray);
         glprogramcache->addGLProgram(glprogram, key);
     }

     return GLProgramState::getOrCreateWithGLProgram(glprogram);
 }
