#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QProcess;
class QToolButton;

namespace zen::ui {

class TerminalPanel : public QWidget {
    Q_OBJECT
public:
    explicit TerminalPanel(QWidget* parent = nullptr);
    ~TerminalPanel() override;

    void setWorkingDirectory(const QString& path);

private slots:
    void onReadyRead();
    void onSubmit();
    void onProcessFinished(int code);
    void restartShell();
    void clearOutput();

private:
    void buildUi();
    void startShell();

    QPlainTextEdit* m_output {nullptr};
    QLineEdit*      m_input  {nullptr};
    QLabel*         m_cwdLbl {nullptr};
    QToolButton*    m_clear  {nullptr};
    QToolButton*    m_restart{nullptr};
    QProcess*       m_proc   {nullptr};
    QString         m_cwd;
};

} // namespace zen::ui
