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

#include "PvZ/GlobalVariable.h"
#include "PvZ/HookInit.h"
#include "PvZ/Lawn/Board/Board.h"
#include "PvZ/Lawn/Common/ConstEnums.h"
#include "PvZ/Lawn/GamepadControls.h"
#include "PvZ/Lawn/LawnApp.h"
#include "PvZ/MagicAddr.h"
#include "PvZ/Symbols.h"

#include <jni.h>

#include <cmath>

#include <map>
#include <numbers>
#include <sstream>

static jstring StringToJString(JNIEnv *env, std::string_view sv) {
    return env->NewStringUTF(sv.data());
}

static std::string JStringToString(JNIEnv *env, jstring str) {
    const char *buffer = env->GetStringUTFChars(str, nullptr);
    std::string result(buffer);
    env->ReleaseStringUTFChars(str, buffer);
    return result;
}

/**
 * @brief Homura 模块的初始化函数.
 *
 * Java 层已指定模块加载顺序: 先 libGameMain.so, 后 libHomura.so.
 */
[[gnu::constructor]] static void lib_main() {
    // 获取符号地址
    GetFunctionAddr();

    // 部分安卓4设备无法获取到基址，暂不清楚原因, 但仅影响 Patch 而不影响Hook, 影响不大.
    gLibBaseOffset = ((Board_UpdateAddr != nullptr) && (uintptr_t(Board_UpdateAddr) > BOARD_UPDATE_ADDR_RELATIVE + 1)) ? (uintptr_t(Board_UpdateAddr) - BOARD_UPDATE_ADDR_RELATIVE - 1) : 0;

    // Hook
    CallHook();

    homura::Patcher::UpdateBaseAddrMap("libGameMain.so", gLibBaseOffset);
    ApplyPatches();
}

// jint JNI_OnLoad(JavaVM *vm, void *reserved) {
// return JNI_VERSION_1_6;
// }

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeDisableShop(JNIEnv *env, jclass clazz) {
    disableShop = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeEnableManualCollect(JNIEnv *env, jclass clazz) {
    enableManualCollect = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeSetHeavyWeaponAngle(JNIEnv *env, jclass clazz, jint i) {
    double radian = i * std::numbers::pi / 180;
    angle1 = float(std::cos(radian));
    angle2 = float(std::sin(radian));
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeEnableNewOptionsDialog(JNIEnv *env, jclass clazz) {
    enableNewOptionsDialog = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeGaoJiPause(JNIEnv *env, jclass clazz, jboolean enable) {
    if (isMainMenu) {
        return;
    }
    requestPause = enable;
    LawnApp *lawnApp = *gLawnApp_Addr;

    // if (lawnApp->mPlayerInfo != nullptr){
    // lawnApp->mPlayerInfo->mChallengeRecords[GameMode::ChallengeButteredPopcorn - 2] = 0;
    // lawnApp->mPlayerInfo->mChallengeRecords[GameMode::ChallengeHeavyWeapon - 2] = 0;
    // }
    if (enable) {
        lawnApp->PlaySample(*Sexy_SOUND_PAUSE_Addr);
    } else {
        lawnApp->PlaySample(*Sexy_SOUND_GRAVEBUTTON_Addr);
    }
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeIsGaoJiPaused(JNIEnv *env, jclass clazz) {
    return requestPause;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeHideCoverLayer(JNIEnv *env, jclass clazz) {
    hideCoverLayer = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeShowCoolDown(JNIEnv *env, jclass clazz) {
    showCoolDown = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeEnableNormalLevelMode(JNIEnv *env, jclass clazz) {
    normalLevel = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeEnableImitater(JNIEnv *env, jclass clazz) {
    imitater = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeSendSecondTouch(JNIEnv *env, jclass clazz, jint x, jint y, jint action) {
    if (tcp_connected || tcpClientSocket >= 0) {
        return;
    }
    Board *aBoard = (*gLawnApp_Addr)->mBoard;
    if (aBoard == nullptr) {
        return;
    }
    switch (action) {
        case 0:
            aBoard->MouseDownSecond(x, y, 0);
            break;
        case 1:
            aBoard->MouseDragSecond(x, y);
            break;
        case 2:
            aBoard->MouseUpSecond(x, y, 0);
            break;
        default:
            break;
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeEnableNewShovel(JNIEnv *env, jclass clazz) {
    useNewShovel = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeDisableTrashBinZombie(JNIEnv *env, jclass clazz) {
    gZombieTrashBinDef.mPickWeight = 0;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeShowHouse(JNIEnv *env, jclass clazz) {
    showHouse = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeUseNewCobCannon(JNIEnv *env, jclass clazz) {
    useNewCobCannon = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_android_support_Preferences_Changes(JNIEnv *env, jclass clazz, jobject con, jint featNum, jstring featName, jint value, jboolean boolean, jstring str) {
    switch (featNum) {
        case 1:
            infiniteSun = boolean; // 无限阳光
            break;
        case 2:
            seedPacketFastCoolDown = boolean; // 卡片无冷却
            break;
        case 3:
            abilityFastCoolDown = boolean; // 技能无冷却
            break;
        case 4:
            mushroomsNoSleep = boolean; // 蘑菇免唤醒
            break;
        case 5:
            requestPause = boolean; // Pausa avanzada
            break;
        case 6:
            noFog = boolean; // Quitar niebla
            break;
        case 7:
            BanDropCoin = boolean; // No caen soles ni monedas
            break;
        case 12:
            plantFullColumn = boolean; // Plantar columna completa
            break;
        case 8:
            speedUpMode = value; // Velocidad del juego
            break;
        case 9:
            hypnoAllZombies = boolean; // 魅惑所有僵尸
            break;
        case 10:
            freezeAllZombies = boolean; // 冻结所有僵尸
            break;
        case 11:
            startAllMowers = boolean; // 启动所有小推车
            break;
        case 21:
            showPlantHealth = boolean; // 所有植物显血
            break;
        case 22:
            showNutGarlicSpikeHealth = boolean; // 损伤点类植物显血
            break;
        case 23:
            showZombieBodyHealth = boolean; // 本体显血
            break;
        case 24:
            showHelmAndShieldHealth = boolean; // 防具显血
            break;
        case 25:
            showGargantuarHealth = boolean; // 巨人显血
            break;
        case 26:
            drawDebugText = boolean; // 显示出怪信息
            break;
        case 27:
            drawDebugRects = boolean; // 绘制碰撞箱
            break;
        case 41:
            doCheatCodeDialog = boolean; // 秘籍指令
            break;
        case 42:
            FreePlantAt = boolean; // 自由种植
            break;
        case 43:
            transparentVase = boolean; // 罐子透视
            break;
        case 44:
            zombieBloated = boolean; // 普僵啃坚果必过敏
            break;
        case 45:
            ZombieCanNotWon = boolean; // 无视僵尸进家
            break;
        case 46:
            boardEdgeAdjust = value * 10; // 进家线后退
            break;
        case 47:
            zombieSetScale = value; // 进家线后退
            break;
        case 48:
            maidCheats = value; // 女仆秘籍
            break;
        case 61:
            ColdPeaCanPassFireWood = boolean; // 寒冰豌豆无视火炬
            break;
        case 62:
            projectilePierce = boolean; // 子弹帧伤
            break;
        case 63:
            bulletSpinnerChosenNum = value - 1; // 手动选择子弹类型
            break;
        case 64:
            randomBullet = boolean; // 随机子弹类型
            break;
        case 65:
            isOnlyPeaUseable = boolean; // 仅对普通豌豆生效
            break;
        case 66:
            isOnlyTouchFireWood = boolean; // 豌豆穿过火炬后转变
            break;
        case 67:
            banCobCannon = boolean; // Ban玉米炮弹
            break;
        case 68:
            banStar = boolean; // Ban星星子弹
            break;
        case 81:
            doCheatDialog = boolean; // 关卡跳转器
            break;
        case 82:
            passNowLevel = boolean; // 过了这关
            break;
        case 83:
            daveNoPickSeeds = boolean; // 取消必选卡片
            break;
        case 84:
            endlessLastStand = boolean; // 坚不可摧无尽
            break;
        case 85:
            targetWavesToJump = value; // 目标轮数
            break;
        case 86:
            requestJumpSurvivalStage = boolean; // 跳轮
            break;
        case 87:
            stopSpawning = boolean; // 暂停刷怪
            break;
        case 88:
            banMower = boolean; // 不出小推车
            break;
        case 101:
            PumpkinWithLadder = boolean; // 种下南瓜自动搭梯
            break;
        case 102:
            VSBackGround = value; // 更换场景
            break;
        case 103:
            theBuildPlantType = (SeedType)(value >= 49 ? value + 1 : value - 1); // 植物类型
            break;
        case 104:
            theBuildZombieType = (ZombieType)(value - 1); // 僵尸类型
            break;
        case 105:
            theBuildLadderX = value; // 横坐标
            theBuildPlantX = value;
            break;
        case 106:
            theBuildLadderY = value; // 纵坐标
            theBuildPlantY = value;
            break;
        case 107:
            isImitaterPlant = boolean; // 模仿者植物
            break;
        case 108:
            plantBuild = boolean; // 种植植物
            break;
        case 109:
            zombieBuild = boolean; // 放置僵尸
            break;
        case 110:
            ladderBuild = boolean; // 搭梯子(单格)
            break;
        case 111:
            recoverAllMowers = boolean; // 恢复所有小推车
            break;
        case 112:
            ClearAllPlant = boolean; // 清除所有植物
            break;
        case 113:
            clearAllMowers = boolean; // 清除所有小推车
            break;
        case 114:
            graveBuild = boolean; // 冒墓碑
            break;
        case 115:
            clearAllZombies = boolean; // 清除所有僵尸
            break;
        case 116:
            clearAllGraves = boolean; // 清除所有墓碑
            break;
        case 117:
            BuildZombieX = value;
            break;
        case 118:
            BuildZombieY = value;
            break;
        case 119:
            hypnoBuildZombie = boolean;
            break;
        case 126:
            goldenShovel = boolean; // Pala de oro
            break;
        case 127:
            zombieAllies = boolean; // Zombis aliados
            break;
        case 128:
            randomPlants = boolean; // Plantas aleatorias
            break;
        case 129:
            sunRain = boolean; // Lluvia de soles
            break;
        case 130:
            superMowers = boolean; // Super cortacéspedes (kept for references per user)
            break;
        case 131:
            autoCollect = boolean; // Auto-recolector
            break;
        case 132:
            plantOutline = boolean; // Plantas con contorno
            break;
        case 133:
            zombieOutline = boolean; // Zombis con contorno
            break;
        case 134:
            sunNeonEffect = boolean; // Soles con efecto neón
            break;
        case 135:
            projectileNeon = boolean; // Proyectiles neón
            break;
        case 136:
            coinRain = boolean; // Lluvia de monedas
            break;
        case 137:
            coinRainType = value; // Tipo de moneda
            break;
        case 138:
            infiniteMoney = boolean; // Dinero infinito
            break;
        case 139:
            unlockAll = boolean; // Desbloquear todo
            break;
        case 150:
            sunRainType = value; // Tipo de sol
            break;

        case 160:
            projectileRain = boolean; // Lluvia de proyectiles
            break;
        case 161:
            projectileRainType = value; // Tipo proyectil lluvia
            break;
        case 163:
            globalButterChance = value; // Prob. Mantequilla
            break;
        case 164:
            zombieMartyr = boolean; // Zombis mártires
            break;
        case 165:
            homingProjectiles = boolean; // Proyectiles buscadores
            break;
        case 166:
            butterGlove = boolean; // Guante de mantequilla
            break;

        case 121:
            formationId = value - 1; // 选择白天泳池阵型
            break;
        case 122:
            layChoseFormation = boolean; // 布置选择阵型
            break;
            // case 123:
            // break; // 复制阵型代码
        case 124:
            customFormation = JStringToString(env, str); // 粘贴阵型代码
            break;
        case 125:
            layPastedFormation = boolean; // 布置粘贴阵型
            break;
        case 141:
            targetSeedBank = value + 1; // 目标卡槽
            break;
        case 142:
            choiceSeedPacketIndex = value; // 卡片位置
            break;
        case 143:
            if (value <= 48) {
                choiceSeedType = (SeedType)(value - 1); // [豌豆射手, 模仿者)
            } else if (value <= 52) {
                choiceSeedType = (SeedType)(value + 1); // [爆炸坚果, NUM_SEED_TYPES)
            } else if (value <= 76) {
                choiceSeedType = (SeedType)(value + 8); // [墓碑, 气球僵尸]
            }
            break; // 卡片类型
        case 144:
            isImitaterSeed = boolean; // 模仿者植物卡片
            break;
        case 145:
            setSeedPacket = boolean; // 更换卡片
        case 200:
            checkZombiesAllowed[ZombieType::ZOMBIE_NORMAL] = boolean; // 普通僵尸
            break;
        case 202:
            checkZombiesAllowed[ZombieType::ZOMBIE_TRAFFIC_CONE] = boolean; // 路障僵尸
            break;
        case 203:
            checkZombiesAllowed[ZombieType::ZOMBIE_POLEVAULTER] = boolean; // 撑杆僵尸
            break;
        case 204:
            checkZombiesAllowed[ZombieType::ZOMBIE_PAIL] = boolean; // 铁桶僵尸
            break;
        case 205:
            checkZombiesAllowed[ZombieType::ZOMBIE_NEWSPAPER] = boolean; // 报纸僵尸
            break;
        case 206:
            checkZombiesAllowed[ZombieType::ZOMBIE_DOOR] = boolean; // 铁网门僵尸
            break;
        case 207:
            checkZombiesAllowed[ZombieType::ZOMBIE_FOOTBALL] = boolean; // 橄榄球僵尸
            break;
        case 208:
            checkZombiesAllowed[ZombieType::ZOMBIE_DANCER] = boolean; // 舞者僵尸
            break;
        case 211:
            checkZombiesAllowed[ZombieType::ZOMBIE_SNORKEL] = boolean; // 潜水僵尸
            break;
        case 212:
            checkZombiesAllowed[ZombieType::ZOMBIE_ZAMBONI] = boolean; // 雪橇车僵尸
            break;
        case 214:
            checkZombiesAllowed[ZombieType::ZOMBIE_DOLPHIN_RIDER] = boolean; // 海豚骑士僵尸
            break;
        case 215:
            checkZombiesAllowed[ZombieType::ZOMBIE_JACK_IN_THE_BOX] = boolean; // 小丑僵尸
            break;
        case 216:
            checkZombiesAllowed[ZombieType::ZOMBIE_BALLOON] = boolean; // 气球僵尸
            break;
        case 217:
            checkZombiesAllowed[ZombieType::ZOMBIE_DIGGER] = boolean; // 矿工僵尸
            break;
        case 218:
            checkZombiesAllowed[ZombieType::ZOMBIE_POGO] = boolean; // 蹦蹦僵尸
            break;
        case 219:
            checkZombiesAllowed[ZombieType::ZOMBIE_YETI] = boolean; // 僵尸雪人
            break;
        case 220:
            checkZombiesAllowed[ZombieType::ZOMBIE_BUNGEE] = boolean; // 飞贼僵尸
            break;
        case 221:
            checkZombiesAllowed[ZombieType::ZOMBIE_LADDER] = boolean; // 梯子僵尸
            break;
        case 222:
            checkZombiesAllowed[ZombieType::ZOMBIE_CATAPULT] = boolean; // 投石车僵尸
            break;
        case 223:
            checkZombiesAllowed[ZombieType::ZOMBIE_GARGANTUAR] = boolean; // 白眼巨人僵尸
            break;
        case 226:
            checkZombiesAllowed[ZombieType::ZOMBIE_TRASHCAN] = boolean; // 垃圾桶僵尸
            break;
        case 227:
            checkZombiesAllowed[ZombieType::ZOMBIE_PEA_HEAD] = boolean; // 豌豆射手僵尸
            break;
        case 228:
            checkZombiesAllowed[ZombieType::ZOMBIE_WALLNUT_HEAD] = boolean; // 坚果僵尸
            break;
        case 229:
            checkZombiesAllowed[ZombieType::ZOMBIE_JALAPENO_HEAD] = boolean; // 火爆辣椒僵尸
            break;
        case 230:
            checkZombiesAllowed[ZombieType::ZOMBIE_GATLING_HEAD] = boolean; // 机枪射手僵尸
            break;
        case 231:
            checkZombiesAllowed[ZombieType::ZOMBIE_SQUASH_HEAD] = boolean; // 窝瓜僵尸
            break;
        case 232:
            checkZombiesAllowed[ZombieType::ZOMBIE_TALLNUT_HEAD] = boolean; // 高坚果僵尸
            break;
        case 233:
            checkZombiesAllowed[ZombieType::ZOMBIE_REDEYE_GARGANTUAR] = boolean; // 红眼巨人僵尸
            break;
        case 234:
            choiceSpawnMode = value; // 刷怪模式
            break;
        case 236:
            buttonSetSpawn = boolean; // 设置出怪
            break;
        default:
            break;
    }
}

extern "C" JNIEXPORT jobjectArray JNICALL Java_com_android_support_CkHomuraMenu_GetFeatureList(JNIEnv *env, jobject thiz) {

    static constexpr const char *features[] = {

        "Collapse_Funciones Comunes",
        "CollapseAdd_RichTextView_<font color='cyan'>Recursos y Recompensas:",
        "1_CollapseAdd_Toggle_Soles infinitos",
        "129_CollapseAdd_Toggle_Lluvia de soles",
        "150_CollapseAdd_Spinner_Tipo de sol:_Sol pequeño,Sol mediano,Sol grande,Todos",
        "138_CollapseAdd_Toggle_Dinero infinito",
        "136_CollapseAdd_Toggle_Lluvia de monedas",
        "137_CollapseAdd_Spinner_Tipo de moneda:_Plata,Oro,Diamante,Todas",

        "CollapseAdd_RichTextView_<font color='cyan'>Mecánicas de Juego:",
        "42_CollapseAdd_Toggle_Plantar sin restricciones",
        "12_CollapseAdd_Toggle_Plantar en columna",
        "2_CollapseAdd_Toggle_Cartas sin enfriamiento",
        "3_CollapseAdd_Toggle_Habilidades sin enfriamiento",
        "4_CollapseAdd_Toggle_Setas siempre despiertas",
        "131_CollapseAdd_Toggle_Auto-recolector",
        "8_CollapseAdd_Spinner_Velocidad del juego:_Normal,1.2x,1.5x,2x,2.5x,3x,5x,10x",

        "CollapseAdd_RichTextView_<font color='cyan'>Efectos Visuales:",
        "132_CollapseAdd_Toggle_Plantas con contorno",
        "133_CollapseAdd_Toggle_Zombis con contorno",
        "134_CollapseAdd_Toggle_Soles con efecto neón",
        "135_CollapseAdd_Toggle_Proyectiles neón",
        "160_CollapseAdd_Toggle_Lluvia de proyectiles",
        "161_CollapseAdd_Spinner_Tipo proyectil lluvia:_Guisante,Col,Sandía,Hielo,Fuego,Maíz,Mantequilla,Todos",
        "163_CollapseAdd_Spinner_Prob. Mantequilla:_Apagado,10%,25%,50%,75%,100%",
        "164_CollapseAdd_Toggle_Zombis mártires",
        "165_CollapseAdd_Toggle_Proyectiles buscadores",
        "166_CollapseAdd_Toggle_Guante de mantequilla",
        "126_CollapseAdd_Toggle_Pala de oro",

        "CollapseAdd_RichTextView_<font color='cyan'>Progreso y Otros:",
        "139_CollapseAdd_OnceCheckBox_Desbloquear todo",
        "82_CollapseAdd_OnceCheckBox_Completar nivel",
        "127_CollapseAdd_Toggle_Zombis aliados",
        "128_CollapseAdd_Toggle_Plantas aleatorias",


        "Collapse_Funciones de Depuración",
        "CollapseAdd_RichTextView_<font color='green'>Vida de plantas:",
        "21_CollapseAdd_Toggle_Todas las plantas",
        "22_CollapseAdd_Toggle_Solo plantas dañadas",
        "CollapseAdd_RichTextView_<font color='green'>Vida de zombis:",
        "23_CollapseAdd_Toggle_Cuerpo",
        "25_CollapseAdd_Toggle_Gargantúas",
        "24_CollapseAdd_Toggle_Armaduras",
        "CollapseAdd_RichTextView_<font color='green'>Información:",
        "26_CollapseAdd_Toggle_Oleadas",
        "27_CollapseAdd_Toggle_Cajas de impacto",


        "Collapse_Funciones de Diversión",
        "6_CollapseAdd_Toggle_Eliminar niebla",
        "43_CollapseAdd_Toggle_Ver jarrones",
        "45_CollapseAdd_Toggle_Ignorar zombis en la casa",
        "44_CollapseAdd_Toggle_Zombis débiles a las nueces",
        "7_CollapseAdd_Toggle_No caen soles ni monedas",
        "48_CollapseAdd_Spinner_<font color='green'>Zombi bailarín:_Normal,Avanza siempre,Baila sin invocar,Invoca siempre",
        "46_CollapseAdd_Spinner_<font color='green'>Retroceder zombis:_Desactivado,10,20,30,40,50,60,70,80",
        "47_CollapseAdd_Spinner_<font color='green'>Tamaño de zombis:_Normal,0.2x,0.4x,0.6x,0.8x,1.0x,1.2x,1.4x,1.6x,1.8x,2.0x",
        "CollapseAdd_RichTextView_<font color='yellow'>#La vida se multiplica al cuadrado",
        "41_CollapseAdd_OnceCheckBox_Comandos del Árbol de la Sabiduría",


        "Collapse_Configuración de Nivel",
        "87_CollapseAdd_Toggle_Pausar oleadas",
        "88_CollapseAdd_Toggle_Sin cortacéspedes",
        "83_CollapseAdd_Toggle_Sin Semillas de Crazy Dave",
        "84_CollapseAdd_Toggle_Modo Infinito I, Zombi",
        "81_CollapseAdd_OnceCheckBox_Saltar nivel",
        "102_CollapseAdd_Spinner_<font color='green'>Escenario:_Normal,Día,Noche,Piscina,Niebla,Tejado,Noche lunar,Invernadero,Jardín de setas,Acuario",
        "CollapseAdd_RichTextView_<font color='green'>Saltar oleadas (Infinito):",
        "85_CollapseAdd_InputValue_Oleada objetivo",
        "86_CollapseAdd_OnceCheckBox_Saltar oleada",
        "CollapseAdd_RichTextView_<font color='yellow'>#Completa la selección de cartas primero",


        "Collapse_Configuración de Ranuras",
        "141_CollapseAdd_Spinner_<font color='green'>Seleccionar ranura:_Ranura 1,Ranura 2",
        "142_CollapseAdd_Spinner_<font color='green'>Posición de carta:_1,2,3,4,5,6,7,8,9,10",

        "143_CollapseAdd_Spinner_<font color='green'>Tipo de carta:_Ninguna,Lanzaguisantes,Girasol,Cereza bomba,Nuez,Patata mina,Guisante helado,Atrapamoscas,Lanzaguisantes doble,Seta pequeña,Seta "
        "solar,Seta de humo,Mordedor de tumbas,Hipnoseta,Seta miedosa,Seta congelada,Petaseta,Nenúfar,Aplastadora,Lanzaguisantes triple,Alga enredadera,Jalapeño,Pincho,Tronco ardiente,Nuez alta,Seta "
        "marina,Planterna,Cactus,Trébol,Lanzaguisantes doble cara,Fruta estelar,Calabaza,Imán-seta,Lanzacoles,Maceta,Lanzamaíz,Grano de café,Ajo,Parasol,Caléndula,Melonpulta,Repetidora,Girasol "
        "gemelo,Seta pequeña doble,Hierba gatuna,Sandía helada,Imán dorado,Pincho de acero,Mazorcañón,Nuez explosiva,Nuez gigante,Brote,Lanzaguisantes inverso,Lápida,Zombi,Zombi con cubo de "
        "basura,Zombi con cono,Zombi con pértiga,Zombi con cubo,Abanderado,Zombi con periódico,Zombi con puerta,Zombi futbolista,Zombi bailarín,Zombi en trineo,Zombi payaso,Zombi minero,Zombi "
        "saltarín,Zombi bungee,Zombi con escalera,Zombi catapulta,Gargantúa,Gargantúa rojo,Zombi con flotador,Zombi buzo,Zombi delfín,Diablillo,Zombi con globo",

        "144_CollapseAdd_CheckBox_Usar planta imitadora",
        "145_CollapseAdd_OnceCheckBox_Reemplazar carta",


        "Collapse_Configuración de Proyectiles",
        "62_CollapseAdd_Toggle_Daño continuo",
        "61_CollapseAdd_Toggle_Guisante helado atraviesa troncos",
        "63_CollapseAdd_Spinner_<font color='green'>Proyectil:_Normal,Guisante,Guisante helado,Col,Sandía,Espora,Sandía helada,Guisante de fuego,Estrella,Espina,Balón,Grano de "
        "maíz,Mazorca,Mantequilla,Autodestrucción",
        "64_CollapseAdd_Toggle_Tipo aleatorio",
        "65_CollapseAdd_Toggle_Solo guisantes normales",
        "66_CollapseAdd_Toggle_Cambiar al pasar por tronco",
        "67_CollapseAdd_CheckBox_Bloquear mazorcas",
        "68_CollapseAdd_CheckBox_Bloquear estrellas",


        "Collapse_Configuración de Oleadas",
        "CollapseAdd_RichTextView_<font color='green'>Zombis disponibles:",
        "200_CollapseAdd_CheckBox_Zombi básico",
        "202_CollapseAdd_CheckBox_Zombi con cono",
        "203_CollapseAdd_CheckBox_Zombi con pértiga",
        "204_CollapseAdd_CheckBox_Zombi con cubo",
        "205_CollapseAdd_CheckBox_Zombi con periódico",
        "206_CollapseAdd_CheckBox_Zombi con puerta",
        "207_CollapseAdd_CheckBox_Zombi futbolista",
        "208_CollapseAdd_CheckBox_Zombi bailarín",
        "211_CollapseAdd_CheckBox_Zombi buzo",
        "212_CollapseAdd_CheckBox_Zombi en trineo",
        "214_CollapseAdd_CheckBox_Zombi delfín",
        "215_CollapseAdd_CheckBox_Zombi payaso",
        "216_CollapseAdd_CheckBox_Zombi con globo",
        "217_CollapseAdd_CheckBox_Zombi minero",
        "218_CollapseAdd_CheckBox_Zombi saltarín",
        "219_CollapseAdd_CheckBox_Yeti",
        "220_CollapseAdd_CheckBox_Zombi bungee",
        "221_CollapseAdd_CheckBox_Zombi con escalera",
        "222_CollapseAdd_CheckBox_Zombi catapulta",
        "223_CollapseAdd_CheckBox_Gargantúa",
        "233_CollapseAdd_CheckBox_Gargantúa rojo",
        "226_CollapseAdd_CheckBox_Zombi con cubo de basura",
        "227_CollapseAdd_CheckBox_Zombi lanzaguisantes",
        "228_CollapseAdd_CheckBox_Zombi nuez",
        "229_CollapseAdd_CheckBox_Zombi jalapeño",
        "230_CollapseAdd_CheckBox_Zombi repetidora",
        "231_CollapseAdd_CheckBox_Zombi aplastadora",
        "232_CollapseAdd_CheckBox_Zombi nuez alta",
        "234_CollapseAdd_Spinner_<font color='green'>Generación:_Normal,Natural,Extrema",
        "236_CollapseAdd_OnceCheckBox_Aplicar configuración",

        "Collapse_Disposición de Plantas",
        "101_CollapseAdd_Toggle_Escalera automática en calabazas",
        "105_CollapseAdd_Spinner_<font color='yellow'>Coordenada X (Planta):_Columna 1,Columna 2,Columna 3,Columna 4,Columna 5,Columna 6,Columna 7,Columna 8,Columna 9,Todas",
        "106_CollapseAdd_Spinner_<font color='yellow'>Coordenada Y (Planta):_Fila 1,Fila 2,Fila 3,Fila 4,Fila 5,Fila 6,Todas",

        "103_CollapseAdd_Spinner_<font color='green'>Tipo de planta:_"
        "Sin seleccionar,Lanzaguisantes,Girasol,Cereza bomba,Nuez,Patata mina,Guisante helado,Atrapa-zombis,Lanzaguisantes doble,"
        "Petaseta,Seta solar,Humoseta,Mordedor de tumbas,Hipnoseta,Cobardiseta,Frigiseta,Petaseta,"
        "Nenúfar,Aplastadora,Lanzaguisantes triple,Alga enredadera,Jalapeño,"
        "Pincho,Tronco ardiente,Nuez alta,Setamar,Planterna,Cactus,Trébol,Guisante bífido,Fruta estelar,Calabaza,Imán-seta,"
        "Lanzacoles,Maceta,Lanzamaíz,Grano de café,Ajo,Parasol,Caléndula,Melonpulta,"
        "Repetidora,Girasol gemelo,Petaseta gemela,Hierba gatuna,Sandía helada,Imán dorado,Pincho de acero,Mazorcañón,"
        "Nuez explosiva,Nuez gigante,Brote,Lanzaguisantes inverso",

        "107_CollapseAdd_CheckBox_Usar planta imitadora",
        "108_CollapseAdd_OnceCheckBox_Plantar",

        "Collapse_Disposición de Zombis",
        "117_CollapseAdd_Spinner_<font color='yellow'>Coordenada X (Zombi):_Columna 1,Columna 2,Columna 3,Columna 4,Columna 5,Columna 6,Columna 7,Columna 8,Columna 9,Todas,Punto de aparición",
        "118_CollapseAdd_Spinner_<font color='yellow'>Coordenada Y (Zombi):_Fila 1,Fila 2,Fila 3,Fila 4,Fila 5,Fila 6,Todas",

        "104_CollapseAdd_Spinner_<font color='green'>Tipo de zombi:_"
        "Sin seleccionar,Zombi,Zombi abanderado,Zombi con cono,Zombi con pértiga,Zombi con cubo,Zombi con periódico,Zombi con puerta,Zombi futbolista,"
        "Zombi bailarín,Zombi de apoyo,Zombi con flotador,Zombi buzo,Zombi en trineo,Equipo de trineos,Zombi delfín,Zombi payaso,Zombi con globo,"
        "Zombi minero,Zombi saltarín,Yeti,Zombi bungee,Zombi con escalera,Zombi catapulta,Gargantúa,Diablillo,Dr. Zomboss,"
        "Zombi con cubo de basura,Zombi lanzaguisantes,Zombi nuez,Zombi jalapeño,Zombi repetidora,Zombi aplastadora,Zombi nuez alta,Gargantúa rojo",

        "119_CollapseAdd_CheckBox_Hipnotizar zombi",
        "109_CollapseAdd_OnceCheckBox_Colocar zombi",

        "CollapseAdd_RichTextView_<font color='green'>Otros:",
        "114_CollapseAdd_OnceCheckBox_Colocar lápida",
        "110_CollapseAdd_OnceCheckBox_Colocar escalera",
        "111_CollapseAdd_OnceCheckBox_Restaurar cortacéspedes",

        "CollapseAdd_RichTextView_<font color='yellow'>Limpiar:",
        "112_CollapseAdd_OnceCheckBox_Eliminar todas las plantas",
        "115_CollapseAdd_OnceCheckBox_Eliminar todos los zombis",
        "116_CollapseAdd_OnceCheckBox_Eliminar todas las lápidas",
        "113_CollapseAdd_OnceCheckBox_Eliminar cortacéspedes",

        "CollapseAdd_RichTextView_<font color='yellow'>Varios:",
        "9_CollapseAdd_OnceCheckBox_Hipnotizar zombis",
        "10_CollapseAdd_OnceCheckBox_Congelar zombis",
        "11_CollapseAdd_OnceCheckBox_Activar cortacéspedes",


        "Collapse_Formaciones Predefinidas",
        "121_CollapseAdd_Spinner_<font color='green'>Formación de Piscina (Infinito):_Sin seleccionar,"
        "[0]Reloj de ondas sin cañones,[1]Minimalista sin cañones,[2]Falso sin daño sin cañones,[3]Control natural de payasos,"
        "[4]Fuego sin cañones,[5]Fuego dividido sin cañones,[6]Retroceso sin cañones,[7]Ultra adelantado sin cañones,"
        "[8]Príncipe sin cañones,[9]Reloj mecánico sin cañones,"
        "[10]Divino sin cañones,[11]Reloj de cuarzo sin cañones,[12]Dependiente del azar sin cañones,"
        "[13]Bloques sin dioses sin cañones,[14]56 acelerado sin dioses,[15]Supresión de un cañón,"
        "[16]Doble cañón simple,[17]Doble cañón de fuego,[18]Doble cañón nuclear,[19]Doble cañón dividido,"
        "[20]Doble cañón cuadrado,[21]Doble cañón clásico,[22]Triple cañón de asalto,[23]Cuádruple cañón Taiji,"
        "[24]Cuádruple cañón metálico,[25]Cuádruple cañón de bloques,[26]Cuádruple cañón verde,"
        "[27]Cuádruple cañón sin plantas (agua),[28]Cuádruple cañón cuadrado,[29]Cuádruple cañón divino,"
        "[30]Cuádruple cañón doble núcleo,[31]Cuádruple cañón clásico,[32]Cuádruple cañón de fuego,"
        "[33]Cuádruple cañón de fondo,[34]Cuádruple cañón tradicional,[35]Quíntuple cañón sin plantas,"
        "[36]Quíntuple cañón disperso,[37]Quíntuple cañón corazón,[38]Sextuple cañón sin plantas (tierra),"
        "[39]Sextuple cañón sin plantas (agua),[40]Sextuple cañón musgo,[41]Jardín zen profundo,"
        "[42]Sextuple cañón divino,[43]Sextuple cañón de maíz,[44]Sextuple cañón aéreo,"
        "[45]Sextuple cañón ultra atrasado,[46]Sextuple cañón cuadrado,[47]Mariposa rítmica,"
        "[48]Una cucharada de tangyuan,[49]Séptuple cañón sin plantas,"
        "[50]Conejo de jade,[51]Octuple cañón sin protección,[52]Octuple cañón árbol,"
        "[53]Octuple cañón árbol simétrico,[54]Octuple cañón rectangular,[55]Octuple cañón divino,"
        "[56]Octuple cañón yin-yang,[57]Octuple cañón flotante,[58]Octuple cañón atrasado,"
        "[59]Criando delfines,[60]Octuple cañón de maíz,[61]Octuple cañón clásico,"
        "[62]Octuple cañón mar de flores,[63]Octuple cañón C2,[64]Octuple cañón separado,"
        "[65]Octuple cañón totalmente simétrico,[66]Octuple cañón 3C,[67]Octuple cañón faro,"
        "[68]Cañón 13,[69]Nueve cañones en bloque,"
        "[70]Nueve cañones C6i,[71]Nueve cañones corazón,[72]Nueve cañones rotativos,"
        "[73]Doble cañón II,[74]Diez cañones estrella,[75]Diez cañones hexagonal,"
        "[76]Diez cañones en bloque,[77]Diez cañones rombo,[78]Diez cañones simplificado,"
        "[79]Diez cañones atrasado,[80]Diez cañones clásico,[81]Prisión de seis líneas,"
        "[82]Diez cañones diagonal,[83]Diez cañones cubo,[84]La hamburguesa de Dave,"
        "[85]Cóctel,[86]Doce cañones tangyuan,[87]Doce cañones jarra de jade,"
        "[88]Doce cañones medio campo,[89]Doce cañones simplificado,"
        "[90]Doce cañones clásico,[91]Doce cañones de fuego,[92]Doce cañones lluvia helada (mod),"
        "[93]Doce cañones lluvia helada ++,[94]Doce cañones carta morada,"
        "[95]Doce cañones pilar divino,[96]Doce cañones divino,"
        "[97]Doce cañones sin plantas (agua),[98]Doce cañones blanco puro flotante,"
        "[99]Doce cañones patio trasero,"
        "[100]Doce cañones de maíz,[101]Doble ruta de furia,[102]Doce cañones nueve columnas,"
        "[103]Doce cañones de escaleras,[104]Doce cañones Junhai,[105]Arpa antigua,"
        "[106]Trece cañones flor de ciruelo,[107]Obra final,[108]Linterna de hielo,"
        "[109]Catorce cañones Taiji,[110]Verdadero cuádruple cañón,"
        "[111]Catorce cañones bastón divino,[112]Catorce cañones divino,"
        "[113]Catorce cañones atravesados,[114]Quince cañones diamante,"
        "[115]Quince cañones divino,[116]Verdadero doble cañón,[117]Linterna frigorífica,"
        "[118]Anillo de cañones y flores,[119]Dieciséis cañones hielo simple,"
        "[120]Dieciséis cañones simétrico,[121]Dieciséis cañones divino,"
        "[122]Dieciséis cañones sin protección,[123]Dieciséis cañones doble hielo,"
        "[124]Dieciséis cañones ultra adelantado,[125]Dieciséis cañones de fuego,"
        "[126]Dieciséis cañones clásico,[127]Dieciséis cañones en zigzag,"
        "[128]Dieciocho cañones pulmón (límite),[129]Dieciocho cañones puro,"
        "[130]Verdadero dieciocho cañones,[131]Dieciocho cañones espíritu de hielo,"
        "[132]Dieciocho cañones explosión final,[133]Dieciocho cañones clásico,"
        "[134]Veinte cañones puro,[135]Veinte cañones aéreo,[136]Veinte cañones rastrillo,"
        "[137]Nuevo veinte cañones,[138]Veinte cañones sin sandía helada,"
        "[139]Camino de la desesperación,"
        "[140]Veintiún cañones,[141]Nuevo veintidós cañones,[142]Veintidós cañones,"
        "[143]Veintidós cañones sin sandía helada,[144]Veintidós cañones nueve columnas,"
        "[145]Veinticuatro cañones,[146]Veinticuatro cañones con soporte,[147]Defensa valiente (límite)",

        "122_CollapseAdd_OnceCheckBox_Aplicar formación",
        "CollapseAdd_RichTextView_<font color='green'>Exportar / Importar:",
        "123_CollapseAdd_FormationCopy_Copiar código",
        "124_CollapseAdd_InputText_Pegar código",
        "125_CollapseAdd_OnceCheckBox_Aplicar código",
        "CollapseAdd_RichTextView_<font color='yellow'>#Puedes pausar el juego para colocar la formación",


    };

    int featuresCount = std::size(features);
    jobjectArray ret = env->NewObjectArray(featuresCount, env->FindClass("java/lang/String"), nullptr);
    for (int i = 0; i < featuresCount; ++i) {
        env->SetObjectArrayElement(ret, i, StringToJString(env, features[i]));
    }
    return ret;
}

extern "C" JNIEXPORT jobjectArray JNICALL Java_com_android_support_CkHomuraMenu_SettingsList(JNIEnv *env, jobject thiz) {
    static constexpr const char *features[] = {
        "Category_Configuración del Menú",
        "-1_Toggle_Guardar al salir", //-1 is checked on Preferences.java
        "Category_Más opciones",
        "-6_Button_<font color='green'>Volver al menú</font>",
    };

    int featuresCount = std::size(features);
    jobjectArray ret = env->NewObjectArray(featuresCount, env->FindClass("java/lang/String"), nullptr);
    for (int i = 0; i < featuresCount; ++i) {
        env->SetObjectArrayElement(ret, i, StringToJString(env, features[i]));
    }
    return ret;
}

// Generar cadena de código de formación
static std::string generateLineupStr(const std::multimap<int, int> &theMap) {
    std::ostringstream ss1; // Macetas y Nenúfares
    std::ostringstream ss2; // Plantas normales
    std::ostringstream ss3; // Calabazas

    auto it = theMap.cbegin();
    auto end = theMap.cend();
    while (it != end) {
        const auto &[key, value] = *it;
        int aSeedType = key & 0x3F;
        std::ostringstream *ssPtr;
        switch (aSeedType) {
            case SeedType::SEED_LILYPAD:
            case SeedType::SEED_FLOWERPOT:
                ssPtr = &ss1;
                break;
            case SeedType::SEED_PUMPKINSHELL:
                ssPtr = &ss3;
                break;
            default:
                ssPtr = &ss2;
                break;
        }
        bool wakeUp = (key >> 6) & 1;
        bool imitaterMorphed = (key >> 7) & 1;
        bool ladder = (key >> 8) & 1;
        int aPlantCol = value & 0x0F;
        int aRow = value >> 4;
        *ssPtr << aSeedType << ' ';
        if (wakeUp) {
            *ssPtr << 'W';
        }
        if (imitaterMorphed) {
            *ssPtr << 'I';
        }
        if (ladder) {
            *ssPtr << 'L';
        }
        if (wakeUp || imitaterMorphed || ladder) {
            *ssPtr << ' ';
        }
        *ssPtr << aPlantCol << ',' << aRow;
        while ((++it != end) && (it->first == key)) {
            int v = it->second;
            int col = v & 0x0F;
            int row = v >> 4;
            *ssPtr << ' ' << col << ',' << row;
        }
        *ssPtr << " ; ";
    }

    ss1 << ss2.view() << ss3.view();
    return std::move(ss1).str();
}

extern "C" JNIEXPORT jstring JNICALL Java_com_android_support_CkHomuraMenu_GetCurrentFormation(JNIEnv *env, jobject thiz) {
    Board *aBoard = (*gLawnApp_Addr)->mBoard;
    if (aBoard == nullptr) {
        return StringToJString(env, "");
    }

    std::multimap<int, int> map;
    for (Plant *aPlant = nullptr; aBoard->IteratePlants(aPlant);) {
        if (aPlant->mDead) {
            continue;
        }
        SeedType aSeedType = aPlant->mSeedType;
        SeedType aImitaterType = aPlant->mImitaterType;
        if (aSeedType == SeedType::SEED_IMITATER) {
            aSeedType = aImitaterType;
        }
        int aPlantCol = aPlant->mPlantCol;
        int aRow = aPlant->mRow;
        bool aIsAsleep = aPlant->mIsAsleep;
        bool canHaveLadder = aSeedType == SeedType::SEED_WALLNUT || aSeedType == SeedType::SEED_TALLNUT || aSeedType == SeedType::SEED_PUMPKINSHELL;
        bool canBeAsleep = Plant::IsNocturnal(aSeedType);
        bool wakeUp = canBeAsleep && !aIsAsleep;
        bool imitaterMorphed = aSeedType == SeedType::SEED_IMITATER || aImitaterType == SeedType::SEED_IMITATER;
        bool ladder = canHaveLadder && (aBoard->GetLadderAt(aPlantCol, aRow) != nullptr);

        int key = aSeedType | (wakeUp << 6) | (imitaterMorphed << 7) | (ladder << 8);
        int value = aPlantCol | (aRow << 4);
        map.emplace(key, value);
    }
    return StringToJString(env, generateLineupStr(map));
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_native1PButtonDown(JNIEnv *env, jclass clazz, jint code) {
    Board *aBoard = (*gLawnApp_Addr)->mBoard;
    if (aBoard) {
        gButtonDownP1 = true;
        gButtonCodeP1 = GamepadButton(code);
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_native2PButtonDown(JNIEnv *env, jclass clazz, jint code) {
    Board *aBoard = (*gLawnApp_Addr)->mBoard;
    if (aBoard) {
        gButtonDownP2 = true;
        gButtonCodeP2 = GamepadButton(code);
    }
}


extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeSwitchTwoPlayerMode(JNIEnv *env, jclass clazz, jboolean is_on) {
    isKeyboardTwoPlayerMode = doKeyboardTwoPlayerDialog = is_on;
    if (is_on) {
        return;
    }
    LawnApp *anApp = *gLawnApp_Addr;
    Board *aBoard = anApp->mBoard;
    if (anApp->IsCoopMode() || anApp->mGameMode == GameMode::GAMEMODE_MP_VS) {
        return;
    }
    anApp->ClearSecondPlayer();
    if (aBoard) {
        aBoard->mGamepadControls2->mPlayerIndex2 = -1;
    }
    anApp->mTwoPlayerState = -1;
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeIsInGame(JNIEnv *env, jclass clazz) {
    LawnApp *anApp = *gLawnApp_Addr;
    Board *aBoard = anApp->mBoard;
    auto *aFocusWidget = anApp->mWidgetManager->mFocusWidget;
    if (aBoard && aFocusWidget == reinterpret_cast<Sexy::Widget *>(aBoard)) {
        return true;
    }
    SeedChooserScreen *aSeedChooser = anApp->mSeedChooserScreen;
    if (anApp->IsCoopMode() && aSeedChooser && (aFocusWidget == reinterpret_cast<Sexy::Widget *>(aSeedChooser))) {
        return true;
    }
    if (anApp->IsVSMode() && anApp->mVSSetupScreen && (anApp->mVSSetupScreen->mState == VS_SETUP_SIDES || anApp->mVSSetupScreen->mState == VS_CUSTOM_BATTLE)) {
        return true;
    }

    return false;
}


extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeSendButtonEvent(JNIEnv *env, jclass clazz, jboolean is_key_down, jint button_code) {
    bool aIsPlayer2 = button_code >= 256;
    bool aGamepad1Is2P = gGamepad1ToPlayerIndex == 1;
    GamepadButton aButtonCode = GamepadButton(aIsPlayer2 ? button_code - 256 : button_code);

    LawnApp *anApp = *gLawnApp_Addr;
    Board *aBoard = anApp->mBoard;
    auto *aFocusWidget = anApp->mWidgetManager->mFocusWidget;

    if (!aBoard || aFocusWidget != reinterpret_cast<Sexy::Widget *>(aBoard)) {
        SeedChooserScreen *aSeedChooser = anApp->mSeedChooserScreen;
        if (is_key_down && anApp->IsCoopMode() && aSeedChooser && aFocusWidget == reinterpret_cast<Sexy::Widget *>(aSeedChooser)) {
            gButtonDownSeedChooser = true;
            gButtonCode = aButtonCode;
            gGamePlayerIndex = aIsPlayer2 ? 1 : 0;
            return;
        }

        VSSetupMenu *aVSSetup = anApp->mVSSetupScreen;
        if (is_key_down && anApp->IsVSMode() && aVSSetup && (aVSSetup->mState == VS_SETUP_SIDES || aVSSetup->mState == VS_CUSTOM_BATTLE)) {
            gButtonDownVSSetup = true;
            gButtonCode = aButtonCode;
            gGamePlayerIndex = aIsPlayer2 ? 1 : 0;
            return;
        }
        return;
    }

    float &aX = (aIsPlayer2 == true) ? gGamepadP2VelX : gGamepadP1VelX;
    float &aY = (aIsPlayer2 == true) ? gGamepadP2VelY : gGamepadP1VelY;
    if (is_key_down) {
        switch (aButtonCode) {
            case GamepadButton::BUTTONCODE_B:
                gKeyDown = true;
                gGamePlayerIndex = aIsPlayer2 ? 1 : 0;
                break;
            case GamepadButton::BUTTONCODE_UP:
                aY = -400;
                break;
            case GamepadButton::BUTTONCODE_DOWN:
                aY = 400;
                break;
            case GamepadButton::BUTTONCODE_LEFT:
                aX = -400;
                break;
            case GamepadButton::BUTTONCODE_RIGHT:
                aX = 400;
                break;
            default:
                gButtonDown = true;
                gButtonCode = aButtonCode;
                if (aGamepad1Is2P) {
                    gGamePlayerIndex = aIsPlayer2 ? 0 : 1;
                } else {
                    gGamePlayerIndex = aIsPlayer2 ? 1 : 0;
                }
                break;
        }
    } else {
        switch (aButtonCode) {
            case GamepadButton::BUTTONCODE_UP:
            case GamepadButton::BUTTONCODE_DOWN:
                aY = 0;
                break;
            case GamepadButton::BUTTONCODE_LEFT:
            case GamepadButton::BUTTONCODE_RIGHT:
                aX = 0;
                break;
            default:
                break;
        }
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeAutoFixPosition(JNIEnv *env, jclass clazz) {
    positionAutoFix = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeUseXboxMusics(JNIEnv *env, jclass clazz) {
    useXboxMusic = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeSeedBankPin(JNIEnv *env, jclass clazz) {
    seedBankPin = true;
}
extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeDynamicPreview(JNIEnv *env, jclass clazz) {
    dynamicPreview = true;
}
extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeEnableOpenSL(JNIEnv *env, jclass clazz) {
    InitOpenSL();
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeJumpLogo(JNIEnv *env, jclass clazz) {
    jumpLogo = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeMoreZombieSeeds(JNIEnv *env, jclass clazz) {
    gMoreZombieSeeds = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeVSBalanceAdjustment(JNIEnv *env, jclass clazz) {
    gVSBalanceAdjustment = true;
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeIntroVideoCompleted(JNIEnv *env, jclass clazz) {
    TitleScreen *aTitleScreen = (*gLawnApp_Addr)->mTitleScreen;
    if (aTitleScreen != nullptr) {
        aTitleScreen->mVideoCompleted = true;
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_transmension_mobile_EnhanceActivity_nativeHeavyWeaponAccel(JNIEnv *env, jclass clazz) {
    heavyWeaponAccel = true;
}
