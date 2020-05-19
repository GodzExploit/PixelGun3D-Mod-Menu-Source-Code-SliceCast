#include <pthread.h>
#include <jni.h>
#include <memory.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include <Includes/Vector3.hpp>
#include <Includes/Unity.h>
#include <Includes/Utils.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <bits/sysconf.h>
#include <sys/mman.h>

#include "Includes/Logger.h"
#include "Patching/Patch.h"
#import "Includes/Utils.h"

#if defined(__aarch64__)
#include "X64Hook/And64InlineHook.hpp"
#else
#include "Substrate/CydiaSubstrate.h"
#endif

bool chams = false;
bool grenadere = false;
bool ImVisible = false;
bool imInvisible = false;
bool norecoil = false;
bool healthregen = false;
bool recoverhp = false;
bool damage = false;
bool suicide = false;
bool bulletexplode = false;
bool shotgun = false;
bool railgun = false;
bool reflectdamage = false;
bool ammo = false;
bool firerate = false;
bool markenemy = false;
bool rocketfire = false;
bool nospread = false;
bool markenemyshot = false;
bool damageReflectionValue = false;
bool telekill = false;
bool addcoins = false;
bool addgems = false;
int fovModifier = 70;
int level = 1;
const char* spinnerExampleString;

struct Patches{
    Patch *HealthRegeneration;
    Patch *RecoilDisabled;
    Patch *GrenadeReload;
    Patch *RecoverDamage;
    Patch *WeaponDamage;
    Patch *currentammoinclip;
    Patch *rapidfire;
    Patch *BulletSpread;
}patch;

class color{
public:
    float r;
    float g;
    float b;

    color(float r,float g,float b){
        this->r = r;
        this->g = g;
        this->b = b;

    }
};

bool PlayerMoveCHookUpdateInitialized = false;
bool weapsoundsHookInitialized = false;
bool BankControllerHookInitialized = false;
const char* libName = "libil2cpp.so";

void octo_hook(void *orig_fcn, void* new_fcn, void **orig_fcn_ptr)
{
#if defined(__aarch64__)
    A64HookFunction(orig_fcn, new_fcn, orig_fcn_ptr);
#else
    MSHookFunction(orig_fcn, new_fcn, orig_fcn_ptr);
#endif
}

void *(*Component_GetTransform)(void* component) = (void *(*)(void* ))getAbsoluteAddress(libName, 0xE6EBD8); // Component$$get_transform
void *(*Component_GetGameObject)(void* component) = (void *(*)(void* ))getAbsoluteAddress(libName, 0xE6EC68); // Component$$get_gameObject

Vector3 (*Transform_get_position)(void* transform) = (Vector3 (*)(void *))getAbsoluteAddress(libName, 0x38E6234); // Transform$$get_position
void (*Transform_Set_Position)(void* transform, Vector3 pos) = (void (*)(void*, Vector3))getAbsoluteAddress(libName, 0x3A87434); // Transform$$set_position
void (*Transform_Get_parent)(void* transform) = (void (*)(void*))getAbsoluteAddress(libName, 0x3A884D4); // Transform$$get_parent
//void (*Transform_Set_Rotation)(void* transform, Quaternion pos) = (void (*)(void*, Quaternion))getRealOffset(0x2159EC0);
//Quaternion (*Transform_Get_Rotation)(void* transform) = (Quaternion (*)(void*))getRealOffset(0x2159E30);
void (*Transform_LookAt)(void* transform, Vector3 pos) = (void (*)(void*, Vector3))getAbsoluteAddress(libName, 0x3A89368); // public void LookAt(Vector3 worldPosition)
void (*Transform_Rotate)(void* transform, Vector3 pos) = (void (*)(void*, Vector3))getAbsoluteAddress(libName, 0x3A88DC8); // public void Rotate(Vector3 eulers)
void (*Transform_set_localScale)(void* transform, Vector3 pos) = (void (*)(void*, Vector3))getAbsoluteAddress(libName, 0x3A88434); // Transform$$set_localScale
Vector3 (*Transform_get_localScale)(void* transform) = (Vector3 (*)(void *))getAbsoluteAddress(libName, 0x3A8836C); // Transform$$get_localScale

void (*GameObject_set_active)(void* obj, bool) = (void (*)(void* obj, bool))getAbsoluteAddress(libName, 0x39241F4); // GameObject$$set_active
//void *(*PhotonNetwork_InstantiateSceneObject)(void* _this,void* prefabName, Vector3 position, Quaternion rotation, int group, void*) = (void *(*)(void*, void*, Vector3, Quaternion, int, void*))getRealOffset(0x947E7C); // PhotonNetwork$$InstantiateSceneObject

bool isEnemy(void* me, void* it)
{
    bool (*isEnemy)(void *me, void *it) =
    (bool(*)(void *, void *))getAbsoluteAddress(libName, 0x18B44BC); // PlayerDamageable$$IsEnemyTo
    return isEnemy(me, it);
}
bool isDead(void* it)
{
    bool (*IsDead)(void *me) =
    (bool(*)(void *))getAbsoluteAddress(libName, 0x18B47E4); // PlayerDamageable$$IsDead
    return IsDead(it);
}

void* get_weaponManagerStatic()
{
    void *var = *(void **) getAbsoluteAddress(libName, 0x4D1D724); // Class$WeaponManager
    if (var)
    {
        void* var1 = *(void **)((uint64_t)var + 0x5C);
        return var1;
    }
}

void* sharedWeaponManager()
{
    void *var = get_weaponManagerStatic();
    if (var)
    {
        void* var1 = *(void **)((uint64_t)var + 0x108);
        return var1;
    }
}

void* get_MyPlayer()
{
    void *var = sharedWeaponManager();
    if (var)
    {
        void* var1 = *(void **)((uint64_t)var + 0x24);
        return var1;
    }
}

void* get_PlayerTransform(void* ply)
{
    void *var = ply;
    if (var)
    {
        void* var1 = *(void **)((uint64_t)var + 0x1B4);
        return var1;
    }
}

Vector3 get_PlayerPosition()
{
    void *transform = get_PlayerTransform(get_MyPlayer());
    if (transform)
    {
        return Transform_get_position(Component_GetTransform(transform));
    }
}

void* get_PlayerDamageable(void* ply)
{
    void *var = ply;
    if (var)
    {
        void* var1 = *(void **)((uint64_t)var + 0x318);
        return var1;
    }
}

void* get_PlayerNetworkStartTable(void* ply)
{
    void *var = ply;
    if (var)
    {
        void* var1 = *(void **)((uint64_t)var + 0x324);
        return var1;
    }
}

monoList<void **>* get_PlayerList()
{
    monoList<void **> *(*PlayerList)(void *NetworkStartTable) =
    (monoList<void **>* (*)(void *))getAbsoluteAddress(libName, 0x19757A0);
    //LOGD("2");
    LOGD("Player List check");
    if (PlayerList(get_PlayerNetworkStartTable(get_MyPlayer()))){
        LOGD("Player List Good #1");
    }
    else
    {
        LOGD("Player List Bad #1");
    }
    return PlayerList(PlayerList(get_PlayerNetworkStartTable(get_MyPlayer())));
}

void TeleKill()
{
    if (telekill)
    {
        void* myTransform = get_PlayerTransform(get_MyPlayer());

        monoList<void **> *playersList = get_PlayerList();
        for (int i = 0; i < playersList->getSize(); i++)
        {

            void *player = playersList->getItems()[i];
            void *themtransform = get_PlayerTransform(player);
            void* damagable = get_PlayerDamageable(player);

            if (isEnemy(damagable, get_MyPlayer()) && !isDead(damagable))
            {
                Vector3 themPos = Transform_get_position(Component_GetTransform(themtransform));
                //Vector3 myPlayerPos = Transform_get_position(Component_GetTransform(myTransform));

                Vector3 myPos = themPos;
                myPos.Z += 3;
                Transform_Set_Position(Component_GetTransform(myTransform), myPos);
            }
        }
    }
}

void(*old_PlayerMoveCUpdate)(void *instance);
void PlayerMoveCUpdate(void *instance) {
    if(instance != NULL) {
        {
            *(int *) ((uint64_t) instance + 0x328) = 203219142;
        }
        if(!PlayerMoveCHookUpdateInitialized){
            PlayerMoveCHookUpdateInitialized = true;
            LOGI("GameManager_LateUpdate hooked");
        }
        if (chams) {
            *(bool *) ((uint64_t) instance + 0x3A8) = true;
        }
        if (imInvisible) {
            *(bool *) ((uint64_t) instance + 0x2BA) = true;
        }
        if (ImVisible) {
            *(bool *) ((uint64_t) instance + 0x2B7) = false;
        }
        if (suicide) {
            *(bool *) ((uint64_t) instance + 0x76C) = true;
        }

    }
    old_PlayerMoveCUpdate(instance);
}

void(*old_WeapSoundsUpdate)(void *instance);
void WeapSoundsUpdate(void *instance) {
    if(instance != NULL) {
        if (telekill)
        {
            std::thread t(TeleKill); t.join();
        }
        if(!weapsoundsHookInitialized){
            weapsoundsHookInitialized = true;
            LOGI("GameManager_LateUpdate hooked");
        }
        if (bulletexplode) {
            *(bool *) ((uint64_t) instance + 0x105) = true;
        }
        if (shotgun) {
            *(bool *) ((uint64_t) instance + 0x106) = true;
        }
        if (reflectdamage) {
            *(int *) ((uint64_t) instance + 0x209) = 50;
        }
        if (railgun) {
            *(bool *) ((uint64_t) instance + 0x110) = true;
        }
        if (markenemy) {
            *(bool *) ((uint64_t) instance + 0x179) = true;
        }
        if (rocketfire) {
            *(bool *) ((uint64_t) instance + 0xA4) = true;
        }
        if (damageReflectionValue) {
            *(float *) ((uint64_t) instance + 0x20C) = 200.0;
        }
        if (markenemyshot) {
            *(bool *) ((uint64_t) instance + 0x17B) = true;
        }

    }
    old_WeapSoundsUpdate(instance);
}

void (*AddGems)(void* _this, int amount, bool countToEarned);

void(*old_BankController)(void *instance);
void BankController(void *instance) {
    if(instance != NULL) {
        if (addcoins)
        {
            void (*AddCoins)(void* _this, int, bool, int) =
            (void (*)(void* _this, int, bool, int))getAbsoluteAddress(libName, 0x211ADEC); // BankController$$AddCoins

            AddCoins(instance, 100, true, 1);
            addcoins = !AddCoins;
        }
        if(!BankControllerHookInitialized){
            BankControllerHookInitialized = true;
            LOGI("GameManager_LateUpdate hooked");
            if (addgems) {
                AddGems(instance, 100, true);
            }
        }

    }
    old_BankController(instance);
}

float (*old_get_fieldOfView)(void *instance);
float get_fieldOfView(void *instance) {
    if (instance != NULL && fovModifier > 1) {
        return (float)fovModifier;
    }
    return old_get_fieldOfView(instance);
}

int (*old_getlevel)(void *instance);
int getlevel(void *instance) {
    if (instance != NULL && level > 1) {
        return (int)level;
    }
    return old_getlevel(instance);
}


// we will run our patches in a new thread so our while loop doesn't block process main thread
void* hack_thread(void*) {
    LOGI("I have been loaded. Mwuahahahaha");
    // loop until our target library is found
    do {
        sleep(1);
    } while (!isLibraryLoaded(libName));
    LOGI("I found the il2cpp lib. Address is: %p", (void*)findLibrary(libName));
    LOGI("Hooking GameManager_LateUpdate");
    AddGems = (void (*)(void*, int, bool))getAbsoluteAddress(libName, 0x211B030);
    octo_hook((void*)getAbsoluteAddress(libName, 0x1248414), (void*)PlayerMoveCUpdate, (void**)&old_PlayerMoveCUpdate);
    octo_hook((void*)getAbsoluteAddress(libName, 0x16025CC), (void*)WeapSoundsUpdate, (void**)&old_WeapSoundsUpdate);
    octo_hook((void*)getAbsoluteAddress(libName, 0xE6A66C), (void*)get_fieldOfView, (void**)&old_get_fieldOfView);
    octo_hook((void*)getAbsoluteAddress(libName, 0x17BC674), (void*)getlevel, (void**)&old_getlevel);
    octo_hook((void*)getAbsoluteAddress(libName, 0x21169A8), (void*)BankController, (void**)&old_BankController);
    patch.HealthRegeneration = Patch::Setup((void*)getAbsoluteAddress(libName, 0x22F0A3C), (char*)"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1", 8);
    patch.RecoilDisabled = Patch::Setup((void*)getAbsoluteAddress(libName, 0x22F6C1C), (char*)"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1", 8);
    patch.GrenadeReload = Patch::Setup((void*)getAbsoluteAddress(libName, 0x22F6CF0), (char*)"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1", 8);
    patch.RecoverDamage = Patch::Setup((void*)getAbsoluteAddress(libName, 0x22F6DC4), (char*)"\x01\x00\xa0\xe3\x1e\xff\x2f\xe1", 8);
    patch.WeaponDamage = Patch::Setup((void*)getAbsoluteAddress(libName, 0x126388C), (char*)"\xFA\x04\x44\xE3\x1E\xFF\x2F\xE1", 8);
    patch.currentammoinclip = Patch::Setup((void*)getAbsoluteAddress(libName, 0x1FD57CC), (char*)"\xDC\x0F\x0F\xE3\x1e\xff\x2f\xe1", 8);
    patch.rapidfire = Patch::Setup((void*)getAbsoluteAddress(libName, 0x14C592C), (char*)"\xFA\x04\x44\xE3\x1E\xFF\x2F\xE1", 8);
    patch.BulletSpread = Patch::Setup((void*)getAbsoluteAddress(libName, 0x1287BEC), (char*)"\x00\x00\xA0\xE3\x1E\xFF\x2F\xE1", 8);
    return NULL;
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_dark_force_NativeLibrary_getListFT(JNIEnv *env, jclass jobj){
    jobjectArray ret;
    int i;
    const char *features[]= {"Chams", "Invisible Mod", "No Recoil", "No Spread", "Drop Grenade on reload", "Health Regeneration", "Recover HP by damage", "Damage - JNI", "Suicide Test", "Ammo - JNI", "Firerate - JNI", "Shotgun Shot", "Enemy Takes Damage by shooting at you", "Exploding Bullets", "Laser Shot", "Rocket Shot", "Mark Enemy", "Telekill test", "Coins", "Gems", "SeekBar_FOV Modifier_0_250", "SeekBar_Level Modifier_0_65"};
    int Total_Feature = (sizeof features / sizeof features[0]); //Now you dont have to manually update the number everytime
    ret= (jobjectArray)env->NewObjectArray(Total_Feature,
                                           env->FindClass("java/lang/String"),
                                           env->NewStringUTF(""));

    for(i=0;i<Total_Feature;i++) {
        env->SetObjectArrayElement(
                ret,i,env->NewStringUTF(features[i]));
    }
    return(ret);
}


extern "C"
JNIEXPORT void JNICALL Java_com_dark_force_NativeLibrary_changeToggle(JNIEnv *env, jclass thisObj, jint number) {
    int i = (int) number;
    switch (i) {
        case 0:
            chams = !chams;
            break;
        case 1:
            imInvisible = !imInvisible;
            ImVisible = !ImVisible;
            break;
        case 2:
            norecoil = !norecoil;
            if (norecoil) {
                patch.RecoilDisabled->Apply();
            } else {
                patch.RecoilDisabled->Reset();
            }
            break;
        case 3:
            nospread = !nospread;
            if (nospread) {
                patch.BulletSpread->Apply();
            } else {
                patch.BulletSpread->Reset();
            }
            break;
        case 4:
            grenadere = !grenadere;
            if (grenadere) {
                patch.GrenadeReload->Apply();
            } else {
                patch.GrenadeReload->Reset();
            }
            break;
        case 5:
            healthregen = !healthregen;
            if (healthregen) {
                patch.HealthRegeneration->Apply();
            } else {
                patch.HealthRegeneration->Reset();
            }
            break;
        case 6:
            recoverhp = !recoverhp;
            if (recoverhp) {
                patch.RecoverDamage->Apply();
            } else {
                patch.RecoverDamage->Reset();
            }
            break;
        case 7:
            damage = !damage;
            if (damage) {
                patch.WeaponDamage->Apply();
            } else {
                patch.WeaponDamage->Reset();
            }
            break;
        case 8:
            suicide = !suicide;
            break;
        case 9:
            ammo = !ammo;
            if (ammo) {
                patch.currentammoinclip->Apply();
            } else {
                patch.currentammoinclip->Reset();
            }
            break;
        case 10:
            firerate = !firerate;
            if (firerate) {
                patch.rapidfire->Apply();
            } else {
                patch.rapidfire->Reset();
            }
            break;
        case 11:
            shotgun = !shotgun;
            break;
        case 12:
            reflectdamage = !reflectdamage;
            damageReflectionValue = !damageReflectionValue;
            break;
        case 13:
            bulletexplode = !bulletexplode;
            break;
        case 14:
            railgun = !railgun;
            break;
        case 15:
            rocketfire = !rocketfire;
            break;
        case 16:
            markenemy = !markenemy;
            markenemyshot = !markenemyshot;
            break;
        case 17:
            telekill = !telekill;
            break;
        case 18:
            addcoins = !addcoins;
            break;
        case 19:
            addgems = !addgems;
            break;
        default:
            break;
    }
    return;
}


extern "C"
JNIEXPORT void JNICALL Java_com_dark_force_NativeLibrary_init(JNIEnv * env, jclass obj, jobject thiz){
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);

    //Add our toast in here so it wont be easy to change by simply editing the smali and cant
    //be cut out because this method is needed to start the hack (I'm smart)
    jstring jstr = env->NewStringUTF("Mod-Menu by Slice Cast"); //Edit this text to your desired toast message!
    jclass toast = env->FindClass("android/widget/Toast");
    jmethodID methodMakeText =
            env->GetStaticMethodID(
                    toast,
                    "makeText",
                    "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
    if (methodMakeText == NULL) {
        LOGE("toast.makeText not Found");
        return;
    }
    //The last int is the length on how long the toast should be displayed
    //0 = Short, 1 = Long
    jobject toastobj = env->CallStaticObjectMethod(toast, methodMakeText,
                                                      thiz, jstr, 1);

    jmethodID methodShow = env->GetMethodID(toast, "show", "()V");
    if (methodShow == NULL) {
        LOGE("toast.show not Found");
        return;
    }
    env->CallVoidMethod(toastobj, methodShow);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dark_force_NativeLibrary_changeSeekBar(JNIEnv *env, jclass clazz, jint i, jint seekbarValue) {
    int li = (int) i;
    switch (li) {
        case 20:
            fovModifier = seekbarValue;
            break;
        case 21:
            level = seekbarValue;
            break;
        default:
            break;
    }
    return;
}


