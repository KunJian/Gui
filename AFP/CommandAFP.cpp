
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
		std::vector<App::DocumentObject*> partObjs = _AFPPart->getObjects();

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
		std::vector<App::DocumentObject*> partObjs = _AFPDoc->getObjects();

		for (std::vector<App::DocumentObject*>::const_iterator it = partObjs.begin(); it != partObjs.end(); ++it) {
			if ((*it)->getTypeId().isDerivedFrom(PartDesign::AFPGroup::getClassTypeId())) {
				AFPGrp = static_cast<PartDesign::AFPGroup*>(*it);
				break;
			}
		}
		return AFPGrp;
	}

	bool getAFPPrerequisits(App::Part** _AFPPart, App::Document** _AFPDoc, PartDesign::AFPGroup** AFPGrp)
	{
		*_AFPPart = PartDesignGui::getActivePart();
		if (*_AFPPart)
		{
			// find the AFP group of the active PartDesign
			*AFPGrp = getPartAFPGroup(*_AFPPart);
			if (!*AFPGrp) { // if it hasen't aleardy one, create one:
				Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().addObject('PartDesign::AFPGroup','AFPGroup')");
				Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().ActiveObject.Label = 'AFPGroup'");
				Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.Annotations = App.activeDocument().%s.Annotations + [App.activeDocument().ActiveObject]",
					(*_AFPPart)->getNameInDocument(), (*_AFPPart)->getNameInDocument());
			}
			*AFPGrp = getPartAFPGroup(*_AFPPart); // find now

			if (*AFPGrp) return true;
		}

		// find the AFP group of the active PartDesign
		*AFPGrp = getDocAFPGroup(*_AFPDoc);
		if (!*AFPGrp) { // if it hasen't aleardy one, create one:
			Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().addObject('PartDesign::AFPGroup','AFPGroup')");
			Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().ActiveObject.Label = 'AFPGroup'");
			Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.Annotations = App.activeDocument().%s.Annotations + [App.activeDocument().ActiveObject]",
				(*_AFPDoc)->getName(), (*_AFPDoc)->getName());
		}
		*AFPGrp = getDocAFPGroup(*_AFPDoc); // find now

		if (!*AFPGrp)
			throw Base::Exception("Could not create PartDesign::AFPGroup in active PartDesign");

		// return with no error
		return false;
	}

	std::string asSubLinkString(Part::Feature* _feat, std::string _element)
	{
		std::string buf;
		buf += "(App.ActiveDocument.";
		buf += _feat->getNameInDocument();
		buf += ",['";
		buf += _element;
		buf += "'])";
		return buf;
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
	if (!PartDesignGui::assureModernWorkflow(appDoc))
		return;

	Gui::Document* guiDoc = Gui::Application::Instance->getDocument(appDoc);

	//PartDesign::Body *pcActiveBody = PartDesignGui::getBody(true);
	//if (!pcActiveBody)
	//	return;

	//App::DocumentObject* feature = NULL;
	//PartDesignGui::TaskAFPFaceFace::featureStatus status;

	//// Show dialog and let user pick plane
	//Gui::TaskView::TaskDialog *dlg = Gui::Control().activeDialog();
	//PartDesignGui::TaskDlgAFPFaceFace* facePick = qobject_cast<PartDesignGui::TaskDlgAFPFaceFace *>(dlg);
	//if (dlg && !facePick) {
	//	QMessageBox msgBox;
	//	msgBox.setText(QObject::tr("A dialog is already open in the task panel"));
	//	msgBox.setInformativeText(QObject::tr("Do you want to close this dialog?"));
	//	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	//	msgBox.setDefaultButton(QMessageBox::Yes);
	//	int ret = msgBox.exec();
	//	if (ret == QMessageBox::Yes)
	//		Gui::Control().closeDialog();
	//	else {
	//		return;
	//	}
	//}

	//if (dlg)
	//	Gui::Control().closeDialog();

	//Gui::Selection().clearSelection();
	//Gui::Control().showDialog(new PartDesignGui::TaskDlgAFPFaceFace(feature));

	App::Part* AFPPart = 0;
	PartDesign::AFPGroup* AFPGrp = 0;

	// retrive the standard objects needed
	if (PartDesignGui::getAFPPrerequisits(&AFPPart, &appDoc, &AFPGrp))
		return;

	std::vector<Gui::SelectionObject> objs = Gui::Selection().getSelectionEx();
	if (objs.size() != 1) {
		QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Wrong selection"), QObject::tr("Only one geometries supported by AFPs"));
		return;
	};

	App::DocumentObject* obj = objs.front().getObject();
	if (obj == NULL || !obj->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId()))
		return;

	Part::Feature* selFeat = dynamic_cast<PartDesign::Feature*>(obj);
	if (selFeat == NULL)
		return;

	openCommand("Insert Face-Face AFP");
	std::string AFPName = getUniqueObjectName("AFP-FaceFace");
	doCommand(Doc, "App.activeDocument().addObject('PartDesign::AFP','%s')", AFPName.c_str());
	doCommand(Doc, "App.activeDocument().ActiveObject.m_type = 'AFP-FaceFace'");

	std::vector<Gui::ViewProvider*> vps = guiDoc->getViewProvidersOfType(PartDesignGui::ViewProviderAFP::getClassTypeId());
	if (vps.size() != 1)
	{
		QMessageBox::warning(Gui::getMainWindow(), QObject::tr("No ViewProviderAFP"), QObject::tr("No ViewProviderAFP has been constructed!"));
		return;
	}

	std::stringstream AFStr;
	PartDesignGui::ViewProviderAFP* AFPViewProvider = dynamic_cast<PartDesignGui::ViewProviderAFP*>(vps[0]);
	PartDesign::AFP* AFPObj = dynamic_cast<PartDesign::AFP*>(AFPViewProvider->getObject());
	if (AFPObj == NULL)
		AFStr << "App.activeDocument().ActiveObject.m_first = " << PartDesignGui::asSubLinkString(selFeat, objs[0].getSubNames()[0]);
	else
	{
		if (!AFPObj->m_first.getValue())
			AFStr << "App.activeDocument().ActiveObject.m_first = " << PartDesignGui::asSubLinkString(selFeat, objs[0].getSubNames()[0]);
		else if (!AFPObj->m_second.getValue())
			AFStr << "App.activeDocument().ActiveObject.m_second = " << PartDesignGui::asSubLinkString(selFeat, objs[0].getSubNames()[0]);
		else
			AFStr << "App.activeDocument().ActiveObject.m_first = " << PartDesignGui::asSubLinkString(selFeat, objs[0].getSubNames()[0]);
	}

	doCommand(Doc, AFStr.str().c_str());
	doCommand(Doc, "App.activeDocument().%s.m_AFPs = App.activeDocument().%s.m_AFPs + [App.activeDocument().ActiveObject]", AFPGrp->getNameInDocument(), AFPGrp->getNameInDocument());

	//updateActive();
	doCommand(Doc, "Gui.ActiveDocument.setEdit('%s',0)", AFPName.c_str());

	commitCommand();

	Gui::Selection().clearCompleteSelection();
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