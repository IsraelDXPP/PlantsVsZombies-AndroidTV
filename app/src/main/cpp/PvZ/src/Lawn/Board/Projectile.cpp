/*
 * Copyright (C) 2023-2025  PvZ TV Touch Team
 *
 * This file is part of PlantsVsZombies-AndroidTV.
 *
 * PlantsVsZombies-AndroidTV is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * PlantsVsZombies-AndroidTV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * PlantsVsZombies-AndroidTV.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "PvZ/Lawn/Board/Projectile.h"
#include "PvZ/GlobalVariable.h"
#include "PvZ/Lawn/Board/Board.h"
#include "PvZ/Lawn/Board/GridItem.h"
#include "PvZ/Lawn/Board/Plant.h"
#include "PvZ/Lawn/Board/Zombie.h"
#include "PvZ/Lawn/LawnApp.h"
#include "PvZ/Misc.h"
#include "PvZ/Symbols.h"
#include "PvZ/TodLib/Common/TodCommon.h"
#include "PvZ/TodLib/Effect/Attachment.h"
#include "PvZ/TodLib/Effect/Reanimator.h"

#include "Homura/Logger.h"

using namespace Sexy;

ProjectileDefinition gProjectileDefinition[] = {
    {ProjectileType::PROJECTILE_PEA, 0, 20},
    {ProjectileType::PROJECTILE_SNOWPEA, 0, 20},
    {ProjectileType::PROJECTILE_CABBAGE, 0, 40},
    {ProjectileType::PROJECTILE_MELON, 0, 80},
    {ProjectileType::PROJECTILE_PUFF, 0, 20},
    {ProjectileType::PROJECTILE_WINTERMELON, 0, 80},
    {ProjectileType::PROJECTILE_FIREBALL, 0, 40},
    {ProjectileType::PROJECTILE_STAR, 0, 20},
    {ProjectileType::PROJECTILE_SPIKE, 0, 20},
    {ProjectileType::PROJECTILE_BASKETBALL, 0, 75},
    {ProjectileType::PROJECTILE_KERNEL, 0, 20},
    {ProjectileType::PROJECTILE_COBBIG, 0, 300},
    {ProjectileType::PROJECTILE_BUTTER, 0, 40},
    {ProjectileType::PROJECTILE_ZOMBIE_PEA, 0, 20},
};

void Projectile::ProjectileInitialize(int theX, int theY, int theRenderOrder, int theRow, ProjectileType theProjectileType) {
    // projectile->mNewProjectileLastX = theX;
    // projectile->mNewProjectileLastY = theY;
    if (!isOnlyTouchFireWood) {
        // 僵尸子弹与加农炮子弹NULL
        if (theProjectileType == ProjectileType::PROJECTILE_COBBIG || theProjectileType == ProjectileType::PROJECTILE_ZOMBIE_PEA) {
            old_Projectile_ProjectileInitialize(this, theX, theY, theRenderOrder, theRow, theProjectileType);
            return;
        }
        if (theProjectileType == ProjectileType::PROJECTILE_STAR && banStar) {
            old_Projectile_ProjectileInitialize(this, theX, theY, theRenderOrder, theRow, theProjectileType);
            return;
        }
        if (isOnlyPeaUseable && theProjectileType != ProjectileType::PROJECTILE_PEA) {
            old_Projectile_ProjectileInitialize(this, theX, theY, theRenderOrder, theRow, theProjectileType);
            return;
        }
        if (bulletSpinnerChosenNum != -1) {
            theProjectileType = (ProjectileType)bulletSpinnerChosenNum;
        }
        if (randomBullet) {
            theProjectileType = (ProjectileType)randomInt(1, 12);
        }
        if (theProjectileType == ProjectileType::PROJECTILE_COBBIG && banCobCannon) {
            theProjectileType = (ProjectileType)randomInt(1, 10); // 同时降低好友玉米黄油的概率!!
        }
    }

    old_Projectile_ProjectileInitialize(this, theX, theY, theRenderOrder, theRow, theProjectileType);
    mIsRainProjectile = false;
    mTargetZombieID = ZombieID::ZOMBIEID_NULL;

    if (homingProjectiles) {
        mMotionType = ProjectileMotion::MOTION_HOMING;
        mDamageRangeFlags |= 2; // DAMAGES_FLYING flag (bit 1)
    }
}

void Projectile::Update() {
    if (requestPause) {
        return;
    }

    if (mIsRainProjectile) {
        if (!projectileRain || mPosY > 600 || mPosX < -50 || mPosX > 850) {
            Die();
            return;
        }
    }

    // Lógica de proyectiles buscadores
    if (homingProjectiles && mBoard != nullptr && !mDead) {
        Zombie *aTargetZombie = nullptr;

        // Búsqueda ultra-segura del zombi por ID para evitar crashes en la lectura de memoria (DataArrayGet) con IDs basura
        if (mTargetZombieID != ZombieID::ZOMBIEID_NULL) {
            Zombie *aTestZombie = nullptr;
            while (mBoard->IterateZombies(aTestZombie)) {
                if (aTestZombie != nullptr && ((int)mBoard->mZombies.DataArrayGetID(aTestZombie)) == (int)mTargetZombieID) {
                    aTargetZombie = aTestZombie;
                    break;
                }
            }
        }

        if (aTargetZombie == nullptr || aTargetZombie->mDead || aTargetZombie->mBodyHealth <= 0 || !aTargetZombie->EffectedByDamage((unsigned int)mDamageRangeFlags)) {
            // Buscar un nuevo objetivo inteligente (Cattail style): El zombi más avanzado (menor mX) en cualquier carril
            float aMinX = 1000.0f;
            Zombie *aZombie = nullptr;
            aTargetZombie = nullptr;
            while (mBoard->IterateZombies(aZombie)) {
                if (aZombie != nullptr && !aZombie->mDead && aZombie->mBodyHealth > 0 && aZombie->EffectedByDamage((unsigned int)mDamageRangeFlags)) {
                    // Priorizamos al zombi más a la izquierda (más peligroso)
                    if (aZombie->mX < aMinX) {
                        aMinX = aZombie->mX;
                        aTargetZombie = aZombie;
                    }
                }
            }
            if (aTargetZombie != nullptr) {
                mTargetZombieID = (ZombieID)mBoard->mZombies.DataArrayGetID(aTargetZombie);
            } else {
                mTargetZombieID = ZombieID::ZOMBIEID_NULL;
            }
        }

        if (aTargetZombie != nullptr) {
            if (mProjectileType == ProjectileType::PROJECTILE_COBBIG) {
                // Cob Cannon: Actualizar constantemente la posición de impacto
                mCobTargetX = aTargetZombie->mX + 40;
                mCobTargetRow = aTargetZombie->mRow;
                // El misil sigue cayendo (MOTION_LOBBED), ajustamos levemente su X para que coincida con el objetivo
                mPosX = (mPosX * 0.95f) + (mCobTargetX * 0.05f);
            } else {
                // Girar progresivamente hacia el objetivo (suavidad y seguridad en variables de estado)
                float aTargetX = aTargetZombie->mX + 40;
                float aTargetY = aTargetZombie->mY + 40;
                float aDx = aTargetX - mPosX;
                float aDy = aTargetY - mPosY;
                float aDist = sqrt(aDx * aDx + aDy * aDy);

                if (aDist > 0.1f) {
                    float aCurrentSpeed = sqrt(mVelX * mVelX + mVelY * mVelY);
                    float aTargetSpeed = aCurrentSpeed;
                    if (aTargetSpeed < 3.33f) aTargetSpeed = 3.33f;
                    if (aTargetSpeed > 15.0f) aTargetSpeed = 15.0f;

                    // Ajustar velocidad gradualmente, sin perder momento súbito
                    mVelX = (mVelX * 0.9f) + (aDx / aDist * aTargetSpeed * 0.1f);
                    mVelY = (mVelY * 0.9f) + (aDy / aDist * aTargetSpeed * 0.1f);
                }
            }
        } else if (mProjectileType != ProjectileType::PROJECTILE_COBBIG) {
            // Sin objetivo: El proyectil sigue en su dirección actual (estilo Cattail)
            float aSpeed = sqrt(mVelX * mVelX + mVelY * mVelY);
            if (aSpeed < 0.5f) {
                mVelX = 3.33f;
                mVelY = 0.0f;
            }
        }
    }

    old_Projectile_Update(this);
}

Plant *Projectile::FindCollisionTargetPlant() {
    Rect aProjectileRect = GetProjectileRect();

    Plant *aPlant = nullptr;
    while (mBoard->IteratePlants(aPlant)) {
        if (aPlant->mRow != mRow)
            continue;

        if (mProjectileType == ProjectileType::PROJECTILE_ZOMBIE_PEA) {
            if (aPlant->mSeedType == SeedType::SEED_PUFFSHROOM || aPlant->mSeedType == SeedType::SEED_SUNSHROOM || aPlant->mSeedType == SeedType::SEED_POTATOMINE
                || aPlant->mSeedType == SeedType::SEED_SPIKEWEED || aPlant->mSeedType == SeedType::SEED_SPIKEROCK || aPlant->mSeedType == SeedType::SEED_LILYPAD) // 僵尸豌豆不能击中低矮植物
                continue;
        }

        Rect aPlantRect = aPlant->GetPlantRect();
        if (GetRectOverlap(aProjectileRect, aPlantRect) > 8) {
            if (mProjectileType == ProjectileType::PROJECTILE_ZOMBIE_PEA) {
                return mBoard->GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, PlantPriority::TOPPLANT_EATING_ORDER);
            } else {
                return mBoard->GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, PlantPriority::TOPPLANT_CATAPULT_ORDER);
            }
        }
    }

    return nullptr;
}

bool Projectile::PeaAboutToHitTorchwood() {
    if (mMotionType != ProjectileMotion::MOTION_STRAIGHT)
        return false;

    if (mProjectileType != ProjectileType::PROJECTILE_PEA && mProjectileType != ProjectileType::PROJECTILE_SNOWPEA)
        return false;

    Plant *aPlant = nullptr;
    while (mBoard->IteratePlants(aPlant)) {
        if (aPlant->mSeedType == SeedType::SEED_TORCHWOOD && aPlant->mRow == mRow && !aPlant->NotOnGround() && mHitTorchwoodGridX != aPlant->mPlantCol) {
            Rect aPlantAttackRect = aPlant->GetPlantAttackRect(PlantWeapon::WEAPON_PRIMARY);
            Rect aProjectileRect = GetProjectileRect();
            aProjectileRect.mX += 40;

            if (GetRectOverlap(aPlantAttackRect, aProjectileRect) > 10) {
                return true;
            }
        }
    }

    return false;
}

Zombie *Projectile::FindCollisionTarget() {
    if (PeaAboutToHitTorchwood()) // “卡火炬”的原理，这段代码在两版内测版中均不存在
        return nullptr;

    Rect aProjectileRect = GetProjectileRect();
    Zombie *aBestZombie = nullptr;
    int aMinX = 0;

    Zombie *aZombie = nullptr;
    while (mBoard->IterateZombies(aZombie)) {
        if ((aZombie->mZombieType == ZombieType::ZOMBIE_BOSS || aZombie->mRow == mRow) && aZombie->EffectedByDamage((unsigned int)mDamageRangeFlags)) {
            if (aZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_WALKING_IN_POOL && mPosZ >= 45.0f) {
                continue;
            }

            if (mProjectileType == ProjectileType::PROJECTILE_STAR && mProjectileAge < 25 && mVelX >= 0.0f && aZombie->mZombieType == ZombieType::ZOMBIE_DIGGER) {
                continue;
            }

            Rect aZombieRect = aZombie->GetZombieRect();
            if (GetRectOverlap(aProjectileRect, aZombieRect) > 0) {
                if (aBestZombie == nullptr || aZombie->mX < aMinX) {
                    aBestZombie = aZombie;
                    aMinX = aZombie->mX;
                }
            }
        }
    }

    return aBestZombie;
}

Rect Projectile::GetProjectileRect() {
    if (mProjectileType == ProjectileType::PROJECTILE_PEA || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA || mProjectileType == ProjectileType::PROJECTILE_ZOMBIE_PEA) {
        return Rect(mX - 15, mY, mWidth + 15, mHeight);
    } else if (mProjectileType == ProjectileType::PROJECTILE_COBBIG) {
        return Rect(mX + mWidth / 2 - 115, mY + mHeight / 2 - 115, 230, 230);
    } else if (mProjectileType == ProjectileType::PROJECTILE_MELON || mProjectileType == ProjectileType::PROJECTILE_WINTERMELON) {
        return Rect(mX + 20, mY, 60, mHeight);
    } else if (mProjectileType == ProjectileType::PROJECTILE_FIREBALL) {
        return Rect(mX, mY, mWidth - 10, mHeight);
    } else if (mProjectileType == ProjectileType::PROJECTILE_SPIKE) {
        return Rect(mX - 25, mY, mWidth + 25, mHeight);
    } else {
        return Rect(mX, mY, mWidth, mHeight);
    }
}

void Projectile::ConvertToFireball(int theGridX) {
    if (isOnlyTouchFireWood) {
        if (bulletSpinnerChosenNum != -1) {
            mProjectileType = (ProjectileType)bulletSpinnerChosenNum;
            return;
        }
        if (randomBullet) {
            mProjectileType = (ProjectileType)randomInt(1, 12);
            return;
        }
    }

    if (mHitTorchwoodGridX == theGridX)
        return;

    mProjectileType = ProjectileType::PROJECTILE_FIREBALL;
    mHitTorchwoodGridX = theGridX;
    mApp->PlayFoley(FoleyType::FOLEY_FIREPEA);

    float aOffsetX = -25.0f;
    float aOffsetY = -25.0f;
    Reanimation *aFirePeaReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_FIRE_PEA);
    if (mMotionType == ProjectileMotion::MOTION_BACKWARDS) {
        aFirePeaReanim->OverrideScale(-1.0f, 1.0f);
        aOffsetX += 80.0f;
    }

    aFirePeaReanim->SetPosition(mPosX + aOffsetX, mPosY + aOffsetY);
    aFirePeaReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
    aFirePeaReanim->mAnimRate = RandRangeFloat(50.0f, 80.0f);
    AttachReanim(mAttachmentID, aFirePeaReanim, aOffsetX, aOffsetY);
}

void Projectile::ConvertToPea(int theGridX) {
    if (ColdPeaCanPassFireWood) {
        if (mHitTorchwoodGridX != theGridX) {
            AttachmentDie(mAttachmentID);
            mHitTorchwoodGridX = theGridX;
            mProjectileType = ProjectileType::PROJECTILE_SNOWPEA;
            mApp->PlayFoley(FoleyType::FOLEY_THROW);
        }
        return;
    }

    old_Projectile_ConvertToPea(this, theGridX);
}


void Projectile::UpdateNormalMotion() {
    old_Projectile_UpdateNormalMotion(this);
}


void Projectile::PlayImpactSound(Zombie *theZombie) {
    bool aPlayHelmSound = true;
    bool aPlaySplatSound = true;
    if (mProjectileType == ProjectileType::PROJECTILE_KERNEL) {
        mApp->PlayFoley(FoleyType::FOLEY_KERNEL_SPLAT);
        aPlayHelmSound = false;
        aPlaySplatSound = false;
    } else if (mProjectileType == ProjectileType::PROJECTILE_BUTTER) {
        mApp->PlayFoley(FoleyType::FOLEY_BUTTER);
        aPlaySplatSound = false;
    } else if (mProjectileType == ProjectileType::PROJECTILE_FIREBALL && IsSplashDamage(theZombie)) {
        mApp->PlayFoley(FoleyType::FOLEY_IGNITE);
        aPlayHelmSound = false;
        aPlaySplatSound = false;
    } else if (mProjectileType == ProjectileType::PROJECTILE_MELON || mProjectileType == ProjectileType::PROJECTILE_WINTERMELON) {
        mApp->PlayFoley(FoleyType::FOLEY_MELONIMPACT);
        aPlaySplatSound = false;
    }

    if (aPlayHelmSound && theZombie) {
        if (theZombie->mHelmType == HELMTYPE_PAIL) {
            mApp->PlayFoley(FoleyType::FOLEY_SHIELD_HIT);
            aPlaySplatSound = false;
        } else if (theZombie->mHelmType == HELMTYPE_TRAFFIC_CONE || theZombie->mHelmType == HELMTYPE_DIGGER || theZombie->mHelmType == HELMTYPE_FOOTBALL) {
            mApp->PlayFoley(FoleyType::FOLEY_PLASTIC_HIT);
        }
    }

    if (aPlaySplatSound) {
        mApp->PlayFoley(FoleyType::FOLEY_SPLAT);
    }
}

void Projectile::DoImpact(Zombie *theZombie) {
    if (theZombie) {
        if (mApp->mGameMode == GameMode::GAMEMODE_MP_VS) {
            if (theZombie->IsFlying()) {
                if (mProjectileType == ProjectileType::PROJECTILE_SPIKE) {
                    unsigned int aDamageFlags = GetDamageFlags(theZombie);
                    theZombie->TakeDamage(theZombie->mFlyingHealth, aDamageFlags);
                    return;
                }
            }
        }
    }


    if (globalButterChance > 0 && theZombie) {
        int chanceValues[] = {0, 10, 25, 50, 75, 100};
        if (globalButterChance < 6 && rand() % 100 < chanceValues[globalButterChance]) {
            theZombie->ApplyButter();
        }
    }

    if (!projectilePierce) {
        old_Projectile_DoImpact(this, theZombie);
        return;
    }
    // 负责 直线子弹帧伤
    PlayImpactSound(theZombie);

    if (IsSplashDamage(theZombie)) {
        if (theZombie && mProjectileType == ProjectileType::PROJECTILE_FIREBALL) {
            theZombie->RemoveColdEffects();
        }

        DoSplashDamage(theZombie, nullptr);
    } else if (theZombie) {
        unsigned int mDamageFlags = GetDamageFlags(theZombie);
        theZombie->TakeDamage(GetProjectileDef().mDamage, mDamageFlags);
    }

    float aLastPosX = mPosX - mVelX;
    float aLastPosY = mPosY + mPosZ - mVelY - mVelZ;
    ParticleEffect aEffect = ParticleEffect::PARTICLE_NONE;
    float aSplatPosX = mPosX + 12.0f;
    float aSplatPosY = mPosY + 12.0f;
    if (mProjectileType == ProjectileType::PROJECTILE_MELON) {
        mApp->AddTodParticle(aLastPosX + 30.0f, aLastPosY + 30.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_MELONSPLASH);
    } else if (mProjectileType == ProjectileType::PROJECTILE_WINTERMELON) {
        mApp->AddTodParticle(aLastPosX + 30.0f, aLastPosY + 30.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_WINTERMELON);
    } else if (mProjectileType == ProjectileType::PROJECTILE_COBBIG) {
        int aRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, mCobTargetRow, 2);
        mApp->AddTodParticle(mPosX + 80.0f, mPosY + 40.0f, aRenderOrder, ParticleEffect::PARTICLE_BLASTMARK);
        mApp->AddTodParticle(mPosX + 80.0f, mPosY + 40.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_POPCORNSPLASH);
        mApp->PlaySample(*Sexy_SOUND_DOOMSHROOM_Addr);
        mBoard->ShakeBoard(3, -4);
    } else if (mProjectileType == ProjectileType::PROJECTILE_PEA) {
        aSplatPosX -= 15.0f;
        aEffect = ParticleEffect::PARTICLE_PEA_SPLAT;
    } else if (mProjectileType == ProjectileType::PROJECTILE_SNOWPEA) {
        aSplatPosX -= 15.0f;
        aEffect = ParticleEffect::PARTICLE_SNOWPEA_SPLAT;
    } else if (mProjectileType == ProjectileType::PROJECTILE_FIREBALL) {
        if (IsSplashDamage(theZombie)) {
            Reanimation *aFireReanim = mApp->AddReanimation(mPosX + 38.0f, mPosY - 20.0f, mRenderOrder + 1, ReanimationType::REANIM_JALAPENO_FIRE);
            aFireReanim->mAnimTime = 0.25f;
            aFireReanim->mAnimRate = 24.0f;
            aFireReanim->OverrideScale(0.7f, 0.4f);
        }
    } else if (mProjectileType == ProjectileType::PROJECTILE_STAR) {
        aEffect = ParticleEffect::PARTICLE_STAR_SPLAT;
    } else if (mProjectileType == ProjectileType::PROJECTILE_PUFF) {
        aSplatPosX -= 20.0f;
        aEffect = ParticleEffect::PARTICLE_PUFF_SPLAT;
    } else if (mProjectileType == ProjectileType::PROJECTILE_CABBAGE) {
        aSplatPosX = aLastPosX - 38.0f;
        aSplatPosY = aLastPosY + 23.0f;
        aEffect = ParticleEffect::PARTICLE_CABBAGE_SPLAT;
    } else if (mProjectileType == ProjectileType::PROJECTILE_BUTTER) {
        aSplatPosX = aLastPosX - 20.0f;
        aSplatPosY = aLastPosY + 63.0f;
        aEffect = ParticleEffect::PARTICLE_BUTTER_SPLAT;

        if (theZombie) {
            theZombie->ApplyButter();
        }
    }

    if (aEffect != ParticleEffect::PARTICLE_NONE) {
        if (theZombie) {
            float aPosX = aSplatPosX + 52.0f - theZombie->mX;
            float aPosY = aSplatPosY - theZombie->mY;
            if (theZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_WALKING_IN_POOL || theZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_WALKING_IN_POOL) {
                aPosY += 60.0f;
            }
            if (mMotionType == ProjectileMotion::MOTION_BACKWARDS) {
                aPosX -= 80.0f;
            } else if (mPosX > theZombie->mX + 40 && mMotionType != ProjectileMotion::MOTION_LOBBED) {
                aPosX -= 60.0f;
            }
            if (aPosY > 100.0f)
                aPosY = 100.0f;
            if (aPosY < 20.0f)
                aPosY = 20.0f;
            theZombie->AddAttachedParticle(aPosX, aPosY, aEffect);
        } else {
            mApp->AddTodParticle(aSplatPosX, aSplatPosY, mRenderOrder + 1, aEffect);
        }
    }

    if (mMotionType == ProjectileMotion::MOTION_LOBBED && theZombie == nullptr) {
        // 如果玩家开启了“子弹帧伤”,且子弹是抛物线轨迹
        Die();
        return;
    }
}

Zombie *Projectile::FindCollisionMindControlledTarget() {
    // 豌豆僵尸的子弹专用的寻敌函数，寻找被魅惑的僵尸。
    Zombie *aZombie = nullptr;
    Zombie *aBestZombie = nullptr;
    int aMinX = 0;

    Sexy::Rect aProjectileRect = GetProjectileRect();
    while (mBoard->IterateZombies(aZombie)) {
        if (!aZombie->mDead && aZombie->mRow == mRow && aZombie->mMindControlled) {
            Sexy::Rect zombieRect = aZombie->GetZombieRect();
            int rectOverlap = GetRectOverlap(aProjectileRect, zombieRect);
            if (rectOverlap >= 0 && (aBestZombie != nullptr || aZombie->mX > aMinX)) {
                aBestZombie = aZombie;
                aMinX = aZombie->mX;
            }
        }
    }

    return aBestZombie;
}

void Projectile::CheckForCollision() {
    // 修复豌豆僵尸的子弹无法击中魅惑僵尸、修复随即子弹飞出屏幕不自动消失导致闪退。
    if (mMotionType == ProjectileMotion::MOTION_PUFF && mProjectileAge >= 75) {
        Die();
        return;
    }

    if (mPosX > 800.0f || mPosX + mWidth < 0.0f) {
        Die();
        return;
    }

    if (mMotionType == ProjectileMotion::MOTION_HOMING) {
        Zombie *aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
        if (aZombie && aZombie->EffectedByDamage(mDamageRangeFlags)) {
            Sexy::Rect aProjectileRect = GetProjectileRect();
            Sexy::Rect aZombieRect = aZombie->GetZombieRect();
            int rectOverlap = GetRectOverlap(aProjectileRect, aZombieRect);
            if (rectOverlap >= 0 && mPosY > aZombieRect.mY && mPosY < aZombieRect.mY + aZombieRect.mHeight) {
                DoImpact(aZombie);
            }
        }
        return;
    }

    // if (mProjectileType == ProjectileType::PROJECTILE_STAR && (mPosY > 600.0f || mPosY < 40.0f)) {
    // Die(projectile);
    // return;
    // }

    if (mMotionType == ProjectileMotion::MOTION_STAR && (mPosY > 600.0f || mPosY < 40.0f)) {
        // 将判断条件从mProjectileType改为mMotionType，从而修复随机杨桃子弹在Y方向出界后不消失导致的闪退
        Die();
        return;
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_HEAVY_WEAPON && (mPosY > 600.0f || mPosY < 40.0f)) {
        // 添加一段逻辑，让重型武器中所有Y方向出界的子弹都会消失。无论子弹种类。
        Die();
        return;
    }


    if ((mProjectileType == ProjectileType::PROJECTILE_PEA || mProjectileType == ProjectileType::PROJECTILE_STAR) && mShadowY - mPosY > 90.0f) {
        return;
    }
    if (mMotionType == ProjectileMotion::MOTION_FLOAT_OVER) {
        return;
    }

    if (mProjectileType == ProjectileType::PROJECTILE_ZOMBIE_PEA) {
        Plant *aPlant = FindCollisionTargetPlant();
        if (aPlant) {
            const ProjectileDefinition &aProjectileDef = GetProjectileDef();
            aPlant->mPlantHealth -= aProjectileDef.mDamage;
            aPlant->mEatenFlashCountdown = std::max(aPlant->mEatenFlashCountdown, 25);

            mApp->PlayFoley(FoleyType::FOLEY_SPLAT);
            mApp->AddTodParticle(mPosX - 3.0f, mPosY + 17.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_PEA_SPLAT);
            Die();
            return;
        }
        Zombie *aZombie = FindCollisionMindControlledTarget();
        if (aZombie) {
            if (aZombie->mOnHighGround && CantHitHighGround()) {
                return;
            }
            mProjectileType = ProjectileType::PROJECTILE_PEA; // 将子弹类型修改为普通豌豆，从而修复子弹打到魅惑僵尸身上没有击中特效。
            DoImpact(aZombie);
        }
        return;
    }

    // if ((mDamageRangeFlags & 1) == 0) { //TV的原版代码中存在这个，但是我这么写会导致仙人掌打不到气球。因此注释
    // return;
    // }

    Zombie *aZombie = FindCollisionTarget();
    if (aZombie) {
        if (aZombie->mOnHighGround && CantHitHighGround()) {
            return;
        }
        DoImpact(aZombie);
    } else if (mApp->mGameMode == GameMode::GAMEMODE_MP_VS) {
        GridItem *aGridItem = FindCollisionTargetGridItem();
        if (aGridItem) {
            DoImpactGridItem(aGridItem);
        }
    }
}

bool Projectile::CantHitHighGround() {
    if (mMotionType == ProjectileMotion::MOTION_BACKWARDS || mMotionType == ProjectileMotion::MOTION_HOMING)
        return false;

    return (mProjectileType == ProjectileType::PROJECTILE_PEA || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA || mProjectileType == ProjectileType::PROJECTILE_STAR
            || mProjectileType == ProjectileType::PROJECTILE_PUFF || mProjectileType == ProjectileType::PROJECTILE_FIREBALL)
        && !mOnHighGround;
}

bool Projectile::IsSplashDamage(Zombie *theZombie) {
    if (mProjectileType && theZombie && theZombie->IsFireResistant())
        return false;

    return mProjectileType == ProjectileType::PROJECTILE_MELON || mProjectileType == ProjectileType::PROJECTILE_WINTERMELON || mProjectileType == ProjectileType::PROJECTILE_FIREBALL;
}

unsigned int Projectile::GetDamageFlags(Zombie *theZombie) {
    unsigned int aDamageFlags = 0U;

    if (IsSplashDamage(theZombie)) {
        SetBit(aDamageFlags, (int)DamageFlags::DAMAGE_HITS_SHIELD_AND_BODY, true);
    } else if (mMotionType == ProjectileMotion::MOTION_LOBBED || mMotionType == ProjectileMotion::MOTION_BACKWARDS) {
        SetBit(aDamageFlags, (int)DamageFlags::DAMAGE_BYPASSES_SHIELD, true);
    } else if (mMotionType == ProjectileMotion::MOTION_STAR && mVelX < 0.0f) {
        SetBit(aDamageFlags, (int)DamageFlags::DAMAGE_BYPASSES_SHIELD, true);
    }

    if (mProjectileType == ProjectileType::PROJECTILE_SNOWPEA || mProjectileType == ProjectileType::PROJECTILE_WINTERMELON) {
        SetBit(aDamageFlags, (int)DamageFlags::DAMAGE_FREEZE, true);
    }

    return aDamageFlags;
}

ProjectileDefinition &Projectile::GetProjectileDef() {
    ProjectileDefinition &aProjectileDef = gProjectileDefinition[(int)mProjectileType];
    return aProjectileDef;
}

void Projectile::Draw(Graphics *g) {
    // Efecto neón/arcoíris para proyectiles
    if (projectileNeon) {
        g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
        g->SetColorizeImages(true);
        // Ciclo de colores arcoíris basado en posición y tiempo
        int hue = ((int)(mPosX + mBoard->mMainCounter * 2) % 360);
        Color rainbowColor;
        if (hue < 60) {
            rainbowColor = {255, (unsigned char)(hue * 255 / 60), 0, 200};
        } else if (hue < 120) {
            rainbowColor = {(unsigned char)((120 - hue) * 255 / 60), 255, 0, 200};
        } else if (hue < 180) {
            rainbowColor = {0, 255, (unsigned char)((hue - 120) * 255 / 60), 200};
        } else if (hue < 240) {
            rainbowColor = {0, (unsigned char)((240 - hue) * 255 / 60), 255, 200};
        } else if (hue < 300) {
            rainbowColor = {(unsigned char)((hue - 240) * 255 / 60), 0, 255, 200};
        } else {
            rainbowColor = {255, 0, (unsigned char)((360 - hue) * 255 / 60), 200};
        }
        g->SetColor(rainbowColor);
    }

    if (mProjectileType < NUM_PROJECTILES) {
        old_Projectile_Draw(this, g);

        if (projectileNeon) {
            g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
            g->SetColorizeImages(false);
        }
        return;
    }

    Graphics gProj(*g);
    // gProj.SetColorizeImages(true);
    // gProj.SetColor(mOverrideColor);

    const ProjectileDefinition &aProjectileDef = GetProjectileDef();

    Image *aImage = nullptr;
    float aScaleX = 1.0f;
    float aScaleY = 1.0f;

    bool aMirror = false;
    if (mMotionType == ProjectileMotion::MOTION_BEE_BACKWARDS) {
        aMirror = true;
    }

    if (aImage) {
        int aCelWidth = aImage->GetCelWidth();
        int aCelHeight = aImage->GetCelHeight();
        Rect aSrcRect(aCelWidth * mFrame, aCelHeight * aProjectileDef.mImageRow, aCelWidth, aCelHeight);
        if (FloatApproxEqual(mRotation, 0.0f) && FloatApproxEqual(aScaleX, 1.0f) && FloatApproxEqual(aScaleY, 1.0f)) {
            Rect aDestRect(0, 0, aCelWidth, aCelHeight);
            gProj.DrawImageMirror(aImage, aDestRect, aSrcRect, aMirror);
        } else {
            float aOffsetX = mPosX + aCelWidth * 0.5f;
            float aOffsetY = mPosZ + mPosY + aCelHeight * 0.5f;
            float aWideScreenOffsetX = 240;
            float aWideScreenOffsetY = 80;
            if (aMirror)
                aScaleX *= -1;

            SexyTransform2D aTransform;
            TodScaleRotateTransformMatrix(aTransform, aOffsetX + mBoard->mX + aWideScreenOffsetX, aOffsetY + mBoard->mY + aWideScreenOffsetY, mRotation, aScaleX, aScaleY);
            TodBltMatrix(&gProj, aImage, aTransform, gProj.mClipRect, Color::White, gProj.mDrawMode, aSrcRect);
        }
    }

    if (mAttachmentID != AttachmentID::ATTACHMENTID_NULL) {
        Graphics theParticleGraphics(gProj);
        MakeParentGraphicsFrame(&theParticleGraphics);
        AttachmentDraw(mAttachmentID, &theParticleGraphics, false);
    }
}

void Projectile::DrawShadow(Graphics *g) {
    int aCelCol = 0;
    float aScale = 1.0f;
    float aStretch = 1.0f;
    float aOffsetX = mPosX - mX;
    float aOffsetY = mPosY - mY;

    int aGridX = mBoard->PixelToGridXKeepOnBoard(mX, mY);
    bool isHighGround = false;
    if (mBoard->mGridSquareType[aGridX][mRow] == GridSquareType::GRIDSQUARE_HIGH_GROUND) {
        isHighGround = true;
    }
    if (mOnHighGround && !isHighGround) {
        aOffsetY += HIGH_GROUND_HEIGHT;
    } else if (!mOnHighGround && isHighGround) {
        aOffsetY -= HIGH_GROUND_HEIGHT;
    }

    if (mBoard->StageIsNight()) {
        aCelCol = 1;
    }

    switch (mProjectileType) {
        case ProjectileType::PROJECTILE_PEA:
        case ProjectileType::PROJECTILE_ZOMBIE_PEA:
            aOffsetX += 3.0f;
            break;

        case ProjectileType::PROJECTILE_SNOWPEA:
            aOffsetX += -1.0f;
            aScale = 1.3f;
            break;

        case ProjectileType::PROJECTILE_STAR:
            aOffsetX += 7.0f;
            break;

        case ProjectileType::PROJECTILE_CABBAGE:
        case ProjectileType::PROJECTILE_KERNEL:
        case ProjectileType::PROJECTILE_BUTTER:
        case ProjectileType::PROJECTILE_MELON:
        case ProjectileType::PROJECTILE_WINTERMELON:
            aOffsetX += 3.0f;
            aOffsetY += 10.0f;
            aScale = 1.6f;
            break;

        case ProjectileType::PROJECTILE_PUFF:
            return;

        case ProjectileType::PROJECTILE_COBBIG:
            aScale = 1.0f;
            aStretch = 3.0f;
            aOffsetX += 57.0f;
            break;

        case ProjectileType::PROJECTILE_FIREBALL:
            aScale = 1.4f;
            break;
        default:
            break;
    }

    if (mMotionType == ProjectileMotion::MOTION_LOBBED) {
        float aHeight = ClampFloat(-mPosZ, 0.0f, 200.0f);
        aScale *= 200.0f / (aHeight + 200.0f);
    }

    TodDrawImageCelScaledF(g, *IMAGE_PEA_SHADOWS, aOffsetX, (mShadowY - mPosY + aOffsetY), aCelCol, 0, aScale * aStretch, aScale);
}
