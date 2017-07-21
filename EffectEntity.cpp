#include "EffectEntity.h"
#include "../external/xxhash/xxhash.h"

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
    //}
}