#include "zen/git/GitService.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>

namespace zen::git {

GitService::GitService(QObject* parent) : QObject(parent) {}
GitService::~GitService() = default;

void GitService::setRepoPath(const QString& folderPath) {
    m_requestedFolder = folderPath;
    m_repoPath = folderPath;
    // Detect whether the folder (or any parent) is inside a git repo.
    QProcess p;
    p.setWorkingDirectory(folderPath);
    p.start(QStringLiteral("git"),
            {QStringLiteral("rev-parse"), QStringLiteral("--show-toplevel")});
    p.waitForFinished(3000);
    if (p.exitCode() == 0) {
        m_repoPath = QString::fromUtf8(p.readAllStandardOutput()).trimmed();
        m_isRepo   = !m_repoPath.isEmpty();
    } else {
        m_isRepo = false;
    }
    emit repoChanged(m_repoPath, m_isRepo);
    if (m_isRepo) runStatusInternal();
    else emit statusUpdated({});
}

FileStatus GitService::mapCode(QChar c) {
    switch (c.toLatin1()) {
        case 'M': return FileStatus::Modified;
        case 'A': return FileStatus::Added;
        case 'D': return FileStatus::Deleted;
        case 'R': return FileStatus::Renamed;
        case 'C': return FileStatus::Copied;
        case 'U': return FileStatus::Conflicted;
        case 'T': return FileStatus::TypeChanged;
        case '?': return FileStatus::Untracked;
        case '!': return FileStatus::Ignored;
        case ' ':
        default:  return FileStatus::Unmodified;
    }
}

void GitService::runStatusInternal() {
    StatusSnapshot snap;
    snap.isRepo = m_isRepo;

    if (!m_isRepo) { emit statusUpdated(snap); return; }

    // Branch + ahead/behind via `git status --porcelain=v2 --branch`.
    QProcess proc;
    proc.setWorkingDirectory(m_repoPath);
    proc.start(QStringLiteral("git"),
               {QStringLiteral("status"),
                QStringLiteral("--porcelain=v2"),
                QStringLiteral("--branch")});
    if (!proc.waitForFinished(5000)) {
        emit logLine(QStringLiteral("[git status] timed out"));
        emit statusUpdated(snap);
        return;
    }
    const QString out = QString::fromUtf8(proc.readAllStandardOutput());
    const QStringList lines = out.split(QLatin1Char('\n'));

    for (const QString& raw : lines) {
        if (raw.isEmpty()) continue;
        if (raw.startsWith(QLatin1String("# branch.head "))) {
            snap.branch = raw.mid(14).trimmed();
        } else if (raw.startsWith(QLatin1String("# branch.upstream "))) {
            snap.upstream = raw.mid(18).trimmed();
        } else if (raw.startsWith(QLatin1String("# branch.ab "))) {
            const auto rest = raw.mid(12).split(QLatin1Char(' '), Qt::SkipEmptyParts);
            for (const auto& tok : rest) {
                if (tok.startsWith(QLatin1Char('+'))) snap.ahead  = tok.mid(1).toInt();
                if (tok.startsWith(QLatin1Char('-'))) snap.behind = tok.mid(1).toInt();
            }
        } else if (raw.startsWith(QLatin1Char('1'))) {
            // "1 XY sub mH mI mW hH hI path"
            const auto parts = raw.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.size() < 9) continue;
            FileEntry e;
            e.indexStatus   = mapCode(parts[1].at(0));
            e.workingStatus = mapCode(parts[1].at(1));
            e.path = parts.mid(8).join(QLatin1Char(' '));
            snap.entries.push_back(e);
        } else if (raw.startsWith(QLatin1Char('2'))) {
            const auto parts = raw.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.size() < 10) continue;
            FileEntry e;
            e.indexStatus   = mapCode(parts[1].at(0));
            e.workingStatus = mapCode(parts[1].at(1));
            e.path = parts.mid(9).join(QLatin1Char(' '));
            snap.entries.push_back(e);
        } else if (raw.startsWith(QLatin1String("? "))) {
            FileEntry e;
            e.indexStatus   = FileStatus::Unmodified;
            e.workingStatus = FileStatus::Untracked;
            e.path = raw.mid(2).trimmed();
            snap.entries.push_back(e);
        } else if (raw.startsWith(QLatin1String("u "))) {
            const auto parts = raw.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.size() < 11) continue;
            FileEntry e;
            e.indexStatus   = FileStatus::Conflicted;
            e.workingStatus = FileStatus::Conflicted;
            e.path = parts.mid(10).join(QLatin1Char(' '));
            snap.entries.push_back(e);
        }
    }

    emit statusUpdated(snap);
}

void GitService::refreshStatus() {
    if (!m_isRepo) return;
    runStatusInternal();
}

void GitService::runGit(const QString& command,
                        const QStringList& args,
                        bool refreshAfter) {
    if (!m_isRepo) {
        emit logLine(QStringLiteral("[git] no repository"));
        return;
    }
    emit logLine(QStringLiteral("$ git ") + command + QLatin1Char(' ') + args.join(QLatin1Char(' ')));

    auto* proc = new QProcess(this);
    proc->setWorkingDirectory(m_repoPath);
    proc->setProcessChannelMode(QProcess::MergedChannels);

    QStringList full;
    full << command;
    full.append(args);

    connect(proc, &QProcess::readyReadStandardOutput, this, [this, proc] {
        const QString chunk = QString::fromUtf8(proc->readAllStandardOutput());
        for (const auto& l : chunk.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
            emit logLine(l);
        }
    });
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc, command, refreshAfter](int code, QProcess::ExitStatus) {
                emit commandDone(command, code);
                proc->deleteLater();
                if (refreshAfter) runStatusInternal();
            });
    proc->start(QStringLiteral("git"), full);
}

void GitService::stage(const QStringList& paths) {
    if (paths.isEmpty()) return;
    QStringList args; args << QStringLiteral("--") << paths;
    runGit(QStringLiteral("add"), args, true);
}
void GitService::unstage(const QStringList& paths) {
    if (paths.isEmpty()) return;
    QStringList args; args << QStringLiteral("HEAD") << QStringLiteral("--") << paths;
    runGit(QStringLiteral("reset"), args, true);
}
void GitService::stageAll() {
    runGit(QStringLiteral("add"), {QStringLiteral("-A")}, true);
}
void GitService::discard(const QStringList& paths) {
    if (paths.isEmpty()) return;
    QStringList args; args << QStringLiteral("--") << paths;
    runGit(QStringLiteral("checkout"), args, true);
}
void GitService::commit(const QString& message) {
    if (message.trimmed().isEmpty()) {
        emit logLine(QStringLiteral("[git] empty commit message — aborted"));
        return;
    }
    runGit(QStringLiteral("commit"),
           {QStringLiteral("-m"), message}, true);
}
void GitService::initRepo() {
    if (m_requestedFolder.isEmpty()) {
        emit logLine(QStringLiteral("[git] no folder — open a folder first"));
        return;
    }
    emit logLine(QStringLiteral("$ git init  (in %1)").arg(m_requestedFolder));
    auto* proc = new QProcess(this);
    proc->setWorkingDirectory(m_requestedFolder);
    proc->setProcessChannelMode(QProcess::MergedChannels);
    connect(proc, &QProcess::readyReadStandardOutput, this, [this, proc] {
        const QString chunk = QString::fromUtf8(proc->readAllStandardOutput());
        for (const auto& l : chunk.split(QLatin1Char('\n'), Qt::SkipEmptyParts))
            emit logLine(l);
    });
    const QString folder = m_requestedFolder;
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc, folder](int code, QProcess::ExitStatus) {
                emit commandDone(QStringLiteral("init"), code);
                proc->deleteLater();
                if (code == 0) setRepoPath(folder);
            });
    proc->start(QStringLiteral("git"), {QStringLiteral("init")});
}

void GitService::push()  { runGit(QStringLiteral("push"),  {}, true); }
void GitService::pull()  { runGit(QStringLiteral("pull"),  {}, true); }
void GitService::fetch() { runGit(QStringLiteral("fetch"), {}, true); }

} // namespace zen::git
