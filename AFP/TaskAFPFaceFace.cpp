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
#include <QFocusEvent>

using namespace PartDesignGui;
using namespace Attacher;

//TaskAFPFaceFace::TaskAFPFaceFace(PartDesign::AFP* _AFP, QWidget* _parent)
TaskAFPFaceFace::TaskAFPFaceFace(ViewProviderAFP* _vp, QWidget* _parent)
	: TaskBox(Gui::BitmapFactory().pixmap("edit-select-box"), tr("AFP Face-Face"), true, _parent)
	, m_ui(new Ui_TaskAFPFaceFace)
	, m_vp(_vp)
	, m_AF1HasFocus(false)
	, m_AF2HasFocus(false)
{
	if (_vp->getObject()) m_AFP = dynamic_cast<PartDesign::AFP*>(_vp->getObject());

	Gui::Document* doc = _vp->getDocument();
	this->attachDocument(doc);
	this->enableNotifications(DocumentObserver::Delete);

	m_proxy = new QWidget(this);
	m_ui->setupUi(m_proxy);
	groupLayout()->addWidget(m_proxy);

	m_ui->AFPFaceFaceIDLineEdit->setEnabled(false);
	m_ui->AF1LineEdit->setEnabled(true);
	m_ui->AF1GeomComboBox->setEnabled(false);
	m_ui->AF2LineEdit->setEnabled(true);
	m_ui->AF2GeomComboBox->setEnabled(false);

	m_ui->AF1LineEdit->installEventFilter(this);
	m_ui->AF2LineEdit->installEventFilter(this);

	connect(m_ui->AF1LineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onUpdate(const QString&)));
	connect(m_ui->AF2LineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onUpdate(const QString&)));
	connect(m_ui->constraintTypeComboBox, SIGNAL(activated(const QString &)), this, SLOT(onConstraintSelection(const QString&)));

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
				m_ui->AF1LineEdit->setText(QString::fromLatin1(feature->getNameInDocument()) + QString::fromLatin1(":") + tr("Face") + QString::number(faceId));
		}
		else
			QMetaObject::invokeMethod(m_ui->AF1LineEdit, "setFocus", Qt::QueuedConnection);
	}
}

TaskAFPFaceFace::~TaskAFPFaceFace()
{
	delete m_ui;
}

void TaskAFPFaceFace::onUpdate(const QString& _lineText)
{
	QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(sender());
	if (_lineText.isEmpty())
	{
		if ((lineEdit == m_ui->AF1LineEdit && m_ui->AF2LineEdit->text().isEmpty()) ||
			(lineEdit == m_ui->AF2LineEdit && m_ui->AF1LineEdit->text().isEmpty()))
		{
			m_ui->AF1LineEdit->setEnabled(true);
			m_ui->AF1GeomComboBox->setEnabled(true);

			m_ui->AF2LineEdit->setEnabled(true);
			m_ui->AF2GeomComboBox->setEnabled(true);
		}
	}
	else
	{
		if (lineEdit == m_ui->AF1LineEdit)
		{
			m_ui->AF1LineEdit->setEnabled(true);
			m_ui->AF1GeomComboBox->setEnabled(true);
			onUpdateAFGeom(1, m_ui->AF1GeomComboBox);

			m_ui->AF2LineEdit->setEnabled(false);
			m_ui->AF2GeomComboBox->setEnabled(false);
			if (!m_ui->AF2LineEdit->text().isEmpty())
				m_ui->AF2LineEdit->clear();
		}
		else if (lineEdit == m_ui->AF2LineEdit)
		{
			m_ui->AF2LineEdit->setEnabled(true);
			m_ui->AF2GeomComboBox->setEnabled(true);
			onUpdateAFGeom(2, m_ui->AF2GeomComboBox);

			m_ui->AF1LineEdit->setEnabled(false);
			m_ui->AF1GeomComboBox->setEnabled(false);
			if (!m_ui->AF1LineEdit->text().isEmpty())
				m_ui->AF1LineEdit->clear();
		}
	}
}

void TaskAFPFaceFace::onUpdateAFGeom(const int _idx, QComboBox* _combBox)
{
	GeomAbs_SurfaceType surfType;
	m_AFP->getAFSurfaceType(_idx, surfType);
	switch (surfType)
	{
	case GeomAbs_Plane:
	{
		_combBox->setCurrentIndex(0);
		_combBox->setEnabled(false);
		break;
	}
	case GeomAbs_Cylinder:
	{
		_combBox->setCurrentIndex(1);
		_combBox->setEnabled(false);
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
			return;

		QString refText = onAddSelection(_msg, selObj, AFPText);
		if (refText.length() > 0) {
			std::vector<std::string> subNames;
			subNames.push_back(_msg.pSubName);

			if (m_AF1HasFocus)
			{
				m_AFP->First.setValue(selObj, subNames);
				if (m_AFP->Second.getValue())
					m_AFP->Second.setValue((0));
				setLineEdit(m_ui->AF1LineEdit, refText, _msg.pSubName);
			}
			else if (m_AF2HasFocus)
			{
				m_AFP->Second.setValue(selObj, subNames);
				if (m_AFP->First.getValue())
					m_AFP->First.setValue((0));
				setLineEdit(m_ui->AF2LineEdit, refText, _msg.pSubName);
			}
		}
		else
		{
			if (m_ui->AF1LineEdit->isEnabled())
				setLineEdit(m_ui->AF1LineEdit, tr(""), "");
			else if (m_ui->AF2LineEdit->isEnabled())
				setLineEdit(m_ui->AF2LineEdit, tr(""), "");
		}
	}
	else if (_msg.Type == Gui::SelectionChanges::ClrSelection) {
		if (m_ui->AF1LineEdit->isEnabled())
			setLineEdit(m_ui->AF1LineEdit, tr(""), "");
		else if (m_ui->AF2LineEdit->isEnabled())
			setLineEdit(m_ui->AF2LineEdit, tr(""), "");
	}
}

void TaskAFPFaceFace::setLineEdit(QLineEdit* const _lnEdit, const QString _refText, const char* _subName)
{
	_lnEdit->setText(_refText);
	if (_subName == "")
		_lnEdit->setProperty("FaceName", QByteArray());
	else
		_lnEdit->setProperty("FaceName", QByteArray(_subName));
}

const QString TaskAFPFaceFace::onAddSelection(const Gui::SelectionChanges& _msg, App::DocumentObject* _selObj, QString& _AFPText)
{
	PartDesign::Body *pcBody = PartDesignGui::getBodyFor(_selObj, false);
	if (pcBody == NULL) return QString();

	App::Part *pPart = PartDesignGui::getPartFor(pcBody, false);
	if (pPart == NULL) return QString();

	QString refStr;
	std::string subname = _msg.pSubName;
	// Remove subname for planes and datum features
	if (subname.size() > 4) {
		int faceId = std::atoi(&subname[4]);
		refStr = QString::fromLatin1(_selObj->getNameInDocument()) + QString::fromLatin1(":") + QObject::tr("Face") + QString::number(faceId);
		_AFPText = QString::fromLatin1(pPart->getNameInDocument()) + QString::fromLatin1(":") + QString::fromLatin1(pcBody->getNameInDocument()) + QString::fromLatin1(":") + refStr;
	}
	return refStr;
}

void TaskAFPFaceFace::onFaceSelection(bool _pressed) {
	try {
		if (_pressed) {
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

void TaskAFPFaceFace::onConstraintSelection(const QString& _selText)
{
	//{ "Distance", "Align", "Coincident", "Orientation", "Angle", "None", NULL }
	if (_selText == QObject::tr("Distance"))
	{
		m_AFP->Constraint.setValue("Distance");
		m_ui->constraintParamLineEdit->setEnabled(true);
		QString str = m_ui->constraintParamLineEdit->text();
		if (!str.isEmpty())
			m_AFP->Value.setValue(str.toDouble());
		m_AFP->Orientation.setValue(dcm::opposite);
	}
	else if (_selText == QObject::tr("Align"))
	{
		m_AFP->Constraint.setValue("Align");
		m_ui->constraintParamLineEdit->setEnabled(false);
		m_ui->constraintParamLineEdit->setText(tr("0"));
		m_AFP->Value.setValue(0);
		m_AFP->Orientation.setValue(dcm::equal);
	}
	else if (_selText == QObject::tr("Coincident"))
	{
		m_AFP->Constraint.setValue("Coincident");
		m_ui->constraintParamLineEdit->setEnabled(false);
		m_ui->constraintParamLineEdit->setText(tr("0"));
		m_AFP->Value.setValue(0);
		m_AFP->Orientation.setValue(dcm::parallel);
	}
	else if (_selText == QObject::tr("Orientation"))
	{
		m_AFP->Constraint.setValue("Orientation");
		m_ui->constraintParamLineEdit->setEnabled(true);
		QString str = m_ui->constraintParamLineEdit->text();
		if (!str.isEmpty())
			m_AFP->Value.setValue(str.toDouble());
	}
	else if (_selText == QObject::tr("Angle"))
	{
		m_AFP->Constraint.setValue("Angle");
		m_ui->constraintParamLineEdit->setEnabled(true);
		QString str = m_ui->constraintParamLineEdit->text();
		if (!str.isEmpty())
			m_AFP->Value.setValue(str.toDouble());
	}
	else if (_selText == QObject::tr("None"))
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
}

void TaskAFPFaceFace::slotUndoDocument(const Gui::Document& _Doc)
{
	QTimer::singleShot(100, &Gui::Control(), SLOT(closeDialog()));
}

void TaskAFPFaceFace::slotDeleteDocument(const Gui::Document& _Doc)
{
	QTimer::singleShot(100, &Gui::Control(), SLOT(closeDialog()));
}

void TaskAFPFaceFace::slotDeletedObject(const Gui::ViewProviderDocumentObject& Obj)
{
	if (this->m_vp == &Obj)
		this->m_vp = nullptr;
}

void TaskAFPFaceFace::exitSelectionMode()
{
	Gui::Selection().rmvSelectionGate();
}

bool TaskAFPFaceFace::eventFilter(QObject* _watched, QEvent* _event)
{
	if (_watched == m_ui->AF1LineEdit)         //首先判斷控制項(這裡指 lineEdit1)  
	{
		if (_event->type() == QEvent::FocusIn)     //然後再判斷控制項的具體事件 (這裡指獲得焦點事件)  
		{
			m_AF1HasFocus = true;
			m_AF2HasFocus = false;
			onFaceSelection(true);
		}
	}
	if (_watched == m_ui->AF2LineEdit)           //這裡來處理 lineEdit2 , 和處理lineEdit1 是一樣的  
	{
		if (_event->type() == QEvent::FocusIn)
		{
			m_AF1HasFocus = false;
			m_AF2HasFocus = true;
			onFaceSelection(true);
		}
	}
	return QWidget::eventFilter(_watched, _event);     // 最後將事件交給上層對話框  
}

//**************************************************************************
//**************************************************************************
// TaskDialog
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//TaskDlgAFPFaceFace::TaskDlgAFPFaceFace(PartDesign::AFP* _AFP) : TaskDialog(), m_AFP(_AFP)
TaskDlgAFPFaceFace::TaskDlgAFPFaceFace(ViewProviderAFP* _vp) : TaskDialog(), m_vp(_vp)
{
	m_facePick = new TaskAFPFaceFace(_vp);
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
bool TaskDlgAFPFaceFace::accept()
{
	//std::string document = getDocumentName(); // needed because resetEdit() deletes this instance
	//Gui::Command::doCommand(Gui::Command::Gui, "Gui.getDocument('%s').resetEdit()", document.c_str());

	// detach the task panel from the selection to avoid to invoke
	// eventually onAddSelection when the selection changes
	std::vector<QWidget*> subwidgets = getDialogContent();
	for (auto it : subwidgets) {
		TaskAFPFaceFace* facePick = qobject_cast<TaskAFPFaceFace*>(it);
		if (facePick)
			facePick->detachSelection();
	}

	Gui::Command::doCommand(Gui::Command::Gui, "Gui.activeDocument().resetEdit()");
	Gui::Command::commitCommand();

	return true;
}

bool TaskDlgAFPFaceFace::reject()
{
	// detach the task panel from the selection to avoid to invoke
	// eventually onAddSelection when the selection changes
	std::vector<QWidget*> subwidgets = getDialogContent();
	for (auto it : subwidgets) {
		TaskAFPFaceFace* facePick = qobject_cast<TaskAFPFaceFace*>(it);
		if (facePick)
			facePick->detachSelection();
	}

	// roll back the done things
	Gui::Command::abortCommand();
	Gui::Command::doCommand(Gui::Command::Gui, "Gui.activeDocument().resetEdit()");

	//std::string document = getDocumentName(); // needed because resetEdit() deletes this instance
	////Gui::Command::doCommand(Gui::Command::Gui, "Gui.getDocument('%s').resetEdit()", document.c_str());
	//PartDesign::AFP* AFPObj = dynamic_cast<PartDesign::AFP*>(m_vp->getObject());

	////AFPObj->ID.setValue("");
	////AFPObj->UUID.setValue("");
	////AFPObj->Type.setValue(long(6));
	////AFPObj->First.setValue((0));
	////AFPObj->Second.setValue((0));
	////AFPObj->Constraint.setValue(long(6));
	////AFPObj->Value.setValue(0);
	////AFPObj->Orientation.setValue(long(4));
	////AFPObj->SolutionSpace.setValue(long(3));

	//Gui::Command::doCommand(Gui::Command::Doc, "Gui.getDocument('%s').removeObject("")", document.c_str(), AFPObj->getNameInDocument());

	return true;
}


#include "AFP/moc_TaskAFPFaceFace.cpp"