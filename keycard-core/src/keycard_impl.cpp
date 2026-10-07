// keycard_impl.cpp — forwards the universal API to KeycardService (see keycard_impl.h).
#include "keycard_impl.h"
#include "plugin.h"
#include <QString>
#include <QVariantList>

namespace {
QString q(const std::string& s) { return QString::fromStdString(s); }
std::string s(const QString& v) { return v.toStdString(); }
}

KeycardImpl::KeycardImpl() = default;
KeycardImpl::~KeycardImpl() = default;

KeycardService& KeycardImpl::svc() {
    if (!m_svc) {
        m_svc = std::make_unique<KeycardService>();
        QObject::connect(m_svc.get(), &KeycardService::eventResponse, m_svc.get(),
            [this](const QString& name, const QVariantList& data) {
                const std::string a = data.size() > 0 ? s(data.at(0).toString()) : std::string();
                const std::string b = data.size() > 1 ? s(data.at(1).toString()) : std::string();
                if (name == QLatin1String("keycardAuthComplete")) keycardAuthComplete(a, b);
                else if (name == QLatin1String("keycardAuthRejected")) keycardAuthRejected(a, b);
            });
        QObject::connect(m_svc.get(), &KeycardService::activityLogged, m_svc.get(),
            [this](const QString& ts, const QString& msg, const QString& level) {
                activityLogged(s(ts), s(msg), s(level));
            });
    }
    return *m_svc;
}

// Create the service on the module event loop thread, where every later call also runs.
void KeycardImpl::onContextReady() { svc(); }

std::string KeycardImpl::initialize() {
    return s(svc().initialize());
}
std::string KeycardImpl::discoverReader() {
    return s(svc().discoverReader());
}
std::string KeycardImpl::discoverCard() {
    return s(svc().discoverCard());
}
std::string KeycardImpl::checkPairing() {
    return s(svc().checkPairing());
}
std::string KeycardImpl::pairCard(std::string pairingPassword) {
    return s(svc().pairCard(q(pairingPassword)));
}
std::string KeycardImpl::unpairCard() {
    return s(svc().unpairCard());
}
std::string KeycardImpl::authorize(std::string pin) {
    return s(svc().authorize(q(pin)));
}
std::string KeycardImpl::deriveKey(std::string domain) {
    return s(svc().deriveKey(q(domain)));
}
std::string KeycardImpl::getState() {
    return s(svc().getState());
}
std::string KeycardImpl::closeSession() {
    return s(svc().closeSession());
}
std::string KeycardImpl::getLastError() {
    return s(svc().getLastError());
}
std::string KeycardImpl::testPCSC() {
    return s(svc().testPCSC());
}
std::string KeycardImpl::checkReaderPresent() {
    return s(svc().checkReaderPresent());
}
std::string KeycardImpl::checkCardPresent() {
    return s(svc().checkCardPresent());
}
std::string KeycardImpl::unblockPIN(std::string puk, std::string newPIN) {
    return s(svc().unblockPIN(q(puk), q(newPIN)));
}
std::string KeycardImpl::getCardStatus() {
    return s(svc().getCardStatus());
}
std::string KeycardImpl::detectMode() {
    return s(svc().detectMode());
}
std::string KeycardImpl::loadKey(std::string jsonArgs) {
    return s(svc().loadKey(q(jsonArgs)));
}
std::string KeycardImpl::removeKey() {
    return s(svc().removeKey());
}
std::string KeycardImpl::getCardPresence() {
    return s(svc().getCardPresence());
}
std::string KeycardImpl::requestAuth(std::string domain, std::string caller) {
    return s(svc().requestAuth(q(domain), q(caller)));
}
std::string KeycardImpl::checkAuthStatus(std::string authId) {
    return s(svc().checkAuthStatus(q(authId)));
}
std::string KeycardImpl::getPendingAuths() {
    return s(svc().getPendingAuths());
}
std::string KeycardImpl::authorizeRequest(std::string authId, std::string pin) {
    return s(svc().authorizeRequest(q(authId), q(pin)));
}
std::string KeycardImpl::rejectRequest(std::string authId) {
    return s(svc().rejectRequest(q(authId)));
}
std::string KeycardImpl::hashMessage(std::string message) {
    return s(svc().hashMessage(q(message)));
}
std::string KeycardImpl::requestSign(std::string jsonArgs) {
    return s(svc().requestSign(q(jsonArgs)));
}
std::string KeycardImpl::checkSignStatus(std::string signId) {
    return s(svc().checkSignStatus(q(signId)));
}
std::string KeycardImpl::getPendingSigns() {
    return s(svc().getPendingSigns());
}
std::string KeycardImpl::approveSign(std::string jsonArgs) {
    return s(svc().approveSign(q(jsonArgs)));
}
std::string KeycardImpl::rejectSign(std::string signId) {
    return s(svc().rejectSign(q(signId)));
}
std::string KeycardImpl::requestXPUB(std::string jsonArgs) {
    return s(svc().requestXPUB(q(jsonArgs)));
}
std::string KeycardImpl::approveXPUB(std::string jsonArgs) {
    return s(svc().approveXPUB(q(jsonArgs)));
}
std::string KeycardImpl::rejectXPUB(std::string xpubId) {
    return s(svc().rejectXPUB(q(xpubId)));
}
std::string KeycardImpl::checkXPUBStatus(std::string xpubId) {
    return s(svc().checkXPUBStatus(q(xpubId)));
}
std::string KeycardImpl::getPendingXPUBs() {
    return s(svc().getPendingXPUBs());
}
std::string KeycardImpl::testXPUBExport(std::string jsonArgs) {
    return s(svc().testXPUBExport(q(jsonArgs)));
}
std::string KeycardImpl::testMasterExport(std::string pin) {
    return s(svc().testMasterExport(q(pin)));
}
std::string KeycardImpl::testEip1581Export(std::string pin) {
    return s(svc().testEip1581Export(q(pin)));
}
