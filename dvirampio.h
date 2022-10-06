#ifndef DVIRAMPIO_H
#define DVIRAMPIO_H

#include <QObject>
#include <QByteArray>
#include <QMap>
class QIODevice;



///*  341 */     this.mnuDr2Mode_Grp.add(this.mnuDr2Mode_Pix2Pix);
///*  342 */     this.mnuDr2Mode_Pix2Pix.setText("Single Extraction - Pixel-to-Pixel");
///*  343 */     this.mnuDr2Mode_Pix2Pix.setActionCommand("WOS SDI FUNC PIX2PIX");
///*  344 */     this.mnuDr2Mode_Pix2Pix.addActionListener(new ActionListener() {
///*      */       public void actionPerformed(ActionEvent evt) {
///*  346 */         JDviramp2.this.mnuDr2Mode_setModeActionPerformed(evt);
///*      */       }
///*      */
///*  349 */     });
///*  350 */     this.mnuApplication.add(this.mnuDr2Mode_Pix2Pix);
///*      */
///*  352 */     this.mnuDr2Mode_Grp.add(this.mnuDr2Mode_Scaler);
///*  353 */     this.mnuDr2Mode_Scaler.setText("Single Extraction - Scaling");
///*  354 */     this.mnuDr2Mode_Scaler.setActionCommand("WOS SDI FUNC SCALER");
///*  355 */     this.mnuDr2Mode_Scaler.addActionListener(new ActionListener() {
///*      */       public void actionPerformed(ActionEvent evt) {
///*  357 */         JDviramp2.this.mnuDr2Mode_setModeActionPerformed(evt);
///*      */       }
///*      */
///*  360 */     });
///*  361 */     this.mnuApplication.add(this.mnuDr2Mode_Scaler);
///*      */
///*  363 */     this.mnuDr2Mode_Grp.add(this.mnuDr2Mode_FillKey);
///*  364 */     this.mnuDr2Mode_FillKey.setText("Dual Extraction - Fill & Key");
///*  365 */     this.mnuDr2Mode_FillKey.setActionCommand("WOS SDI FUNC FILL&KEY");
///*  366 */     this.mnuDr2Mode_FillKey.addActionListener(new ActionListener() {
///*      */       public void actionPerformed(ActionEvent evt) {
///*  368 */         JDviramp2.this.mnuDr2Mode_setModeActionPerformed(evt);
///*      */       }
///*      */
///*  371 */     });
///*  372 */     this.mnuApplication.add(this.mnuDr2Mode_FillKey);
///*      */
///*  374 */     this.mnuDr2Mode_Grp.add(this.mnuDr2Mode_Independant);
///*  375 */     this.mnuDr2Mode_Independant.setText("Dual Extraction - Pixel-to-Pixel");
///*  376 */     this.mnuDr2Mode_Independant.setActionCommand("WOS SDI FUNC INDEP");
///*  377 */     this.mnuDr2Mode_Independant.addActionListener(new ActionListener() {
///*      */       public void actionPerformed(ActionEvent evt) {
///*  379 */         JDviramp2.this.mnuDr2Mode_setModeActionPerformed(evt);
///*      */       }
///*      */
///*  382 */     });
///*  383 */     this.mnuApplication.add(this.mnuDr2Mode_Independant);
///*      */
///*  385 */     this.mnuDr2Mode_Grp.add(this.mnuDr2Mode_SDHD);
///*  386 */     this.mnuDr2Mode_SDHD.setText("Simultaneous SD/HD");
///*  387 */     this.mnuDr2Mode_SDHD.setActionCommand("WOS SDI FUNC SD/HD");
///*  388 */     this.mnuDr2Mode_SDHD.addActionListener(new ActionListener() {
///*      */       public void actionPerformed(ActionEvent evt) {
///*  390 */         JDviramp2.this.mnuDr2Mode_setModeActionPerformed(evt);
///*      */       }
///*      */
///*  393 */     });
///*  394 */     this.mnuApplication.add(this.mnuDr2Mode_SDHD);
///*      */
///*  396 */     this.mnuDviIn2_DviSrcGrp.add(this.mnuDviInMode_SingleLink);
///*  397 */     this.mnuDviInMode_SingleLink.setText("Mode: Single Link");

class dviRampIo : public QObject
{
	Q_OBJECT
public:
    enum systemMode {
        modeSingleP2p=0,
        modeSingleScaler,
        modeDualFillKey,
        modeDualP2p,
        modeSdHd
    };
    enum dviMode {
        dviSingleLink=0,
        dviDualLink,
        dviDualHead
    };
	enum outputLUT
	{
		lutLinear = 0,
		lutITU_709
	};
	enum outputVideoFmt
	{
		sdiNTSC=0,
		sdiPAL,
		sdi720p60,
		sdi720p59,
		sdi720p50,
		sdi1080i60,
		sdi1080i59,
		sdi1080i50,
		sdi1080PsF24,
		sdi1080PsF23,
		sdi1080p25,
		sdi1080p24,
		sdi1080p23
	};
	enum tpgMode
	{
		tpgOff=0,
		tpgBars=1,
		tpgRamp=2
	};
    static QMap<outputVideoFmt, QString> sdiFormats();
    static QMap<QString, outputVideoFmt> formatCodes();

	explicit dviRampIo(QIODevice * link, QObject *parent = nullptr);

	void queryStatus();

	void setScalerAspect(QString opt);
	void setScalerCutoff(QString opt);
	void setExtractionWindow(int output, const QRect window);
	void setOutputLUT(int channel, outputLUT lut);
	void setOutputFmt(outputVideoFmt fmt);
    outputVideoFmt outputFmt() const;
    void setPrefInputFmt(int channel, int width, int height, int rate);
    void setTPG(int channel, tpgMode);

    void setDviMode(dviMode mode);
    QString property(QString key) const;
    QMap<QString, QString> properties() const;

    void writeNv();

    void setDeflicker(int channel, int value);

    systemMode getMode() const;


    /*
     * make private later
    **/
    bool sendCmd(QString cmd);
private:
	QIODevice * mIoDev;
	QByteArray mRxBuffer;
    QMap<QString, QString> mProperties;
signals:
	void statusUpdateComplete();
private slots:
	void onData();
	bool parseMsg(const QString &);
public slots:
};

#endif // DVIRAMPIO_H
