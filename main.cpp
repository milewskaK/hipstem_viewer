#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)


#include "mainwindow.h"

#include <QApplication>
#include <QVTKOpenGLWidget.h>
#include <QMainWindow>
using namespace std;

int main(int argc, char *argv[])
{


    QApplication a(argc, argv);

    //QVTKOpenGLWidget qvtkWidget;
    MainWindow w;
    w.setWindowTitle("Hip Stem Viewer");
    w.show();

    return a.exec();
}
