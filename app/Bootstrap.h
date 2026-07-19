#pragma once

class QApplication;

namespace zen::app {

class Bootstrap {
public:
    static int run(int argc, char** argv);

private:
    static void configureApplication(QApplication& app);
};

} // namespace zen::app
