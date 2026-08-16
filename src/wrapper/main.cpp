
#include <valhalla/worker.h>
#include "main.h"
#include "valhalla_actor.h"

#include <cxxabi.h>
#include <typeinfo>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <stdexcept>
#include <new>

namespace {

/** Minimal JSON string escaper.
 *
 *  These messages carry file paths, quoted identifiers and RTTI names, any of
 *  which can contain a quote or a backslash. Interpolating them raw produced
 *  invalid JSON, so the caller's parse threw and the real error was lost a
 *  second time — after the wrapper had gone to the trouble of reporting it.
 */
std::string json_escape(const std::string &s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (unsigned char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    return out;
}

/** Demangled dynamic type of the exception currently being handled.
 *
 *  A bare `catch (...)` used to report the literal string "unknown exception",
 *  which is the least useful thing it could possibly say. It matters more than
 *  it looks: on Apple builds RTTI identity is per-image, so an exception
 *  crossing the xcframework boundary can fail to match `catch (const
 *  std::exception &)` and land here even though it is a perfectly ordinary,
 *  fully-described error. Field failures then report only "-1 / unknown
 *  exception" and are undiagnosable by construction.
 *
 *  The type name survives that mismatch because it is read from the exception
 *  object itself rather than resolved through a dynamic_cast.
 */
std::string current_exception_type() {
    const std::type_info *t = abi::__cxa_current_exception_type();
    if (t == nullptr || t->name() == nullptr) return "unknown exception";
    int status = 0;
    std::unique_ptr<char, void (*)(void *)> demangled(
        abi::__cxa_demangle(t->name(), nullptr, nullptr, &status), std::free);
    return (status == 0 && demangled) ? std::string(demangled.get())
                                      : std::string(t->name());
}

/** Best-effort what() for the exception currently being handled.
 *
 *  Rethrowing inside the handler gives a second chance to reach what(): the
 *  rethrow re-matches in THIS image, which can succeed where the original
 *  clause failed. If it still does not match we fall back to the type name
 *  alone rather than losing the error entirely.
 */
std::string current_exception_message() {
    // Ladder from concrete to abstract. `catch (const std::exception &)` alone
    // is not enough: RTTI identity is per-image on Apple, and the failure we
    // chased reported its type as exactly "std::out_of_range" while STILL
    // refusing to match std::exception — i.e. the mismatch is in the base-class
    // walk, not in the type itself. Catching the concrete types first sidesteps
    // that walk entirely, which is what recovers what().
    try {
        throw;
    } catch (const std::out_of_range &e) {
        return e.what() ? e.what() : "";
    } catch (const std::length_error &e) {
        return e.what() ? e.what() : "";
    } catch (const std::invalid_argument &e) {
        return e.what() ? e.what() : "";
    } catch (const std::logic_error &e) {
        return e.what() ? e.what() : "";
    } catch (const std::range_error &e) {
        return e.what() ? e.what() : "";
    } catch (const std::runtime_error &e) {
        return e.what() ? e.what() : "";
    } catch (const std::bad_alloc &e) {
        return e.what() ? e.what() : "";
    } catch (const std::exception &e) {
        return e.what() ? e.what() : "";
    } catch (...) {
        return {};
    }
}

/** JSON for an exception that reached a catch-all, naming it as precisely as
 *  the runtime allows. */
std::string unknown_exception_json(const char *action) {
    const std::string type = current_exception_type();
    const std::string msg  = current_exception_message();
    const std::string detail = msg.empty() ? type : (type + ": " + msg);
    printf("[ValhallaActor] %s uncaught exception: %s\n", action, detail.c_str());
    return "{\"code\":-1,\"message\":\"" + json_escape(detail) + "\"}";
}

}  // namespace

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

        result = "{\"code\":" + code + ",\"message\":\"" + json_escape(message) + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] route std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + json_escape(std::string(err.what())) + "\"}";
    } catch (...) {
        result = unknown_exception_json("route");
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

        result = "{\"code\":" + code + ",\"message\":\"" + json_escape(message) + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] trace_route std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + json_escape(std::string(err.what())) + "\"}";
    } catch (...) {
        result = unknown_exception_json("trace_route");
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

        result = "{\"code\":" + code + ",\"message\":\"" + json_escape(message) + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] trace_attributes std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + json_escape(std::string(err.what())) + "\"}";
    } catch (...) {
        result = unknown_exception_json("trace_attributes");
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

        result = "{\"code\":" + code + ",\"message\":\"" + json_escape(message) + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] sources_to_targets std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + json_escape(std::string(err.what())) + "\"}";
    } catch (...) {
        result = unknown_exception_json("sources_to_targets");
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

        result = "{\"code\":" + code + ",\"message\":\"" + json_escape(message) + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] optimized_route std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + json_escape(std::string(err.what())) + "\"}";
    } catch (...) {
        result = unknown_exception_json("optimized_route");
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

        result = "{\"code\":" + code + ",\"message\":\"" + json_escape(message) + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] route std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + json_escape(std::string(err.what())) + "\"}";
    } catch (...) {
        result = unknown_exception_json("route");
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

        result = "{\"code\":" + code + ",\"message\":\"" + json_escape(message) + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] trace_route std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + json_escape(std::string(err.what())) + "\"}";
    } catch (...) {
        result = unknown_exception_json("trace_route");
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

        result = "{\"code\":" + code + ",\"message\":\"" + json_escape(message) + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] trace_attributes std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + json_escape(std::string(err.what())) + "\"}";
    } catch (...) {
        result = unknown_exception_json("trace_attributes");
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

        result = "{\"code\":" + code + ",\"message\":\"" + json_escape(message) + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] sources_to_targets std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + json_escape(std::string(err.what())) + "\"}";
    } catch (...) {
        result = unknown_exception_json("sources_to_targets");
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

        result = "{\"code\":" + code + ",\"message\":\"" + json_escape(message) + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] optimized_route std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + json_escape(std::string(err.what())) + "\"}";
    } catch (...) {
        result = unknown_exception_json("optimized_route");
    }

    return result;
}
#endif
