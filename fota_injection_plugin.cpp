#include "fota_injection_plugin.h"

#include <QByteArray>
#include <QFileInfo>

FotaInjectionPlugin::FotaInjectionPlugin()
    : control_(nullptr)
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
    return QStringLiteral("Sends FOTA injections through DLT Viewer control interface.");
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
    return QStringList() << QStringLiteral("Command: send <connection-index> <application-id> <context-id> <service-id> <data>")
                         << QStringLiteral("Example: send 0 FOTA MAIN 5505 tcucpkg;package.iso;hash;/ota/package.iso");
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

bool FotaInjectionPlugin::stateChanged(int, QDltConnection::QDltConnectionState, QString)
{
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

bool FotaInjectionPlugin::command(QString commandName, QList<QString> params)
{
    error_.clear();
    if (commandName.compare(QStringLiteral("send"), Qt::CaseInsensitive) != 0) {
        setError(QStringLiteral("Unknown command. Use: send <connection-index> <application-id> <context-id> <service-id> <data>"));
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
