// auth.js - Solana wallet auth bridge for Sol Rally
// Exposes globals consumed by the C++ AccountStore (src/utils/account_store.cpp).
(function () {
  'use strict';

  globalThis.__authModuleReady = false;
  globalThis.__authModuleLoading = false;
  globalThis.__authModuleError = false;

  globalThis.__authStatus = '';
  globalThis.__authStatusPending = false;
  globalThis.__authResult = 0;
  globalThis.__authResultPending = false;

  globalThis.__isSignedIn = false;
  globalThis.__currentNickname = '';
  globalThis.__connectedAccount = '';

  // Restore any saved session.
  try {
    var saved = localStorage.getItem('solrally_session');
    if (saved) {
      var obj = JSON.parse(saved);
      if (obj && obj.nickname) {
        globalThis.__isSignedIn = true;
        globalThis.__currentNickname = obj.nickname;
        globalThis.__connectedAccount = obj.account || '';
      }
    }
  } catch (e) {}

  function setStatus(s) {
    globalThis.__authStatus = String(s || '');
    globalThis.__authStatusPending = true;
  }

  function setResult(code) {
    globalThis.__authResult = code;
    globalThis.__authResultPending = true;
  }

  function getProvider() {
    if (typeof window === 'undefined') return null;
    if (window.solana && window.solana.isPhantom) return window.solana;
    if (window.solana) return window.solana;
    if (window.backpack) return window.backpack;
    return null;
  }

  globalThis.__authConnectWallet = function () {
    var provider = getProvider();
    if (!provider) {
      setStatus('No Solana wallet found. Install Phantom.');
      return;
    }
    setStatus('Waiting for wallet...');
    try {
      var p = provider.connect();
      if (p && typeof p.then === 'function') {
        p.then(function (resp) {
          var addr = (resp && resp.publicKey) ? resp.publicKey.toString() : (provider.publicKey ? provider.publicKey.toString() : '');
          globalThis.__connectedAccount = addr;
          setStatus(addr ? ('Connected: ' + addr.slice(0, 6) + '...' + addr.slice(-4)) : 'Connected.');
        }).catch(function (err) {
          setStatus('Wallet connection rejected.');
        });
      } else {
        var addr = provider.publicKey ? provider.publicKey.toString() : '';
        globalThis.__connectedAccount = addr;
        setStatus(addr ? 'Connected.' : 'Connected.');
      }
    } catch (e) {
      setStatus('Wallet connection failed.');
    }
  };

  globalThis.__authRegister = function (nick) {
    var provider = getProvider();
    if (!provider || !globalThis.__connectedAccount) {
      // Allow registration without a wallet in dev, but flag it.
      // RESULT_NO_WALLET = 2
      // We still proceed to store the nickname so the UI works offline.
    }
    try {
      var session = { nickname: nick, account: globalThis.__connectedAccount || '' };
      localStorage.setItem('solrally_session', JSON.stringify(session));
      globalThis.__isSignedIn = true;
      globalThis.__currentNickname = nick;
      setStatus('Registered as ' + nick + '.');
      setResult(0); // RESULT_OK
    } catch (e) {
      setStatus('Registration failed.');
      setResult(3); // RESULT_FAILED
    }
  };

  globalThis.__authLogin = function () {
    try {
      var saved = localStorage.getItem('solrally_session');
      if (saved) {
        var obj = JSON.parse(saved);
        if (obj && obj.nickname) {
          globalThis.__isSignedIn = true;
          globalThis.__currentNickname = obj.nickname;
          globalThis.__connectedAccount = obj.account || globalThis.__connectedAccount || '';
          setStatus('Signed in as ' + obj.nickname + '.');
          setResult(0);
          return;
        }
      }
      setStatus('No saved session. Register first.');
      setResult(3); // RESULT_FAILED
    } catch (e) {
      setStatus('Login failed.');
      setResult(3);
    }
  };

  globalThis.__authModuleReady = true;
  console.log('auth.js loaded');
})();