#include "EffectTextureCache.h"

#define GENERATE_TEX_NAME(_FILENAME_,_EFFECTNAME_) (_FILENAME_+_EFFECTNAME_)


EffectTextureCache* EffectTextureCache::s_instance = nullptr;

EffectTextureCache * EffectTextureCache::getInstance(void)
{
    if (s_instance == nullptr)
    {
        s_instance = new EffectTextureCache();
        s_instance->init();
    }
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

 Texture2D* EffectTextureCache::getTextureWithName(const std::string& name,const std::string& effectname)
 {
     auto it = _textures.find(GENERATE_TEX_NAME(name,effectname));
     if (it != _textures.end())
         return it->second;

     return nullptr;
 }

 void EffectTextureCache::addTextureWithName(const std::string& name,const std::string& effectname,Texture2D* texture)
 {
     _textures.insert(std::make_pair(GENERATE_TEX_NAME(name,effectname), texture));
     auto it = std::find(_loadingTextures.begin(), _loadingTextures.end(), name);
     if (it != _loadingTextures.end())
         _loadingTextures.erase(it);
 }

 void EffectTextureCache::pretrentTextureAsync(const std::string& name, IEffectSink* sink)
 {
     sink->OnPretrent(name);
     _mutex.lock();
     _sinks.push_back(sink);
     _mutex.unlock();
 }

 void EffectTextureCache::pretrentTexture(const std::string& name,const std::string& effectname, IEffectSink* sink)
 {
     if (std::find(_loadingTextures.begin(), _loadingTextures.end(), GENERATE_TEX_NAME(name,effectname)) == _loadingTextures.end())
     {
         _loadingTextures.push_back(GENERATE_TEX_NAME(name,effectname));
         auto t = std::thread(&EffectTextureCache::pretrentTextureAsync,this ,name ,sink);
         t.detach();
     }
 }

 void EffectTextureCache::init()
 {
     Director::getInstance()->getScheduler()->schedule(CC_SCHEDULE_SELECTOR(EffectTextureCache::update), this, 0, false);
 }

 void EffectTextureCache::update(float dt)
 {
     while (!_sinks.empty())
     {
         _mutex.lock();
         auto sink = _sinks.front();
         _sinks.pop_front();
         _mutex.unlock();

         sink->OnPretrentSuccess();
     }
 }
