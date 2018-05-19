
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

#include "TaskAFPFaceFace.h"
#include "../ReferenceSelection.h"
#include "../Utils.h"
#include "../WorkflowManager.h"

using namespace std;
//using namespace Attacher;


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
	App::Document *doc = getDocument();
	if (!PartDesignGui::assureModernWorkflow(doc))
		return;

	PartDesign::Body *pcActiveBody = PartDesignGui::getBody(true);

	if (!pcActiveBody)
		return;

	App::DocumentObject* feature = NULL;
	PartDesignGui::TaskAFPFaceFace::featureStatus status;
	
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
		if (ret == QMessageBox::Yes)
			Gui::Control().closeDialog();
		else {
			return;
		}
	}

	if (dlg)
		Gui::Control().closeDialog();

	Gui::Selection().clearSelection();
	Gui::Control().showDialog(new PartDesignGui::TaskDlgAFPFaceFace(feature, status));


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