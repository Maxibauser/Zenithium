#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

class QProcess;

namespace zen::git {

enum class FileStatus {
    Unmodified,
    Modified,
    Added,
    Deleted,
    Renamed,
    Copied,
    Untracked,
    Ignored,
    Conflicted,
    TypeChanged,
};

struct FileEntry {
    QString    path;
    FileStatus indexStatus   {FileStatus::Unmodified}; // staged
    FileStatus workingStatus {FileStatus::Unmodified}; // unstaged
};

struct StatusSnapshot {
    QString            branch;
    QString            upstream;
    int                ahead  {0};
    int                behind {0};
    QVector<FileEntry> entries;
    bool               isRepo {false};
};

class GitService : public QObject {
    Q_OBJECT
public:
    explicit GitService(QObject* parent = nullptr);
    ~GitService() override;

    void setRepoPath(const QString& folderPath);
    QString repoPath()     const { return m_repoPath; }
    QString requestedPath() const { return m_requestedFolder; }
    bool    isRepo()        const { return m_isRepo; }

    void refreshStatus();
    void initRepo();

    void stage       (const QStringList& paths);
    void unstage     (const QStringList& paths);
    void stageAll    ();
    void discard     (const QStringList& paths);

    void commit(const QString& message);
    void push();
    void pull();
    void fetch();

    // Set origin (creates if missing, updates if present) and push current branch with -u.
    void publishToRemote(const QString& url, const QString& branch);

signals:
    void statusUpdated(const zen::git::StatusSnapshot& snap);
    void logLine     (const QString& line);
    void commandDone (const QString& command, int exitCode);
    void repoChanged (const QString& repoPath, bool isRepo);

private:
    void runGit(const QString& command,
                const QStringList& args,
                bool refreshAfter);
    void runStatusInternal();
    static FileStatus mapCode(QChar c);

    QString m_repoPath;
    QString m_requestedFolder;
    bool    m_isRepo {false};
};

} // namespace zen::git
