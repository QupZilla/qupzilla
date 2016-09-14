/***************************************************************************
 *   Copyright (C) 2007, 2010 by John Layt <john@layt.net>                 *
 *                                                                         *
 *   Adaptations for QupZilla                                              *
 *   Copyright (C) 2016 by Kevin Kofler <kevin.kofler@chello.at>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef FILEPRINTER_H
#define FILEPRINTER_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtPrintSupport/QPrinter>

#include "qzcommon.h"

class QSize;

namespace Qz {

class QUPZILLA_EXPORT FilePrinter
{
public:

    /** Whether file(s) get deleted by the application or by the print system.
     *
     *  You may need to chose system deletion if your temp file clean-up
     *  deletes the file before the print system is finished with it.
     */
    enum FileDeletePolicy { ApplicationDeletesFiles, SystemDeletesFiles };

    /** Whether pages to be printed are selected by the application or the print system.
     *
     *  If application side, then the generated file will only contain those pages
     *  selected by the user, so FilePrinter will print all the pages in the file.
     *
     *  If system side, then the file will contain all the pages in the document, and
     *  the print system will print the users selected print range from out of the file.
     *
     *  Note system side only works in CUPS, not LPR.
     */
    enum PageSelectPolicy { ApplicationSelectsPages, SystemSelectsPages };

    /** Print a file using the settings in QPrinter
     *
     *  Only supports CUPS and LPR on *NIX.  Page Range only supported in CUPS.
     *  Most settings unsupported by LPR, some settings unsupported by CUPS.
     *
     * @param printer the print settings to use
     * @param file the file to print
     * @param fileDeletePolicy if the application or system deletes the file
     * @param pageSelectPolicy if the application or system selects the pages to print
     * @param pageRange page range to print if SystemSelectsPages and user chooses Selection in Print Dialog
     */

    static void printFile( QPrinter *printer, const QString &file,
                           FileDeletePolicy fileDeletePolicy = FilePrinter::ApplicationDeletesFiles,
                           PageSelectPolicy pageSelectPolicy = FilePrinter::ApplicationSelectsPages,
                           const QString &pageRange = QString() );

    /** Return if CUPS Print System is available on this system
     *
     * @returns Returns true if CUPS available
     */
    static bool cupsAvailable();

protected:

    void doPrintFile( QPrinter &printer, const QString &file,
                      FileDeletePolicy fileDeletePolicy, PageSelectPolicy pageSelectPolicy,
                      const QString &pageRange );

    QStringList printArguments( QPrinter &printer,
                                FileDeletePolicy fileDeletePolicy, PageSelectPolicy pageSelectPolicy,
                                bool useCupsOptions, const QString &pageRange, const QString &version );

    QStringList destination( QPrinter &printer, const QString &version );
    QStringList copies( QPrinter &printer, const QString &version );
    QStringList jobname( QPrinter &printer, const QString &version );
    QStringList deleteFile( QPrinter &printer, FileDeletePolicy fileDeletePolicy,
                                   const QString &version );
    QStringList pages( QPrinter &printer, PageSelectPolicy pageSelectPolicy,
                              const QString &pageRange, bool useCupsOptions, const QString &version );

    QStringList cupsOptions( QPrinter &printer );
    QStringList optionMedia( QPrinter &printer );
    QString mediaPageSize( QPrinter &printer );
    QString mediaPaperSource( QPrinter &printer );
    QStringList optionDoubleSidedPrinting( QPrinter &printer );
    QStringList optionPageOrder( QPrinter &printer );
    QStringList optionCollateCopies( QPrinter &printer );
    QStringList optionCupsProperties( QPrinter &printer );
};

}

#endif // FILEPRINTER_H
