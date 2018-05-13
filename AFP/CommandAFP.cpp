
#include "../PreCompiled.h"

#ifndef _PreComp_
# include <QMessageBox>
#endif

#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Control.h>

#include "TaskAFPFaceFace.h"


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

	std::vector<App::DocumentObject*> planes;
	std::vector<PartDesignGui::TaskAFPFaceFace::featureStatus> status;
	boost::function<bool(std::vector<App::DocumentObject*>)> afunc;
	
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
	Gui::Control().showDialog(new PartDesignGui::TaskDlgAFPFaceFace(planes, status, afunc));
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