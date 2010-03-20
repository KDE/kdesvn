/*
 * EditIgnorePattern.h
 *
 *  Created on: 20.03.2010
 *      Author: ral
 */

#ifndef EDITIGNOREPATTERN_H_
#define EDITIGNOREPATTERN_H_

#include "ui_editignorepattern.h"

#include "src/svnqt/svnqttypes.h"

#include <QWidget>
#include <QStringList>

class EditIgnorePattern:public QWidget, public Ui::EditIgnorePattern {
	Q_OBJECT

public:
	EditIgnorePattern(QWidget*parent=0);
	virtual ~EditIgnorePattern();

	QStringList items()const;
	svn::Depth depth()const;
	bool unignore()const;
};

#endif /* EDITIGNOREPATTERN_H_ */
