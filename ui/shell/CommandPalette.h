#pragma once

#include <QFrame>
#include <QString>
#include <QVector>
#include <functional>

class QLineEdit;
class QListWidget;
class QKeyEvent;

namespace zen::ui {

struct Command {
    QString              name;
    QString              category;
    QString              shortcut;
    std::function<void()> run;
};

class CommandPalette : public QFrame {
    Q_OBJECT
public:
    explicit CommandPalette(QWidget* parent = nullptr);

    void setCommands(QVector<Command> commands);
    void showPalette();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void keyPressEvent(QKeyEvent* e) override;

private slots:
    void onTextChanged(const QString& text);
    void runSelected();

private:
    void repopulate();

    QLineEdit*       m_query {nullptr};
    QListWidget*     m_list  {nullptr};
    QVector<Command> m_all;
    QVector<int>     m_filtered;   // indices into m_all
};

} // namespace zen::ui
