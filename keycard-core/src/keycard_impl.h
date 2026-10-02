#pragma once
// keycard_impl.h — the keycard module in universal authoring (builder 0.3.x).
//
// The generated plugin glue is derived from THIS header: every public method is the module API
// (std::string in/out, JSON-serialisable). The card logic itself is unchanged and lives in
// KeycardService (plugin.{h,cpp}); each method forwards to it. Methods run one at a time on the
// module event loop (concurrency "single"), which is the threading the service always assumed.
// Keep declaration lines free of trailing // comments and default arguments: the glue
// generator skips the former and silently drops the latter.
#include <memory>
#include <string>
#include "logos_module_context.h"

class KeycardService;

class KeycardImpl : public LogosModuleContext {
public:
    KeycardImpl();
    ~KeycardImpl() override;

    std::string initialize();
    std::string discoverReader();
    std::string discoverCard();
    std::string checkPairing();
    std::string pairCard(std::string pairingPassword);
    std::string unpairCard();
    std::string authorize(std::string pin);
    std::string deriveKey(std::string domain);
    std::string getState();
    std::string closeSession();
    std::string getLastError();
    std::string testPCSC();
    std::string checkReaderPresent();
    std::string checkCardPresent();
    std::string unblockPIN(std::string puk, std::string newPIN);
    std::string getCardStatus();
    std::string detectMode();
    std::string loadKey(std::string jsonArgs);
    std::string removeKey();
    std::string getCardPresence();
    std::string requestAuth(std::string domain, std::string caller);
    std::string checkAuthStatus(std::string authId);
    std::string getPendingAuths();
    std::string authorizeRequest(std::string authId, std::string pin);
    std::string rejectRequest(std::string authId);
    std::string hashMessage(std::string message);
    std::string requestSign(std::string jsonArgs);
    std::string checkSignStatus(std::string signId);
    std::string getPendingSigns();
    std::string approveSign(std::string jsonArgs);
    std::string rejectSign(std::string signId);
    std::string requestXPUB(std::string jsonArgs);
    std::string approveXPUB(std::string jsonArgs);
    std::string rejectXPUB(std::string xpubId);
    std::string checkXPUBStatus(std::string xpubId);
    std::string getPendingXPUBs();
    std::string testXPUBExport(std::string jsonArgs);
    std::string testMasterExport(std::string pin);
    std::string testEip1581Export(std::string pin);

    void onContextReady() override;

logos_events:
    // An auth request (requestAuth) was approved: the requesting module may collect its key.
    void keycardAuthComplete(const std::string& authId, const std::string& caller);
    // An auth request was rejected by the user.
    void keycardAuthRejected(const std::string& authId, const std::string& caller);
    // Human-readable activity line (also returned inline in each response's "activity").
    void activityLogged(const std::string& timestamp, const std::string& message, const std::string& level);

private:
    KeycardService& svc();
    std::unique_ptr<KeycardService> m_svc;
};
