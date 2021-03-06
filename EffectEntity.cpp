#include "EffectEntity.h"
#include "../external/xxhash/xxhash.h"
#include "EffectTextureCache.h"

#define STRINGIFY(A)  #A

void EffectCommond::init(float globalOrder, GLuint textureID, GLProgramState* glProgramState, BlendFunc blendType, const Triangles& triangles, const Mat4& mv, uint32_t flags)
{
	CCASSERT(glProgramState, "Invalid GLProgramState");
	CCASSERT(glProgramState->getVertexAttribsFlags() == 0, "No custom attributes are supported in QuadCommand");

	RenderCommand::init(globalOrder, mv, flags);

	_triangles = triangles;
	if (_triangles.indexCount % 3 != 0)
	{
		ssize_t count = _triangles.indexCount;
		_triangles.indexCount = count / 3 * 3;
		CCLOGERROR("Resize indexCount from %zd to %zd, size must be multiple times of 3", count, _triangles.indexCount);
	}
	_mv = mv;

	if (_textureID != textureID || _blendType.src != blendType.src || _blendType.dst != blendType.dst || _glProgramState != glProgramState) {

		_textureID = textureID;
		_blendType = blendType;
		_glProgramState = glProgramState;

		generateMaterialID();
	}
}

void EffectCommond::generateMaterialID()
{

    //TODO: 找到合理的处理批渲染的方式，暂时只屏蔽了Uniform这个东西
    /*if (_glProgramState->getUniformCount() > 0)
    {
		_materialID = Renderer::MATERIAL_ID_DO_NOT_BATCH;
    }
    else
    {*/

		int glProgram = (int)_glProgramState->getGLProgram()->getProgram();
		int intArray[4] = { glProgram, (int)_textureID, (int)_blendType.src, (int)_blendType.dst };

		_materialID = XXH32((const void*)intArray, sizeof(intArray), 0);
		//_materialID = 3;
    //}
}

const char* EFFECT_NAME_OUTGLOW = "OutGlowEntity";
const char* EFFECT_NAME_GRAY = "GrayEntity";

void EffectEntity::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
#if CC_USE_CULLING
	// Don't do calculate the culling if the transform was not updated
	auto visitingCamera = Camera::getVisitingCamera();
	auto defaultCamera = Camera::getDefaultCamera();
	if (visitingCamera == defaultCamera) {
		_insideBounds = ((flags & FLAGS_TRANSFORM_DIRTY) || visitingCamera->isViewProjectionUpdated()) ? renderer->checkVisibility(transform, _contentSize) : _insideBounds;
	}
	else
	{
		_insideBounds = renderer->checkVisibility(transform, _contentSize);
	}

	if (_insideBounds)
#endif
	{
		_effectCommond.init(_globalZOrder, _texture->getName(), getGLProgramState(), _blendFunc, _polyInfo.triangles, transform, flags);
		renderer->addCommand(&_effectCommond);

#if CC_SPRITE_DEBUG_DRAW
		_debugDrawNode->clear();
		auto count = _polyInfo.triangles.indexCount / 3;
		auto indices = _polyInfo.triangles.indices;
		auto verts = _polyInfo.triangles.verts;
		for (ssize_t i = 0; i < count; i++)
		{
			//draw 3 lines
			Vec3 from = verts[indices[i * 3]].vertices;
			Vec3 to = verts[indices[i * 3 + 1]].vertices;
			_debugDrawNode->drawLine(Vec2(from.x, from.y), Vec2(to.x, to.y), Color4F::WHITE);

			from = verts[indices[i * 3 + 1]].vertices;
			to = verts[indices[i * 3 + 2]].vertices;
			_debugDrawNode->drawLine(Vec2(from.x, from.y), Vec2(to.x, to.y), Color4F::WHITE);

			from = verts[indices[i * 3 + 2]].vertices;
			to = verts[indices[i * 3]].vertices;
			_debugDrawNode->drawLine(Vec2(from.x, from.y), Vec2(to.x, to.y), Color4F::WHITE);
		}
#endif //CC_SPRITE_DEBUG_DRAW
	}
}

bool EffectEntity::initWithTexture(Texture2D *texture, const Rect& rect, bool rotated)
{
    bool result;
    if (Node::init())
    {
        _batchNode = nullptr;

        _recursiveDirty = false;
        setDirty(false);

        _opacityModifyRGB = true;

        _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;

        _flippedX = _flippedY = false;

        // default transform anchor: center
        setAnchorPoint(Vec2(0.5f, 0.5f));

        // zwoptex default values
        _offsetPosition.setZero();

        // clean the Quad
        memset(&_quad, 0, sizeof(_quad));

        // Atlas: Color
        _quad.bl.colors = Color4B::WHITE;
        _quad.br.colors = Color4B::WHITE;
        _quad.tl.colors = Color4B::WHITE;
        _quad.tr.colors = Color4B::WHITE;

        // shader state

        //auto glProgram = GLProgram::getAttribLocation()
        setGLProgramState(EffectTextureCache::getInstance()->getOrCreateProgramStateWithShader(_effectName,ccPositionTextureColor_noMVP_vert, _fragShader.c_str()));
        setUniformInfo();
        // update texture (calls updateBlendFunc)
        setTexture(texture);
        setTextureRect(rect, rotated, rect.size);

        // by default use "Self Render".
        // if the sprite is added to a batchnode, then it will automatically switch to "batchnode Render"
        setBatchNode(nullptr);
        result = true;
    }
    else
    {
        result = false;
    }
    _recursiveDirty = true;
    setDirty(true);
    return result;
}

bool EffectEntity::initWithTexture(Texture2D *texture)
{
    CCASSERT(texture != nullptr, "Invalid texture for sprite");

    Rect rect = Rect::ZERO;
    rect.size = texture->getContentSize();

    return initWithTexture(texture, rect);
}

bool EffectEntity::initWithTexture(Texture2D *texture, const Rect& rect)
{
    return initWithTexture(texture, rect, false);
}

bool EffectEntity::initWithFile(const std::string& filename)
{
    if (filename.empty())
    {
        CCLOG("Call Sprite::initWithFile with blank resource filename.");
        return false;
    }

    _fileName = filename;
    _fileType = 0;

    Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(filename);
    if (texture)
    {
        Rect rect = Rect::ZERO;
        rect.size = texture->getContentSize();
        return initWithTexture(texture, rect);
    }

    // don't release here.
    // when load texture failed, it's better to get a "transparent" sprite then a crashed program
    // this->release();
    return false;
}

bool EffectEntity::initWithFrameName(const std::string& framename)
{
    return false;
}



EffectTextureEntity::EffectTextureEntity()
: _addWidth(0)
, _addHeight(0)
{

}

bool EffectTextureEntity::initWithFile(const std::string& filename)
{
    if (filename.empty())
    {
        CCLOG("Call Sprite::initWithFile with blank resource filename.");
        return false;
    }

    _fileName = GENERATE_TEX_NAME(filename,_effectName);
    _fileType = 0;
    _originType = IEffectSink::ORIGIN_TYPE::FILE;
    _originName = filename;

    Texture2D *texture = EffectTextureCache::getInstance()->getTextureWithName(filename,_effectName);
    if (texture)
    {
        Rect rect = Rect::ZERO;
        rect.size = texture->getContentSize();
        return initWithTexture(texture, rect);
    }

    //return pretrentTexture(filename);
    EffectTextureCache::getInstance()->pretrentTexture(filename,_effectName,this);
    return initWithTexture(nullptr, Rect::ZERO );
}

bool EffectTextureEntity::initWithTexture(Texture2D* texture)
{
    CCASSERT(texture != nullptr, "Invalid texture for sprite");

    Rect rect = Rect::ZERO;
    rect.size = texture->getContentSize();

    return initWithTexture(texture, rect);
}

bool EffectTextureEntity::initWithTexture(Texture2D *texture, const Rect& rect)
{
    return initWithTexture(texture, rect, false);
}

bool EffectTextureEntity::initWithTexture(Texture2D *texture, const Rect& rect, bool rotated)
{
    bool result;
    if (Node::init())
    {
        _batchNode = nullptr;

        _recursiveDirty = false;
        setDirty(false);

        _opacityModifyRGB = true;

        _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;

        _flippedX = _flippedY = false;

        // default transform anchor: center
        setAnchorPoint(Vec2(0.5f, 0.5f));

        // zwoptex default values
        _offsetPosition.setZero();

        // clean the Quad
        memset(&_quad, 0, sizeof(_quad));

        // Atlas: Color
        _quad.bl.colors = Color4B::WHITE;
        _quad.br.colors = Color4B::WHITE;
        _quad.tl.colors = Color4B::WHITE;
        _quad.tr.colors = Color4B::WHITE;

        // shader state

        //auto glProgram = GLProgram::getAttribLocation()
        setGLProgramState(EffectTextureCache::getInstance()->getOrCreateProgramStateWithShader(_effectName, ccPositionTextureColor_noMVP_vert, _fragShader.c_str()));
        setUniformInfo();

        // update texture (calls updateBlendFunc)
        setTexture(texture);
        setTextureRect(rect, rotated, rect.size);

        // by default use "Self Render".
        // if the sprite is added to a batchnode, then it will automatically switch to "batchnode Render"
        setBatchNode(nullptr);
        result = true;
    }
    else
    {
        result = false;
    }
    _recursiveDirty = true;
    setDirty(true);
    return result;
}

void EffectTextureEntity::OnPretrentSuccess()
{
    Texture2D *texture = EffectTextureCache::getInstance()->getTextureWithName(_fileName);
    if (texture)
    {
        Rect rect = Rect::ZERO;
        rect.size = texture->getContentSize();
        initWithTexture(texture, rect);
    }
    else
    {
        int iWidth = _texData.iWidth;
        int iHeight = _texData.iHeight;

        Texture2D* tex = new Texture2D();
        tex->initWithData(_texData.pAddress, _texData.iDataLen, Texture2D::PixelFormat::RGBA8888, iWidth, iHeight, CCSize(iWidth, iHeight));
        EffectTextureCache::getInstance()->addTextureWithName(_fileName, tex);
        _texData.freeM();
        OnPretrentSuccess();
    }
}

bool EffectTextureEntity::initWithFrameName(const std::string& framename)
{
    if (framename.empty())
    {
        CCLOG("Call Sprite::initWithFile with blank resource filename.");
        return false;
    }

    _fileName = GENERATE_TEX_NAME(framename, _effectName);
    _fileType = 0;
    _originType = _originType = ORIGIN_TYPE::FRAME;;
    _originName = framename;

    Texture2D *texture = EffectTextureCache::getInstance()->getTextureWithName(framename, _effectName);
    if (texture)
    {
        Rect rect = Rect::ZERO;
        rect.size = texture->getContentSize();
        return initWithTexture(texture, rect);
    }

    //return pretrentTexture(filename);
    EffectTextureCache::getInstance()->pretrentTexture(framename, _effectName, this);
    return initWithTexture(nullptr, Rect::ZERO);
}

void EffectTextureEntity::OnPretrentWithFile(std::string filename)
{
    std::string fullpath = FileUtils::getInstance()->fullPathForFilename(filename);
    auto image = new Image();
    image->initWithImageFile(fullpath);

    unsigned char* pImgData = image->getData();
    _texData.allocM(image->getWidth(), image->getHeight(), image->getDataLen());
    _texData.copyM(image->getData(),image->getDataLen());

    CC_SAFE_DELETE(image);

    OnPretrent();
}

void EffectTextureEntity::OnPretrentWithFrame(std::string framename)
{
    auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(framename);

    auto rect = frame->getRectInPixels();
    auto frameRect = frame->getRect();
    auto rotate = frame->isRotated();
    auto offsetPix = rect.origin;

    std::string fullpath = FileUtils::getInstance()->fullPathForFilename(frame->getTexture()->getPath());
    auto image = new Image();
    image->initWithImageFile(fullpath);

    unsigned char* pImgData = image->getData();
    auto iDataLen = image->getDataLen();
    int iWidth = rect.size.width;
    int iHeight = rect.size.height;
    int iOriginWidth = image->getWidth();
    int iOriginHeight = image->getHeight();
    bool bRotate = frame->isRotated();

    iWidth += _addWidth;
    iHeight += _addHeight;
    _texData.allocM(iWidth, iHeight, iWidth*iHeight*4);

    auto dataAddr = _texData.getData();

    int iAddHeight = _addHeight / 2;
    int iAddWidth = _addWidth / 2;

    int i;
    int j;
    if (bRotate)
    {
        for (i = iAddHeight; i < iHeight - iAddHeight; ++i)
        {
            for (j = iAddWidth; j < iWidth - iAddWidth; ++j)
            {
                int offset = (iWidth * i + j) * 4;
                int originOffset = (iOriginWidth * (j - iAddWidth + (int)offsetPix.y) - i - iAddHeight + iHeight + (int)offsetPix.x) * 4;
                *(dataAddr + offset + 0) = *(pImgData + originOffset + 0);
                *(dataAddr + offset + 1) = *(pImgData + originOffset + 1);
                *(dataAddr + offset + 2) = *(pImgData + originOffset + 2);
                *(dataAddr + offset + 3) = *(pImgData + originOffset + 3);
            }
        }
    }
    else
    {
        
        for (i = iAddHeight; i < iHeight - iAddHeight; ++i)
        {
            for (j = iAddWidth; j < iWidth - iAddWidth; ++j)
            {
                int offset = (iWidth * i + j) * 4;
                int originOffset = (iOriginWidth * (i - iAddHeight + (int)offsetPix.y) + j - iAddWidth + (int)offsetPix.x) * 4;
                *(dataAddr + offset + 0) = *(pImgData + originOffset + 0);
                *(dataAddr + offset + 1) = *(pImgData + originOffset + 1);
                *(dataAddr + offset + 2) = *(pImgData + originOffset + 2);
                *(dataAddr + offset + 3) = *(pImgData + originOffset + 3);
            }
        }
    }
    //get target image data buffer
    
    CC_SAFE_DELETE(image);
    OnPretrent();
}

EffectTextureEntity::~EffectTextureEntity()
{
    _texData.freeM();
}

const char* effect_gray_frag = STRINGIFY(
\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

void main(void)
{
    vec4 c = texture2D(CC_Texture0, v_texCoord);
    gl_FragColor.xyz = vec3(0.2126*c.r + 0.7152*c.g + 0.0722*c.b);
    gl_FragColor.w = c.w;
}
);

GrayEntity* GrayEntity::create(const std::string& filename)
{
    GrayEntity *entity = new (std::nothrow) GrayEntity();
    if (entity && entity->initWithFile(filename))
    {
        entity->autorelease();
        return entity;
    }
    CC_SAFE_DELETE(entity);
    return nullptr;
}

GrayEntity::GrayEntity()
{
    _effectName = EFFECT_NAME_GRAY;
    _fragShader = effect_gray_frag;
}

const char* effect_outglow_frag = STRINGIFY(
\n#ifdef GL_ES\n
precision mediump float;
\n#endif\n

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform int iRange;
uniform int iMinRange;
uniform vec4 glowColor;

void main(void)
{
    float fTime = sin(CC_Time[2]);
    //int iMinRange = 10;

    float fCurRange = (float(iRange) - float(iMinRange))*fTime*0.5 + 15.0f;
    vec4 col = texture2D(CC_Texture0, v_texCoord) * v_fragmentColor;
    gl_FragColor.rgb = glowColor.rgb;

    float out_alpha = smoothstep(0.0f, 20.0f, fCurRange);
    gl_FragColor.a = out_alpha*col.a;
}
);

OutGlowEntity* OutGlowEntity::create(const std::string& filename, Color4F glowColor, int rangeMin, int rangeMax)
{
    OutGlowEntity* entity = new (std::nothrow)OutGlowEntity();
    if (entity && entity->init(filename,glowColor,rangeMin,rangeMax))
    {
        entity->autorelease();
        return entity;
    }
    CC_SAFE_DELETE(entity);
    return nullptr;
}


bool OutGlowEntity::init(const std::string& filename, Color4F glowColor, int rangeMin, int rangeMax)
{
    _glowColor = glowColor;
    _rangeMax = rangeMax;
    _rangeMin = rangeMin;

    return initWithFile(filename);
}

bool OutGlowEntity::initWithFrameName(const std::string& filename, Color4F glowColor, int rangeMin, int rangeMax)
{
    _glowColor = glowColor;
    _rangeMax = rangeMax;
    _rangeMin = rangeMin;

    _addWidth = rangeMax*2;
    _addHeight = rangeMax*2;
    
    return EffectTextureEntity::initWithFrameName(filename);
}

void OutGlowEntity::setUniformInfo()
{
    getGLProgramState()->setUniformInt("iRange", _rangeMax);
    getGLProgramState()->setUniformInt("iMinRange", _rangeMin);
    getGLProgramState()->setUniformVec4("glowColor", Vec4(_glowColor.r, _glowColor.g, _glowColor.b, _glowColor.a));
}

OutGlowEntity::OutGlowEntity()
{
    _effectName = EFFECT_NAME_OUTGLOW;
    _fragShader = effect_outglow_frag;
}

void OutGlowEntity::OnPretrent()
{
    unsigned char* pImgData = _texData.getData();
    int iWidth = _texData.iWidth;
    int iHeight = _texData.iHeight;

    //target data
    unsigned char* pTarData = (unsigned char*)(malloc(_texData.getDataLen() * sizeof(unsigned char)));
    memset(pTarData,0,_texData.getDataLen() * sizeof(unsigned char));

    //change image buffer data
    int i;
    int j;
    int iMaxOffset = _rangeMax;
    unsigned char* pDataStart = _texData.getData();
    for (i = 0; i < iHeight ; ++i)
    {
        for (j = 0; j < iWidth; ++j)
        {
            //不透明的不需要处理
            int offset = (iWidth* i + j) * 4;
            GLubyte byteValue = *(pImgData += 3);
            pImgData++;
            if (byteValue == 255)
            {
                *(pTarData + offset+ 3) = 0;
                continue;
            }
                
            int iOriginA = *(pImgData - 1);
            int iStartX = std::max(0,j - iMaxOffset);
            int iEndX = std::min(j + iMaxOffset, iWidth);
            int iStartY = std::max(0, i - iMaxOffset);
            int iEndY = std::min(i + iMaxOffset, iHeight);

            int iCnt = 0;

            //在周围的像素点取alpha均值
            for (int m = iStartY; m < iEndY; ++m)
            {
                for (int n = iStartX; n < iEndX; ++n)
                {
					int iAlphaTmp = *(pDataStart + (m*iWidth + n) * 4 + 3);
                    if (iAlphaTmp > 200) //不透明计数
                        iCnt++;
                }
            }

            int iAlpha = std::max(iCnt * 255 / ((iEndY - iStartY) * (iEndX - iStartX)),iOriginA);

            //test
            //*(pTarData + offset + 1) = 255;
            //save alpha
            *(pTarData + offset + 3) = iAlpha;
        }
    }

    //copy image data
    _texData.copyM(pTarData, _texData.getDataLen());
    CC_SAFE_FREE(pTarData);
}

OutGlowEntity* OutGlowEntity::createWithFrameName(const std::string& frameName, Color4F glowColor, int rangeMin, int rangeMax)
{
    OutGlowEntity* entity = new (std::nothrow)OutGlowEntity();
    if (entity && entity->initWithFrameName(frameName, glowColor, rangeMin, rangeMax))
    {
        entity->autorelease();
        return entity;
    }
    CC_SAFE_DELETE(entity);
    return nullptr;
}




