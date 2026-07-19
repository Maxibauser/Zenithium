#pragma once

#include <QWidget>

#include "zen/git/GitService.h"

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
class QPushButton;
class QScrollArea;
class QToolButton;
class QVBoxLayout;

namespace zen::ui {

class GitActivityCard;

class GitPanel : public QWidget {
    Q_OBJECT
public:
    explicit GitPanel(QWidget* parent = nullptr);

    void setRepoFolder(const QString& folderPath);
    zen::git::GitService* service() { return m_git; }

signals:
    // Emitted whenever the service reports a new status snapshot so the shell
    // can update the status bar chip.
    void branchInfoChanged(const QString& branch, int ahead, int behind);

private slots:
    void onStatusUpdated(const zen::git::StatusSnapshot& snap);
    void onLogLine(const QString& line);
    void onCommandDone(const QString& command, int exitCode);

private:
    void buildUi();
    void wireActions();

    QStringList selectedPaths(QListWidget* list) const;
    void        addFileRow(QListWidget* list,
                           const zen::git::FileEntry& e,
                           bool staged);
    void        pushActivityCard(const QString& command);

    zen::git::GitService* m_git {nullptr};

    // Header + branch
    QLabel*         m_branch        {nullptr};
    QLabel*         m_upstream      {nullptr};
    QLabel*         m_ahead         {nullptr};
    QLabel*         m_repoHint      {nullptr};
    QPushButton*    m_init          {nullptr};
    QPushButton*    m_publish       {nullptr};
    QString         m_currentBranch;

    // Commit
    QPlainTextEdit* m_message       {nullptr};
    QPushButton*    m_commit        {nullptr};
    QPushButton*    m_stageAll      {nullptr};

    // Toolbar
    QToolButton*    m_push          {nullptr};
    QToolButton*    m_pull          {nullptr};
    QToolButton*    m_fetch         {nullptr};
    QToolButton*    m_refresh       {nullptr};

    // File lists
    QLabel*         m_stagedHeader  {nullptr};
    QLabel*         m_changesHeader {nullptr};
    QListWidget*    m_staged        {nullptr};
    QListWidget*    m_unstaged      {nullptr};

    // Activity stream
    QScrollArea*      m_activityScroll {nullptr};
    QWidget*          m_activityHost   {nullptr};
    QVBoxLayout*      m_activityLayout {nullptr};
    GitActivityCard*  m_activeCard     {nullptr};
    QLabel*           m_activityHint   {nullptr};
};

} // namespace zen::ui
