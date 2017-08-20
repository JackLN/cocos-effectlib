#include "EffectTextureCache.h"

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

 Texture2D* EffectTextureCache::getTextureWithName(const std::string& name)
 {
     auto it = _textures.find(name);
     if (it != _textures.end())
         return it->second;

     return nullptr;
 }

 void EffectTextureCache::addTextureWithName(const std::string& name,const std::string& effectname,Texture2D* texture)
 {
     /*_textures.insert(std::make_pair(GENERATE_TEX_NAME(name,effectname), texture));
     auto it = _waitingTextures.find(GENERATE_TEX_NAME(name,effectname));
     if (it != _waitingTextures.end())
         _waitingTextures.erase(it);*/
     addTextureWithName(GENERATE_TEX_NAME(name, effectname), texture);
 }

 void EffectTextureCache::addTextureWithName(const std::string& name, Texture2D* texture)
 {
     _textures.insert(std::make_pair(name, texture));
     auto it = _waitingTextures.find(name);
     if (it != _waitingTextures.end())
     {
         auto iter = it->second.begin();
         for (; iter != it->second.end(); ++iter)
             (*iter)->OnPretrentSuccess();
         _waitingTextures.erase(it);
     }
         
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
     auto it = _waitingTextures.find(GENERATE_TEX_NAME(name, effectname));

     if (it != _waitingTextures.end())
     {
         it->second.push_back(sink);
     }
     else
     {
         std::vector<IEffectSink*> vec;
         _waitingTextures.insert(std::make_pair(GENERATE_TEX_NAME(name, effectname), vec));
         auto t = std::thread(&EffectTextureCache::pretrentTextureAsync, this, name, sink);
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

 void EffectTextureCache::onPretrentSuccess(IEffectSink* sink)
 {
     sink->OnPretrentSuccess();
 }


 bool BenchmarkEffect::init()
 {
	 if (!Scene::init())
		 return false;


	 _layerDis = LayerColor::create(Color4B::BLACK);
	 addChild(_layerDis);

	 auto winSize = Director::getInstance()->getWinSize();
	 auto originRect = Director::getInstance()->getVisibleOrigin();
	 auto visibleSize = Director::getInstance()->getVisibleSize();

	 SpriteFrameCache::getInstance()->addSpriteFramesWithFile("Boilogys1.plist");

	 Sprite* test = Sprite::createWithSpriteFrameName("Biology0_1.png");
	 test->setPosition(200, 200);
	 _layerDis->addChild(test);

	 auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName("Biology0_1.png");

	 auto rect = frame->getRectInPixels();
	 auto rotate = frame->isRotated();
	 auto offsetPix = rect.origin;

	 std::string fullpath = FileUtils::getInstance()->fullPathForFilename(frame->getTexture()->getPath());
	 auto image = new Image();
	 image->initWithImageFile(fullpath);

	 unsigned char* pImgData = image->getData();
	 auto iDataLen = image->getDataLen();
	 int iWidth = frame->getOriginalSizeInPixels().width;
	 int iHeight = frame->getOriginalSizeInPixels().height;

	 /*int iWidth = rect.size.width;
	 int iHeight = rect.size.height;*/
	 int iOriginWidth = image->getWidth();
	 int iOriginHeight = image->getHeight();



	 //target data
	 unsigned char* pTarData = (unsigned char*)(malloc(iWidth * iHeight * 4 * sizeof(unsigned char)));
	 memset(pTarData, 0, iWidth * iHeight * 4 * sizeof(unsigned char));
	 //change image buffer data
	 int i;
	 int j;
	 for (i = 0; i < iWidth; ++i)
	 {
		 for (j = 0; j < iHeight; ++j)
		 {
			 int offset = (iWidth* i + j) * 4;
			 //int originOffset = (iOriginWidth * (i) + j ) * 4;
			 int originOffset = (iOriginWidth * (i + (int)offsetPix.y) + j + (int)offsetPix.x) * 4;
			 *(pTarData + offset + 0) = *(pImgData + originOffset + 0);
			 *(pTarData + offset + 1) = *(pImgData + originOffset + 1);
			 *(pTarData + offset + 2) = *(pImgData + originOffset + 2);
			 *(pTarData + offset + 3) = *(pImgData + originOffset + 3);

			 //int originOffset = ((iWidth + origin.width) * i + j + origin.height) * 4;
			 /**(pTarData + offset + 0) = *(pImgData + originOffset + 0);
			 *(pTarData + offset + 1) = *(pImgData + originOffset + 1);
			 *(pTarData + offset + 2) = *(pImgData + originOffset + 2);
			 *(pTarData + offset + 3) = *(pImgData + originOffset + 3);*/
		 }
	 }

	 //copy image data
	 //memcpy(image->getData(), pTarData, image->getDataLen());
	 //CC_SAFE_FREE(pTarData);

	 //create texture & sprite
	 Texture2D* tex = new Texture2D();
	 tex->initWithData(pTarData, iWidth * iHeight * 4, Texture2D::PixelFormat::RGBA8888, iWidth, iHeight, CCSize(iWidth, iHeight));
	 //EffectTextureCache::getInstance()->addTextureWithName(filename, _effectName, tex);
	 CC_SAFE_DELETE(image);

	 Sprite* demo = Sprite::createWithTexture(tex);
	 demo->setPosition(200, 400);
	 //demo->setScale(0.8);
	 _layerDis->addChild(demo);


	 /*Rect origin = pFoo->getBoundingBox();
	 auto pRender = RenderTexture::create(origin.size.width, origin.size.height, kCCTexture2DPixelFormat_RGBA8888);
	 pRender->beginWithClear(0.0f, 0.0f, 0.0f, 0.0f);
	 auto tmpSp = Sprite::createWithSpriteFrame(pFoo->getSpriteFrame());
	 tmpSp->setPosition(tmpSp->getContentSize() / 2);
	 pGlow->setTarget(tmpSp);
	 tmpSp->visit();
	 pRender->end();
	 pRender->saveToFile("Outer.png", cocos2d::Image::Format::PNG);*/

	 /* Rect origin = pFoo->getBoundingBox();
	 auto pRender = RenderTexture::create(winSize.width, winSize.height, kCCTexture2DPixelFormat_RGBA8888);
	 pRender->beginWithClear(0.0f, 0.0f, 0.0f, 0.0f);
	 pFoo->visit();
	 pRender->end();
	 pRender->saveToFile("Outer.png", cocos2d::Image::Format::PNG);*/

	 return true;
 }
