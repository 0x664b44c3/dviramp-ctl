#include "dvirampio.h"
#include <QIODevice>
#include <QDebug>
#include <QMap>
#include <QRect>
QMap<QString, QString> sdiResolutions;
//"SD-525", "SD-625", "1280x720@60p", "1280x720@59.94p", "1280x720@50p", "1920x1080@60i", "1920x1080@59.94i", "1920x1080@50i/25PsF", "1920x1080@24PsF", "1920x1080@23.98PsF", "1920x1080@25p", "1920x1080@24p", "1920x1080@23.98p" });

QMap<dviRampIo::outputVideoFmt, QString> dviRampIo::sdiFormats()
{
    QMap<outputVideoFmt, QString> map;
    map.insert(sdiNTSC, "SD: PAL 480i30");
    map.insert(sdiPAL, "SD: PAL 576i25");
    map.insert(sdi720p60, "HD: 720p60");
    map.insert(sdi720p59, "HD: 720p59");
    map.insert(sdi720p50, "HD: 720p50");
    map.insert(sdi1080i60, "HD: 1080i60");
    map.insert(sdi1080i59, "HD: 1080i59.94");
    map.insert(sdi1080i50, "HD: 1080i50");
    map.insert(sdi1080PsF24, "HD: 1080PsF24");
    map.insert(sdi1080PsF23, "HD: 1080PsF23.98");
    map.insert(sdi1080p25, "HD: 1080p25");
    map.insert(sdi1080p24, "HD: 1080p24");
    map.insert(sdi1080p23, "HD: 1080p23.98");
    return map;
}

QMap<QString, dviRampIo::outputVideoFmt> dviRampIo::formatCodes()
{
    QMap<QString, outputVideoFmt> map;
    map.insert("480i", sdiNTSC);
    map.insert("576i", sdiPAL);
    map.insert("720p60", sdi720p60);
    map.insert("720p59", sdi720p59);
    map.insert("720p50", sdi720p50);
    map.insert("1080i60", sdi1080i60);
    map.insert("1080i59.94", sdi1080i59);
    map.insert("1080i50", sdi1080i50);
    map.insert("1080PsF24", sdi1080PsF24);
    map.insert("1080PsF23.98", sdi1080PsF23);
    map.insert("1080p25", sdi1080p25);
    map.insert("1080p24", sdi1080p24);
    map.insert("1080p23.98", sdi1080p23);
    return map;
}

dviRampIo::dviRampIo(QIODevice *link, QObject *parent) : QObject(parent), mIoDev(link)
{
    connect(link, SIGNAL(readyRead()), SLOT(onData()));
}

void dviRampIo::queryStatus()
{
    sendCmd("STATUS");
}

bool dviRampIo::sendCmd(QString cmd)
{
    if(!cmd.startsWith('>'))
        cmd.prepend('>');
    if (!cmd.endsWith('\r'))
        cmd.append('\r');
    qDebug()<<"cmd"<<cmd;
    mIoDev->write(cmd.toLatin1());
    return mIoDev->waitForBytesWritten(300);
}

void dviRampIo::onData()
{

    QByteArray data = mIoDev->readAll();
    data.replace('\n',"");
    mRxBuffer.push_back(data);
    int i =  mRxBuffer.indexOf('\r', 0);
    while (i>-1)
    {
        QByteArray bmsg = mRxBuffer.left(i);
        //		qDebug()<<bmsg;
        mRxBuffer = mRxBuffer.remove(0, i + 1);
        i =  mRxBuffer.indexOf('\r', 0);
        QString msg = QString::fromLatin1(bmsg);
        if (!parseMsg(msg))
        {
            qDebug()<<"Error parsing message:" << msg;
        }
    }
}

bool dviRampIo::parseMsg(const QString & msg)
{
    if (msg.startsWith("<STA"))
    {
        emit statusUpdateComplete();
        return true;
    }
    int splitter = msg.indexOf(':', 0);
    if (splitter > 0)
    {
        QString property = msg.left(splitter);
        QString value = msg.mid(splitter+1);
        mProperties[property] = value;
        //		qDebug()<<"prop"<<property<<value;
    }
    else
    {
        return false;
    }
    return true;
}

//setters

void dviRampIo::setScalerAspect(QString opt)
{
    QString cmd = "WSC ASPECT " + opt.toUpper();
    sendCmd(cmd);
}

void dviRampIo::setScalerCutoff(QString opt)
{
    QString cmd = "WSC CUTOFF " + opt;
    sendCmd(cmd);
}

void dviRampIo::setExtractionWindow(int output, const QRect window)
{

    QString wnd = QString::asprintf("%04d,%04d,%04d,%04d",
                                    window.x(),
                                    window.y(),
                                    window.right(),
                                    window.bottom());
    switch (output)
    {
    case 0:
        wnd = "WPW A " + wnd;
        sendCmd(wnd);
        break;
    case 1:
        wnd = "WPW B " + wnd;
        sendCmd(wnd);
        break;
    default:
        break;
    }
}

void dviRampIo::setOutputLUT(int channel, dviRampIo::outputLUT lut)
{
    QString cmd;
    switch(channel)
    {
    case 0:
        cmd = "WSL A A";
        break;
    case 1:
        cmd = "WSL B A";
        break;
    default:
        break;
    }
    if (cmd.isEmpty())
        return;
    if (lut == lutITU_709)
        cmd+=" 709";
    else
        cmd+=" LIN";
    sendCmd(cmd);
}

void dviRampIo::setOutputFmt(dviRampIo::outputVideoFmt fmt)
{
    QString cmd = "WSR ";
    switch(fmt)
    {
    case sdiNTSC:
        cmd+="NTSC";
        break;
    case sdiPAL:
        cmd+="PAL";
        break;
    case sdi720p60:
        cmd+="1280x0720_60p";
        break;
    case sdi720p59:
        cmd+="1280x0720_59p";
        break;
    case sdi720p50:
        cmd+="1280x0720_50p";
        break;
    case sdi1080i60:
        cmd+="1920x1080_30i";
        break;
    case sdi1080i59:
        cmd+="1920x1080_29i";
        break;
    case sdi1080i50:
        cmd+="1920x1080_25i";
        break;
    case sdi1080PsF24:
        cmd+="1920x1080_24i";
        break;
    case sdi1080PsF23:
        cmd+="1920x1080_23i";
        break;
    case sdi1080p25:
        cmd+="1920x1080_25p";
        break;
    case sdi1080p24:
        cmd+="1920x1080_24p";
        break;
    case sdi1080p23:
        cmd+="1920x1080_23p";
        break;
    default:
        return;
    }
    sendCmd(cmd);
}

dviRampIo::outputVideoFmt dviRampIo::outputFmt() const
{

}

/**
 * @brief dviRampIo::setPrefInputFmt
 * @param width active widh
 * @param height active heigt
 * @param rate refresh rate *100
 */
void dviRampIo::setPrefInputFmt(int channel, int width, int height, int rate)
{
    channel = std::min(std::max(0, channel), 1);
    QString cmd;
    double pxClock = (double)width * height * rate  / 100.0;
    if (pxClock < 12E+7) //normal blanking
    {
        cmd = "WDI " + QString::asprintf("%d %04d %04d %04d",
                                         channel + 1,
                                         width,
                                         height,
                                         rate);
    } else { //reduced blanking
        cmd = "WDI " + QString::asprintf("%d %04d %04d %04d 048 032 080 001 003 026",
                                         channel + 1,
                                         width,
                                         height,
                                         rate);
    }
    sendCmd(cmd);
}

void dviRampIo::setTPG(int channel, dviRampIo::tpgMode mode)
{
    QString cmd = "WTP ";
    switch (mode)
    {
    case tpgBars:
        cmd+="CBAR ";
        break;
    case tpgRamp:
        cmd+="RAMP ";
        break;
    case tpgOff:
    default:
        cmd+="OFF ";
        break;
    }
    switch(channel)
    {
    case 0:
        cmd+="SDIA";
        break;
    case 1:
        cmd+="SDIB";
        break;
    default:
        return;
    }
    sendCmd(cmd);
}

void dviRampIo::setDviMode(dviMode mode)
{
    QString cmd = "WIS DVI CON ";
    switch(mode)
    {
    case dviSingleLink:
        cmd += "SL";
        break;
    case dviDualLink:
        cmd += "DL";
        break;
    case dviDualHead:
        cmd += "DH";
        break;
    }
    sendCmd(cmd);
}

QString dviRampIo::property(QString key) const
{
    return mProperties.value(key, "");
}

QMap<QString, QString> dviRampIo::properties() const
{
    return mProperties;
}

void dviRampIo::writeNv()
{
    sendCmd("WNV");
}

void dviRampIo::setDeflicker(int channel, int value)
{
    return;
}

dviRampIo::systemMode dviRampIo::getMode() const
{
    QString mode = mProperties.value("DR2 MODE", "");
    return modeSingleP2p;
}

