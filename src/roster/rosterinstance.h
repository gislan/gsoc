#ifndef ROSTER_ROSTERINSTANCE_H
#define ROSTER_ROSTERINSTANCE_H

#include <QObject>

class PsiAccount;

namespace Roster {
	class Model;
	class View;
	class RosterBuilder;
	class StatusIconProvider;
	class Manager;
	class ViewStateManager;
	class Roster;
	class ViewActionsService;

	class RosterInstance : public QObject {
		Q_OBJECT

		public:
			RosterInstance();
			~RosterInstance();

			Model* getModel();
			View* getView();
			RosterBuilder* getRosterBuilder();

		public slots:
			void accountAdded(PsiAccount* acc);
			void accountRemoved(PsiAccount* acc);
			void accountUpdated(PsiAccount* acc);

			void setShowAvatars(bool showAvatars);
			void setShowStatusMessages(bool showStatusMessages);
			void setSlimContacts(bool slimContacts);

		private:
			View* view_;
			Manager* manager_;
			RosterBuilder* rb_;
			ViewStateManager* vsm_;
			Model* model_;
			StatusIconProvider* statusIconProvider_;
			Roster* data_;
			ViewActionsService* actionsService_;
	};	

}

#endif
