#include "../PreCompiled.h"

#ifndef _PreComp_
//# include <QListIterator>
# include <QTimer>
#endif

#include <Gui/Application.h>
#include <Gui/BitmapFactory.h>
#include <Gui/MainWindow.h>
#include <Gui/Document.h>
#include <Gui/Control.h>
#include <App/Document.h>
#include <App/Part.h>
#include <Base/Tools.h>
#include <Base/Reader.h>

#include <Mod/PartDesign/App/Body.h>

#include "../Utils.h"
#include "../ReferenceSelection.h"

#include "TaskAFPFaceFace.h"
#include "ui_TaskAFPFaceFace.h"

#include <Mod/Part/App/DatumFeature.h>
#include <Mod/PartDesign/App/DatumPlane.h>
#include <Mod/PartDesign/App/Feature.h>

#include <QMessageBox>

using namespace PartDesignGui;
using namespace Attacher;

// TODO Do ve should snap here to App:Part or GeoFeatureGroup/DocumentObjectGroup ? (2015-09-04, Fat-Zer)
const QString TaskAFPFaceFace::getFeatureStatusString(const featureStatus _st)
{
	switch (_st) {
	case validFeature: return tr("Valid");
	case invalidShape: return tr("Invalid shape");
	case otherBody: return tr("Belongs to another body");
	case otherPart: return tr("Belongs to another part");
	case notInBody: return tr("Doesn't belong to any body");
	case AFPFace: return tr("AFP face");
	case afterTip: return tr("Feature is located after the tip feature");
	}

	return QString();
}


TaskAFPFaceFace::TaskAFPFaceFace(App::DocumentObject* _feature, const featureStatus& _status, QWidget* _parent)
	: TaskBox(Gui::BitmapFactory().pixmap("edit-select-box"), tr("AFP Face-Face"), true, _parent), m_ui(new Ui_TaskAFPFaceFace), m_doSelection(false)
{
	m_proxy = new QWidget(this);
	m_ui->setupUi(m_proxy);
	groupLayout()->addWidget(m_proxy);

	m_ui->AFPFaceFaceIDLineEdit->setEnabled(false);
	m_ui->AF1LineEdit->setEnabled(true);
	m_ui->AF1GeomComboBox->setEnabled(false);
	m_ui->AF2LineEdit->setEnabled(true);
	m_ui->AF2GeomComboBox->setEnabled(false);

	connect(m_ui->AF1LineEdit, SIGNAL(focusInEvent()), this, SLOT(onFaceSelection()));
	connect(m_ui->AF2LineEdit, SIGNAL(focusInEvent()), this, SLOT(onFaceSelection()));
	connect(m_ui->AF1LineEdit, SIGNAL(textChanged()), this, SLOT(onUpdate()));
	connect(m_ui->AF2LineEdit, SIGNAL(textChanged()), this, SLOT(onUpdate()));

	bool attached = false;
	if (_feature)
	{
		if (m_ui->AF1LineEdit->text().isEmpty() && m_ui->AF1LineEdit->isEnabled())
		{
			std::vector<Gui::SelectionObject> selection = Gui::Selection().getSelectionEx();
			if (!selection.empty() && selection.front().hasSubNames()) {
				App::DocumentObject* feature = selection.front().getObject();
				std::string sub = selection.front().getSubNames().front();

				int faceId = -1;
				if (sub.substr(0, 4) == "Face")
					faceId = std::atoi(&sub[4]);

				if (faceId >= 0)
					m_ui->AF1LineEdit->setText(QString::fromLatin1(feature->getNameInDocument()) + QString::fromLatin1(":") + tr("Face") + QString::number(faceId));
			}
		}

		App::Document* pDoc = _feature->getDocument();
		m_documentName = pDoc->getName();
		if (!attached) {
			attached = true;
			attachDocument(Gui::Application::Instance->getDocument(pDoc));
		}
	}
	else
	{
		if (m_ui->AF1LineEdit->text().isEmpty() && m_ui->AF1LineEdit->isEnabled())
			QMetaObject::invokeMethod(m_ui->AF1LineEdit, "setFocus", Qt::QueuedConnection);
	}

	m_status = _status;
}

TaskAFPFaceFace::~TaskAFPFaceFace()
{
	delete m_ui;
}

void TaskAFPFaceFace::onUpdate()
{
	if (!m_ui->AF1LineEdit->text().isEmpty() && m_ui->AF1LineEdit->isEnabled())
	{
		m_ui->AF1GeomComboBox->setEnabled(true);

		m_ui->AF2LineEdit->setEnabled(false);
		m_ui->AF2GeomComboBox->setEnabled(false);
	}
	else if (!m_ui->AF2LineEdit->text().isEmpty() && m_ui->AF2LineEdit->isEnabled())
	{
		m_ui->AF2GeomComboBox->setEnabled(true);

		m_ui->AF1LineEdit->setEnabled(false);
		m_ui->AF1GeomComboBox->setEnabled(false);
	}
}

App::DocumentObject* TaskAFPFaceFace::getFeature()
{
	App::DocumentObject* result = NULL;
	if (!m_featureStr.isEmpty())
		result = App::GetApplication().getDocument(m_documentName.c_str())->getObject(m_featureStr.toLatin1().data());

	return result;
}


void TaskAFPFaceFace::onSelectionChanged(const Gui::SelectionChanges& _msg)
{
	if (_msg.Type == Gui::SelectionChanges::AddSelection) {
		QString refText = onAddSelection(_msg);
		if (refText.length() > 0) {
			if (m_ui->AF1LineEdit->isEnabled())
			{
				m_ui->AF1LineEdit->blockSignals(true);
				m_ui->AF1LineEdit->setText(refText);
				m_ui->AF1LineEdit->setProperty("FaceName", QByteArray(_msg.pSubName));
				m_ui->AF1LineEdit->blockSignals(false);
				// Turn off reference selection mode
				onFaceSelection(false);
				onUpdate();
			}
			else if (m_ui->AF2LineEdit->isEnabled())
			{
				m_ui->AF2LineEdit->blockSignals(true);
				m_ui->AF2LineEdit->setText(refText);
				m_ui->AF2LineEdit->setProperty("FaceName", QByteArray(_msg.pSubName));
				m_ui->AF2LineEdit->blockSignals(false);
				// Turn off reference selection mode
				onFaceSelection(false);
				onUpdate();
			}
		}
		else {
			if (m_ui->AF1LineEdit->isEnabled())
			{
				m_ui->AF1LineEdit->blockSignals(true);
				m_ui->AF1LineEdit->setText(tr(""));
				m_ui->AF1LineEdit->setProperty("FaceName", QByteArray());
				m_ui->AF1LineEdit->blockSignals(false);
				onUpdate();
			}
			else if (m_ui->AF2LineEdit->isEnabled())
			{
				m_ui->AF2LineEdit->blockSignals(true);
				m_ui->AF2LineEdit->setText(tr(""));
				m_ui->AF2LineEdit->setProperty("FaceName", QByteArray());
				m_ui->AF2LineEdit->blockSignals(false);
				onUpdate();
			}
		}
	}
	else if (_msg.Type == Gui::SelectionChanges::ClrSelection) {
		if (m_ui->AF1LineEdit->isEnabled())
		{
			m_ui->AF1LineEdit->blockSignals(true);
			m_ui->AF1LineEdit->setText(tr(""));
			m_ui->AF1LineEdit->setProperty("FaceName", QByteArray());
			m_ui->AF1LineEdit->blockSignals(false);
			onUpdate();
		}
		else if (m_ui->AF2LineEdit->isEnabled())
		{
			m_ui->AF2LineEdit->blockSignals(true);
			m_ui->AF2LineEdit->setText(tr(""));
			m_ui->AF2LineEdit->setProperty("FaceName", QByteArray());
			m_ui->AF2LineEdit->blockSignals(false);
			onUpdate();
		}
	}
}

const QString TaskAFPFaceFace::onAddSelection(const Gui::SelectionChanges& _msg)
{
	App::DocumentObject* selObj = NULL;
	std::vector<Gui::SelectionObject> selection = Gui::Selection().getSelectionEx();
	if (!selection.empty())
		selObj = selection.front().getObject();

	if (selObj == NULL)
		return QString();
	std::string subname = _msg.pSubName;
	QString refStr;

	// Remove subname for planes and datum features
	if (subname.size() > 4) {
		int faceId = std::atoi(&subname[4]);
		refStr = QString::fromLatin1(selObj->getNameInDocument()) + QString::fromLatin1(":") + QObject::tr("Face") + QString::number(faceId);
	}

	return refStr;
}

void TaskAFPFaceFace::onFaceSelection(const bool pressed) {
	try {
		if (pressed) {
			auto activeBody = PartDesignGui::getBody(false);
			if (!activeBody)
				return;

			auto activePart = PartDesignGui::getPartFor(activeBody, false);

			Gui::Selection().clearSelection();
			Gui::Selection().addSelectionGate(new ReferenceSelection(activePart, false, true, false));
		}
		else
		{
			Gui::Selection().rmvSelectionGate();
		}
	}
	catch (const Base::Exception& e) {
		e.ReportException();
	}
}

QString TaskAFPFaceFace::getFaceName(void) const
{
	QString faceName = m_ui->AF1LineEdit->property("FaceName").toString();
	if (!faceName.isEmpty()) {
		return getFaceReference(m_ui->AF1LineEdit->text(), faceName);
	}
	return QString();
}

QString TaskAFPFaceFace::getFaceReference(const QString& obj, const QString& sub)
{
	QString o = obj.left(obj.indexOf(QString::fromLatin1(":")));

	if (o == tr("No face selected"))
		return QString();
	else
		return QString::fromLatin1("(App.activeDocument().") + o + QString::fromLatin1(", [\"") + sub + QString::fromLatin1("\"])");
}

void TaskAFPFaceFace::slotUndoDocument(const Gui::Document& _Doc)
{
	//if (origins.empty()) {
	//	QTimer::singleShot(100, &Gui::Control(), SLOT(closeDialog()));
	//}
}

void TaskAFPFaceFace::slotDeleteDocument(const Gui::Document& _Doc)
{
	//origins.clear();
	//QTimer::singleShot(100, &Gui::Control(), SLOT(closeDialog()));
}

void TaskAFPFaceFace::exitSelectionMode()
{
	Gui::Selection().rmvSelectionGate();
}


//**************************************************************************
//**************************************************************************
// TaskDialog
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
TaskDlgAFPFaceFace::TaskDlgAFPFaceFace(App::DocumentObject* _feature, const TaskAFPFaceFace::featureStatus& _status) : TaskDialog(), m_accepted(false)
{
	m_facePick = new TaskAFPFaceFace(_feature, _status);
	Content.push_back(m_facePick);
}

TaskDlgAFPFaceFace::~TaskDlgAFPFaceFace()
{
	for (auto it : Content) {
		delete it;
	}
	Content.clear();
}

//==== calls from the TaskView ===============================================================
void TaskDlgAFPFaceFace::open()
{

}

void TaskDlgAFPFaceFace::clicked(int)
{

}

bool TaskDlgAFPFaceFace::accept()
{
	m_accepted = true;
	return true;
}

bool TaskDlgAFPFaceFace::reject()
{
	m_accepted = false;
	return true;
}


#include "AFP/moc_TaskAFPFaceFace.cpp"