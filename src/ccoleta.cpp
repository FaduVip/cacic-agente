#include "ccoleta.h"

CColeta::CColeta(QObject *parent)
{

}

void CColeta::coletaHardware()
{
    qDebug() << "coletaHardware() executado";
    oHardware.iniciaColeta();
}


void CColeta::coletaSoftware()
{
    qDebug() << "coletaSoftware() executado";
    oSoftware.iniciaColeta();

}

void CColeta::configuraColetas(){
    QObject::connect(this, SIGNAL(beginHardware()), this, SLOT(coletaHardware()));
    QObject::connect(this, SIGNAL(beginSoftware()), this, SLOT(coletaSoftware()));
}

CACIC_Computer CColeta::getOComputer() const
{
    return oComputer;
}

cacic_software CColeta::getOSoftware() const
{
    return oSoftware;
}

cacic_hardware CColeta::getOHardware() const
{
    return oHardware;
}

void CColeta::run()
{
    QJsonObject coleta = oCacic.getJsonFromFile("configReq.json");

    if( coleta.contains("hardware") )
        emit beginHardware();
    if ( coleta.contains("software") )
        emit beginSoftware();

}

QJsonObject CColeta::toJsonObject()
{
    QJsonObject coletaJson;
    coletaJson["computer"] = oComputer.toJsonObject();
    coletaJson["software"] = oSoftware.toJsonObject();
    coletaJson["hardware"] = oHardware.toJsonObject();
    return coletaJson;
}
