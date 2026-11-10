#include "utils/account_store.hpp"

#include "utils/string_utils.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <cstring>

namespace
{
    // Shared buffer used to ferry UTF-8 strings out of JS into C++.
    static char s_buf[512];

    const char* readBuf()
    {
        return s_buf;
    }
}

void AccountStore::ensureModule()
{
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (!globalThis.__authModuleLoading && !globalThis.__authModuleReady) {
            globalThis.__authModuleLoading = true;
            var s = document.createElement('script');
            s.src = '/auth.js';
            s.onload = function () { globalThis.__authModuleReady = true; globalThis.__authModuleLoading = false; };
            s.onerror = function () { globalThis.__authModuleReady = false; globalThis.__authModuleLoading = false; globalThis.__authModuleError = true; };
            document.head.appendChild(s);
        }
    });
#endif
}

bool AccountStore::moduleReady()
{
#ifdef __EMSCRIPTEN__
    return EM_ASM_INT({ return globalThis.__authModuleReady ? 1 : 0; }) != 0;
#else
    return false;
#endif
}

bool AccountStore::available()
{
#ifdef __EMSCRIPTEN__
    return EM_ASM_INT({
        return (globalThis.__authModuleReady && (typeof window !== 'undefined') &&
                (window.solana !== undefined || globalThis.__authForceAvailable)) ? 1 : 0;
    }) != 0;
#else
    return false;
#endif
}

void AccountStore::connectWallet()
{
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (globalThis.__authModuleReady && typeof globalThis.__authConnectWallet === 'function') {
            globalThis.__authConnectWallet();
        } else {
            globalThis.__authStatus = 'Auth module not loaded.';
            globalThis.__authStatusPending = true;
        }
    });
#endif
}

std::string AccountStore::connectedAccount()
{
#ifdef __EMSCRIPTEN__
    EM_ASM({
        var s = globalThis.__connectedAccount || "";
        if (s.length > 510) s = s.substring(0, 510);
        stringToUTF8(s, $0, 512);
    }, s_buf);
    return std::string(s_buf);
#else
    return std::string();
#endif
}

void AccountStore::registerAccount(const std::string& nick)
{
#ifdef __EMSCRIPTEN__
    // Pass the nickname through a shared buffer to avoid EM_ASM string escaping issues.
    std::strncpy(s_buf, nick.c_str(), 511);
    s_buf[511] = '\0';
    EM_ASM({
        var nick = UTF8ToString($0);
        if (globalThis.__authModuleReady && typeof globalThis.__authRegister === 'function') {
            globalThis.__authRegister(nick);
        } else {
            globalThis.__authResult = 1; // RESULT_UNSUPPORTED
            globalThis.__authResultPending = true;
        }
    }, s_buf);
#else
    (void)nick;
#endif
}

void AccountStore::login()
{
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (globalThis.__authModuleReady && typeof globalThis.__authLogin === 'function') {
            globalThis.__authLogin();
        } else {
            globalThis.__authResult = 1; // RESULT_UNSUPPORTED
            globalThis.__authResultPending = true;
        }
    });
#endif
}

bool AccountStore::pollResult(Result& result)
{
#ifdef __EMSCRIPTEN__
    int pending = EM_ASM_INT({ return globalThis.__authResultPending ? 1 : 0; });
    if (!pending) return false;
    int code = EM_ASM_INT({ return globalThis.__authResult || 0; });
    EM_ASM({ globalThis.__authResultPending = false; });
    result = static_cast<Result>(code);
    // Clamp to valid range.
    if (result != RESULT_OK && result != RESULT_UNSUPPORTED &&
        result != RESULT_NO_WALLET && result != RESULT_FAILED &&
        result != RESULT_CANCELLED)
        result = RESULT_UNSUPPORTED;
    return true;
#else
    (void)result;
    return false;
#endif
}

bool AccountStore::pollStatus(std::string& status)
{
#ifdef __EMSCRIPTEN__
    int pending = EM_ASM_INT({ return globalThis.__authStatusPending ? 1 : 0; });
    if (!pending) return false;
    EM_ASM({
        var s = globalThis.__authStatus || "";
        if (s.length > 510) s = s.substring(0, 510);
        stringToUTF8(s, $0, 512);
        globalThis.__authStatusPending = false;
    }, s_buf);
    status = std::string(s_buf);
    return true;
#else
    (void)status;
    return false;
#endif
}

std::string AccountStore::describe(Result r)
{
    switch (r)
    {
        case RESULT_OK: return "OK";
        case RESULT_UNSUPPORTED: return "Auth module not available.";
        case RESULT_NO_WALLET: return "No wallet connected.";
        case RESULT_FAILED: return "Account operation failed.";
        case RESULT_CANCELLED: return "Cancelled.";
        default: return "Unknown result.";
    }
}

bool AccountStore::isSignedIn()
{
#ifdef __EMSCRIPTEN__
    return EM_ASM_INT({ return globalThis.__isSignedIn ? 1 : 0; }) != 0;
#else
    return false;
#endif
}

std::string AccountStore::currentNickname()
{
#ifdef __EMSCRIPTEN__
    EM_ASM({
        var s = globalThis.__currentNickname || "";
        if (s.length > 510) s = s.substring(0, 510);
        stringToUTF8(s, $0, 512);
    }, s_buf);
    return std::string(s_buf);
#else
    return std::string();
#endif
}

void AccountStore::signOut()
{
#ifdef __EMSCRIPTEN__
    EM_ASM({
        globalThis.__isSignedIn = false;
        globalThis.__currentNickname = "";
        globalThis.__connectedAccount = "";
        try { localStorage.removeItem('solrally_session'); } catch (e) {}
        globalThis.__authStatus = 'Signed out.';
        globalThis.__authStatusPending = true;
    });
#endif
}