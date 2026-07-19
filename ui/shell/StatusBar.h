#pragma once

#include <QWidget>

class QLabel;

namespace zen::ui {

class StatusBar : public QWidget {
    Q_OBJECT
public:
    explicit StatusBar(QWidget* parent = nullptr);

    void setMessage(const QString& text);
    void setWorkspaceName(const QString& name);   // empty → hide chip
    void setBranch(const QString& branch,
                   int ahead = 0, int behind = 0); // empty → hide chip

private:
    QLabel* m_message   {nullptr};
    QLabel* m_workspace {nullptr};
    QLabel* m_branch    {nullptr};
};

} // namespace zen::ui
