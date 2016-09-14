/***************************************************************************
 *   Copyright (C) 2007,2010 by John Layt <john@layt.net>                  *
 *                                                                         *
 *   Adaptations for QupZilla                                              *
 *   Copyright (C) 2016 by Kevin Kofler <kevin.kofler@chello.at>           *
 *                                                                         *
 *   FilePrinterPreview based on KPrintPreview (originally LGPL)           *
 *   Copyright (c) 2007 Alex Merry <huntedhacker@tiscali.co.uk>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "fileprinter.h"

#include <QtPrintSupport/QPrinter>
#include <QPrintEngine>
#include <QStringList>
#include <QSize>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtWidgets/QLabel>
#include <QtGui/QShowEvent>
#include <QtNetwork/QTcpSocket>
#include <QStandardPaths>

using namespace Qz;

void FilePrinter::printFile( QPrinter *printer, const QString &file,
                             FileDeletePolicy fileDeletePolicy,
                             PageSelectPolicy pageSelectPolicy, const QString &pageRange )
{
    FilePrinter fp;
    fp.doPrintFile( *printer, file, fileDeletePolicy, pageSelectPolicy, pageRange );
}

void FilePrinter::doPrintFile( QPrinter &printer, const QString &file, FileDeletePolicy fileDeletePolicy,
                               PageSelectPolicy pageSelectPolicy, const QString &pageRange )
{

    if (!QFile::exists(file)) {
        return;
    }

    bool doDeleteFile = (fileDeletePolicy == FilePrinter::SystemDeletesFiles);

    if ( printer.printerState() == QPrinter::Aborted || printer.printerState() == QPrinter::Error ) {
        if ( doDeleteFile ) {
            QFile::remove( file );
        }
        return;
    }


    // Print to a printer via lpr command

    //Decide what executable to use to print with, need the CUPS version of lpr if available
    //Some distros name the CUPS version of lpr as lpr-cups or lpr.cups so try those first 
    //before default to lpr, or failing that to lp

    QString exe;
    if ( !QStandardPaths::findExecutable(QStringLiteral("lpr-cups")).isEmpty() ) {
        exe = QStringLiteral("lpr-cups");
    } else if ( !QStandardPaths::findExecutable(QStringLiteral("lpr.cups")).isEmpty() ) {
        exe = QStringLiteral("lpr.cups");
    } else if ( !QStandardPaths::findExecutable(QStringLiteral("lpr")).isEmpty() ) {
        exe = QStringLiteral("lpr");
    } else if ( !QStandardPaths::findExecutable(QStringLiteral("lp")).isEmpty() ) {
        exe = QStringLiteral("lp");
    } else {
        if ( doDeleteFile ) {
            QFile::remove( file );
        }
        return;
    }

    bool useCupsOptions = cupsAvailable();
    QStringList argList = printArguments( printer, fileDeletePolicy, pageSelectPolicy,
                                          useCupsOptions, pageRange, exe ) << file;

    QProcess *process = new QProcess();
    QObject::connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error), [=](QProcess::ProcessError) {
        if ( doDeleteFile ) {
            QFile::remove( file );
        }
        process->deleteLater();
    });
    QObject::connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus) {
        if ( doDeleteFile && (exitStatus != QProcess::NormalExit || exitCode != 0) ) {
            // lpr failed, so delete the temporary file in case it still exists.
            // In case of success, we let lpr delete it, it knows best when it is safe to do so.
            // (lpr queues jobs asynchronously.)
            QFile::remove( file );
        }
        process->deleteLater();
    });
    process->start( exe, argList );
}

bool FilePrinter::cupsAvailable()
{
#if defined(Q_OS_OSX)
    return true;
#elif defined(Q_OS_UNIX)
    // Ideally we would have access to the private Qt method
    // QCUPSSupport::cupsAvailable() to do this as it is very complex routine.
    // However, if CUPS is available then QPrinter::numCopies() will always return 1
    // whereas if CUPS is not available it will return the real number of copies.
    // This behaviour is guaranteed never to change, so we can use it as a reliable substitute.
    QPrinter testPrinter;
    testPrinter.setNumCopies( 2 );
    return ( testPrinter.numCopies() == 1 );
#else
    return false;
#endif
}



QStringList FilePrinter::printArguments( QPrinter &printer, FileDeletePolicy fileDeletePolicy,
                                         PageSelectPolicy pageSelectPolicy, bool useCupsOptions,
                                         const QString &pageRange, const QString &version )
{
    QStringList argList;

    if ( ! destination( printer, version ).isEmpty() ) {
        argList << destination( printer, version );
    }

    if ( ! copies( printer, version ).isEmpty() ) {
        argList << copies( printer, version );
    }

    if ( ! jobname( printer, version ).isEmpty() ) {
        argList << jobname( printer, version );
    }

    if ( ! pages( printer, pageSelectPolicy, pageRange, useCupsOptions, version ).isEmpty() ) {
        argList << pages( printer, pageSelectPolicy, pageRange, useCupsOptions, version );
    }

    if ( useCupsOptions && ! cupsOptions( printer ).isEmpty() ) {
        argList << cupsOptions( printer );
    }

    if ( ! deleteFile( printer, fileDeletePolicy, version ).isEmpty() ) {
        argList << deleteFile( printer, fileDeletePolicy, version );
    }

    if ( version == QLatin1String("lp") ) {
        argList << QStringLiteral("--");
    }

    return argList;
}

QStringList FilePrinter::destination( QPrinter &printer, const QString &version )
{
    if ( version == QLatin1String("lp") ) {
        return QStringList(QStringLiteral("-d")) << printer.printerName();
    }

    if ( version.startsWith( QLatin1String("lpr") ) ) {
        return QStringList(QStringLiteral("-P")) << printer.printerName();
    }

    return QStringList();
}

QStringList FilePrinter::copies( QPrinter &printer, const QString &version )
{
    int cp = printer.actualNumCopies();

    if ( version == QLatin1String("lp") ) {
        return QStringList(QStringLiteral("-n")) << QStringLiteral("%1").arg( cp );
    }

    if ( version.startsWith( QLatin1String("lpr") ) ) {
        return QStringList() << QStringLiteral("-#%1").arg( cp );
    }

    return QStringList();
}

QStringList FilePrinter::jobname( QPrinter &printer, const QString &version )
{
    if ( ! printer.docName().isEmpty() ) {

        if ( version == QLatin1String("lp") ) {
            return QStringList(QStringLiteral("-t")) << printer.docName();
        }

        if ( version.startsWith( QLatin1String("lpr") ) ) {
            const QString shortenedDocName = QString::fromUtf8(printer.docName().toUtf8().left(255));
            return QStringList(QStringLiteral("-J")) << shortenedDocName;
        }
    }

    return QStringList();
}

QStringList FilePrinter::deleteFile( QPrinter &, FileDeletePolicy fileDeletePolicy, const QString &version )
{
    if ( fileDeletePolicy == FilePrinter::SystemDeletesFiles && version.startsWith( QLatin1String("lpr") ) ) {
        return QStringList(QStringLiteral("-r"));
    }

    return QStringList();
}

QStringList FilePrinter::pages( QPrinter &printer, PageSelectPolicy pageSelectPolicy, const QString &pageRange,
                                    bool useCupsOptions, const QString &version )
{
    if ( pageSelectPolicy == FilePrinter::SystemSelectsPages ) {

        if ( printer.printRange() == QPrinter::Selection && ! pageRange.isEmpty() ) {

            if ( version == QLatin1String("lp") ) {
                return QStringList(QStringLiteral("-P")) << pageRange ;
            }

            if ( version.startsWith( QLatin1String("lpr") ) && useCupsOptions ) {
                return QStringList(QStringLiteral("-o")) << QStringLiteral("page-ranges=%1").arg( pageRange );
            }

        }

        if ( printer.printRange() == QPrinter::PageRange ) {

            if ( version == QLatin1String("lp") ) {
                return QStringList(QStringLiteral("-P")) << QStringLiteral("%1-%2").arg( printer.fromPage() )
                                                            .arg( printer.toPage() );
            }

            if ( version.startsWith( QLatin1String("lpr") ) && useCupsOptions ) {
                return QStringList(QStringLiteral("-o")) << QStringLiteral("page-ranges=%1-%2").arg( printer.fromPage() )
                                                                        .arg( printer.toPage() );
            }

        }

    }

    return QStringList(); // AllPages
}

QStringList FilePrinter::cupsOptions( QPrinter &printer )
{
    QStringList optionList;

    if ( ! optionMedia( printer ).isEmpty() ) {
        optionList << optionMedia( printer );
    }

    if ( ! optionDoubleSidedPrinting( printer ).isEmpty() ) {
        optionList << optionDoubleSidedPrinting( printer );
    }

    if ( ! optionPageOrder( printer ).isEmpty() ) {
        optionList << optionPageOrder( printer );
    }

    if ( ! optionCollateCopies( printer ).isEmpty() ) {
        optionList << optionCollateCopies( printer );
    }

    optionList << optionCupsProperties( printer );

    return optionList;
}

QStringList FilePrinter::optionMedia( QPrinter &printer )
{
    if ( ! mediaPageSize( printer ).isEmpty() && 
         ! mediaPaperSource( printer ).isEmpty() ) {
        return QStringList(QStringLiteral("-o")) <<
                QStringLiteral("media=%1,%2").arg( mediaPageSize( printer ) )
                                      .arg( mediaPaperSource( printer ) );
    }

    if ( ! mediaPageSize( printer ).isEmpty() ) {
        return QStringList(QStringLiteral("-o")) <<
                QStringLiteral("media=%1").arg( mediaPageSize( printer ) );
    }

    if ( ! mediaPaperSource( printer ).isEmpty() ) {
        return QStringList(QStringLiteral("-o")) <<
                QStringLiteral("media=%1").arg( mediaPaperSource( printer ) );
    }

    return QStringList();
}

QString FilePrinter::mediaPageSize( QPrinter &printer )
{
    switch ( printer.pageSize() ) {
    case QPrinter::A0:         return QStringLiteral("A0");
    case QPrinter::A1:         return QStringLiteral("A1");
    case QPrinter::A2:         return QStringLiteral("A2");
    case QPrinter::A3:         return QStringLiteral("A3");
    case QPrinter::A4:         return QStringLiteral("A4");
    case QPrinter::A5:         return QStringLiteral("A5");
    case QPrinter::A6:         return QStringLiteral("A6");
    case QPrinter::A7:         return QStringLiteral("A7");
    case QPrinter::A8:         return QStringLiteral("A8");
    case QPrinter::A9:         return QStringLiteral("A9");
    case QPrinter::B0:         return QStringLiteral("B0");
    case QPrinter::B1:         return QStringLiteral("B1");
    case QPrinter::B10:        return QStringLiteral("B10");
    case QPrinter::B2:         return QStringLiteral("B2");
    case QPrinter::B3:         return QStringLiteral("B3");
    case QPrinter::B4:         return QStringLiteral("B4");
    case QPrinter::B5:         return QStringLiteral("B5");
    case QPrinter::B6:         return QStringLiteral("B6");
    case QPrinter::B7:         return QStringLiteral("B7");
    case QPrinter::B8:         return QStringLiteral("B8");
    case QPrinter::B9:         return QStringLiteral("B9");
    case QPrinter::C5E:        return QStringLiteral("C5");     //Correct Translation?
    case QPrinter::Comm10E:    return QStringLiteral("Comm10"); //Correct Translation?
    case QPrinter::DLE:        return QStringLiteral("DL");     //Correct Translation?
    case QPrinter::Executive:  return QStringLiteral("Executive");
    case QPrinter::Folio:      return QStringLiteral("Folio");
    case QPrinter::Ledger:     return QStringLiteral("Ledger");
    case QPrinter::Legal:      return QStringLiteral("Legal");
    case QPrinter::Letter:     return QStringLiteral("Letter");
    case QPrinter::Tabloid:    return QStringLiteral("Tabloid");
    case QPrinter::Custom:     return QStringLiteral("Custom.%1x%2mm")
                                            .arg( printer.heightMM() )
                                            .arg( printer.widthMM() );
    default:                   return QString();
    }
}

// What about Upper and MultiPurpose?  And others in PPD???
QString FilePrinter::mediaPaperSource( QPrinter &printer )
{
    switch ( printer.paperSource() ) {
    case QPrinter::Auto:            return QString();
    case QPrinter::Cassette:        return QStringLiteral("Cassette");
    case QPrinter::Envelope:        return QStringLiteral("Envelope");
    case QPrinter::EnvelopeManual:  return QStringLiteral("EnvelopeManual");
    case QPrinter::FormSource:      return QStringLiteral("FormSource");
    case QPrinter::LargeCapacity:   return QStringLiteral("LargeCapacity");
    case QPrinter::LargeFormat:     return QStringLiteral("LargeFormat");
    case QPrinter::Lower:           return QStringLiteral("Lower");
    case QPrinter::MaxPageSource:   return QStringLiteral("MaxPageSource");
    case QPrinter::Middle:          return QStringLiteral("Middle");
    case QPrinter::Manual:          return QStringLiteral("Manual");
    case QPrinter::OnlyOne:         return QStringLiteral("OnlyOne");
    case QPrinter::Tractor:         return QStringLiteral("Tractor");
    case QPrinter::SmallFormat:     return QStringLiteral("SmallFormat");
    default:                        return QString();
    }
}

QStringList FilePrinter::optionDoubleSidedPrinting( QPrinter &printer )
{
    switch ( printer.duplex() ) {
    case QPrinter::DuplexNone:       return QStringList(QStringLiteral("-o")) << QStringLiteral("sides=one-sided");
    case QPrinter::DuplexAuto:       if ( printer.orientation() == QPrinter::Landscape ) {
                                         return QStringList(QStringLiteral("-o")) << QStringLiteral("sides=two-sided-short-edge");
                                     } else {
                                         return QStringList(QStringLiteral("-o")) << QStringLiteral("sides=two-sided-long-edge");
                                     }
    case QPrinter::DuplexLongSide:   return QStringList(QStringLiteral("-o")) << QStringLiteral("sides=two-sided-long-edge");
    case QPrinter::DuplexShortSide:  return QStringList(QStringLiteral("-o")) << QStringLiteral("sides=two-sided-short-edge");
    default:                         return QStringList();  //Use printer default
    }
}

QStringList FilePrinter::optionPageOrder( QPrinter &printer )
{
    if ( printer.pageOrder() == QPrinter::LastPageFirst ) {
        return QStringList(QStringLiteral("-o")) << QStringLiteral("outputorder=reverse");
    }
    return QStringList(QStringLiteral("-o")) << QStringLiteral("outputorder=normal");
}

QStringList FilePrinter::optionCollateCopies( QPrinter &printer )
{
    if ( printer.collateCopies() ) {
        return QStringList(QStringLiteral("-o")) << QStringLiteral("Collate=True");
    }
    return QStringList(QStringLiteral("-o")) << QStringLiteral("Collate=False");
}

QStringList FilePrinter::optionCupsProperties( QPrinter &printer )
{
    QStringList dialogOptions = printer.printEngine()->property(QPrintEngine::PrintEnginePropertyKey(0xfe00)).toStringList();
    QStringList cupsOptions;

    for ( int i = 0; i < dialogOptions.count(); i = i + 2 ) {
        if ( dialogOptions[i+1].isEmpty() ) {
            cupsOptions << QStringLiteral("-o") << dialogOptions[i];
        } else {
            cupsOptions << QStringLiteral("-o") << dialogOptions[i] + QLatin1Char('=') + dialogOptions[i+1];
        }
    }

    return cupsOptions;
}

/* kate: replace-tabs on; indent-width 4; */
