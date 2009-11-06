#include "kdeobservatory.h"

#include <KConfig>
#include <KConfigDialog>

#include "kdeobservatoryconfiggeneral.h"
#include "kdeobservatoryconfigprojects.h"
#include "ui_kdeobservatoryconfigcommitsummary.h"

#include "commitcollector.h"
#include "topactiveprojectsview.h"

#include "iview.h"

K_EXPORT_PLASMA_APPLET(kdeobservatory, KdeObservatory)

KdeObservatory::KdeObservatory(QObject *parent, const QVariantList &args)
: Plasma::Applet(parent, args)
{
    setBackgroundHints(DefaultBackground);
    setHasConfigurationInterface(true);  
    resize(300, 200);
}

KdeObservatory::~KdeObservatory()
{
}

void KdeObservatory::init()
{
    m_configGroup = config();

    m_commitExtent = m_configGroup.readEntry("commitExtent", 1);
    m_synchronizationDelay = m_configGroup.readEntry("synchronizationDelay", 60);
    m_cacheContents = m_configGroup.readEntry("cacheContents", true);
    m_enableAnimations = m_configGroup.readEntry("enableAnimations", true);
    m_enableTransitionEffects = m_configGroup.readEntry("enableTransitionEffects", true);
    m_enableAutoViewChange = m_configGroup.readEntry("enableAutoViewChange", true);
    m_viewsDelay = m_configGroup.readEntry("viewsDelay", 5);

    QStringList viewNames = m_configGroup.readEntry("viewNames", QStringList());
    QList<bool> viewActives = m_configGroup.readEntry("viewActives", QList<bool>());

    m_views.clear();
    int viewsCount = viewNames.count();
    for (int i = 0; i < viewsCount; ++i)
        m_views.append(QPair<QString, bool>(viewNames.at(i), viewActives.at(i)));

    QStringList projectNames = m_configGroup.readEntry("projectNames", QStringList());
    QStringList projectCommitSubjects = m_configGroup.readEntry("projectCommitSubjects", QStringList());
    QStringList projectIcons = m_configGroup.readEntry("projectIcons", QStringList());

    m_projects.clear();
    int projectsCount = projectNames.count();
    for (int i = 0; i < projectsCount; ++i)
    {
        Project project;
        project.commitSubject = projectCommitSubjects.at(i);
        project.icon = projectIcons.at(i);
        m_projects[projectNames.at(i)] = project;
    }
}

void KdeObservatory::createConfigurationInterface(KConfigDialog *parent)
{
    m_configGeneral = new KdeObservatoryConfigGeneral(parent);
    parent->addPage(m_configGeneral, i18n("General"), "applications-development");

    m_configProjects = new KdeObservatoryConfigProjects(parent);
    parent->addPage(m_configProjects, i18n("Projects"), "project-development");

    QWidget *configCommitSummary = new QWidget(parent);
    Ui::KdeObservatoryConfigCommitSummary *ui_configCommitSummary = new Ui::KdeObservatoryConfigCommitSummary;
    ui_configCommitSummary->setupUi(configCommitSummary);
    parent->addPage(configCommitSummary, i18n("Commit Summary"), "svn-commit");

    m_configGeneral->commitExtent->setValue(m_commitExtent);
    m_configGeneral->synchronizationDelay->setTime(QTime(m_synchronizationDelay/3600, (m_synchronizationDelay/60)%60, m_synchronizationDelay%60));
    m_configGeneral->cacheContents->setChecked(m_cacheContents);
    m_configGeneral->enableAnimations->setChecked(m_enableAnimations);
    m_configGeneral->enableTransitionEffects->setChecked(m_enableTransitionEffects);
    m_configGeneral->enableAutoViewChange->setChecked(m_enableAutoViewChange);
    m_configGeneral->viewsDelay->setTime(QTime(m_viewsDelay/3600, (m_viewsDelay/60)%60, m_viewsDelay%60));

    int viewsCount = m_views.count();
    for (int i = 0; i < viewsCount; ++i)
    {
        QListWidgetItem * item = m_configGeneral->activeViews->findItems(m_views.at(i).first, Qt::MatchFixedString).at(0);
        item->setCheckState(m_views.at(i).second == true ? Qt::Checked:Qt::Unchecked);
        m_configGeneral->activeViews->takeItem(m_configGeneral->activeViews->row(item));
        m_configGeneral->activeViews->insertItem(i, item);
    }

    foreach(QString projectName, m_projects.keys())
    {
        Project project = m_projects.value(projectName);
        m_configProjects->createTableWidgetItem(projectName, project.commitSubject, project.icon);
    }

    m_configProjects->projects->setCurrentItem(m_configProjects->projects->item(0, 0));
    m_configProjects->projects->resizeColumnsToContents();
    m_configProjects->projects->horizontalHeader()->setStretchLastSection(true);

    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void KdeObservatory::configAccepted()
{
    QStringList projectNames;
    QStringList projectCommitSubjects;
    QStringList projectIcons;

    m_projects.clear();

    int projectsCount = m_configProjects->projects->rowCount();
    for (int i = 0; i < projectsCount; ++i)
    {
        Project project;
        project.commitSubject = m_configProjects->projects->item(i, 1)->text();
        project.icon = m_configProjects->projects->item(i, 0)->data(Qt::UserRole).value<QString>();
        m_projects[m_configProjects->projects->item(i, 0)->text()] = project;
        projectNames << m_configProjects->projects->item(i, 0)->text();
        projectCommitSubjects << project.commitSubject;
        projectIcons << project.icon;
    }

    // General properties
    m_configGroup.writeEntry("commitExtent", m_commitExtent = m_configGeneral->commitExtent->value());
    QTime synchronizationDelay = m_configGeneral->synchronizationDelay->time();
    m_configGroup.writeEntry("synchronizationDelay", m_synchronizationDelay = synchronizationDelay.second() + synchronizationDelay.minute()*60 + synchronizationDelay.hour()*3600);
    m_configGroup.writeEntry("cacheContents", m_cacheContents = (m_configGeneral->cacheContents->checkState() == Qt::Checked) ? true:false);
    m_configGroup.writeEntry("enableAnimations", m_enableAnimations = (m_configGeneral->enableAnimations->checkState() == Qt::Checked) ? true:false);
    m_configGroup.writeEntry("enableTransitionEffects", m_enableTransitionEffects = (m_configGeneral->enableTransitionEffects->checkState() == Qt::Checked) ? true:false);
    m_configGroup.writeEntry("enableAutoViewChange", m_enableAutoViewChange = (m_configGeneral->enableAutoViewChange->checkState() == Qt::Checked) ? true:false);
    QTime viewsDelay = m_configGeneral->viewsDelay->time();
    m_configGroup.writeEntry("viewsDelay", m_viewsDelay = viewsDelay.second() + viewsDelay.minute()*60 + viewsDelay.hour()*3600);

    m_views.clear();
    QStringList viewNames;
    QList<bool> viewActives;

    for (int i = 0; i < m_configGeneral->activeViews->count(); ++i)
    {
        QListWidgetItem *item = m_configGeneral->activeViews->item(i);
        QString viewName = item->text();
        bool viewActive = (item->checkState() == Qt::Checked) ? true:false;
        m_views << QPair<QString, bool>(viewName, viewActive);
        viewNames << viewName;
        viewActives << viewActive;
    }
    m_configGroup.writeEntry("viewNames", viewNames);
    m_configGroup.writeEntry("viewActives", viewActives);

    // Projects properties
    m_configGroup.writeEntry("projectNames", projectNames);
    m_configGroup.writeEntry("projectCommitSubjects", projectCommitSubjects);
    m_configGroup.writeEntry("projectIcons", projectIcons);

    m_collector = new CommitCollector(m_projects, this);
    m_collector->setExtent(m_commitExtent);
    connect(m_collector, SIGNAL(collectFinished()), this, SLOT(collectFinished()));

    setBusy(true);
    m_collector->run();
    emit configNeedsSaving();
}

void KdeObservatory::collectFinished()
{
    setBusy(false);
    new TopActiveProjectsView(m_projects, m_collector, i18n("Top Active Projects"), contentsRect(), this);
}

#include "kdeobservatory.moc"
