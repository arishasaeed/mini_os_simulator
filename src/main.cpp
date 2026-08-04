#include <QApplication>
#include "controller/AppController.h"

int main(int argc, char *argv[]) {
    // Initialize standard Qt Application
    QApplication app(argc, argv);
    
    // Launch MVC system controller
    AppController controller;
    controller.start();
    
    // Execute standard event loop
    return app.exec();
}
