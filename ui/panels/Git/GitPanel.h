#pragma once

#include <QWidget>

#include "zen/git/GitService.h"

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
class QPushButton;
class QToolButton;

namespace zen::ui {

class GitPanel : public QWidget {
    Q_OBJECT
public:
    explicit GitPanel(QWidget* parent = nullptr);

    void setRepoFolder(const QString& folderPath);
    zen::git::GitService* service() { return m_git; }

private slots:
    void onStatusUpdated(const zen::git::StatusSnapshot& snap);
    void onLogLine(const QString& line);

private:
    void buildUi();
    void wireActions();

    QStringList selectedPaths(QListWidget* list) const;
    void        addFileRow(QListWidget* list,
                           const zen::git::FileEntry& e,
                           bool staged);

    zen::git::GitService* m_git {nullptr};

    QLabel*         m_branch     {nullptr};
    QLabel*         m_ahead      {nullptr};
    QLabel*         m_repoHint   {nullptr};
    QPushButton*    m_init       {nullptr};

    QListWidget*    m_staged     {nullptr};
    QListWidget*    m_unstaged   {nullptr};

    QPlainTextEdit* m_message    {nullptr};

    QPushButton*    m_stageAll   {nullptr};
    QPushButton*    m_commit     {nullptr};
    QToolButton*    m_push       {nullptr};
    QToolButton*    m_pull       {nullptr};
    QToolButton*    m_fetch      {nullptr};
    QToolButton*    m_refresh    {nullptr};

    QPlainTextEdit* m_output     {nullptr};
};

} // namespace zen::ui
