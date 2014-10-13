#ifndef CHECKMODULES_H
#define CHECKMODULES_H

#include <ccacic.h>
#include <cacic_comm.h>
#include <cacic_computer.h>

class CheckModules
{
public:
    CheckModules(const QString &workingPath);
//    QStringList verificaModulos();
    bool start();
private:
    CCacic oCacic;
    CacicComm oCacicComm;
    QString applicationUrl;
    QVariantMap modules; //modules["name"] = hash;
    bool verificaModulo(const QString &moduloName, const QString &moduloHash);
//    void lerArquivoConfig(const QJsonObject& jsonConfig);
};

#endif // CHECKMODULES_H