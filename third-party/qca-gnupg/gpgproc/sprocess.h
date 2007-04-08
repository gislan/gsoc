/*
 * Copyright (C) 2003-2005  Justin Karneges <justin@affinix.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef SPROCESS_H
#define SPROCESS_H

#include <QtCore>

namespace gpgQCAPlugin {

class SProcess : public QProcess
{
	Q_OBJECT
public:
	SProcess(QObject *parent = 0);
	~SProcess();

#ifdef Q_OS_UNIX
	void setClosePipeList(const QList<int> &);

protected:
	virtual void setupChildProcess();

private:
	QList<int> pipeList;
#endif
};

}

#endif
