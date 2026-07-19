#pragma once

#include <QWidget>

class QToolButton;

namespace zen::ui {

class TitleBar : public QWidget {
    Q_OBJECT
public:
    explicit TitleBar(QWidget* parent = nullptr);

    QToolButton* gitButton()      const { return m_gitBtn; }
    QToolButton* terminalButton() const { return m_termBtn; }

private:
    QToolButton* m_gitBtn  {nullptr};
    QToolButton* m_termBtn {nullptr};
};

} // namespace zen::ui
