/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2012  David Rosca <nowrep@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */

#include "sourcehighlighter.h"
#include <QStringList>


SourceHighlighter::SourceHighlighter(QString source)
{
	m_source = source;

}

void SourceHighlighter::setTitle(QString title)
{
	m_title = title;
}

QString SourceHighlighter::highlight()
{
	m_source.replace("<", "&lt;");
	m_source.replace(">", "&gt;");
	QStringList rows = m_source.split(QRegExp("(\r\n)|([\r\n])"));
	QString result = "<table border='0'>";
	for (int i = 0; i < rows.size(); i++){
		result += "<tr><td class='line-number'>"+QString::number(i+1)+"</td><td class='line-content'>"+rows.at(i)+"</td></tr>";
	}
	result += "</table>";
	//result += "<a target='_blank' href='1.html'>123</a>";


	return "<html><head><link rel='stylesheet' href='qrc:html/view-source.css' type='text/css'><title>"+m_title+"</title></head><body>"+result+"</body></html>";
}

SourceHighlighter::~SourceHighlighter()
{

}
