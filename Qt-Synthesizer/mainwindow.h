#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

QT_BEGIN_NAMESPACE

class QLabel;

namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_RefreshButton_clicked();
    void on_KeyPressed();
    void on_KeyReleased();
    void on_DialReleased();
    void on_DialChanged();



    void on_ConnectButton_clicked();

private:
    void showStatusMessage(const QString &message);
    void refreshCOMPorts();
    void writeData(const QByteArray &data);
    void readData();
    void sendMessage(uint8_t messageType, void *parameter, int32_t parameterSize);

    Ui::MainWindow *ui;
    QSerialPort *m_serial = nullptr;
    QLabel *m_status = nullptr;
};
#endif // MAINWINDOW_H
