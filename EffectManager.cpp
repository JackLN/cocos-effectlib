#include "EffectManager.h"
#include "CustomEffectBase.h"

static EffectManager *s_SharedEffectMgr = nullptr;

typedef struct _hashEffect
{
    struct _ccArray     *targets;
    CustomEffectBase    *effect;
    UT_hash_handle      hh;
} tHashEffect;

EffectManager::EffectManager()
: _effects(nullptr)
{
}

EffectManager::~EffectManager()
{
}

EffectManager* EffectManager::getInstance()
{
    if (!s_SharedEffectMgr)
    {
        s_SharedEffectMgr = new (std::nothrow) EffectManager();
        s_SharedEffectMgr->init();
    }

    return s_SharedEffectMgr;
}

void EffectManager::init()
{

}

void EffectManager::addEffect(CustomEffectBase* effect, Sprite* target)
{
    CCASSERT(effect != nullptr, "effect can't be nullptr!");
    CCASSERT(target != nullptr, "target can't be nullptr!");

    tHashEffect *element = nullptr;
    Ref* tmp = effect;
    HASH_FIND_PTR(_effects, &tmp, element);
    if (!element)
    {
        element = (tHashEffect*)calloc(sizeof(*element), 1);
        effect->retain();
        HASH_ADD_PTR(_effects, effect, element);
    }

    //AllocEffectArray(element);
    //ccArrayAppendObject(element->targets, target);
}

void EffectManager::removeAllEffect()
{

}

void EffectManager::AllocEffectArray(tHashEffect* element)
{
    if (element->targets == nullptr)
    {
        //element->targets = ccArrayNew(4);
    }
    else if (element->targets->num == element->targets->max)
    {
        //ccArrayDoubleCapacity(element->targets);
    }
}


