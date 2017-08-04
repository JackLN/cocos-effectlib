#include "EffectTextureCache.h"


EffectTextureCache* EffectTextureCache::s_instance = nullptr;

EffectTextureCache * EffectTextureCache::getInstance(void)
{
    if (s_instance == nullptr)
        s_instance = new EffectTextureCache();
    
    return s_instance;
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

 Texture2D* EffectTextureCache::getTextureWithName(const std::string& name)
 {
     auto it = _textures.find(name);
     if (it != _textures.end())
         return it->second;

     return nullptr;
 }

 void EffectTextureCache::addTextureWithName(const std::string& name,Texture2D* texture)
 {
     _textures.insert(std::make_pair(name, texture));
 }
