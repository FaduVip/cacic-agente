#include "checkmodules.h"

CheckModules::CheckModules(const QString &workingPath, const QString &workingModule)
{
    logcacic = new LogCacic(LOG_CHECKMODULES, workingPath+"/Logs");
    cacicMainFolder = workingPath;
    QDir tempPath(cacicMainFolder + "/temp");
    if (!tempPath.exists()){
        tempPath.mkdir(cacicMainFolder + "/temp");
    }
}

CheckModules::~CheckModules()
{
    logcacic->~LogCacic();
    delete logcacic;
}

QVariantMap CheckModules::getModules() const {
    return modules;
}

/*********************************************************************************
 * Verificar os módulos instalado de acordo com os hashs locais e do gerente,
 * se houver diferença entre eles, baixa o novo do servidor e salva na pasta temp.
 *********************************************************************************/
bool CheckModules::start(){
    bool ok = true;
    QJsonObject configFile;
    configFile = CCacic::getJsonFromFile(cacicMainFolder + "/getConfig.json");
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
        logcacic->escrever(LogCacic::ErrorLevel, QString("Erro ao pegar informações do arquivo " + cacicMainFolder + "/getConfig.json"));
    }
    if (!modules.isEmpty()){
        QVariantMap::const_iterator i = modules.constBegin();
        do {
            //            qDebug() << "Módulo: " << i.key() << " | Hash: " << i.value().toString();
            ok = (this->verificaModulo(i.key(), i.value().toString()) && ok);
            i++;
        } while (i != modules.constEnd());
    } else {
        logcacic->escrever(LogCacic::InfoLevel, QString("Não há modulo a ser verificado."));
    }

    return ok;
}

/*******************************************************
 * Método que vai verificar cada módulo individualmente.
 *******************************************************/
bool CheckModules::verificaModulo(const QString &moduloName, const QString &moduloHash)
{
    QFile *modulo, *moduloTemp;
    bool downloadOk = false;
    //pega o arquivo do módulo selecionado
#ifdef Q_OS_WIN
    modulo = new QFile(cacicMainFolder + "/bin/" + moduloName);
#else
    modulo = new QFile(cacicMainFolder + "/" + moduloName);
#endif
    modulo->open(QFile::ReadOnly);
    moduloTemp = new QFile(cacicMainFolder + "/temp/" + moduloName);
    moduloTemp->open(QFile::ReadOnly);
    //verifica se o módulo não existe e se o tamaho não é maior que 1 byte ou se o hash é diferente ao informado pelo json
    if ((!(modulo->exists() && modulo->size() > 1) ||
         !CCacic::Md5IsEqual(modulo->readAll(), moduloHash))){

        QString filePath = modulo->exists() ? cacicMainFolder + "/temp/":
                                              cacicMainFolder + "/";
        modulo->close();

        logcacic->escrever(LogCacic::InfoLevel, QString("Atualização de " + moduloName + " necessária."));
        QFile *novoModulo;
        QJsonObject metodoDownload;
        //verifica o tipo de download e tenta baixar o módulo para a pasta temporária.
        metodoDownload = CCacic::getJsonFromFile(cacicMainFolder + "/getConfig.json")
                ["agentcomputer"].toObject()
                ["metodoDownload"].toObject();
        if (!metodoDownload.isEmpty()){
            //verifica se já possuía o módulo atualizado na pasta temporária, se não baixa um novo.
            if ((!(moduloTemp->exists() && moduloTemp->size()>1) || !CCacic::Md5IsEqual(moduloTemp->readAll(), moduloHash))){
                moduloTemp->close();
                CacicComm *oCacicComm = new CacicComm(LOG_CHECKMODULES, this->cacicMainFolder);
                if (!metodoDownload["usuario"].isNull())
                    oCacicComm->setFtpUser(metodoDownload["usuario"].toString());
                if (!metodoDownload["senha"].isNull())
                    oCacicComm->setFtpPass(metodoDownload["senha"].toString());

                downloadOk = oCacicComm->fileDownload(metodoDownload["tipo"].toString(),
                        this->applicationUrl,
                        metodoDownload["path"].toString() +
                        (metodoDownload["path"].toString().endsWith("/") ? moduloName : "/" + moduloName),
                        filePath);
            } else {
                moduloTemp->close();
                return true;
            }
            novoModulo = new QFile(filePath + moduloName);
            if (downloadOk){
                //faz uma verificação do novo módulo.
                if (!(novoModulo->exists() && novoModulo->size()>1)){
                    logcacic->escrever(LogCacic::ErrorLevel,
                                       QString("Falha ao baixar " + moduloName +
                                               "("+metodoDownload["tipo"].toString()+ "://" +
                                       this->applicationUrl + metodoDownload["path"].toString() +
                            (metodoDownload["path"].toString().endsWith("/") ? moduloName : "/" + moduloName)+")"));
                    novoModulo->remove();
                    return false;
                } else {
                    logcacic->escrever(LogCacic::InfoLevel, QString(moduloName + " baixado com sucesso!"));
                    return true;
                }
            } else {
                logcacic->escrever(LogCacic::ErrorLevel, QString("Problemas durante o download de " + moduloName));
                return false;
            }
        } else {
            logcacic->escrever(LogCacic::ErrorLevel, QString("Não foi possível recuperar json de " +
                                                             cacicMainFolder + "/getConfig.json ao tentar baixar " +
                                                             moduloName));
            return false;
        }
    } else {
        modulo->close();
        return true;
    }
}
