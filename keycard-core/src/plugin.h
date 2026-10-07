#pragma once

#include "KeycardBridge.h"
#include "secure_buffer.h"
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QDateTime>
#include <QSet>
#include <vector>

// The keycard card logic. Formerly the module's Qt plugin class (KeycardPlugin); since the
// universal port it is a plain QObject owned by KeycardImpl (keycard_impl.h), which is the
// module API. Behaviour is unchanged.
class KeycardService : public QObject
{
    Q_OBJECT

public:
    explicit KeycardService(QObject* parent = nullptr);
    ~KeycardService() override;


    // No override keyword (Lesson #19 - called reflectively)
    QString initialize();

    // Core keycard operations
    QString discoverReader();
    QString discoverCard();
    QString checkPairing();
    QString pairCard(const QString& pairingPassword);
    QString unpairCard();
    QString authorize(const QString& pin);
    QString deriveKey(const QString& domain);
    QString getState();
    QString closeSession();
    QString getLastError();
    QString testPCSC();  // Debug: test PC/SC directly
    QString checkReaderPresent();  // Fresh PC/SC check
    QString checkCardPresent();   // Fresh PC/SC check
    QString unblockPIN(const QString& puk, const QString& newPIN);
    QString getCardStatus();  // Get PIN/PUK attempts remaining
    QString detectMode();     // Returns {"mode":"BIP39"|"LEE"|"none"}
    QString loadKey(const QString& jsonArgs); // Debug: load key, expects {"seedHex":"...","keyType":0|1}
    QString removeKey();      // Debug: remove loaded key

    // Card presence (for consuming modules to poll)
    QString getCardPresence();

    // Authorization request API
    QString requestAuth(const QString& domain, const QString& caller);
    QString checkAuthStatus(const QString& authId);
    QString getPendingAuths();
    QString authorizeRequest(const QString& authId, const QString& pin);
    QString rejectRequest(const QString& authId);

    // Utility
    QString hashMessage(const QString& message);  // SHA-256 hex of UTF-8 message

    // Signing request API (#98, #149, #150)
    QString requestSign(const QString& jsonArgs); // {"domain","payloadHash","caller","scheme","bip32_path"?}
    QString checkSignStatus(const QString& signId);
    QString getPendingSigns();
    QString approveSign(const QString& jsonArgs); // {"signId":"...","pin":"..."}
    QString rejectSign(const QString& signId);

    // XPUB export API (#142)
    QString requestXPUB(const QString& jsonArgs);  // {"domain","caller"}
    QString approveXPUB(const QString& jsonArgs);  // {"xpubId","pin"}
    QString rejectXPUB(const QString& xpubId);
    QString checkXPUBStatus(const QString& xpubId);
    QString getPendingXPUBs();
    QString testXPUBExport(const QString& jsonArgs); // Debug: {"domain","pin"} — direct export, bypasses request queue
    QString testMasterExport(const QString& pin);   // Debug: authorize + export master (no derive) — chain code probe
    QString testEip1581Export(const QString& pin);  // Debug: authorize + export at m/43'/60'/1581' — EIP-1581 root probe

signals:
    void eventResponse(const QString& eventName, const QVariantList& data);
    void activityLogged(const QString& timestamp, const QString& message, const QString& level);

private:
    struct AuthRequest {
        QString id;
        QString domain;
        QString caller;
        QString status;  // "pending", "complete", "consumed", "failed"
        SecureBuffer key; // Result key (if complete) — wiped after first read
        QString error;   // Error message (if failed)
        qint64 timestamp;

        // Move-only (SecureBuffer is non-copyable)
        AuthRequest() = default;
        AuthRequest(AuthRequest&&) = default;
        AuthRequest& operator=(AuthRequest&&) = default;
        AuthRequest(const AuthRequest&) = delete;
        AuthRequest& operator=(const AuthRequest&) = delete;
    };

    void purgeCompletedRequests();

    QString mapBridgeStateToSpec(KeycardBridge::State state);
    QString domainToPath(const QString& domain);      // m/43'/60'/1581' — auth/key-export subtree
    QString domainToSignPath(const QString& domain);  // m/43'/60'/1582' — signing subtree (#150)
    void logActivity(const QString& message, const QString& level = "info");
    void addActivityToResponse(QJsonObject& response);

    enum class SessionState {
        NoSession,
        Active
    };

private:
    struct SignRequest {
        QString id;
        QString domain;
        QString payloadHash;  // hex-encoded 32-byte digest
        QString caller;
        QString scheme;       // "ecdsa" or "schnorr"
        QString bip32_path;   // explicit BIP32 path (#149); if set, overrides domain derivation
        QString status;       // "pending", "complete", "rejected", "failed"
        SecureBuffer signature; // Result — wiped after first read
        QString error;
        qint64 timestamp;

        SignRequest() = default;
        SignRequest(SignRequest&&) = default;
        SignRequest& operator=(SignRequest&&) = default;
        SignRequest(const SignRequest&) = delete;
        SignRequest& operator=(const SignRequest&) = delete;
    };

    struct XPUBRequest {
        QString id;
        QString bip32_path;
        QString caller;
        QString status;   // "pending", "complete", "rejected", "failed"
        SecureBuffer xpub; // pubkey_hex + chaincode_hex — wiped after first read
        QString error;
        qint64 timestamp;

        XPUBRequest() = default;
        XPUBRequest(XPUBRequest&&) = default;
        XPUBRequest& operator=(XPUBRequest&&) = default;
        XPUBRequest(const XPUBRequest&) = delete;
        XPUBRequest& operator=(const XPUBRequest&) = delete;
    };

private:
    KeycardBridge* m_bridge = nullptr;
    SessionState m_sessionState = SessionState::NoSession;
    std::vector<AuthRequest> m_authRequests;
    std::vector<SignRequest> m_signRequests;
    std::vector<XPUBRequest> m_xpubRequests;

    // Activity log queue (for QML)
    struct ActivityEntry {
        QString timestamp;
        QString message;
        QString level;
    };
    QList<ActivityEntry> m_recentActivity;
    QSet<QString> m_loggedRequestIds;
};
