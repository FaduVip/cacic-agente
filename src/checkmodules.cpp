#include "checkmodules.h"

CheckModules::CheckModules(const QString &workingPath, const QString &workingModule)
{

    logManager = QLogger::QLoggerManager::getInstance();
    logManager->addDestination(workingPath + "/Logs/cacic.log", "CheckModules",QLogger::InfoLevel);
    logManager->addDestination(workingPath + "/Logs/cacic_error.log", "CheckModules",QLogger::ErrorLevel);
    oCacic.setCacicMainFolder(workingPath);

    QDir tempPath(oCacic.getCacicMainFolder() + "/temp");
    if (!tempPath.exists()){
        tempPath.mkdir(oCacic.getCacicMainFolder() + "/temp");
    }
}

CheckModules::~CheckModules()
{
    logManager->closeLogger();
    logManager->wait();
    delete logManager;
}

QVariantMap CheckModules::getModules() const {
    return modules;
}

bool CheckModules::start(){
    bool ok = true;
    QLogger::QLog_Info("CheckModules", QString("Verificando módulos."));
    QJsonObject configFile;
    configFile = oCacic.getJsonFromFile(oCacic.getCacicMainFolder() + "/getConfig.json");
    if (!configFile.isEmpty()) {
        //pega url do gerente.
        this->applicationUrl = configFile["agentcomputer"].toObject()["metodoDownload"].toObject()["url"].toString();

        QJsonArray modulos;
        //pega o jsonarray dos módulos
        modulos = configFile["agentcomputer"].toObject()["modulos"].toObject()["cacic"].toArray();
        foreach (QJsonValue modulo, modulos){
            //grava o nome com o hash de cada modulo
            modules[modulo.toObject()["nome"].toString()] = modulo.toObject()["hash"].toString();
        }
    } else {
        QLogger::QLog_Error("CheckModules", QString("Erro ao pegar informações do arquivo " + oCacic.getCacicMainFolder() + "/getConfig.json"));
    }
    if (!modules.isEmpty()){
        QVariantMap::const_iterator i = modules.constBegin();
        do {
//            qDebug() << "Módulo: " << i.key() << " | Hash: " << i.value().toString();
            ok = (this->verificaModulo(i.key(), i.value().toString()) && ok);
            i++;
        } while (i != modules.constEnd());
    } else {
        QLogger::QLog_Error("CheckModules", QString("Não há modulo a ser verificado."));
    }

    return ok;
}

bool CheckModules::verificaModulo(const QString &moduloName, const QString &moduloHash)
{
    QFile *modulo;
    bool downloadOk = false;
    //pega o arquivo do módulo selecionado
    modulo = new QFile(oCacic.getCacicMainFolder() + "/" + moduloName);
    modulo->open(QFile::ReadOnly);
    //verifica se o módulo não existe e se o tamaho não é maior que 1 byte ou se o hash é diferente ao informado pelo json
    if (!(modulo->exists() && modulo->size()>1) || !oCacic.Md5IsEqual(modulo->readAll(), moduloHash)){
        modulo->close();
        QLogger::QLog_Info("CheckModules", QString("Atualização de " + moduloName + " necessária."));
        QFile *novoModulo;
        QJsonObject metodoDownload;
        //verifica o tipo de download e tenta baixar o módulo para a pasta temporária.
        metodoDownload = oCacic.getJsonFromFile(oCacic.getCacicMainFolder() + "/getConfig.json")["agentcomputer"].toObject()["metodoDownload"].toObject();
        if (!metodoDownload.isEmpty()){
            oCacicComm.setFtpUser(metodoDownload["usuario"].toString());
            oCacicComm.setFtpPass(metodoDownload["senha"].toString());

            downloadOk = oCacicComm.fileDownload(metodoDownload["tipo"].toString(),
                                                this->applicationUrl,
                                                metodoDownload["path"].toString() +
                                                (metodoDownload["path"].toString().endsWith("/") ? moduloName : "/" + moduloName),
                                                oCacic.getCacicMainFolder() + "/temp");

            novoModulo = new QFile(oCacic.getCacicMainFolder() + "/temp/" + moduloName);
            if (downloadOk){
                //faz uma verificação do novo módulo.
                if (!(novoModulo->exists() && novoModulo->size()>1)){
                    QLogger::QLog_Error("CheckModules",
                                        QString("Falha ao baixar " + moduloName +
                                                "("+metodoDownload["tipo"].toString()+ "://" +
                                                this->applicationUrl + metodoDownload["path"].toString() +
                                                (metodoDownload["path"].toString().endsWith("/") ? moduloName : "/" + moduloName)+")"));
                    novoModulo->remove();
                    return false;
                } else {
                    QLogger::QLog_Info("CheckModules", QString(moduloName + " baixado com sucesso!"));
                    return true;
                }
            } else {
                return false;
            }
        } else {
            QLogger::QLog_Error("CheckModules", QString("Não foi possível recuperar json de " +
                                                        oCacic.getCacicMainFolder() + "/getConfig.json ao tentar baixar " +
                                                        moduloName));
            return false;
        }
    } else {
        modulo->close();
        QLogger::QLog_Info("CheckModules", QString("Atualização de " + moduloName + " desnecessária."));
        return true;
    }
}
