#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "spocktcpsocket.h"
#include "inputdev/DeviceFinder.hpp"
#include "inputdev/DualshockPad.hpp"
#include "inputdev/XInputPad.hpp"
#include "inputdev/GenericPad.hpp"
#include <QtAV/QtAV.h>

namespace Ui {
class MainWindow;
}
class SpockDriveTrackpad;

class DeviceFinder : public QObject, public boo::DeviceFinder,
        public boo::IDualshockPadCallback, public boo::IXInputPadCallback
{
    Q_OBJECT
    std::shared_ptr<boo::DeviceBase> m_device = nullptr;
public:
    DeviceFinder()
        : boo::DeviceFinder({typeid(boo::DualshockPad),
                             typeid(boo::XInputPad),
                             typeid(boo::GenericPad)})
    {
    }

    ~DeviceFinder()
    {
        if (m_device)
            m_device->closeDevice();
    }

    void deviceConnected(boo::DeviceToken& tok)
    {
        if (!m_device)
        {
            m_device = tok.openAndGetDevice();
            if (boo::DualshockPad* pad = dynamic_cast<boo::DualshockPad*>(m_device.get()))
                pad->setCallback(this);
            else if (boo::XInputPad* pad = dynamic_cast<boo::XInputPad*>(m_device.get()))
                pad->setCallback(this);
            gamepadConnected();
        }
    }

    void deviceDisconnected(boo::DeviceToken&, boo::DeviceBase* dev)
    {
        if (dev == m_device.get())
        {
            gamepadDisconnected();
            m_device.reset();
        }
    }

    void controllerUpdate(boo::DualshockPad&, const boo::DualshockPadState& state)
    {
        emit axisLeftXChanged(state.m_leftStick[0] / 127.f - 1.f);
        emit axisLeftYChanged(state.m_leftStick[1] / 127.f - 1.f);
        emit axisRightXChanged(state.m_rightStick[0] / 127.f - 1.f);
        emit axisRightYChanged(state.m_rightStick[1] / 127.f - 1.f);
        emit buttonsChanged(state.m_buttonState);
        emit shoulderLeftChanged(state.m_pressureL2);
        emit shoulderRightChanged(state.m_pressureR2);
    }

    void controllerUpdate(boo::XInputPad&, const boo::XInputPadState& state)
    {
        emit axisLeftXChanged(state.sThumbLX / 32768.f);
        emit axisLeftYChanged(-state.sThumbLY / 32768.f);
        emit axisRightXChanged(state.sThumbRX / 32768.f);
        emit axisRightYChanged(-state.sThumbRY / 32768.f);

        int buttons = 0;
        buttons |= state.wButtons & 0x8000 ? 0x1000 : 0;
        buttons |= state.wButtons & 0x2000 ? 0x2000 : 0;
        buttons |= state.wButtons & 0x1000 ? 0x4000 : 0;
        buttons |= state.wButtons & 0x4000 ? 0x8000 : 0;
        buttons |= state.wButtons & 0x200 ? 0x800 : 0;
        buttons |= state.wButtons & 0x100 ? 0x100 : 0;
        buttons |= state.bLeftTrigger > 127 ? 0x400 : 0;
        emit buttonsChanged(buttons);

        emit shoulderLeftChanged(state.bLeftTrigger);
        emit shoulderRightChanged(state.bRightTrigger);
    }

signals:
    void gamepadConnected();
    void gamepadDisconnected();
    void axisLeftXChanged(double val);
    void axisLeftYChanged(double val);
    void axisRightXChanged(double val);
    void axisRightYChanged(double val);
    void shoulderLeftChanged(double val);
    void shoulderRightChanged(double val);
    void buttonsChanged(int val);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void timerEvent(QTimerEvent *event);
    void changeLeftThrottle(double value);
    void changeRightThrottle(double value);
    void resetBucketButtons();

public slots:
    void armSliderChanged(int value);
    void bucketSliderChanged(int value);
    void driveMove(SpockDriveTrackpad* sender, QMouseEvent* ev);
    void driveRelease(SpockDriveTrackpad* sender, QMouseEvent* ev);
    void bucketDrive();
    void bucketScoop();
    void bucketDump();
    void bucketWeigh();
    void bucketPanic();
    void gamepadConnected();
    void gamepadDisconnected();
    void axisLeftXChanged(double value);
    void axisLeftYChanged(double value);
    void axisRightXChanged(double value);
    void axisRightYChanged(double value);
    void shoulderLeftChanged(double value);
    void shoulderRightChanged(double value);
    void buttonsChanged(int buttons);
    void connectionEstablished();
    void connectionLost();
    void doReconnect();
    void sendCommandPacket();

private:
    std::unique_ptr<Ui::MainWindow> ui;
    SpockTCPSocket m_socket;
    SpockCommandData m_commandData;
    DeviceFinder m_devFinder;
    bool m_moveDragging = false;
    int m_lastButtons = 0;
    bool m_canSend = false;
    QtAV::AVPlayer m_cam0;
    QtAV::AVPlayer m_cam1;
    QtAV::AVPlayer m_cam2;
};

#endif // MAINWINDOW_H
