// PENTING: file zygisk.hpp di folder ini adalah PLACEHOLDER.
// Header asli di-download otomatis oleh CI (lihat .github/workflows/build.yml)
// dari repo resmi topjohnwu/zygisk-module-sample sebelum compile.
// Kalau build lokal manual, ganti dulu file zygisk.hpp dengan versi resmi.
#include "zygisk.hpp"

#include <jni.h>
#include <string>
#include <cstring>
#include <android/log.h>

#define LOG_TAG "IPadZygisk"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ModuleBase;

// ==== Ganti sesuai model iPad target ====
static const char* FAKE_MANUFACTURER = "Apple";
static const char* FAKE_BRAND = "Apple";
static const char* FAKE_MODEL = "iPad13,8";       // iPad Pro 12.9" gen 6
static const char* FAKE_DEVICE = "J620AP";
static const char* FAKE_PRODUCT = "J620AP";
static const char* FAKE_HARDWARE = "t8110";
static const char* FAKE_BOARD = "J620AP";
static const char* FAKE_FINGERPRINT =
        "Apple/iPad13,8/J620AP:16.0/20A5328h/user/release-keys";

// ==== Daftar package yang mau kena spoof. Kosongkan = semua app ====
static const char* TARGET_PACKAGES[] = {
        // "com.contoh.game1",
};
static const int TARGET_COUNT = sizeof(TARGET_PACKAGES) / sizeof(TARGET_PACKAGES[0]);

static void setBuildField(JNIEnv *env, jclass buildClass, const char *fieldName, const char *value) {
    jfieldID fid = env->GetStaticFieldID(buildClass, fieldName, "Ljava/lang/String;");
    if (fid == nullptr) {
        env->ExceptionClear();
        LOGD("field %s tidak ditemukan di Build.class", fieldName);
        return;
    }
    jstring jval = env->NewStringUTF(value);
    env->SetStaticObjectField(buildClass, fid, jval);
}

class IPadZygiskModule : public ModuleBase {
public:
    void onLoad(Api *apiRef, JNIEnv *envRef) override {
        api = apiRef;
        env = envRef;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *nice_name_c = env->GetStringUTFChars(args->nice_name, nullptr);
        std::string pkg(nice_name_c ? nice_name_c : "");
        env->ReleaseStringUTFChars(args->nice_name, nice_name_c);

        bool shouldHook = true;
        if (TARGET_COUNT > 0) {
            shouldHook = false;
            for (int i = 0; i < TARGET_COUNT; i++) {
                if (pkg == TARGET_PACKAGES[i]) {
                    shouldHook = true;
                    break;
                }
            }
        }

        if (!shouldHook) {
            // lepas dari proses ini, modul cuma numpang di app yang ditarget
            api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        jclass buildClass = env->FindClass("android/os/Build");
        if (buildClass == nullptr) {
            env->ExceptionClear();
            LOGD("gagal FindClass android/os/Build di %s", pkg.c_str());
            return;
        }

        setBuildField(env, buildClass, "MANUFACTURER", FAKE_MANUFACTURER);
        setBuildField(env, buildClass, "BRAND", FAKE_BRAND);
        setBuildField(env, buildClass, "MODEL", FAKE_MODEL);
        setBuildField(env, buildClass, "DEVICE", FAKE_DEVICE);
        setBuildField(env, buildClass, "PRODUCT", FAKE_PRODUCT);
        setBuildField(env, buildClass, "HARDWARE", FAKE_HARDWARE);
        setBuildField(env, buildClass, "BOARD", FAKE_BOARD);
        setBuildField(env, buildClass, "FINGERPRINT", FAKE_FINGERPRINT);

        LOGD("spoof iPad diterapkan ke %s", pkg.c_str());
    }

private:
    Api *api = nullptr;
    JNIEnv *env = nullptr;
};

REGISTER_ZYGISK_MODULE(IPadZygiskModule)
