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



EffectTextureEntity::EffectTextureEntity()
:_effectTexture(nullptr)
{

}

bool EffectTextureEntity::init(const std::string& filename)
{
    return false;
}

bool EffectTextureEntity::initWithTexture(Texture2D* texture)
{
    return false;
}

std::string GrayEntity::_effectName = "GrayEntity";
std::string GrayEntity::_fragShader = STRINGIFY(
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


