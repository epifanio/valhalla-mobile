
#include <valhalla/worker.h>
#include "main.h"
#include "valhalla_actor.h"

#ifdef __ANDROID__
// The Android JNI interface uses a different function signature.
#include <jni.h>

extern "C"
JNIEXPORT jstring

JNICALL
Java_com_valhalla_valhalla_ValhallaKotlin_route(JNIEnv *env,
                                                jobject thiz,
                                                jstring jRequest,
                                                jstring jConfigPath) {
    
    const char *request = env->GetStringUTFChars(jRequest, 0);
    const char *config_path = env->GetStringUTFChars(jConfigPath, 0);

    std::string result;
    try {
        // TODO: Android currently creates a new actor every time. Optimize to be like iOS later.
        ValhallaActor valhallaActor(config_path);
        result = valhallaActor.route(request);
    } catch (const valhalla::valhalla_exception_t &err) {
        printf("[ValhallaActor] route valhalla_exception: %s\n", err.what());
        std::string code = std::to_string(err.code);
        std::string message = err.message.c_str();

        result = "{\"code\":" + code + ",\"message\":\"" + message + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] route std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + std::string(err.what()) + "\"}";
    } catch (...) {
        printf("[ValhallaActor] route unknown exception");
        result = "{\"code\":-1,\"message\":\"unknown exception\"}";
    }

    env->ReleaseStringUTFChars(jRequest, request);
    env->ReleaseStringUTFChars(jConfigPath, config_path);

    return env->NewStringUTF(result.c_str());
}

extern "C"
JNIEXPORT jstring

JNICALL
Java_com_valhalla_valhalla_ValhallaKotlin_traceRoute(JNIEnv *env,
                                                     jobject thiz,
                                                     jstring jRequest,
                                                     jstring jConfigPath) {

    const char *request = env->GetStringUTFChars(jRequest, 0);
    const char *config_path = env->GetStringUTFChars(jConfigPath, 0);

    std::string result;
    try {
        // TODO: Android currently creates a new actor every time. Optimize to be like iOS later.
        ValhallaActor valhallaActor(config_path);
        result = valhallaActor.traceRoute(request);
    } catch (const valhalla::valhalla_exception_t &err) {
        printf("[ValhallaActor] trace_route valhalla_exception: %s\n", err.what());
        std::string code = std::to_string(err.code);
        std::string message = err.message.c_str();

        result = "{\"code\":" + code + ",\"message\":\"" + message + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] trace_route std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + std::string(err.what()) + "\"}";
    } catch (...) {
        printf("[ValhallaActor] trace_route unknown exception");
        result = "{\"code\":-1,\"message\":\"unknown exception\"}";
    }

    env->ReleaseStringUTFChars(jRequest, request);
    env->ReleaseStringUTFChars(jConfigPath, config_path);

    return env->NewStringUTF(result.c_str());
}

extern "C"
JNIEXPORT jstring

JNICALL
Java_com_valhalla_valhalla_ValhallaKotlin_traceAttributes(JNIEnv *env,
                                                          jobject thiz,
                                                          jstring jRequest,
                                                          jstring jConfigPath) {

    const char *request = env->GetStringUTFChars(jRequest, 0);
    const char *config_path = env->GetStringUTFChars(jConfigPath, 0);

    std::string result;
    try {
        // TODO: Android currently creates a new actor every time. Optimize to be like iOS later.
        ValhallaActor valhallaActor(config_path);
        result = valhallaActor.traceAttributes(request);
    } catch (const valhalla::valhalla_exception_t &err) {
        printf("[ValhallaActor] trace_attributes valhalla_exception: %s\n", err.what());
        std::string code = std::to_string(err.code);
        std::string message = err.message.c_str();

        result = "{\"code\":" + code + ",\"message\":\"" + message + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] trace_attributes std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + std::string(err.what()) + "\"}";
    } catch (...) {
        printf("[ValhallaActor] trace_attributes unknown exception");
        result = "{\"code\":-1,\"message\":\"unknown exception\"}";
    }

    env->ReleaseStringUTFChars(jRequest, request);
    env->ReleaseStringUTFChars(jConfigPath, config_path);

    return env->NewStringUTF(result.c_str());
}

extern "C"
JNIEXPORT jstring

JNICALL
Java_com_valhalla_valhalla_ValhallaKotlin_sourcesToTargets(JNIEnv *env,
                                                           jobject thiz,
                                                           jstring jRequest,
                                                           jstring jConfigPath) {

    const char *request = env->GetStringUTFChars(jRequest, 0);
    const char *config_path = env->GetStringUTFChars(jConfigPath, 0);

    std::string result;
    try {
        // TODO: Android currently creates a new actor every time. Optimize to be like iOS later.
        ValhallaActor valhallaActor(config_path);
        result = valhallaActor.sourcesToTargets(request);
    } catch (const valhalla::valhalla_exception_t &err) {
        printf("[ValhallaActor] sources_to_targets valhalla_exception: %s\n", err.what());
        std::string code = std::to_string(err.code);
        std::string message = err.message.c_str();

        result = "{\"code\":" + code + ",\"message\":\"" + message + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] sources_to_targets std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + std::string(err.what()) + "\"}";
    } catch (...) {
        printf("[ValhallaActor] sources_to_targets unknown exception");
        result = "{\"code\":-1,\"message\":\"unknown exception\"}";
    }

    env->ReleaseStringUTFChars(jRequest, request);
    env->ReleaseStringUTFChars(jConfigPath, config_path);

    return env->NewStringUTF(result.c_str());
}

extern "C"
JNIEXPORT jstring

JNICALL
Java_com_valhalla_valhalla_ValhallaKotlin_optimizedRoute(JNIEnv *env,
                                                         jobject thiz,
                                                         jstring jRequest,
                                                         jstring jConfigPath) {

    const char *request = env->GetStringUTFChars(jRequest, 0);
    const char *config_path = env->GetStringUTFChars(jConfigPath, 0);

    std::string result;
    try {
        // TODO: Android currently creates a new actor every time. Optimize to be like iOS later.
        ValhallaActor valhallaActor(config_path);
        result = valhallaActor.optimizedRoute(request);
    } catch (const valhalla::valhalla_exception_t &err) {
        printf("[ValhallaActor] optimized_route valhalla_exception: %s\n", err.what());
        std::string code = std::to_string(err.code);
        std::string message = err.message.c_str();

        result = "{\"code\":" + code + ",\"message\":\"" + message + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] optimized_route std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + std::string(err.what()) + "\"}";
    } catch (...) {
        printf("[ValhallaActor] optimized_route unknown exception");
        result = "{\"code\":-1,\"message\":\"unknown exception\"}";
    }

    env->ReleaseStringUTFChars(jRequest, request);
    env->ReleaseStringUTFChars(jConfigPath, config_path);

    return env->NewStringUTF(result.c_str());
}

#elif __APPLE__
void* create_valhalla_actor(const char *config_path, ValhallaMobileHttpClient* http_client) {
    return new ValhallaActor(config_path, http_client);
}

void delete_valhalla_actor(void* actor) {
    delete ((ValhallaActor*) actor);
}

std::string route(const char *request, void* actor) {
    std::string result;
    try {
        result = ((ValhallaActor*) actor)->route(request);
    } catch (const valhalla::valhalla_exception_t &err) {
        printf("[ValhallaActor] route valhalla_exception: %s\n", err.what());
        std::string code = std::to_string(err.code);
        std::string message = err.message.c_str();

        result = "{\"code\":" + code + ",\"message\":\"" + message + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] route std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + std::string(err.what()) + "\"}";
    } catch (...) {
        printf("[ValhallaActor] route unknown exception");
        result = "{\"code\":-1,\"message\":\"unknown exception\"}";
    }

    return result;
}

std::string trace_route(const char *request, void* actor) {
    std::string result;
    try {
        result = ((ValhallaActor*) actor)->traceRoute(request);
    } catch (const valhalla::valhalla_exception_t &err) {
        printf("[ValhallaActor] trace_route valhalla_exception: %s\n", err.what());
        std::string code = std::to_string(err.code);
        std::string message = err.message.c_str();

        result = "{\"code\":" + code + ",\"message\":\"" + message + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] trace_route std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + std::string(err.what()) + "\"}";
    } catch (...) {
        printf("[ValhallaActor] trace_route unknown exception");
        result = "{\"code\":-1,\"message\":\"unknown exception\"}";
    }

    return result;
}

std::string trace_attributes(const char *request, void* actor) {
    std::string result;
    try {
        result = ((ValhallaActor*) actor)->traceAttributes(request);
    } catch (const valhalla::valhalla_exception_t &err) {
        printf("[ValhallaActor] trace_attributes valhalla_exception: %s\n", err.what());
        std::string code = std::to_string(err.code);
        std::string message = err.message.c_str();

        result = "{\"code\":" + code + ",\"message\":\"" + message + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] trace_attributes std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + std::string(err.what()) + "\"}";
    } catch (...) {
        printf("[ValhallaActor] trace_attributes unknown exception");
        result = "{\"code\":-1,\"message\":\"unknown exception\"}";
    }

    return result;
}

std::string sources_to_targets(const char *request, void* actor) {
    std::string result;
    try {
        result = ((ValhallaActor*) actor)->sourcesToTargets(request);
    } catch (const valhalla::valhalla_exception_t &err) {
        printf("[ValhallaActor] sources_to_targets valhalla_exception: %s\n", err.what());
        std::string code = std::to_string(err.code);
        std::string message = err.message.c_str();

        result = "{\"code\":" + code + ",\"message\":\"" + message + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] sources_to_targets std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + std::string(err.what()) + "\"}";
    } catch (...) {
        printf("[ValhallaActor] sources_to_targets unknown exception");
        result = "{\"code\":-1,\"message\":\"unknown exception\"}";
    }

    return result;
}

std::string optimized_route(const char *request, void* actor) {
    std::string result;
    try {
        result = ((ValhallaActor*) actor)->optimizedRoute(request);
    } catch (const valhalla::valhalla_exception_t &err) {
        printf("[ValhallaActor] optimized_route valhalla_exception: %s\n", err.what());
        std::string code = std::to_string(err.code);
        std::string message = err.message.c_str();

        result = "{\"code\":" + code + ",\"message\":\"" + message + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] optimized_route std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + std::string(err.what()) + "\"}";
    } catch (...) {
        printf("[ValhallaActor] optimized_route unknown exception");
        result = "{\"code\":-1,\"message\":\"unknown exception\"}";
    }

    return result;
}
#endif
