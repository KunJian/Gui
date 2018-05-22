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
#include <Gui/Command.h>
#include <App/Document.h>
#include <App/Part.h>
#include <Base/Tools.h>
#include <Base/Reader.h>

#include <Mod/PartDesign/App/Body.h>

#include "../Utils.h"
#include "../ReferenceSelection.h"

#include "TaskAFPFaceFace.h"
#include "ui_TaskAFPFaceFace.h"

#include <Mod/PartDesign/App/AFP/AFP.h>
#include <Mod/PartDesign/App/AFP/AFPGroup.h>

#include <Mod/Part/App/DatumFeature.h>
#include <Mod/PartDesign/App/DatumPlane.h>
#include <Mod/PartDesign/App/Feature.h>

#include <QMessageBox>

using namespace PartDesignGui;
using namespace Attacher;

TaskAFPFaceFace::TaskAFPFaceFace(PartDesign::AFP* _AFP, QWidget* _parent)
	: TaskBox(Gui::BitmapFactory().pixmap("edit-select-box"), tr("AFP Face-Face"), true, _parent)
	, m_ui(new Ui_TaskAFPFaceFace)
	, m_AFP(_AFP)
{
	m_proxy = new QWidget(this);
	m_ui->setupUi(m_proxy);
	groupLayout()->addWidget(m_proxy);

	m_ui->AFPFaceFaceIDLineEdit->setEnabled(false);
	m_ui->AF1LineEdit->setEnabled(true);
	m_ui->AF1GeomComboBox->setEnabled(false);
	m_ui->AF2LineEdit->setEnabled(true);
	m_ui->AF2GeomComboBox->setEnabled(false);

	connect(m_ui->AF1LineEdit, SIGNAL(m_ui->AF1LineEdit->focusInEvent()), this, SLOT(onFaceSelection()));
	connect(m_ui->AF1LineEdit, SIGNAL(m_ui->AF1LineEdit->textChanged()), this, SLOT(onUpdate()));
	connect(m_ui->AF2LineEdit, SIGNAL(m_ui->AF2LineEdit->focusInEvent()), this, SLOT(onFaceSelection()));
	connect(m_ui->AF2LineEdit, SIGNAL(m_ui->AF2LineEdit->textChanged()), this, SLOT(onUpdate()));
	connect(m_ui->constraintTypeComboBox, SIGNAL(m_ui->constraintTypeComboBox->currentIndexChanged(int)), this, SLOT(onConstraintSelection(m_ui->constraintTypeComboBox)));

	m_ui->AFPFaceFaceIDLineEdit->setText(QObject::tr(m_AFP->ID.getValue()));

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
			{
				m_ui->AF1LineEdit->setText(QString::fromLatin1(feature->getNameInDocument()) + QString::fromLatin1(":") + tr("Face") + QString::number(faceId));
				onUpdate();
			}
			
			bool attached = false;
			App::Document* pDoc = feature->getDocument();
			m_documentName = pDoc->getName();
			if (!attached) {
				attached = true;
				attachDocument(Gui::Application::Instance->getDocument(pDoc));
			}
		}
		else
			QMetaObject::invokeMethod(m_ui->AF1LineEdit, "setFocus", Qt::QueuedConnection);
	}
}

TaskAFPFaceFace::~TaskAFPFaceFace()
{
	delete m_ui;
}

void TaskAFPFaceFace::onUpdate()
{
	if (m_ui->AF1LineEdit->text().isEmpty() && m_ui->AF2LineEdit->text().isEmpty())
	{
		m_ui->AF1LineEdit->setEnabled(true);
		m_ui->AF1GeomComboBox->setEnabled(true);

		m_ui->AF2LineEdit->setEnabled(true);
		m_ui->AF2GeomComboBox->setEnabled(true);
	}
	else if (!m_ui->AF1LineEdit->text().isEmpty() && m_ui->AF2LineEdit->text().isEmpty())
	{
		m_ui->AF1LineEdit->setEnabled(true);
		m_ui->AF1GeomComboBox->setEnabled(true);

		m_ui->AF2LineEdit->setEnabled(false);
		m_ui->AF2GeomComboBox->setEnabled(false);
	}
	else if (m_ui->AF1LineEdit->text().isEmpty() && !m_ui->AF2LineEdit->text().isEmpty())
	{
		m_ui->AF1LineEdit->setEnabled(false);
		m_ui->AF1GeomComboBox->setEnabled(false);

		m_ui->AF2LineEdit->setEnabled(true);
		m_ui->AF2GeomComboBox->setEnabled(true);
	}
}

void TaskAFPFaceFace::onUpdateAFGeom(const int _idx)
{
	GeomAbs_SurfaceType surfType;
	m_AFP->getAFSurfaceType(_idx, surfType);

	switch (surfType)
	{
	case GeomAbs_Plane:
	{
		if (_idx == 1)
		{
			m_ui->AF1GeomComboBox->setCurrentIndex(0);
			m_ui->AF1GeomComboBox->setEnabled(false);
		}
		else
		{
			m_ui->AF2GeomComboBox->setCurrentIndex(0);
			m_ui->AF2GeomComboBox->setEnabled(false);
		}
		break;
	}
	case GeomAbs_Cylinder:
	{
		if (_idx == 1)
		{
			m_ui->AF1GeomComboBox->setCurrentIndex(1);
			m_ui->AF1GeomComboBox->setEnabled(false);
		}
		else
		{
			m_ui->AF2GeomComboBox->setCurrentIndex(1);
			m_ui->AF2GeomComboBox->setEnabled(false);
		}
		break;
	}
	default:
		m_ui->AF1GeomComboBox->setEnabled(true);
		m_ui->AF2GeomComboBox->setEnabled(true);
		break;
	}
}

void TaskAFPFaceFace::onSelectionChanged(const Gui::SelectionChanges& _msg)
{
	if (_msg.Type == Gui::SelectionChanges::AddSelection) {
		QString AFPText;
		App::DocumentObject* selObj = NULL;
		std::vector<Gui::SelectionObject> selection = Gui::Selection().getSelectionEx();
		if (!selection.empty())
			selObj = selection.front().getObject();
		if (selObj == NULL || !selObj->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId()))
			return ;

		QString refText = onAddSelection(_msg, selObj, AFPText);
		if (refText.length() > 0) {
			std::vector<std::string> subNames;
			subNames.push_back(_msg.pSubName);

			if (m_ui->AF1LineEdit->isEnabled()/* && m_ui->AF1LineEdit->hasFocus()*/)
			{
				m_ui->AF1LineEdit->blockSignals(true);
				m_AFP->First.setValue(selObj, subNames);
				onUpdateAFGeom(1);

				setLineEdit(m_ui->AF1LineEdit, refText, _msg.pSubName, true, false);
			}
			else if (m_ui->AF2LineEdit->isEnabled()/* && m_ui->AF2LineEdit->hasFocus()*/)
			{
				m_ui->AF2LineEdit->blockSignals(true);
				m_AFP->Second.setValue(selObj, subNames);
				onUpdateAFGeom(2);

				setLineEdit(m_ui->AF2LineEdit, refText, _msg.pSubName, true, false);
			}
		}
		else
		{
			if (m_ui->AF1LineEdit->isEnabled())
				setLineEdit(m_ui->AF1LineEdit, tr(""), "", false, false);
			else if (m_ui->AF2LineEdit->isEnabled())
				setLineEdit(m_ui->AF2LineEdit, tr(""), "", false, false);
		}
	}
	else if (_msg.Type == Gui::SelectionChanges::ClrSelection) {
		if (m_ui->AF1LineEdit->isEnabled())
			setLineEdit(m_ui->AF1LineEdit, tr(""), "", false, false);
		else if (m_ui->AF2LineEdit->isEnabled())
			setLineEdit(m_ui->AF2LineEdit, tr(""), "", false, false);
	}
}

void TaskAFPFaceFace::setLineEdit(QLineEdit* const _lnEdit, const QString _refText, const char* _subName, bool _onFaceSel, bool _faceSel)
{
	_lnEdit->blockSignals(true);
	_lnEdit->setText(_refText);
	if (_subName == "")
		_lnEdit->setProperty("FaceName", QByteArray());
	else
		_lnEdit->setProperty("FaceName", QByteArray(_subName));

	_lnEdit->blockSignals(false);
	if (_onFaceSel)
		onFaceSelection(_faceSel);

	onUpdate();
}

const QString TaskAFPFaceFace::onAddSelection(const Gui::SelectionChanges& _msg, App::DocumentObject* _selObj, QString& _AFPText)
{
	PartDesign::Body *pcBody = PartDesignGui::getBodyFor(_selObj, false);
	if (pcBody == NULL) return QString();

	App::Part *pPart = PartDesignGui::getPartFor(pcBody, false);
	if (pPart == NULL) return QString();

	std::string subname = _msg.pSubName;
	QString refStr;

	// Remove subname for planes and datum features
	if (subname.size() > 4) {
		int faceId = std::atoi(&subname[4]);
		refStr = QString::fromLatin1(_selObj->getNameInDocument()) + QString::fromLatin1(":") + QObject::tr("Face") + QString::number(faceId);
		_AFPText = QString::fromLatin1(pPart->getNameInDocument()) + QString::fromLatin1(":") + QString::fromLatin1(pcBody->getNameInDocument()) + QString::fromLatin1(":") + refStr;
	}

	return refStr;
}

void TaskAFPFaceFace::onFaceSelection(const bool pressed) {
	try {
		if (pressed) {
			auto activeBody = PartDesignGui::getBody(false);
			if (!activeBody) return;

			Gui::Selection().clearSelection();
			auto activePart = PartDesignGui::getPartFor(activeBody, false);
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

void TaskAFPFaceFace::onConstraintSelection(QComboBox* const _combBox)
{
	//{ "Contact", "Coincident", "Distance", "Offset", "Angle", "None", NULL }
	m_AFP->SolutionSpace.setValue(dcm::negative_directional);

	QString cons = _combBox->currentText();
	if (cons == QObject::tr("Contact"))
	{
		m_ui->constraintParamLineEdit->setEnabled(false);
		m_ui->constraintParamLineEdit->setText(tr("0"));
		m_AFP->Constraint.setValue("Contact");
		m_AFP->Value.setValue(0);
	}
	else if (cons == QObject::tr("Distance"))
	{
		m_AFP->Constraint.setValue("Distance");
		m_ui->constraintParamLineEdit->setEnabled(true);
		QString str = m_ui->constraintParamLineEdit->text();
		if (!str.isEmpty())
			m_AFP->Value.setValue(str.toDouble());
		m_AFP->Orientation.setValue(dcm::opposite);
	}
	else if (cons == QObject::tr("Offset"))
	{
		m_AFP->Constraint.setValue("Offset");
		m_ui->constraintParamLineEdit->setEnabled(true);
		QString str = m_ui->constraintParamLineEdit->text();
		if (!str.isEmpty())
			m_AFP->Value.setValue(str.toDouble());

		m_AFP->Orientation.setValue(dcm::parallel);
	}
	else if (cons == QObject::tr("Angle"))
	{
		m_AFP->Constraint.setValue("Angle");
		m_ui->constraintParamLineEdit->setEnabled(true);
		QString str = m_ui->constraintParamLineEdit->text();
		if (!str.isEmpty())
			m_AFP->Value.setValue(str.toDouble());
	}
	else if (cons == QObject::tr("None"))
	{
		m_AFP->Constraint.setValue("None");
		m_ui->constraintParamLineEdit->setEnabled(false);
		m_ui->constraintParamLineEdit->setText(tr("0"));
		m_AFP->Value.setValue(0);
	}
	else
	{
		m_AFP->Constraint.setValue("");
		m_ui->constraintParamLineEdit->setEnabled(false);
		m_ui->constraintParamLineEdit->setText(tr("0"));
		m_AFP->Value.setValue(0);
	}
	//App::GetApplication().getActiveDocument()->recompute();
	//m_view->draw();
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
TaskDlgAFPFaceFace::TaskDlgAFPFaceFace(PartDesign::AFP* _AFP) : TaskDialog(), m_AFP(_AFP)
{
	m_facePick = new TaskAFPFaceFace(_AFP);
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
	//std::string document = getDocumentName(); // needed because resetEdit() deletes this instance
	//Gui::Command::doCommand(Gui::Command::Gui, "Gui.getDocument('%s').resetEdit()", document.c_str());
	return true;
}

bool TaskDlgAFPFaceFace::reject()
{
	//std::string document = getDocumentName(); // needed because resetEdit() deletes this instance
	//Gui::Command::doCommand(Gui::Command::Gui, "Gui.getDocument('%s').resetEdit()", document.c_str());
	return true;
}


#include "AFP/moc_TaskAFPFaceFace.cpp"