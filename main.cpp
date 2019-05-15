#include "audioapp.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Renfeng::AudioApp app;
    app.init();

    return a.exec();
}
