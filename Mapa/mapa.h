#ifndef MAPA_H
#define MAPA_H

#include <QMainWindow>
#include <QMessageBox>
#include "ui_mapa_default.h"

namespace Ui {
class Mapa;
}

class Mapa : public QMainWindow
{
    Q_OBJECT

public:
    explicit Mapa(QWidget *parent = 0);
    ~Mapa();

private slots:
    void on_okButton_clicked();

private:
    Ui::Mapa *ui;
};

#endif // MAPA_H