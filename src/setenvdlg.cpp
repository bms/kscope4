#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

#include <KCompletion>
#include <KLineEdit>

#include "setenvdlg.h"

SetEnvDialog::SetEnvDialog(QWidget *parent, const QString& name) :
	KDialog(parent)
{
	setObjectName(name);
	setCaption("Rebuild Cscope database");
	setButtons(KDialog::Apply | KDialog::Cancel);
	setDefaultButton(KDialog::Apply);
	showButtonSeparator(true);

	QVBoxLayout *vbLo = new QVBoxLayout();

	QLabel *lbl = new QLabel("Setting Cscope environement");
	lbl->setAlignment(Qt::AlignCenter);
	vbLo->addWidget(lbl);
	vbLo->addSpacing(10);

	QGroupBox *grpB = new QGroupBox("INCLUDEDIRS");
	QVBoxLayout *vbLo1 = new QVBoxLayout();
	lbl = new QLabel(QString("Include directories setting"));
	lbl->setAlignment(Qt::AlignCenter);
	vbLo1->addWidget(lbl);
	vbLo1->addSpacing(5);

	includeEd = new KLineEdit();
	includeEd->setClearButtonShown(true);
	includeEd->setToolTip(QString("<p style='white-space:pre'><qt>Enter a colon-separated list of directories<br/>to search for #include files.</qt>"));
	includeEd->setTrapReturnKey(true);
	vbLo1->addWidget(includeEd);
	grpB->setLayout(vbLo1);

	vbLo->addWidget(grpB);
	vbLo->addSpacing(15);

	grpB = new QGroupBox("SOURCEDIRS");
	vbLo1 = new QVBoxLayout();
	lbl = new QLabel(QString("Additional source directories setting"));
	lbl->setAlignment(Qt::AlignCenter);
	vbLo1->addWidget(lbl);
	vbLo1->addSpacing(5);

	sourceEd = new KLineEdit();
	sourceEd->setClearButtonShown(true);
	sourceEd->setToolTip(QString("<p style='white-space:pre'><qt>Enter a colon-separated list of directories<br/>to search for additional source files.</qt>"));
	includeEd->setTrapReturnKey(true);
	vbLo1->addWidget(sourceEd);
	grpB->setLayout(vbLo1);

	vbLo->addWidget(grpB);

	QWidget *mw = new QWidget();
	mw->setLayout(vbLo);

	connect(this, SIGNAL(applyClicked()), this, SLOT(slotApplyClicked()));
	connect(this, SIGNAL(cancelClicked()), this, SLOT(reject()));

	setMainWidget(mw);
	resize(400, 150);
}

SetEnvDialog::~SetEnvDialog()
{
}

const QString& SetEnvDialog::getIncludeSetting()
{
	return m_sIncludeDirs;
}

const QString& SetEnvDialog::getSourceSetting()
{
	return m_sSourceDirs;
}

void SetEnvDialog::setDefaultInclude(const QString& incDirs)
{
	m_sIncludeDirs = incDirs;
	includeEd->setText(incDirs);
}

void SetEnvDialog::setDefaultSource(const QString& srcDirs)
{
	m_sSourceDirs = srcDirs;
	sourceEd->setText(srcDirs);
}

void SetEnvDialog::slotApplyClicked()
{
	m_sIncludeDirs = includeEd->userText();
	m_sSourceDirs = sourceEd->userText();
	KDialog::accept();
}

#include "moc_setenvdlg.cpp"

/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
