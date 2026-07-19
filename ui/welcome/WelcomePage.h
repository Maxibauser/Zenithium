#pragma once

#include <QWidget>

namespace zen::ui {

class WelcomePage : public QWidget {
    Q_OBJECT
public:
    explicit WelcomePage(QWidget* parent = nullptr);

signals:
    void newFileRequested();
    void openFileRequested();
    void openFolderRequested();
};

} // namespace zen::ui
