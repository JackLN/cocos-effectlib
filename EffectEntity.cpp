#include "EffectEntity.h"
#include "../external/xxhash/xxhash.h"

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

EffectEntity* EffectEntity::create(const std::string& filename)
{
	EffectEntity *pRe = new (std::nothrow) EffectEntity();
	if (pRe && pRe->initWithFile(filename))
	{
		pRe->autorelease();
		return pRe;
	}
	CC_SAFE_DELETE(pRe);
	return nullptr;
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
