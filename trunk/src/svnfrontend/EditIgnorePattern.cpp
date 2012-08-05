/*
 * EditIgnorePattern.cpp
 *
 *  Created on: 20.03.2010
 *      Author: ral
 */

#include "EditIgnorePattern.h"

EditIgnorePattern::EditIgnorePattern(QWidget*parent)
	:QWidget(parent),Ui::EditIgnorePattern()
{
	setupUi(this);
}

EditIgnorePattern::~EditIgnorePattern()
{
}

QStringList EditIgnorePattern::items()const
{
	return m_PatternEdit->items();
}

svn::Depth EditIgnorePattern::depth()const
{
	return m_DepthSelector->getDepth();
}

bool EditIgnorePattern::unignore()const
{
	return m_RemoveCheckBox->isChecked();
}

#include "EditIgnorePattern.moc"
