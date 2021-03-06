#ifndef ROSTER_MAINWINDOW_H
#define ROSTER_MAINWINDOW_H

#include <QMainWindow>

class QAbstractItemModel;
class QTreeView;

namespace Roster {
	class Roster;
	class Manager;
	class Model;
	class View;
	class RosterBuilder;
	class ViewStateManager;
	class StatusIconProvider;

	class MainWindow : public QMainWindow {
		Q_OBJECT

		public:
			MainWindow();

		private:
			void buildTestRoster();
			void setupMenus();

			Model* model_;
			View* view_;
			Roster* data_;
			Manager* manager_;
			RosterBuilder* rb_;
			ViewStateManager* vsm_;
			StatusIconProvider* statusIconProvider_;

			QAction *toggleAvatarsAct_, *toggleStatusAct_, *toggleJoinedAccountsAct_, *toggleJoinByNameAct_, *toggleShowOfflineAct_;
	};
}

#endif
