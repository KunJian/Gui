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

#include "TaskAFPFaceFace.h"
#include "ui_TaskAFPFaceFace.h"

#include <Mod/Part/App/DatumFeature.h>
#include <Mod/PartDesign/App/DatumPlane.h>

using namespace PartDesignGui;
using namespace Attacher;

// TODO Do ve should snap here to App:Part or GeoFeatureGroup/DocumentObjectGroup ? (2015-09-04, Fat-Zer)
const QString TaskAFPFaceFace::getFeatureStatusString(const featureStatus st)
{
	switch (st) {
	case validFeature: return tr("Valid");
	case invalidShape: return tr("Invalid shape");
	case otherBody: return tr("Belongs to another body");
	case otherPart: return tr("Belongs to another part");
	case notInBody: return tr("Doesn't belong to any body");
	case basePlane: return tr("Base plane");
	case afterTip: return tr("Feature is located after the tip feature");
	}

	return QString();
}


TaskAFPFaceFace::TaskAFPFaceFace(std::vector<App::DocumentObject*>& objects, const std::vector<featureStatus>& status, QWidget* parent)
	: TaskBox(Gui::BitmapFactory().pixmap("edit-select-box"), tr("AFP Face-Face"), true, parent), ui(new Ui_TaskAFPFaceFace), doSelection(false)
{
	proxy = new QWidget(this);
	ui->setupUi(proxy);

	auto statusIt = status.cbegin();
	auto objIt = objects.begin();
	assert(status.size() == objects.size());

	bool attached = false;
	for (; statusIt != status.end(); ++statusIt, ++objIt) {
		//QListWidgetItem* item = new QListWidgetItem(QString::fromLatin1("%1 (%2)")
		//	.arg(QString::fromUtf8((*objIt)->Label.getValue()))
		//	.arg(getFeatureStatusString(*statusIt)));

		//item->setData(Qt::UserRole, QString::fromLatin1((*objIt)->getNameInDocument()));
		//ui->listWidget->addItem(item);

		App::Document* pDoc = (*objIt)->getDocument();
		documentName = pDoc->getName();
		if (!attached) {
			attached = true;
			attachDocument(Gui::Application::Instance->getDocument(pDoc));
		}

		//check if we need to set any origin in temporary visibility mode
		//if (*statusIt != invalidShape && (*objIt)->isDerivedFrom(App::OriginFeature::getClassTypeId())) {
		//	App::Origin *origin = static_cast<App::OriginFeature*> (*objIt)->getOrigin();
		//	if (origin) {
		//		if ((*objIt)->isDerivedFrom(App::Plane::getClassTypeId())) {
		//			originVisStatus[origin].set(planeBit, true);
		//		}
		//		else if ((*objIt)->isDerivedFrom(App::Line::getClassTypeId())) {
		//			originVisStatus[origin].set(axisBit, true);
		//		}
		//	}
		//}
	}

	groupLayout()->addWidget(proxy);
	statuses = status;
}

TaskAFPFaceFace::~TaskAFPFaceFace()
{
    delete ui;
}

void TaskAFPFaceFace::onUpdate(bool)
{
	//bool enable = false;
	//if (ui->checkOtherBody->isChecked() || ui->checkOtherPart->isChecked())
	//	enable = true;

	//ui->radioDependent->setEnabled(enable);
	//ui->radioIndependent->setEnabled(enable);
	//ui->radioXRef->setEnabled(enable);

	//updateList();
}

std::vector<App::DocumentObject*> TaskAFPFaceFace::getFeature()
{
	//features.clear();
	//QListIterator<QListWidgetItem*> i(ui->listWidget->selectedItems());
	//while (i.hasNext()) {

	//	auto item = i.next();
	//	if (item->isHidden())
	//		continue;

	//	QString t = item->data(Qt::UserRole).toString();
	//	features.push_back(t);
	//}

	std::vector<App::DocumentObject*> result;

	//for (std::vector<QString>::const_iterator s = features.begin(); s != features.end(); ++s)
	//	result.push_back(App::GetApplication().getDocument(documentName.c_str())->getObject(s->toLatin1().data()));

	return result;
}


void TaskAFPFaceFace::onSelectionChanged(const Gui::SelectionChanges& /*msg*/)
{
	//if (doSelection)
	//	return;
	//doSelection = true;
	//ui->listWidget->clearSelection();
	//for (Gui::SelectionSingleton::SelObj obj : Gui::Selection().getSelection()) {
	//	for (int row = 0; row < ui->listWidget->count(); row++) {
	//		QListWidgetItem *item = ui->listWidget->item(row);
	//		QString t = item->data(Qt::UserRole).toString();
	//		if (t.compare(QString::fromLatin1(obj.FeatName)) == 0) {
	//			item->setSelected(true);
	//		}
	//	}
	//}
	//doSelection = false;
}

void TaskAFPFaceFace::slotUndoDocument(const Gui::Document&)
{
	//if (origins.empty()) {
	//	QTimer::singleShot(100, &Gui::Control(), SLOT(closeDialog()));
	//}
}

void TaskAFPFaceFace::slotDeleteDocument(const Gui::Document&)
{
	//origins.clear();
	//QTimer::singleShot(100, &Gui::Control(), SLOT(closeDialog()));
}


//**************************************************************************
//**************************************************************************
// TaskDialog
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
TaskDlgAFPFaceFace::TaskDlgAFPFaceFace(std::vector<App::DocumentObject*> &objects,
	const std::vector<TaskAFPFaceFace::featureStatus> &status,
	boost::function<bool(std::vector<App::DocumentObject*>)> afunc) : TaskDialog(), accepted(false)
{
	facePick = new TaskAFPFaceFace(objects, status);
	Content.push_back(facePick);

	acceptFunction = afunc;
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
	accepted = acceptFunction( facePick->getFeature() );
	return accepted;
}

bool TaskDlgAFPFaceFace::reject()
{
	accepted = false;
	return true;
}


#include "AFP/moc_TaskAFPFaceFace.cpp"