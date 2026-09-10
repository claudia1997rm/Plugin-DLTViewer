#ifndef FOTA_INJECTION_PLUGIN_H
#define FOTA_INJECTION_PLUGIN_H

#include <QObject>
#include "plugininterface.h"

#define FOTA_INJECTION_PLUGIN_VERSION "0.1.0"

class FotaInjectionPlugin : public QObject,
                            QDLTPluginInterface,
                            QDltPluginControlInterface,
                            QDltPluginCommandInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginControlInterface)
    Q_INTERFACES(QDltPluginCommandInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.FotaInjectionPlugin")
#endif

public:
    FotaInjectionPlugin();
    ~FotaInjectionPlugin() override;

    QString name() override;
    QString pluginVersion() override;
    QString pluginInterfaceVersion() override;
    QString description() override;
    QString error() override;
    bool loadConfig(QString filename) override;
    bool saveConfig(QString filename) override;
    QStringList infoConfig() override;

    bool initControl(QDltControl *control) override;
    bool initConnections(QStringList list) override;
    bool controlMsg(int index, QDltMsg &msg) override;
    bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState,
                      QString hostname) override;
    bool autoscrollStateChanged(bool enabled) override;
    void initMessageDecoder(QDltMessageDecoder *messageDecoder) override;
    void initMainTableView(QTableView *tableView) override;
    void configurationChanged() override;

    bool command(QString command, QList<QString> params) override;

private:
    bool send(QStringList params);
    bool connectAndSend(QStringList params);
    bool parseServiceId(const QString &value, int &serviceId);
    void setError(const QString &message);

    QDltControl *control_;
    QString error_;
    QStringList connections_;
    int onlineConnectionIndex_;
};

#endif
