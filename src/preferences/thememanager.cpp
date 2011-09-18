#include "thememanager.h"
#include "ui_thememanager.h"
#include "mainapplication.h"
#include "globalfunctions.h"

ThemeManager::ThemeManager(QWidget* parent)
    : QWidget()
    , ui(new Ui::ThemeManager)
{
    ui->setupUi(parent);
    ui->license->hide();
    QSettings settings(mApp->getActiveProfilPath()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Themes");
    m_activeTheme = settings.value("activeTheme", "default").toString();
    settings.endGroup();

    QDir themeDir(mApp->THEMESDIR);
    QStringList list = themeDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    foreach(QString name, list) {
        Theme themeInfo = parseTheme(name);
        if (!themeInfo.isValid)
            continue;

        QListWidgetItem* item = new QListWidgetItem(ui->listWidget);
        item->setText(themeInfo.name + "\n" + themeInfo.shortDescription);
        item->setIcon(themeInfo.icon);
        item->setData(Qt::UserRole, name);

        if (m_activeTheme == name)
            ui->listWidget->setCurrentItem(item);

        ui->listWidget->addItem(item);
    }

    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(currentChanged()));
    connect(ui->license, SIGNAL(clicked(QPoint)), this, SLOT(showLicense()));

    currentChanged();
}

void ThemeManager::showLicense()
{
    QListWidgetItem* currentItem = ui->listWidget->currentItem();
    if (!currentItem)
        return;

    Theme currentTheme = m_themeHash[currentItem->data(Qt::UserRole).toString()];

    QTextBrowser* b = new QTextBrowser();
    b->setAttribute(Qt::WA_DeleteOnClose);
    b->setWindowTitle(tr("License Viewer"));
//    b->move(mapToGlobal(parent()->pos()));
    b->resize(450, 500);
    b->setText(currentTheme.license);
    b->show();
}

void ThemeManager::currentChanged()
{
    QListWidgetItem* currentItem = ui->listWidget->currentItem();
    if (!currentItem)
        return;

    Theme currentTheme = m_themeHash[currentItem->data(Qt::UserRole).toString()];

    ui->name->setText(currentTheme.name);
    ui->author->setText(currentTheme.author);
    ui->descirption->setText(currentTheme.longDescription);
    ui->license->setHidden(currentTheme.license.isEmpty());
}

ThemeManager::Theme ThemeManager::parseTheme(const QString &name)
{
    Theme info;
    info.name = name;

    QString path = mApp->THEMESDIR + name + "/";
    if (!QFile(path + "main.css").exists() || !QFile(path + "theme.info").exists()) {
        info.isValid = false;
        return info;
    }

    if (QFile(path + "theme.png").exists())
        info.icon = QIcon(path + "theme.png");
    else
        info.icon = QIcon(":icons/preferences/style-default.png");

    if (QFile(path + "theme.license").exists())
        info.license = qz_readAllFileContents(path + "theme.license");

    QString theme_info = qz_readAllFileContents(path + "theme.info");

    QRegExp rx("Name:(.*)\\n");
    rx.setMinimal(true);
    rx.indexIn(theme_info);
    if (rx.captureCount() == 1)
        info.name = rx.cap(1).trimmed();

    rx.setPattern("Author:(.*)\\n");
    rx.indexIn(theme_info);
    if (rx.captureCount() == 1)
        info.author = rx.cap(1).trimmed();

    rx.setPattern("Short Description:(.*)\\n");
    rx.indexIn(theme_info);
    if (rx.captureCount() == 1)
        info.shortDescription = rx.cap(1).trimmed();

    rx.setPattern("Long Description:(.*)\\n");
    rx.indexIn(theme_info);
    if (rx.captureCount() == 1)
        info.longDescription = rx.cap(1).trimmed();

    info.isValid = true;
    m_themeHash.insert(name, info);
    return info;
}

void ThemeManager::save()
{
    QSettings settings(mApp->getActiveProfilPath()+"settings.ini", QSettings::IniFormat);
    settings.beginGroup("Themes");
    settings.setValue("activeTheme", ui->listWidget->currentItem()->data(Qt::UserRole));
    settings.endGroup();
}

ThemeManager::~ThemeManager()
{
    delete ui;
}
