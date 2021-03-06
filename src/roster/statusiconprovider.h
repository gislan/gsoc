#ifndef ROSTER_STATUSICONPROVIDER_H
#define ROSTER_STATUSICONPROVIDER_H

#include <QIcon>

#include "globals.h"

namespace Roster {
	class StatusIconProvider : public QObject {
		Q_OBJECT

		public:
			const QIcon getIconForStatus(const StatusType status) const;
			const QIcon getIconForGroup(const bool expanded) const;

		signals:
			void updated();	
	};
}

#endif
