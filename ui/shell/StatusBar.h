#pragma once

#include <QWidget>

class QLabel;

namespace zen::ui {

class StatusBar : public QWidget {
    Q_OBJECT
public:
    explicit StatusBar(QWidget* parent = nullptr);

    void setMessage(const QString& text);

private:
    QLabel* m_message {nullptr};
};

} // namespace zen::ui
