#pragma once
#include "cocos2d.h"

USING_NS_CC;

class CustomEffectBase;
struct _hashEffect;


class EffectManager
{
public:
    void addEffect(CustomEffectBase* effect, Sprite* target); //add one effect
    void removeAllEffect(); //remove all effect
public:
    static EffectManager* getInstance();
    ~EffectManager();
private:
    EffectManager();
    void init();
    void AllocEffectArray(struct _hashEffect* element);
private:
    struct _hashEffect    *_effects;//hashTable
};

