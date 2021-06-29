#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSerialPortInfo>
#include <QTextStream>
#include <QPushButton>

#include <math.h>

#include "Synthesizer.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_status(new QLabel)
    , m_serial(new QSerialPort(this))
{
    ui->setupUi(this);

    ui->statusbar->addWidget(m_status);

    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);

    QList<QDial*> list_dials = this->findChildren<QDial*>();
    for(int i = 0; i < list_dials.length(); i++)
    {
        connect( list_dials.at(i), SIGNAL(valueChanged(int)), this, SLOT(on_DialChanged()));
        connect( list_dials.at(i), SIGNAL(sliderReleased()), this, SLOT(on_DialReleased()));
    }

    QList<QPushButton*> list = this->findChildren<QPushButton*>(QRegularExpression("Key_\\d+$"));
    for(int i = 0; i < list.length(); i++)
    {
        connect ( list.at(i), SIGNAL( pressed() ), this, SLOT( on_KeyPressed() ) );
        connect ( list.at(i), SIGNAL( released() ), this, SLOT( on_KeyReleased() ) );
    }



    refreshCOMPorts();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshCOMPorts()
{
    const auto serialPortInfos = QSerialPortInfo::availablePorts();

    const QString blankString = "N/A";
    QString portName;

    ui->PortBox->clear();
    for (const QSerialPortInfo &serialPortInfo : serialPortInfos) {
        portName = serialPortInfo.portName();
        ui->PortBox->addItem(portName);
    }
}

void MainWindow::on_RefreshButton_clicked()
{
    refreshCOMPorts();
}

void MainWindow::on_ConnectButton_clicked()
{
    m_serial->setPortName(QString(ui->PortBox->currentText()));
    m_serial->setBaudRate(38400);
    m_serial->setDataBits(QSerialPort::DataBits(8));
    m_serial->setParity(QSerialPort::Parity(false));
    m_serial->setStopBits(QSerialPort::StopBits(1));
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        ui->ConnectButton->setDisabled(true);
        ui->RefreshButton->setDisabled(true);
        ui->PortBox->setDisabled(true);
        showStatusMessage(tr("Connected to %1")
                          .arg(QString(ui->PortBox->currentText())));
    } else {

        showStatusMessage(tr("Open error: %1").arg(QString(m_serial->errorString())));
    }
}

void MainWindow::on_KeyPressed()
{
    QPushButton* buttonSender = qobject_cast<QPushButton*>(sender()); // retrieve the button you have clicked
    QString buttonText = buttonSender->objectName(); // retrive the text from the button clicked
    QString id = buttonText.mid(4, 3);
    uint8_t key = id.toUInt();
    uint8_t velocity = 255;
    //showStatusMessage(id);


    char message[3] = {NOTE_ON, key, velocity};

    //showStatusMessage("Sent: " + QString::number(key));
    showStatusMessage("Sent: " + QString(QByteArray(message, 3).toHex()));
    //writeData(QByteArray(message, 3));

    MESSAGE_MIDI msg;
    msg.status = NOTE_ON;
    msg.key = key+35;
    msg.velocity = velocity;

    uint8_t frame[100];
    uint8_t *auxptr = frame;
    *auxptr = MIDI;
    auxptr++;
    memcpy(auxptr, &msg, sizeof(msg));
    writeData(QByteArray::fromRawData((char *)frame, sizeof(msg)+1));


}

void MainWindow::on_KeyReleased()
{
    QPushButton* buttonSender = qobject_cast<QPushButton*>(sender()); // retrieve the button you have clicked
    QString buttonText = buttonSender->objectName(); // retrive the text from the button clicked
    QString id = buttonText.mid(4, 3);
    uint8_t key = id.toUInt();
    uint8_t velocity = 255;
    //showStatusMessage(id);


    char message[3] = {NOTE_OFF, key, velocity};

    //showStatusMessage("Sent: " + QString::number(key));
    showStatusMessage("Sent: " + QString(QByteArray(message, 3).toHex()));
    //writeData(QByteArray(message, 3));

    MESSAGE_MIDI msg;
    msg.status = NOTE_OFF;
    msg.key = key+35;
    msg.velocity = velocity;

    uint8_t frame[100];
    uint8_t *auxptr = frame;
    *auxptr = MIDI;
    auxptr++;
    memcpy(auxptr, &msg, sizeof(msg));
    writeData(QByteArray::fromRawData((char *)frame, sizeof(msg)+1));


}

void MainWindow::sendMessage(uint8_t messageType, void *parameter, int32_t parameterSize)
{
    uint8_t frame[100];
    uint8_t *auxptr = frame;
    *auxptr = messageType;
    auxptr++;
    memcpy(auxptr, parameter, parameterSize);
    writeData(QByteArray::fromRawData((char *)frame, parameterSize+1));
}

void MainWindow::on_DialReleased()
{
    QDial* dialSender = qobject_cast<QDial*>(sender()); // retrieve the button you have clicked
    QString dialText = dialSender->objectName(); // retrive the text from the button clicked

    if (dialText == ui->OscA_Waveform->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(OSC_A_WAVE_TYPE, (void *)&msg, sizeof(msg));
        switch(msg.data)
        {
            case 0: ui->OscA_Waveform_Value->setText("SINE"); break;
            case 1: ui->OscA_Waveform_Value->setText("SAW"); break;
            case 2: ui->OscA_Waveform_Value->setText("TRIANGLE"); break;
            case 3: ui->OscA_Waveform_Value->setText("SQUARE"); break;
        }
    }
    else if (dialText == ui->OscA_Detune->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(OSC_A_DETUNE, (void *)&msg, sizeof(msg));
        ui->OscA_Detune_Value->setText(QString::number(msg.data) + " S");
    }
    else if (dialText == ui->OscA_PulseWidth->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(OSC_A_SQUARE_PW, (void *)&msg, sizeof(msg));
        ui->OscA_PulseWidth_Value->setText(QString::number(msg.data/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->OscA_Mix->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(OSC_A_MIX, (void *)&msg, sizeof(msg));
        ui->OscA_Mix_Value->setText(QString::number(msg.data/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->OscB_Waveform->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(OSC_B_WAVE_TYPE, (void *)&msg, sizeof(msg));
        switch(msg.data)
        {
            case 0: ui->OscB_Waveform_Value->setText("SINE"); break;
            case 1: ui->OscB_Waveform_Value->setText("SAW"); break;
            case 2: ui->OscB_Waveform_Value->setText("TRIANGLE"); break;
            case 3: ui->OscB_Waveform_Value->setText("SQUARE"); break;
        }
    }
    else if (dialText == ui->OscB_Detune->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        showStatusMessage("");
        sendMessage(OSC_B_DETUNE, (void *)&msg, sizeof(msg));
        ui->OscB_Detune_Value->setText(QString::number(msg.data) + " S");
    }
    else if (dialText == ui->OscB_PulseWidth->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        showStatusMessage("");
        sendMessage(OSC_B_SQUARE_PW, (void *)&msg, sizeof(msg));
        ui->OscB_PulseWidth_Value->setText(QString::number(msg.data/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->OscB_Mix->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(OSC_B_MIX, (void *)&msg, sizeof(msg));
        ui->OscB_Mix_Value->setText(QString::number(msg.data/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->OscC_Waveform->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(OSC_C_WAVE_TYPE, (void *)&msg, sizeof(msg));
        switch(msg.data)
        {
            case 0: ui->OscC_Waveform_Value->setText("SINE"); break;
            case 1: ui->OscC_Waveform_Value->setText("SAW"); break;
            case 2: ui->OscC_Waveform_Value->setText("TRIANGLE"); break;
            case 3: ui->OscC_Waveform_Value->setText("SQUARE"); break;
        }
    }
    else if (dialText == ui->OscC_Detune->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        showStatusMessage("");
        sendMessage(OSC_C_DETUNE, (void *)&msg, sizeof(msg));
        ui->OscC_Detune_Value->setText(QString::number(msg.data) + " S");
    }
    else if (dialText == ui->OscC_PulseWidth->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        showStatusMessage("");
        sendMessage(OSC_C_SQUARE_PW, (void *)&msg, sizeof(msg));
        ui->OscC_PulseWidth_Value->setText(QString::number(msg.data/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->OscC_Mix->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(OSC_C_MIX, (void *)&msg, sizeof(msg));
        ui->OscC_Mix_Value->setText(QString::number(msg.data/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->LFO_Waveform->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(LFO_A_WAVE_TYPE, (void *)&msg, sizeof(msg));
        switch(msg.data)
        {
            case 0: ui->LFO_Waveform_Value->setText("SAW"); break;
            case 1: ui->LFO_Waveform_Value->setText("SQUARE"); break;
        }
    }
    else if (dialText == ui->LFO_Rate->objectName())
    {
        STD_MESSAGE msg;
        float value = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        msg.data = value;
        sendMessage(LFO_A_RATE, (void *)&msg, sizeof(msg));
        ui->LFO_Rate_Value->setText(QString::number(value/4095.0*30000, 'f', 2) + " ms");
    }
    else if (dialText == ui->LFO_Amount->objectName())
    {
        STD_MESSAGE msg;
        float value = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        msg.data = value;
        sendMessage(LFO_A_AMOUNT, (void *)&msg, sizeof(msg));
        ui->LFO_Amount_Value->setText(QString::number(value/4095.0*100, 'f', 2) + " %");
    }
    else if (dialText == ui->LFO_B_Waveform->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(LFO_B_WAVE_TYPE, (void *)&msg, sizeof(msg));
        switch(msg.data)
        {
            case 0: ui->LFO_B_Waveform_Value->setText("SAW"); break;
            case 1: ui->LFO_B_Waveform_Value->setText("SQUARE"); break;
        }
    }
    else if (dialText == ui->LFO_B_Rate->objectName())
    {
        STD_MESSAGE msg;
        float value = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        msg.data = value;
        sendMessage(LFO_B_RATE, (void *)&msg, sizeof(msg));
        ui->LFO_B_Rate_Value->setText(QString::number(value/4095.0*30000, 'f', 2) + " ms");
    }
    else if (dialText == ui->LFO_B_Amount->objectName())
    {
        STD_MESSAGE msg;
        float value = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        msg.data = value;
        sendMessage(LFO_B_AMOUNT, (void *)&msg, sizeof(msg));
        ui->LFO_B_Amount_Value->setText(QString::number(value/4095.0*100, 'f', 2) + " %");
    }
    else if (dialText == ui->LFO_C_Waveform->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(LFO_C_WAVE_TYPE, (void *)&msg, sizeof(msg));
        switch(msg.data)
        {
            case 0: ui->LFO_C_Waveform_Value->setText("SAW"); break;
            case 1: ui->LFO_C_Waveform_Value->setText("SQUARE"); break;
        }
    }
    else if (dialText == ui->LFO_C_Rate->objectName())
    {
        STD_MESSAGE msg;
        float value = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        msg.data = value;
        sendMessage(LFO_C_RATE, (void *)&msg, sizeof(msg));
        ui->LFO_C_Rate_Value->setText(QString::number(value/4095.0*30000, 'f', 2) + " ms");
    }
    else if (dialText == ui->LFO_C_Amount->objectName())
    {
        STD_MESSAGE msg;
        float value = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        msg.data = value;
        sendMessage(LFO_C_AMOUNT, (void *)&msg, sizeof(msg));
        ui->LFO_C_Amount_Value->setText(QString::number(value/4095.0*100, 'f', 2) + " %");
    }
    else if (dialText == ui->ADSR_Attack->objectName())
    {
        STD_MESSAGE msg;
        float value = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        msg.data = (int16_t)value;
        sendMessage(ADSR_ATTACK, (void *)&msg, sizeof(msg));
        ui->ADSR_Attack_Value->setText(QString::number(value/4095.0*10000, 'f', 0) + " ms");
    }
    else if (dialText == ui->ADSR_Decay->objectName())
    {
        STD_MESSAGE msg;
        float value = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        msg.data = (int16_t)value;
        sendMessage(ADSR_DECAY, (void *)&msg, sizeof(msg));
        ui->ADSR_Decay_Value->setText(QString::number(value/4095.0*10000, 'f', 0) + " ms");
    }
    else if (dialText == ui->ADSR_Sustain->objectName())
    {
        STD_MESSAGE msg;
        float value = dialSender->value();
        msg.data = (int16_t)value;
        sendMessage(ADSR_SUSTAIN, (void *)&msg, sizeof(msg));
        ui->ADSR_Sustain_Value->setText(QString::number(value/4095.0, 'f', 2));
    }
    else if (dialText == ui->ADSR_Release->objectName())
    {
        STD_MESSAGE msg;
        float value = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        msg.data = (int16_t)value;
        sendMessage(ADSR_RELEASE, (void *)&msg, sizeof(msg));
        ui->ADSR_Release_Value->setText(QString::number(value/4095.0*10000, 'f', 0) + " ms");
    }
    else if (dialText == ui->Filter_Type->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(FILTER_TYPE, (void *)&msg, sizeof(msg));
        switch(msg.data)
        {
            case 0: ui->Filter_Type_Value->setText("12 dB LP"); break;
            case 1: ui->Filter_Type_Value->setText("24 dB LP"); break;
            case 2: ui->Filter_Type_Value->setText("12 dB HP"); break;
            case 3: ui->Filter_Type_Value->setText("24 dB HP"); break;
            case 4: ui->Filter_Type_Value->setText("12 dB BP"); break;
            case 5: ui->Filter_Type_Value->setText("24 dB BP"); break;
        }
    }
    else if (dialText == ui->Filter_Cutoff->objectName())
    {
        STD_MESSAGE msg;
        float value = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        msg.data = (int16_t)value;
        sendMessage(FILTER_CUTOFF, (void *)&msg, sizeof(msg));
        ui->Filter_Cutoff_Value->setText(QString::number(msg.data/4095.0*20000, 'f', 0) + " Hz");
    }
    else if (dialText == ui->Filter_Resonance->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(FILTER_RESONANCE, (void *)&msg, sizeof(msg));
        ui->Filter_Resonance_Value->setText(QString::number(msg.data/4095.0, 'f', 2));
    }
    else if (dialText == ui->Filter_Envelope->objectName())
    {
        STD_MESSAGE msg;
        msg.data = dialSender->value();
        sendMessage(FILTER_ENVELOPE, (void *)&msg, sizeof(msg));
        ui->Filter_Envelope_Value->setText(QString::number(msg.data/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->Filter_Attack->objectName())
    {
        STD_MESSAGE msg;
        float value = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        msg.data = (int16_t)value;
        sendMessage(FILTER_ATTACK, (void *)&msg, sizeof(msg));
        ui->Filter_Attack_Value->setText(QString::number(msg.data/4095.0*10000, 'f', 0) + " ms");
    }
    else if (dialText == ui->Filter_Decay->objectName())
    {
        STD_MESSAGE msg;
        float value = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        msg.data = (int16_t)value;
        sendMessage(FILTER_DECAY, (void *)&msg, sizeof(msg));
        ui->Filter_Decay_Value->setText(QString::number(msg.data/4095.0*10000, 'f', 0) + " ms");
    }
    else if (dialText == ui->Filter_Sustain->objectName())
    {
        STD_MESSAGE msg;
        float value = dialSender->value();
        msg.data = (int16_t)value;
        sendMessage(FILTER_SUSTAIN, (void *)&msg, sizeof(msg));
        ui->Filter_Sustain_Value->setText(QString::number(msg.data/4095.0, 'f', 2));
    }
    else if (dialText == ui->Filter_Release->objectName())
    {
        STD_MESSAGE msg;
        float value = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        msg.data = (int16_t)value;
        sendMessage(FILTER_RELEASE, (void *)&msg, sizeof(msg));
        ui->Filter_Release_Value->setText(QString::number(msg.data/4095.0*10000, 'f', 0) + " ms");
    }

}

void MainWindow::on_DialChanged()
{
    QDial* dialSender = qobject_cast<QDial*>(sender()); // retrieve the button you have clicked
    QString dialText = dialSender->objectName(); // retrive the text from the button clicked

    int value = dialSender->value();

    if (dialText == ui->OscA_Waveform->objectName())
    {
        switch(value)
        {
            case 0: ui->OscA_Waveform_Value->setText("SINE"); break;
            case 1: ui->OscA_Waveform_Value->setText("SAW"); break;
            case 2: ui->OscA_Waveform_Value->setText("TRIANGLE"); break;
            case 3: ui->OscA_Waveform_Value->setText("SQUARE"); break;
        }
    }
    else if (dialText == ui->OscA_Detune->objectName())
    {
        ui->OscA_Detune_Value->setText(QString::number(value) + " S");
    }
    else if (dialText == ui->OscA_PulseWidth->objectName())
    {
        ui->OscA_PulseWidth_Value->setText(QString::number(value/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->OscA_Mix->objectName())
    {
        ui->OscA_Mix_Value->setText(QString::number(value/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->OscB_Waveform->objectName())
    {
        switch(value)
        {
            case 0: ui->OscB_Waveform_Value->setText("SINE"); break;
            case 1: ui->OscB_Waveform_Value->setText("SAW"); break;
            case 2: ui->OscB_Waveform_Value->setText("TRIANGLE"); break;
            case 3: ui->OscB_Waveform_Value->setText("SQUARE"); break;
        }
    }
    else if (dialText == ui->OscB_Detune->objectName())
    {
        ui->OscB_Detune_Value->setText(QString::number(value) + " S");
    }
    else if (dialText == ui->OscB_PulseWidth->objectName())
    {
        ui->OscB_PulseWidth_Value->setText(QString::number(value/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->OscB_Mix->objectName())
    {
        ui->OscB_Mix_Value->setText(QString::number(value/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->OscC_Waveform->objectName())
    {
        switch(value)
        {
            case 0: ui->OscC_Waveform_Value->setText("SINE"); break;
            case 1: ui->OscC_Waveform_Value->setText("SAW"); break;
            case 2: ui->OscC_Waveform_Value->setText("TRIANGLE"); break;
            case 3: ui->OscC_Waveform_Value->setText("SQUARE"); break;
        }
    }
    else if (dialText == ui->OscC_Detune->objectName())
    {
        ui->OscC_Detune_Value->setText(QString::number(value) + " S");
    }
    else if (dialText == ui->OscC_PulseWidth->objectName())
    {
        ui->OscC_PulseWidth_Value->setText(QString::number(value/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->OscC_Mix->objectName())
    {
        ui->OscC_Mix_Value->setText(QString::number(value/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->LFO_Waveform->objectName())
    {
        switch(value)
        {
            case 0: ui->LFO_Waveform_Value->setText("SAW"); break;
            case 1: ui->LFO_Waveform_Value->setText("SQUARE"); break;
        }
    }
    else if (dialText == ui->LFO_Rate->objectName())
    {
        float value_exp = 4095.0*powf(value/4095.0, 3.0);
        ui->LFO_Rate_Value->setText(QString::number(value_exp/4095.0*30000, 'f', 2) + " ms");
    }
    else if (dialText == ui->LFO_Amount->objectName())
    {
        float value_exp = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        ui->LFO_Amount_Value->setText(QString::number(value_exp/4095.0*100, 'f', 2) + " %");
    }
    else if (dialText == ui->LFO_B_Waveform->objectName())
    {
        switch(value)
        {
            case 0: ui->LFO_B_Waveform_Value->setText("SAW"); break;
            case 1: ui->LFO_B_Waveform_Value->setText("SQUARE"); break;
        }
    }
    else if (dialText == ui->LFO_B_Rate->objectName())
    {
        float value_exp = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        ui->LFO_B_Rate_Value->setText(QString::number(value_exp/4095.0*30000, 'f', 2) + " ms");
    }
    else if (dialText == ui->LFO_B_Amount->objectName())
    {
        float value_exp = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        ui->LFO_B_Amount_Value->setText(QString::number(value_exp/4095.0*100, 'f', 2) + " %");
    }
    else if (dialText == ui->LFO_C_Waveform->objectName())
    {
        switch(value)
        {
            case 0: ui->LFO_C_Waveform_Value->setText("SAW"); break;
            case 1: ui->LFO_C_Waveform_Value->setText("SQUARE"); break;
        }
    }
    else if (dialText == ui->LFO_C_Rate->objectName())
    {
        float value_exp = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        ui->LFO_C_Rate_Value->setText(QString::number(value_exp/4095.0*30000, 'f', 2) + " ms");
    }
    else if (dialText == ui->LFO_C_Amount->objectName())
    {
        float value_exp = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        ui->LFO_C_Amount_Value->setText(QString::number(value_exp/4095.0*100, 'f', 2) + " %");
    }
    else if (dialText == ui->ADSR_Attack->objectName())
    {
        float value_exp = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        ui->ADSR_Attack_Value->setText(QString::number(value_exp/4095.0*10000, 'f', 0) + " ms");
    }
    else if (dialText == ui->ADSR_Decay->objectName())
    {
        float value_exp = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        ui->ADSR_Decay_Value->setText(QString::number(value_exp/4095.0*10000, 'f', 0) + " ms");
    }
    else if (dialText == ui->ADSR_Sustain->objectName())
    {
        ui->ADSR_Sustain_Value->setText(QString::number(value/4095.0, 'f', 2));
    }
    else if (dialText == ui->ADSR_Release->objectName())
    {
        float value_exp = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        ui->ADSR_Release_Value->setText(QString::number(value_exp/4095.0*10000, 'f', 0) + " ms");
    }
    else if (dialText == ui->Filter_Type->objectName())
    {
        switch(value)
        {
            case 0: ui->Filter_Type_Value->setText("12 dB LP"); break;
            case 1: ui->Filter_Type_Value->setText("24 dB LP"); break;
            case 2: ui->Filter_Type_Value->setText("12 dB HP"); break;
            case 3: ui->Filter_Type_Value->setText("24 dB HP"); break;
            case 4: ui->Filter_Type_Value->setText("12 dB BP"); break;
            case 5: ui->Filter_Type_Value->setText("24 dB BP"); break;
        }
    }
    else if (dialText == ui->Filter_Cutoff->objectName())
    {
        float value_exp = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        ui->Filter_Cutoff_Value->setText(QString::number(value_exp/4095.0*20000, 'f', 0) + " Hz");
    }
    else if (dialText == ui->Filter_Resonance->objectName())
    {
        ui->Filter_Resonance_Value->setText(QString::number(value/4095.0, 'f', 2));
    }
    else if (dialText == ui->Filter_Envelope->objectName())
    {
        ui->Filter_Envelope_Value->setText(QString::number(value/4095.0*100, 'f', 0) + " %");
    }
    else if (dialText == ui->Filter_Attack->objectName())
    {
        float value_exp = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        ui->Filter_Attack_Value->setText(QString::number(value_exp/4095.0*10000, 'f', 0) + " ms");
    }
    else if (dialText == ui->Filter_Decay->objectName())
    {
        float value_exp = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        ui->Filter_Decay_Value->setText(QString::number(value_exp/4095.0*10000, 'f', 0) + " ms");
    }
    else if (dialText == ui->Filter_Sustain->objectName())
    {
        ui->Filter_Sustain_Value->setText(QString::number(value/4095.0, 'f', 2));
    }
    else if (dialText == ui->Filter_Release->objectName())
    {
        float value_exp = 4095.0*powf(dialSender->value()/4095.0, 3.0);
        ui->Filter_Release_Value->setText(QString::number(value_exp/4095.0*10000, 'f', 0) + " ms");
    }

}

void MainWindow::writeData(const QByteArray &data)
{
    m_serial->write(data);
}

void MainWindow::readData()
{
    static bool newMessage = true;

    QByteArray data = m_serial->readAll();

    if (newMessage)
    {
        newMessage = false;
        if (data.back() == '\n')
        {
            data.chop(1);
            newMessage = true;
        }
        showStatusMessage(QString(data));
    }
    else
    {
        if (data.back() == '\n')
        {
            data.chop(1);
            newMessage = true;
        }
        showStatusMessage(m_status->text() + QString(data));
    }
}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}
