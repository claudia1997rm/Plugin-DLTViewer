#include "fota_injection_plugin.h"

#include <QByteArray>
#include <QEventLoop>
#include <QFileInfo>
#include <QTimer>

FotaInjectionPlugin::FotaInjectionPlugin()
    : control_(nullptr), onlineConnectionIndex_(-1), expectedStateIndex_(0),
      expectedStateFound_(false), waitLoop_(nullptr)
{
}

FotaInjectionPlugin::~FotaInjectionPlugin()
{
}

QString FotaInjectionPlugin::name()
{
    return QStringLiteral("FOTA Injection Plugin");
}

QString FotaInjectionPlugin::pluginVersion()
{
    return QStringLiteral(FOTA_INJECTION_PLUGIN_VERSION);
}

QString FotaInjectionPlugin::pluginInterfaceVersion()
{
    return QStringLiteral(PLUGIN_INTERFACE_VERSION);
}

QString FotaInjectionPlugin::description()
{
    return QStringLiteral("Sends FOTA injections and waits for the expected FOTA states.");
}

QString FotaInjectionPlugin::error()
{
    return error_;
}

bool FotaInjectionPlugin::loadConfig(QString)
{
    return true;
}

bool FotaInjectionPlugin::saveConfig(QString)
{
    return true;
}

QStringList FotaInjectionPlugin::infoConfig()
{
    return QStringList()
        << QStringLiteral("send <connection-index> <application-id> <context-id> <service-id> <data>")
        << QStringLiteral("connect-wait <connection-index> <expected-states> <timeout-seconds>")
        << QStringLiteral("connect-send-wait <connection-index> <application-id> <context-id> <service-id> <expected-states> <timeout-seconds> <data>")
        << QStringLiteral("Example: connect-send-wait 1 FOTA MAIN 5505 DISTRIBUTE_COMPLETE 600 tcucpkg;package.iso;hash;/ota/package.iso");
}

bool FotaInjectionPlugin::initControl(QDltControl *control)
{
    control_ = control;
    if (control_ == nullptr) {
        setError(QStringLiteral("DLT control interface was not provided."));
        return false;
    }
    return true;
}

bool FotaInjectionPlugin::initConnections(QStringList list)
{
    connections_ = list;
    return true;
}

bool FotaInjectionPlugin::controlMsg(int, QDltMsg &)
{
    return true;
}

bool FotaInjectionPlugin::stateChanged(int index, QDltConnection::QDltConnectionState connectionState, QString)
{
    if (connectionState == QDltConnection::QDltConnectionOnline) {
        onlineConnectionIndex_ = index;
    } else if (onlineConnectionIndex_ == index) {
        onlineConnectionIndex_ = -1;
    }
    return true;
}

bool FotaInjectionPlugin::autoscrollStateChanged(bool)
{
    return true;
}

void FotaInjectionPlugin::initMessageDecoder(QDltMessageDecoder *)
{
}

void FotaInjectionPlugin::initMainTableView(QTableView *)
{
}

void FotaInjectionPlugin::configurationChanged()
{
}

QWidget *FotaInjectionPlugin::initViewer()
{
    return nullptr;
}

void FotaInjectionPlugin::initFileStart(QDltFile *) {}
void FotaInjectionPlugin::initFileFinish() {}
void FotaInjectionPlugin::initMsg(int, QDltMsg &) {}
void FotaInjectionPlugin::initMsgDecoded(int, QDltMsg &) {}
void FotaInjectionPlugin::updateFileStart() {}

void FotaInjectionPlugin::updateMsg(int, QDltMsg &msg)
{
    if (!expectedStateFound_
            && msg.getApid().compare(QStringLiteral("FOTA"), Qt::CaseInsensitive) == 0
            && expectedStateIndex_ < expectedStates_.size()
            && msg.toStringPayload().contains(expectedStates_.at(expectedStateIndex_), Qt::CaseInsensitive)) {
        ++expectedStateIndex_;
        if (expectedStateIndex_ >= expectedStates_.size()) {
            expectedStateFound_ = true;
            if (waitLoop_ != nullptr) {
                waitLoop_->quit();
            }
        }
    }
}

void FotaInjectionPlugin::updateMsgDecoded(int index, QDltMsg &msg)
{
    updateMsg(index, msg);
}

void FotaInjectionPlugin::updateFileFinish() {}
void FotaInjectionPlugin::selectedIdxMsg(int, QDltMsg &) {}
void FotaInjectionPlugin::selectedIdxMsgDecoded(int, QDltMsg &) {}

bool FotaInjectionPlugin::command(QString commandName, QList<QString> params)
{
    error_.clear();
    if (commandName.compare(QStringLiteral("connect-send-wait"), Qt::CaseInsensitive) == 0) {
        return connectSendWait(params);
    }
    if (commandName.compare(QStringLiteral("connect-wait"), Qt::CaseInsensitive) == 0) {
        return connectWait(params);
    }
    if (commandName.compare(QStringLiteral("connect-send"), Qt::CaseInsensitive) == 0) {
        return connectAndSend(params);
    }
    if (commandName.compare(QStringLiteral("send"), Qt::CaseInsensitive) != 0) {
        setError(QStringLiteral("Unknown command. Use: send or connect-send <connection-index> <application-id> <context-id> <service-id> <data>"));
        return false;
    }
    return send(params);
}

bool FotaInjectionPlugin::connectSendWait(QStringList params)
{
    if (params.size() < 6) {
        setError(QStringLiteral("Expected: connection-index application-id context-id service-id expected-states [timeout-seconds] data"));
        return false;
    }

    bool indexOk = false;
    const int connectionIndex = params.at(0).toInt(&indexOk);
    if (!indexOk || connectionIndex < 0) {
        setError(QStringLiteral("Connection index must be a non-negative integer."));
        return false;
    }

    expectedStates_ = params.at(4).split(',', Qt::SkipEmptyParts);
    for (QString &state : expectedStates_) {
        state = state.trimmed();
    }
    if (expectedStates_.isEmpty()) {
        setError(QStringLiteral("At least one expected FOTA state is required."));
        return false;
    }

    int timeoutSeconds = 600;
    QStringList sendParams = params.mid(0, 4) + params.mid(5);
    if (params.size() >= 7) {
        bool timeoutOk = false;
        timeoutSeconds = params.at(5).toInt(&timeoutOk);
        if (!timeoutOk || timeoutSeconds < 1 || timeoutSeconds > 7200) {
            setError(QStringLiteral("Timeout must be an integer between 1 and 7200 seconds."));
            return false;
        }
        sendParams = params.mid(0, 4) + params.mid(6);
    }

    expectedStateIndex_ = 0;
    expectedStateFound_ = false;
    if (!connectAndSend(sendParams)) {
        return false;
    }

    if (expectedStateFound_) {
        return true;
    }

    QEventLoop waitLoop;
    waitLoop_ = &waitLoop;
    QTimer::singleShot(timeoutSeconds * 1000, &waitLoop, &QEventLoop::quit);
    waitLoop.exec();
    waitLoop_ = nullptr;

    if (!expectedStateFound_) {
        setError(QStringLiteral("Timeout waiting for FOTA state sequence: ") + expectedStates_.join(','));
        return false;
    }
    return true;
}

bool FotaInjectionPlugin::connectWait(QStringList params)
{
    if (control_ == nullptr) {
        setError(QStringLiteral("DLT control is not initialized."));
        return false;
    }
    if (params.size() != 3) {
        setError(QStringLiteral("Expected: connection-index expected-states timeout-seconds"));
        return false;
    }

    bool indexOk = false;
    const int connectionIndex = params.at(0).toInt(&indexOk);
    if (!indexOk || connectionIndex < 0) {
        setError(QStringLiteral("Connection index must be a non-negative integer."));
        return false;
    }

    expectedStates_ = params.at(1).split(',', Qt::SkipEmptyParts);
    for (QString &state : expectedStates_) {
        state = state.trimmed();
    }
    if (expectedStates_.isEmpty()) {
        setError(QStringLiteral("At least one expected FOTA state is required."));
        return false;
    }

    bool timeoutOk = false;
    const int timeoutSeconds = params.at(2).toInt(&timeoutOk);
    if (!timeoutOk || timeoutSeconds < 1 || timeoutSeconds > 7200) {
        setError(QStringLiteral("Timeout must be an integer between 1 and 7200 seconds."));
        return false;
    }

    expectedStateIndex_ = 0;
    expectedStateFound_ = false;
    onlineConnectionIndex_ = -1;
    control_->connectEcu(connectionIndex);
    QEventLoop connectionLoop;
    QTimer::singleShot(5000, &connectionLoop, &QEventLoop::quit);
    connectionLoop.exec();
    if (onlineConnectionIndex_ != connectionIndex) {
        setError(QStringLiteral("ECU connection did not reach online state."));
        return false;
    }

    if (expectedStateFound_) {
        return true;
    }

    QEventLoop waitLoop;
    waitLoop_ = &waitLoop;
    QTimer::singleShot(timeoutSeconds * 1000, &waitLoop, &QEventLoop::quit);
    waitLoop.exec();
    waitLoop_ = nullptr;

    if (!expectedStateFound_) {
        setError(QStringLiteral("Timeout waiting for FOTA state sequence: ") + expectedStates_.join(','));
        return false;
    }
    return true;
}

bool FotaInjectionPlugin::connectAndSend(QStringList params)
{
    if (control_ == nullptr) {
        setError(QStringLiteral("DLT control is not initialized."));
        return false;
    }
    if (params.isEmpty()) {
        setError(QStringLiteral("Connection index is required."));
        return false;
    }

    bool indexOk = false;
    const int connectionIndex = params.at(0).toInt(&indexOk);
    if (!indexOk || connectionIndex < 0) {
        setError(QStringLiteral("Connection index must be a non-negative integer."));
        return false;
    }

    onlineConnectionIndex_ = -1;
    control_->connectEcu(connectionIndex);

    QEventLoop waitLoop;
    QTimer::singleShot(5000, &waitLoop, &QEventLoop::quit);
    waitLoop.exec();

    if (onlineConnectionIndex_ != connectionIndex) {
        setError(QStringLiteral("ECU connection did not reach online state."));
        return false;
    }
    return send(params);
}

bool FotaInjectionPlugin::send(QStringList params)
{
    if (control_ == nullptr) {
        setError(QStringLiteral("DLT control is not initialized. Load the plugin in a connected DLT Viewer instance."));
        return false;
    }
    if (params.size() < 5) {
        setError(QStringLiteral("Not enough parameters. Expected: connection-index application-id context-id service-id data"));
        return false;
    }

    bool indexOk = false;
    const int connectionIndex = params.at(0).toInt(&indexOk);
    if (!indexOk || connectionIndex < 0) {
        setError(QStringLiteral("Connection index must be a non-negative integer."));
        return false;
    }

    int serviceId = 0;
    if (!parseServiceId(params.at(3), serviceId)) {
        setError(QStringLiteral("Service ID must be an integer between 0 and 65535."));
        return false;
    }

    const QString applicationId = params.at(1);
    const QString contextId = params.at(2);
    const QByteArray data = params.mid(4).join(QStringLiteral(" ")).toUtf8();
    control_->sendInjection(connectionIndex, applicationId, contextId, serviceId, data);
    return true;
}

bool FotaInjectionPlugin::parseServiceId(const QString &value, int &serviceId)
{
    bool ok = false;
    serviceId = value.toInt(&ok);
    return ok && serviceId >= 0 && serviceId <= 65535;
}

void FotaInjectionPlugin::setError(const QString &message)
{
    error_ = message;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(FotaInjectionPlugin, FotaInjectionPlugin)
#endif
