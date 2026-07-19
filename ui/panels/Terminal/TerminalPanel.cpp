#include "TerminalPanel.h"

#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProcess>
#include <QToolButton>
#include <QVBoxLayout>

namespace zen::ui {

TerminalPanel::TerminalPanel(QWidget* parent) : QWidget(parent) {
    setObjectName("ZenTerminalPanel");
    m_cwd = QDir::homePath();
    buildUi();
    startShell();
}

TerminalPanel::~TerminalPanel() {
    if (m_proc) {
        m_proc->kill();
        m_proc->waitForFinished(500);
    }
}

void TerminalPanel::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 6, 8, 8);
    root->setSpacing(6);

    auto* header = new QHBoxLayout();
    auto* title = new QLabel(tr("TERMINAL"), this);
    title->setObjectName("ZenTerminalHeader");
    m_cwdLbl = new QLabel(m_cwd, this);
    m_cwdLbl->setObjectName("ZenTerminalCwd");
    header->addWidget(title);
    header->addSpacing(12);
    header->addWidget(m_cwdLbl, 1);

    auto makeIconBtn = [this](const QString& text, const QString& tip) {
        auto* b = new QToolButton(this);
        b->setObjectName("ZenTerminalToolBtn");
        b->setText(text);
        b->setToolTip(tip);
        b->setAutoRaise(true);
        b->setCursor(Qt::PointingHandCursor);
        return b;
    };
    m_clear   = makeIconBtn(QStringLiteral("⌫"), tr("Clear"));
    m_restart = makeIconBtn(QStringLiteral("↻"), tr("Restart shell"));
    header->addWidget(m_clear);
    header->addWidget(m_restart);
    root->addLayout(header);

    m_output = new QPlainTextEdit(this);
    m_output->setObjectName("ZenTerminalOutput");
    m_output->setReadOnly(true);
    m_output->setMaximumBlockCount(5000);
    root->addWidget(m_output, 1);

    auto* inputRow = new QHBoxLayout();
    auto* prompt = new QLabel(QStringLiteral("❯"), this);
    prompt->setObjectName("ZenTerminalPrompt");
    m_input = new QLineEdit(this);
    m_input->setObjectName("ZenTerminalInput");
    m_input->setPlaceholderText(tr("Enter a command…"));
    inputRow->addWidget(prompt);
    inputRow->addWidget(m_input, 1);
    root->addLayout(inputRow);

    connect(m_input,   &QLineEdit::returnPressed, this, &TerminalPanel::onSubmit);
    connect(m_clear,   &QToolButton::clicked,     this, &TerminalPanel::clearOutput);
    connect(m_restart, &QToolButton::clicked,     this, &TerminalPanel::restartShell);
}

void TerminalPanel::startShell() {
    m_proc = new QProcess(this);
    m_proc->setProcessChannelMode(QProcess::MergedChannels);
    m_proc->setWorkingDirectory(m_cwd);

    connect(m_proc, &QProcess::readyReadStandardOutput,
            this, &TerminalPanel::onReadyRead);
    connect(m_proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus) { onProcessFinished(code); });

#ifdef Q_OS_WIN
    m_proc->start(QStringLiteral("powershell.exe"),
                  {QStringLiteral("-NoLogo"),
                   QStringLiteral("-NoProfile"),
                   QStringLiteral("-NoExit"),
                   QStringLiteral("-Command"), QStringLiteral("-")});
    m_output->appendPlainText(QStringLiteral("[PowerShell] started in %1").arg(m_cwd));
#else
    m_proc->start(QStringLiteral("bash"), {QStringLiteral("-i")});
    m_output->appendPlainText(QStringLiteral("[bash] started in %1").arg(m_cwd));
#endif
}

void TerminalPanel::setWorkingDirectory(const QString& path) {
    if (path.isEmpty() || path == m_cwd) return;
    m_cwd = path;
    m_cwdLbl->setText(m_cwd);
    if (m_proc && m_proc->state() == QProcess::Running) {
#ifdef Q_OS_WIN
        const QString cmd = QStringLiteral("Set-Location -LiteralPath \"%1\"\n").arg(m_cwd);
#else
        const QString cmd = QStringLiteral("cd \"%1\"\n").arg(m_cwd);
#endif
        m_proc->write(cmd.toUtf8());
    }
}

void TerminalPanel::onReadyRead() {
    const QByteArray data = m_proc->readAllStandardOutput();
    if (data.isEmpty()) return;
    // Append raw output; strip trailing newline so appendPlainText doesn't double-space.
    QString text = QString::fromLocal8Bit(data);
    while (text.endsWith(QLatin1Char('\n')) || text.endsWith(QLatin1Char('\r'))) {
        text.chop(1);
    }
    if (!text.isEmpty()) m_output->appendPlainText(text);
}

void TerminalPanel::onSubmit() {
    if (!m_proc || m_proc->state() != QProcess::Running) return;
    const QString cmd = m_input->text();
    m_output->appendPlainText(QStringLiteral("❯ ") + cmd);
    m_proc->write((cmd + QLatin1Char('\n')).toUtf8());
    m_input->clear();
}

void TerminalPanel::onProcessFinished(int code) {
    m_output->appendPlainText(
        QStringLiteral("[shell exited with code %1]").arg(code));
}

void TerminalPanel::restartShell() {
    if (m_proc) {
        m_proc->kill();
        m_proc->waitForFinished(500);
        m_proc->deleteLater();
        m_proc = nullptr;
    }
    m_output->appendPlainText(QStringLiteral("--- restarting shell ---"));
    startShell();
}

void TerminalPanel::clearOutput() {
    m_output->clear();
}

} // namespace zen::ui
