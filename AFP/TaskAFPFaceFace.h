#ifndef TAKS_AFP_FACE_FACE_H
#define TAKS_AFP_FACE_FACE_H

#include <Gui/TaskView/TaskView.h>
#include <Gui/Selection.h>
#include <Gui/DocumentObserver.h>
#include <Gui/TaskView/TaskDialog.h>
#include <App/DocumentObject.h>

#include <Mod/Assembly/App/opendcm/core.hpp>
#include <Mod/Assembly/App/Solver/Solver.h>

#include <QComboBox>

#include "ViewProviderAFP.h"

namespace PartDesignGui
{
	class SoSwitch; //- 其作用目前还不清楚，需要继续确认，2018-05-12，xzj
	class Ui_TaskAFPFaceFace;

	class TaskAFPFaceFace : public Gui::TaskView::TaskBox, public Gui::SelectionObserver, public Gui::DocumentObserver
	{
		Q_OBJECT

	public:
		//TaskAFPFaceFace(PartDesign::AFP* _AFP, QWidget* _parent = 0); //增加特征状态参数，2018-05-12，xzj
		TaskAFPFaceFace(ViewProviderAFP* _vp, QWidget* _parent = 0); //增加特征状态参数，2018-05-12，xzj
		~TaskAFPFaceFace();

		protected Q_SLOTS:
		void onUpdate(const QString& _lineText);
		void onUpdateAFGeom(const int _idx, QComboBox* _combBox);
		void onSelectionChanged(const Gui::SelectionChanges& _msg);
		void onFaceSelection(bool _pressed);
		void onConstraintSelection(const QString& _selText);

	public:
		bool eventFilter(QObject* _watched, QEvent* _event);    //注意這裡

	protected:
		const QString onAddSelection(const Gui::SelectionChanges& _msg, App::DocumentObject* _selObj, QString& _AFPText);
		void setLineEdit(QLineEdit* const _lnEdit, const QString _refText, const char* _subName);
		void exitSelectionMode();

		/** Notifies on undo */
		virtual void slotUndoDocument(const Gui::Document& _Doc);
		/** Notifies on document deletion */
		virtual void slotDeleteDocument(const Gui::Document& _Doc);

	private:
		Ui_TaskAFPFaceFace* m_ui;
		QWidget*            m_proxy;
		PartDesign::AFP*    m_AFP;
		ViewProviderAFP*    m_vp;

		bool                m_AF1HasFocus;
		bool                m_AF2HasFocus;

		std::string         m_documentName;
		QString             m_featureStr;
	};

	/// simulation dialog for the TaskView
	class TaskDlgAFPFaceFace : public Gui::TaskView::TaskDialog
	{
		Q_OBJECT

	public:
		//TaskDlgAFPFaceFace(PartDesign::AFP* _AFP);
		TaskDlgAFPFaceFace(ViewProviderAFP* _vp);
		~TaskDlgAFPFaceFace();

	public:
		/// is called the TaskView when the dialog is opened
		virtual void open();
		/// is called by the framework if an button is clicked which has no accept or reject role
		virtual void clicked(int);
		/// is called by the framework if the dialog is accepted (Ok)
		virtual bool accept();
		/// is called by the framework if the dialog is rejected (Cancel)
		virtual bool reject();
		/// is called by the framework if the user presses the help button 
		virtual bool isAllowedAlterDocument(void) const
		{
			return false;
		}
		/// returns for Close and Help button 
		virtual QDialogButtonBox::StandardButtons getStandardButtons(void) const
		{
			return QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
		}

	protected:
		TaskAFPFaceFace* m_facePick;
		PartDesign::AFP* m_AFP;
		ViewProviderAFP* m_vp;
	};
}

#endif // TAKS_AFP_FACE_FACE_H
