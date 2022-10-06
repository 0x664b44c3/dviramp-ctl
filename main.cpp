#include <QCoreApplication>
#include <dvirampio.h>
#include <QSerialPort>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QTimer>
#include <iostream>
#include <QRect>

int takeChannelArg(QString & value)
{
    QString ch="a";
    int split = value.indexOf(':');
    if (split>0)
    {
        ch = value.left(split);
        value = value.mid(split+1);
    }
    ch = ch.left(1);
    int channel = QString("ab").indexOf(ch.toLower());
    if (channel<0)
    {
        channel = 0;
        qWarning () << "Invalid channel, assuming channel A.";
    }
    return channel;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QCommandLineOption baudOption(
                QStringList() << "b" << "baud",
                QObject::tr("Baud rate"),
                QObject::tr("rate"),
                "38400");
    
    QCommandLineOption portOption(
                QStringList() << "p" << "port",
                QObject::tr("Serial port"),
                QObject::tr("port"));
    
    QCommandLineOption rawCmdOption(
                "rawcmd",
                "raw command",
                "raw cmd");
    QCommandLineParser cmd;
    
    
    QCommandLineOption optOutputFmt(QStringList() << "O" << "output-fmt", QObject::tr("set sdi output format."), "resolution");
    QCommandLineOption optDeFlicker(QStringList() << "deflicker", QObject::tr("set sdi output deflicker."), "channel:deflicker");
    QCommandLineOption optGetFmt(QStringList() << "o" << "get-fmt", QObject::tr("show output fmt."));
    QCommandLineOption optSetWindow(QStringList() << "W" << "set-window", QObject::tr("set window\nWindow format: channel:x0:y0:w:h."), "window");
    QCommandLineOption optGetWindow(QStringList() << "w" << "window", QObject::tr("get extraction window (channel: a or b)."), "id");
    QCommandLineOption optSetLut(QStringList() << "L" << "set-lut", QObject::tr("Set LUT\nlut either linear or 709."), "lut");
    QCommandLineOption optSetTpg(QStringList() << "T" << "set-tpg", QObject::tr("Set TPG for channel a or b to [off,bars,ramp]."), "channel:mode");
    QCommandLineOption optSetPrefMode(QStringList() << "P" << "set-pref",
                                      QObject::tr("Set preferred input mode.\n."
                                                  "Syntax: [input]:w:h:rate.\n"
                                                  "Input is either A or B, if no inut is given, A will be assumed."),
                                      "mode");

    QCommandLineOption optSystemMode(QStringList() << "M" << "mode",
                                     QObject::tr("Select mode of operation.\n"),
                                     "mode", "?");

    QCommandLineOption optSetDviMode("dvi-mode", QObject::tr("Set DVI mode (single, dual-link, dual-head"), "mode", "single");
    QCommandLineOption optProps(QStringList() << "properties", QObject::tr("Show property map."));
    QCommandLineOption optWriteNv(QStringList() << "write-nv", QObject::tr("Write settings to NV (powerup defaults)."));
    cmd.addOption(optOutputFmt);
    cmd.addOption(optGetFmt);
    cmd.addOption(optSetWindow);
    cmd.addOption(optGetWindow);
    cmd.addOption(optSetLut);
    cmd.addOption(optSetTpg);
    cmd.addOption(optSetPrefMode);
    cmd.addOption(optSetDviMode);
    cmd.addOption(optSystemMode);
    cmd.addOption(optDeFlicker);
    cmd.addOption(optWriteNv);

    
    cmd.addOption(optProps);
    
    cmd.addOption(baudOption);
    cmd.addOption(portOption);
    cmd.addOption(rawCmdOption);
    
    cmd.addPositionalArgument("command",
                              QObject::tr("parameter value"));
    cmd.addHelpOption();
    cmd.addVersionOption();
    cmd.setApplicationDescription(QObject::tr("DVI Ramp Console Utility"));
    
    cmd.process(a);
    if (!cmd.isSet("port"))
    {
        qWarning() << "port not set";
        cmd.showHelp(1);
    }
    int baud = cmd.value("baud").toInt();
    if (!baud)
    {
        qWarning() << "illegal value for baud rate, reset to 38400";
        baud = 38400;
    }
    QSerialPort port(cmd.value("port"), &a);
    if (!port.setBaudRate(baud))
    {
        qCritical() << "Cannot set baud rate"<<baud;
        //        return 1;
    }
    if (!port.open(QIODevice::ReadWrite))
    {
        qCritical() << "Cannot open port"<<port.portName();
        //        return 1;
    }
    
    //	QSerialPort port("/dev/ttyACM0", &a);
    bool hasCmd = false;
    dviRampIo ramp(&port, &a);
    ramp.queryStatus();
    if (cmd.isSet("rawcmd"))
    {
        ramp.sendCmd(cmd.value("rawcmd").toUpper());
        hasCmd = true;
    }
    else
    {
        if (cmd.isSet(optOutputFmt))
        {
            QString request = cmd.value(optOutputFmt);
            auto formats = ramp.formatCodes();
            bool formatFound = false;
            dviRampIo::outputVideoFmt fmt = dviRampIo::sdi1080i60;
            for (auto it = formats.begin(); it != formats.end(); ++it)
            {
                if (it.key().toLower() == request.toLower())
                {
                    formatFound = true;
                    fmt = it.value();
                    break;
                }
            }
            if (!formatFound)
            {
                std::cerr << "Format '" << request.toStdString() << "' not found.\n"
                          << "Valid formats are:\n    ";
                int max = 0;
                foreach(QString f, formats.keys())
                    max = std::max(f.length(), max);
                max+=4;
                int p=0;
                foreach(QString f, formats.keys())
                {
                    if (p>=40)
                    {
                        std::cerr << "\n    ";
                        p=0;
                    }
                    std::cerr << f.leftJustified(max, ' ').toStdString();
                    p+=max;
                }
                std::cerr << "\n";
                return 1;
            }
            ramp.setOutputFmt(fmt);
            hasCmd = true;
        }
        if (cmd.isSet(optGetFmt))
        {
            hasCmd = true;
        }
        if (cmd.isSet(optSetDviMode))
        {
            QString modeString = cmd.value(optSetDviMode).toLower();
            dviRampIo::dviMode mode = dviRampIo::dviSingleLink;
            if (modeString == "single")
                mode = dviRampIo::dviSingleLink;
            if (modeString == "dual-link")
                mode = dviRampIo::dviDualLink;
            if (modeString == "dual-head")
                mode = dviRampIo::dviDualHead;
            ramp.setDviMode(mode);
        }
        if (cmd.isSet(optSetWindow))
        {
            QString id = "a";
            QStringList args = cmd.value(optSetWindow).split(':');
            if ((args.size()<4) || (args.size()>5))
                cmd.showHelp(1);
            if (args.size()==5)
            {
                id = args.takeFirst();
            }
            bool ok;
            int x0 = args.takeFirst().toInt(&ok);
            if (!ok)
            {
                std::cerr << "Arugment x0 not numeric\n";
                cmd.showHelp(1);
            }
            int y0 = args.takeFirst().toInt(&ok);
            if (!ok)
            {
                std::cerr << "Arugment y0 not numeric\n";
                cmd.showHelp(1);
            }
            int w = args.takeFirst().toInt(&ok);
            if (!ok)
            {
                std::cerr << "Arugment w not numeric\n";
                cmd.showHelp(1);
            }
            int h = args.takeFirst().toInt(&ok);
            if (!ok)
            {
                std::cerr << "Arugment h not numeric\n";
                cmd.showHelp(1);
            }
            id = id.left(1);
            int wdwId = QString("ab").indexOf(id.toLower());
            if (wdwId<0)
            {
                std::cerr << "Invalid channel ID. Assuming A.\n";
                wdwId = 0;
            }
            ramp.setExtractionWindow(wdwId, QRect(x0, y0, w, h));
            hasCmd = true;
        }
        if (cmd.isSet(optSetPrefMode))
        {
            QString id = "a";
            QStringList args = cmd.value(optSetPrefMode).split(':');
            if ((args.size()<3) || (args.size()>4))
                cmd.showHelp(1);
            if (args.size()>3)
            {
                id = args.takeFirst();
            }
            std::cerr << "set preferred mode\n";
            bool ok;
            int width = args.takeFirst().toInt(&ok);
            if (!ok)
            {
                std::cerr << "Argument <width> not numeric\n";
                cmd.showHelp(1);
            }
            int height = args.takeFirst().toInt(&ok);
            if (!ok)
            {
                std::cerr << "Argument <height> not numeric\n";
                cmd.showHelp(1);
            }
            double rate = args.takeFirst().toDouble(&ok);
            if (!ok)
            {
                std::cerr << "Argument <rate> not numeric\n";
                cmd.showHelp(1);
            }
            id = id.left(1);
            int inputId = QString("ab").indexOf(id.toLower());
            if (inputId<0)
            {
                std::cerr << "Invalid input ID. Assuming A.\n";
                inputId = 0;
            }
            ramp.setPrefInputFmt(inputId, width, height, rate * 100.0);
            hasCmd = true;
        }
        if (cmd.isSet(optGetWindow))
        {
            hasCmd = true;
        }
        if (cmd.isSet(optSetLut))
        {
            QString lut = cmd.value(optSetLut).toLower();
            int channel = takeChannelArg(lut);
            if (lut=="lin")
                ramp.setOutputLUT(channel, dviRampIo::lutLinear);
            else if (lut=="709")
                ramp.setOutputLUT(channel, dviRampIo::lutITU_709);
            else
            {
                std::cerr << "LUT allowed values: lin, 709\n";
            }
            hasCmd = true;
        }
        
        if (cmd.isSet(optSetTpg))
        {
            QString tpgMode = cmd.value(optSetTpg);
            QString ch = "a";
            int split = tpgMode.indexOf(':');
            if (split>0)
            {
                ch = tpgMode.left(split);
                tpgMode = tpgMode.mid(split+1);
            }
            ch = ch.left(1);
            int channel = QString("ab").indexOf(ch.toLower());
            if (channel<0)
            {
                std::cerr << "Invalid channel ID. Assuming A.\n";
                channel = 0;
            }
            dviRampIo::tpgMode mode = dviRampIo::tpgOff;
            tpgMode = tpgMode.toLower();
            if (tpgMode == "off")
            {
                mode = dviRampIo::tpgOff;
            }
            else if (tpgMode == "bars")
            {
                mode = dviRampIo::tpgBars;
            }
            else if (tpgMode == "ramp")
            {
                mode = dviRampIo::tpgRamp;
            }
            else
            {
                std::cerr << "Invalid TPG mode. Valid modes are: off, bars, ramp.\nDisabling TPG on channel "<<channel+1<<".\n";
                mode = dviRampIo::tpgOff;
            }
            ramp.setTPG(channel, mode);
            hasCmd = true;
        }
    }
    if (cmd.isSet(optDeFlicker))
    {
        QString deflicker = cmd.value(optDeFlicker);
        int channel = takeChannelArg(deflicker);
        bool ok = false;
        int v_deflicker = deflicker.toInt(&ok);
        if (!ok)
        {
            std::cerr << "Expcteing numeric deflicker value\n";
            return 1;
        }
        ramp.setDeflicker(channel, v_deflicker);
    }
    if (cmd.isSet(optSystemMode))
    {
        QString newMode = cmd.value(optSystemMode);
        if (newMode != "?")
        {
            QString cmd;
            newMode = newMode.toLower().trimmed();
            if (newMode == "p2p")
                cmd = "PIX2PIX";
            if (newMode == "scaler")
                cmd = "SCALER";
            if (newMode == "fillkey")
                cmd = "FILL&KEY";
            if (newMode == "dual")
                cmd = "INDEP";
            if (newMode == "sdhd")
                cmd = "SD/HD";
            if (cmd.length())
            {
                cmd = "WOS SDI FUNC " + cmd;
                ramp.sendCmd(cmd);
                hasCmd = true;
            }
        }
    }
    if (cmd.isSet(optWriteNv))
    {
        ramp.writeNv();
    }
    
    if (hasCmd)
    {
        //@TODO: ad some signal like "command queue done" and use it to terminate the app
        QTimer::singleShot(1500, &a, SLOT(quit()));
    }
    if (!hasCmd)
    { //just query the status and quit
        a.connect(&ramp, &dviRampIo::statusUpdateComplete,
                  [&](){
            //            std::cerr << "status update complete\n";
            std::cout << "DVI Ramp h/w " << ramp.property("VER HARD").toStdString() << "\n";
            std::cout << "Firmware " << ramp.property("VER RELEASE").toStdString() << "\n";
            if (cmd.isSet(optProps))
            {
                auto props = ramp.properties();
                int maxId=0;
                for(auto k: props.keys())
                    maxId = std::max(maxId, k.length());
                maxId +=3;
                for(auto it = props.begin(); it != props.end(); ++it)
                {
                    std::cout << it.key().leftJustified(maxId, ' ').toStdString()
                              << it.value().toStdString() << "\n";
                }
                
            }
            if (cmd.isSet(optSystemMode))
            {
                std::cout << "System mode: " << ramp.property("DR2 MODE").toStdString() << "\n";
            }
            a.quit();
        });
        
    }
    
    return a.exec();
}
