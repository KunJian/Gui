
#include "../PreCompiled.h"

#ifndef _PreComp_
# include <TopoDS_Face.hxx>
# include <TopoDS.hxx>
# include <BRepAdaptor_Surface.hxx>
# include <TopExp_Explorer.hxx>
# include <QMessageBox>
# include <Inventor/nodes/SoCamera.h>
#endif

#include <sstream>
#include <algorithm>

#include <App/DocumentObjectGroup.h>
#include <App/Part.h>
#include <App/Document.h>
#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Control.h>
#include <Gui/Selection.h>
#include <Gui/MainWindow.h>
#include <Gui/Document.h>
#include <Gui/View3DInventor.h>
#include <Gui/View3DInventorViewer.h>

#include <Mod/Part/App/FeatureFace.h>

#include <Mod/PartDesign/App/Body.h>
#include <Mod/PartDesign/App/Feature.h>
#include <Mod/PartDesign/App/AFP/AFP.h>
#include <Mod/PartDesign/App/AFP/AFPGroup.h>

#include "TaskAFPFaceFace.h"

#include "ViewProviderAFP.h"

#include "../ReferenceSelection.h"
#include "../Utils.h"
#include "../WorkflowManager.h"

using namespace std;
//using namespace Attacher;


// Helper methods ===========================================================
namespace PartDesignGui
{
	PartDesign::AFPGroup* getPartAFPGroup(App::Part* _AFPPart)
	{
		PartDesign::AFPGroup* AFPGrp = 0;
		std::vector<App::DocumentObject*> partObjs = _AFPPart->Annotations.getValues();

		for (std::vector<App::DocumentObject*>::const_iterator it = partObjs.begin(); it != partObjs.end(); ++it) {
			if ((*it)->getTypeId().isDerivedFrom(PartDesign::AFPGroup::getClassTypeId())) {
				AFPGrp = static_cast<PartDesign::AFPGroup*>(*it);
				break;
			}
		}
		return AFPGrp;
	}

	PartDesign::AFPGroup* getDocAFPGroup(App::Document* _AFPDoc)
	{
		PartDesign::AFPGroup* AFPGrp = 0;
		std::vector<App::DocumentObject*> partObjs = _AFPDoc->Annotations.getValues();

		for (std::vector<App::DocumentObject*>::const_iterator it = partObjs.begin(); it != partObjs.end(); ++it) {
			if ((*it)->getTypeId().isDerivedFrom(PartDesign::AFPGroup::getClassTypeId())) {
				AFPGrp = static_cast<PartDesign::AFPGroup*>(*it);
				break;
			}
		}
		return AFPGrp;
	}

	bool getAFPPrerequisits(App::Part** _AFPPart, App::Document** _AFPDoc, PartDesign::AFPGroup** _AFPGrp)
	{
		std::vector<App::DocumentObject*> parts = (*_AFPDoc)->getObjectsOfType(App::Part::getClassTypeId());
		if (parts.size())
		{
			*_AFPPart = dynamic_cast<App::Part*>(parts[0]);
			if (*_AFPPart)
			{
				Gui::Command::doCommand(Gui::Command::Gui, "Gui.getDocument('%s').ActiveView.setActiveObject('part',App.getDocument('%s').getObject('%s'))",
					(*_AFPDoc)->getName(), (*_AFPDoc)->getName(), (*_AFPPart)->getNameInDocument());
				// find the AFP group of the active PartDesign
				*_AFPGrp = getPartAFPGroup(*_AFPPart);
				if (!*_AFPGrp) { // if it hasen't aleardy one, create one:
					Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().addObject('PartDesign::AFPGroup','AFPGroup')");
					Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().ActiveObject.Label = 'AFPGroup'");
					Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.Annotations = App.activeDocument().%s.Annotations + [App.activeDocument().ActiveObject]", (*_AFPPart)->getNameInDocument(), (*_AFPPart)->getNameInDocument());
				}
				*_AFPGrp = getPartAFPGroup(*_AFPPart); // find now
				if (*_AFPGrp) return false;
			}
		}

		// find the AFP group of the active PartDesign
		*_AFPGrp = getDocAFPGroup(*_AFPDoc);
		if (!*_AFPGrp) { // if it hasen't aleardy one, create one:
			Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().addObject('PartDesign::AFPGroup','AFPs')");
			Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().ActiveObject.Label = 'AFPs'");
			Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().Annotations = App.activeDocument().Annotations + [App.activeDocument().ActiveObject]");
		}
		*_AFPGrp = getDocAFPGroup(*_AFPDoc); // find now

		if (!*_AFPGrp) throw Base::Exception("Could not create PartDesign::AFPGroup in active PartDesign");

		// return with no error
		return false;
	}

	//必须要添加该函数才能正确建立AFP
	PartDesign::AFP* getAFP(PartDesign::AFPGroup** _AFPGrp, const int _AFPCount)
	{
		PartDesign::AFP* AFPObj = 0;
		std::vector<App::DocumentObject*> AFPObjs = (*_AFPGrp)->AFPs.getValues();

		if (_AFPCount == AFPObjs.size())
			return AFPObj;

		if ((AFPObjs.back())->getTypeId().isDerivedFrom(PartDesign::AFP::getClassTypeId()))
			AFPObj = static_cast<PartDesign::AFP*>(AFPObjs.back());

		return AFPObj;
	}

	bool getAFP(PartDesign::AFPGroup** _AFPGrp, const int _AFPCount, PartDesign::AFP** _AFP)
	{
		*_AFP = getAFP(_AFPGrp, _AFPCount);
		if (!*_AFP) { // if it hasen't aleardy one, create one:
			Gui::Command::openCommand("Insert Face-Face AFP");
			Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().addObject('PartDesign::AFP','AFP-FaceFace')");
			Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().ActiveObject.Type = 'AFP-FaceFace'");
			Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.AFPs = App.activeDocument().%s.AFPs + [App.activeDocument().ActiveObject]", (*_AFPGrp)->getNameInDocument(), (*_AFPGrp)->getNameInDocument());
		}
		*_AFP = getAFP(_AFPGrp, _AFPCount);

		if (!(*_AFP)) throw Base::Exception("Could not create PartDesign::AFP in active PartDesign");

		// return with no error
		return false;
	}
} /* PartDesignGui */

/* AFP commands =======================================================*/

DEF_STD_CMD_A(CmdPartDesignAFPFaceFace);

CmdPartDesignAFPFaceFace::CmdPartDesignAFPFaceFace() : Command("PartDesign_AFPFaceFace")
{
	sAppModule = "PartDesign";
	sGroup = QT_TR_NOOP("PartDesign");
	sMenuText = QT_TR_NOOP("Associate a FF AFP");
	sToolTipText = QT_TR_NOOP("Associate a face-face AFP");
	sWhatsThis = sToolTipText;
	sStatusTip = sToolTipText;
	sPixmap = "PartDesign_Plane";
}

void CmdPartDesignAFPFaceFace::activated(int iMsg)
{
	Q_UNUSED(iMsg);
	App::Document* appDoc = getDocument();
	if (!PartDesignGui::assureModernWorkflow(appDoc)) return;

	// Show dialog and let user pick plane
	Gui::TaskView::TaskDialog *dlg = Gui::Control().activeDialog();
	PartDesignGui::TaskDlgAFPFaceFace* facePick = qobject_cast<PartDesignGui::TaskDlgAFPFaceFace *>(dlg);
	if (dlg && !facePick) {
		QMessageBox msgBox;
		msgBox.setText(QObject::tr("A dialog is already open in the task panel"));
		msgBox.setInformativeText(QObject::tr("Do you want to close this dialog?"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::Yes);

		int ret = msgBox.exec();
		if (ret == QMessageBox::Yes) Gui::Control().closeDialog();
		else return;
	}

	if (dlg) Gui::Control().closeDialog();

	Gui::Selection().clearSelection();

	App::Part* AFPPart = 0;
	PartDesign::AFPGroup* AFPGrp = 0;
	if (PartDesignGui::getAFPPrerequisits(&AFPPart, &appDoc, &AFPGrp))
		return;

	int AFPCount = AFPGrp->AFPs.getValues().size();
	PartDesign::AFP* AFPObj = 0;
	if (PartDesignGui::getAFP(&AFPGrp, AFPCount, &AFPObj))
		return;

	std::string id = std::to_string(AFPGrp->AFPs.getValues().size() - 1);
	AFPObj->ID.setValue(id);
	Gui::Control().showDialog(new PartDesignGui::TaskDlgAFPFaceFace(AFPObj));
}

bool CmdPartDesignAFPFaceFace::isActive(void)
{
	if (getActiveGuiDocument())
		return true;
	else
		return false;
}


//===========================================================================
// Initialization
//===========================================================================

void CreateAFPCommands(void)
{
	Gui::CommandManager &rcCmdMgr = Gui::Application::Instance->commandManager();

	rcCmdMgr.addCommand(new CmdPartDesignAFPFaceFace());
}