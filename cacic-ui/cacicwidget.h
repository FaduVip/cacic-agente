#ifndef WIDGET_H
#define WIDGET_H

#include <QFormLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLockFile>
#include <QString>
#include <QWidget>
#include "cacicsystray.h"
#include "../src/ccacic.h"
#include "../src/logcacic.h"
#include "uiclient.h"
#include "uiserver.h"
#include "netdevtab.h"
#include "netadapterconfigtab.h"
#include "cddrivetab.h"
#include "logicaldisktab.h"
#include "pointdevicetab.h"
#include "printertab.h"

namespace Ui {
class Widget;
}

class CacicWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CacicWidget(QWidget *parent = 0);
    ~CacicWidget();

private slots:
    void hardwareItemPressed(QListWidgetItem *item);
    void softwareItemPressed(QListWidgetItem *item);
    void on_finalizar();
    void on_infosClicked();

private:
    void closeEvent(QCloseEvent *event);
    void setupTabGeral(const QJsonObject &coleta);
    void setupTabHardware(const QJsonObject &coleta);
    void setupTabSoftware(const QJsonObject &coleta);

    bool windowOpen;
    QLockFile *lock;
    LogCacic *logcacic;
    Ui::Widget *ui;
    QString cacicMainFolder;
    CacicSysTray *cacicSysTray;
    UiClient *cliente;
    UiServer *servidor;
};

#endif // WIDGET_H
