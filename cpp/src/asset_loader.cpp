#include <planet/asset_manager.hpp>

#include <felspar/exceptions.hpp>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <SDL.h>


namespace {
    jclass jAsset = {};
    jobject jAssetManager = {};
    jmethodID loader = {};

    AAssetManager *native_manager = nullptr;
}


extern "C" JNIEXPORT void JNICALL
        Java_com_blue5alamander_planet_android_Asset_useManager(
                JNIEnv *env, jobject, jclass am) {
    jAsset = reinterpret_cast<jclass>(env->NewGlobalRef(
            env->FindClass("com/blue5alamander/planet/android/Asset")));
    jAssetManager = env->NewGlobalRef(am);
    loader = env->GetStaticMethodID(
            jAsset, "loader",
            "(Landroid/content/res/AssetManager;Ljava/lang/String;)[B");
    native_manager = AAssetManager_fromJava(env, jAssetManager);
}


namespace {


    const struct jassetloader : public planet::asset_loader {
        std::optional<std::vector<std::byte>> try_load(
                std::ostream &log,
                std::filesystem::path const &fn,
                felspar::source_location const &loc) const override {
            JNIEnv *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
            if (not jAsset or not jAssetManager) {
                throw felspar::stdexcept::runtime_error{
                        "useManager has not been called during application "
                        "start up"};
            }
            auto const assetfn = std::filesystem::path{"share"} / fn;
            if (native_manager) {
                AAsset *asset =
                        AAssetManager_open(native_manager, assetfn.c_str(), {});
                if (asset) {
                    std::vector<std::byte> buffer(AAsset_getLength64(asset));
                    std::memcpy(
                            buffer.data(), AAsset_getBuffer(asset),
                            buffer.size());
                    AAsset_close(asset);
                    return buffer;
                } else {
                    log << "Asset could not be loaded from the Android native "
                           "asset manager\n";
                    return {};
                }
            } else {
                jstring asset{reinterpret_cast<jstring>(
                        env->NewLocalRef(env->NewStringUTF(assetfn.c_str())))};
                jobject load_result(env->CallStaticObjectMethod(
                        jAsset, loader, jAssetManager, asset));
                jbyteArray *bytes(reinterpret_cast<jbyteArray *>(&load_result));
                if (*bytes == nullptr) {
                    log << "Asset could not be loaded from the Java asset "
                           "manager\n";
                    return {};
                } else {
                    std::size_t const length = env->GetArrayLength(*bytes);
                    std::vector<std::byte> buffer(length);
                    env->GetByteArrayRegion(
                            *bytes, 0, length,
                            reinterpret_cast<jbyte *>(buffer.data()));
                    return buffer;
                }
            }
        }
    } g_loader;


}
