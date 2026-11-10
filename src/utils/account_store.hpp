#ifndef UTILS_ACCOUNT_STORE_HPP
#define UTILS_ACCOUNT_STORE_HPP

#include <string>

/**
 * AccountStore: bridge between the C++ game and the browser-side auth module
 * (wasm/web/auth.js). The account identity is a Solana wallet; the nickname is
 * stored in browser localStorage so it survives a page reload.
 *
 * All methods are safe to call from the main thread. Async operations (wallet
 * connect, register, login) settle in JS and are drained through pollResult()
 * / pollStatus() from BaseUserScreen::onUpdate.
 */
class AccountStore
{
public:
    enum Result
    {
        RESULT_OK = 0,
        RESULT_UNSUPPORTED,
        RESULT_NO_WALLET,
        RESULT_FAILED,
        RESULT_CANCELLED
    };

    /** Idempotently inject the auth module script if it is not loaded yet. */
    static void ensureModule();

    /** True once the auth module has finished loading. */
    static bool moduleReady();

    /** True if a wallet provider is available in the browser. */
    static bool available();

    /** Trigger a wallet connection (asynchronous; poll via pollStatus). */
    static void connectWallet();

    /** Currently connected wallet address, or empty string. */
    static std::string connectedAccount();

    /** Register a new account under the connected wallet. */
    static void registerAccount(const std::string& nick);

    /** Sign in with the connected wallet. */
    static void login();

    /** Drain a finished register/login attempt. Returns true if one settled. */
    static bool pollResult(Result& result);

    /** Drain a status line. Returns true if a new one is available. */
    static bool pollStatus(std::string& status);

    /** Human-readable description of a result code. */
    static std::string describe(Result r);

    /** True if a session is currently signed in. */
    static bool isSignedIn();

    /** Nickname of the signed-in account, or empty string. */
    static std::string currentNickname();

    /** Clear the signed-in session. */
    static void signOut();
};

#endif // UTILS_ACCOUNT_STORE_HPP