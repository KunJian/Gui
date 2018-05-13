#ifndef TAKS_AFP_FACE_FACE_H
#define TAKS_AFP_FACE_FACE_H

#include <Gui/TaskView/TaskView.h>
#include <Gui/Selection.h>
#include <Gui/DocumentObserver.h>
#include <Gui/TaskView/TaskDialog.h>
#include <App/DocumentObject.h>

namespace PartDesignGui {

	class SoSwitch; //- 其作用目前还不清楚，需要继续确认，2018-05-12，xzj
	class Ui_TaskAFPFaceFace;

	class TaskAFPFaceFace : public Gui::TaskView::TaskBox, public Gui::SelectionObserver, public Gui::DocumentObserver/*public QDialog*/
	{
		Q_OBJECT

	public:
		// 添加枚举变量，用于标识特征的状态，2018-05-12，xzj
		enum featureStatus {
			validFeature = 0,
			invalidShape,
			otherBody,
			otherPart,
			notInBody,
			basePlane,
			afterTip
		};

		TaskAFPFaceFace(std::vector<App::DocumentObject*> &objects, const std::vector<featureStatus> &status, QWidget *parent = 0); //增加特征状态vector参数，2018-05-12，xzj
		~TaskAFPFaceFace();

		std::vector<App::DocumentObject*> getFeature();

		protected Q_SLOTS:
		void onUpdate(bool);
		void onSelectionChanged(const Gui::SelectionChanges& msg);

	protected:
		/** Notifies on undo */
		virtual void slotUndoDocument(const Gui::Document& Doc);
		/** Notifies on document deletion */
		virtual void slotDeleteDocument(const Gui::Document& Doc);

	private:
		Ui_TaskAFPFaceFace* ui;
		QWidget* proxy;
		bool doSelection;
		std::string documentName;

		std::vector<QString> features;
		std::vector<featureStatus> statuses;
		const QString getFeatureStatusString(const featureStatus st);
	};

	/// simulation dialog for the TaskView
	class TaskDlgAFPFaceFace : public Gui::TaskView::TaskDialog
	{
		Q_OBJECT

	public:
		TaskDlgAFPFaceFace(std::vector<App::DocumentObject*> &object, 
			const std::vector<TaskAFPFaceFace::featureStatus> &status,
			boost::function<bool(std::vector<App::DocumentObject*>)> afunc);
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
		TaskAFPFaceFace* facePick;
		bool accepted;

		boost::function<bool(std::vector<App::DocumentObject*>)>  acceptFunction;
	};
}

#endif // TAKS_AFP_FACE_FACE_H
