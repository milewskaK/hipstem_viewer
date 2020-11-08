#include "mainwindow.h"
#include "ui_mainwindow.h"
 #include <QSurfaceFormat>
#include <QFileDialog>
#include <vtkObjectFactory.h>
#include <vtkProp3D.h>
#include <sstream>
#include <vtkInteractorObserver.h>
#include <vtkImageProperty.h>
#include <vtkProp.h>
#include <QMessageBox>


using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
     ui->setupUi(this);


     vtkOpenGLRenderWindow::SetGlobalMaximumNumberOfMultiSamples(0);
     QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());

    newVtkPipe();


     // łączenie sygnałów i slotów
   // connect(ui->actionQt,SIGNAL(triggered()), qApp,SLOT(aboutQt()));

//     // przesłanie informacji z Qt do VTK
  //   connect(ui->horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(imageViewer->SetSlice(slice);));
     //connect(this->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
     // Zestawienie połączeń VTK->Gt:
     connections = vtkSmartPointer<vtkEventQtSlotConnect>::New();

     // przechwycenie zdarzeń VTK:
 //    connections->Connect(ui->qvtkWidget->GetInteractor(),
 //                         vtkCommand::LeftButtonReleaseEvent,
 //                         this,
 //   TODO:                      SLOT(updateCamCoords(vtkObject*, unsigned long, void*, void*)));


     //imageViewer->Render();
     // debug poprawności połączeń VTK->Qt na konsolę:
     connections->PrintSelf(cout, vtkIndent());      // tak można sprawdzać dowolne obiekty VTK

}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::newVtkPipe()
{

    //okno dialogowe do otwierania pliku stl
        QString filePath = QFileDialog::getOpenFileName(this, "Open stl file", QDir::homePath(), "STL Files(*.stl)");

        bone_renderer = vtkSmartPointer<vtkRenderer>::New();
        axial_renderer = vtkSmartPointer<vtkRenderer>::New();
        sagittal_renderer = vtkSmartPointer<vtkRenderer>::New();
        coronal_renderer = vtkSmartPointer<vtkRenderer>::New();

        window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        axial_window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        sagittal_window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        coronal_window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();


        string fname ="/home/marcin/Dokumenty/project/files/image";

        color = vtkSmartPointer<vtkNamedColors>::New();
        array<unsigned char, 4> skinColor{{251, 188, 184}};
        color->SetColor("SkinColor", skinColor.data());
        array<unsigned char, 4> bonecolor{{251, 219, 184}};
        color->SetColor("BoneColor", bonecolor.data());
        array<unsigned char, 4> musclecolor{{175, 97, 93}};
        color->SetColor("MuscleColor", musclecolor.data());

       vtkOpenGLClearErrorMacro();
       dicom_reader =vtkSmartPointer<vtkDICOMImageReader>::New();
       dicom_reader->SetDirectoryName(fname.c_str());
       dicom_reader->Update();
       dicom_reader->GetOutputInformation(0)->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent);
       dicom_reader->GetOutput()->GetSpacing(spacing);
       dicom_reader->GetOutput()->GetOrigin(origin);

       stl_reader = vtkSmartPointer<vtkSTLReader>::New();
       stl_reader ->SetFileName(filePath.toStdString().c_str());
       stl_reader ->Update();
       //stl_reader->getou

       //3d view

       //bones
       bone_extract = vtkSmartPointer<vtkMarchingCubes>::New();
       bone_extract->SetInputConnection(dicom_reader->GetOutputPort());
       bone_extract->SetValue(0, 300);

       bone_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
       bone_mapper->SetInputConnection(bone_extract->GetOutputPort());
       bone_mapper -> ScalarVisibilityOff();

       bone = vtkSmartPointer<vtkActor>::New();
       bone->SetMapper(bone_mapper);
       bone->GetProperty()->SetDiffuseColor(color->GetColor3d("BoneColor").GetData());
       bone->GetProperty()->SetOpacity(0.3);
       bone->GetProperty()->ShadingOn();
       bone->GetProperty()->SetShading(1);
       bone->PickableOff();

       //skin
       skin_extract = vtkSmartPointer<vtkMarchingCubes>::New();
       skin_extract->SetInputConnection(dicom_reader->GetOutputPort());
       skin_extract->SetValue(0,50);

       skin_stripper = vtkSmartPointer<vtkStripper>::New();
       skin_stripper->SetInputConnection(skin_extract->GetOutputPort());

       skin_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
       skin_mapper->SetInputConnection(skin_stripper->GetOutputPort());
       skin_mapper -> ScalarVisibilityOff();

       skin_actor = vtkSmartPointer<vtkActor>::New();
       skin_actor->SetMapper(skin_mapper);
       skin_actor->GetProperty()->SetDiffuseColor(color->GetColor3d("SkinColor").GetData());
       skin_actor->GetProperty()->SetOpacity(0.3);
       skin_actor->VisibilityOff();

       //muscles
       muscle_extract = vtkSmartPointer<vtkMarchingCubes>::New();
       muscle_extract->SetInputConnection(dicom_reader->GetOutputPort());

       muscle_stripper = vtkSmartPointer<vtkStripper>::New();
       muscle_stripper->SetInputConnection(muscle_extract->GetOutputPort());

       muscle_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
       muscle_mapper->SetInputConnection(muscle_stripper->GetOutputPort());
       muscle_mapper -> ScalarVisibilityOff();

       muscle_actor = vtkSmartPointer<vtkActor>::New();
       muscle_actor->SetMapper(muscle_mapper);
       muscle_actor->GetProperty()->SetDiffuseColor(color->GetColor3d("MuscleColor").GetData());
       muscle_actor->GetProperty()->SetOpacity(0.2);
       muscle_actor->VisibilityOff();


       //stl
       stl_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
       stl_mapper->SetInputConnection(stl_reader->GetOutputPort());

       stl_actor = vtkSmartPointer<vtkActor>::New();
       stl_actor->SetMapper(stl_mapper);


       //cursors
       cursor = vtkSmartPointer<vtkCursor3D>::New();
       cursor->SetModelBounds(-20,20,-20,20,-20,20);
       cursor->SetAxes(1);
       cursor->AllOn();
       cursor->OutlineOff();
       cursor->Update();

       cursor_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
       cursor_mapper->SetInputConnection(cursor->GetOutputPort());

       cursor_actor = vtkSmartPointer<vtkActor>::New();
       cursor_actor->GetProperty()->SetColor(1,0,0);
       cursor_actor->SetMapper(cursor_mapper);
        //
       cursor2 = vtkSmartPointer<vtkCursor2D>::New();
       cursor2->SetModelBounds(-10,10,-10,10,0,0);
       cursor2->SetAxes(1);
       cursor2->AllOn();
       cursor2->SetFocalPoint(stl_actor->GetPosition());
       cursor2->OutlineOn();
       cursor2->Update();

       cursorMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
       cursorMapper->SetInputConnection(cursor2->GetOutputPort());

       axial_cursor = vtkSmartPointer<vtkActor>::New();
       axial_cursor ->GetProperty()->SetColor(1,0,0);
       axial_cursor ->SetMapper(cursorMapper);
       //
       cursor3 = vtkSmartPointer<vtkCursor2D>::New();
       cursor3->SetModelBounds(-10,10,-10,10,0,0);
       cursor3->SetAxes(1);
       cursor3->AllOn();
       cursor3->SetFocalPoint(stl_actor->GetPosition());
       cursor3->OutlineOn();
       cursor3->Update();

       cursorMapper3 = vtkSmartPointer<vtkPolyDataMapper>::New();
       cursorMapper3->SetInputConnection(cursor3->GetOutputPort());

       sagittal_cursor = vtkSmartPointer<vtkActor>::New();
       sagittal_cursor ->GetProperty()->SetColor(1,0,0);
       sagittal_cursor ->SetMapper(cursorMapper3);
       //
       cursor4 = vtkSmartPointer<vtkCursor2D>::New();
       cursor4->SetModelBounds(-10,10,-10,10,0,0);
       cursor4->SetAxes(1);
       cursor4->AllOn();
       cursor4->SetFocalPoint(stl_actor->GetPosition());
       cursor4->OutlineOn();
       cursor4->Update();

       cursorMapper4 = vtkSmartPointer<vtkPolyDataMapper>::New();
       cursorMapper4->SetInputConnection(cursor4->GetOutputPort());

       coronal_cursor = vtkSmartPointer<vtkActor>::New();
       coronal_cursor ->GetProperty()->SetColor(1,0,0);
       coronal_cursor ->SetMapper(cursorMapper4);

       vtkOpenGLClearErrorMacro();
       //axial
       axial_connect = vtkSmartPointer<vtkImageMapToColors>::New();
       axial_connect->SetInputConnection(dicom_reader->GetOutputPort());
       axial_view = vtkSmartPointer<vtkImageActor>::New();
       axial_view->GetMapper()->SetInputConnection(dicom_reader->GetOutputPort());
       axial_view->Update();

       //sagittal
       sagittal_view = vtkSmartPointer<vtkImageActor>::New();
       sagittal_view->GetMapper()->SetInputConnection(dicom_reader->GetOutputPort());
//       sagittal_view->SetPosition(256, 256, 50);
       sagittal_view->Update();
       sagittal_view->RotateZ(90);
       sagittal_view->RotateX(180);
       sagittal_view->RotateY(270);
//       sagittal_view->SetScale(0.9, 0.9, 0.9);
       sagittal_view->ForceOpaqueOn();
       sagittal_view->Update();

       //coronal
       coronal_view = vtkSmartPointer<vtkImageActor>::New();
       coronal_view->GetMapper()->SetInputConnection(dicom_reader->GetOutputPort());
       coronal_view->RotateX(90);
       coronal_view->ForceOpaqueOn();
       coronal_view->Update();

       //renderers
       axial_renderer->AddActor(axial_view);
       axial_renderer->AddActor(axial_cursor);
       axial_renderer->SetInteractive(1);
       axial_renderer->ResetCameraClippingRange();

       sagittal_renderer->AddActor(sagittal_view);
       sagittal_renderer->AddActor(sagittal_cursor);
       //sagittal_renderer->SetInteractive(1);
       sagittal_renderer->ResetCameraClippingRange();

       coronal_renderer->AddActor(coronal_view);
       coronal_renderer->AddActor(coronal_cursor);
       coronal_renderer->ResetCameraClippingRange();

       bone_renderer->AddActor(stl_actor);
       bone_renderer->AddActor(bone);
       bone_renderer->AddActor(cursor_actor);
       bone_renderer->AddActor(skin_actor);
       bone_renderer->AddActor(muscle_actor);
       bone_renderer->SetInteractive(1);
       bone_renderer->GetLayer();

       window->AddRenderer(bone_renderer);
       axial_window->AddRenderer(axial_renderer);
       sagittal_window->AddRenderer(sagittal_renderer);
       coronal_window->AddRenderer(coronal_renderer);

       iren_3d=vtkSmartPointer<vtkRenderWindowInteractor>::New();
       switch_style = vtkSmartPointer<vtkInteractorStyleSwitch>::New();


       //interactors and styles

       iren_axial = vtkSmartPointer<vtkRenderWindowInteractor>::New();
       axial_style = vtkSmartPointer<vtkInteractorStyleImage>::New();
       axial_style->SetInteractionModeToImage3D();

       iren_sagittal = vtkSmartPointer<vtkRenderWindowInteractor>::New();
       sagittal_style = vtkSmartPointer<vtkInteractorStyleImage>::New();
       iren_coronal = vtkSmartPointer<vtkRenderWindowInteractor>::New();
       coronal_style = vtkSmartPointer<vtkInteractorStyleImage>::New();

       vtkOpenGLClearErrorMacro();
       ui->qvtkWidget->SetRenderWindow(axial_window);
       iren_axial = ui->qvtkWidget->GetInteractor();
       iren_axial->SetInteractorStyle(axial_style);
       ui->qvtkWidget_2->SetRenderWindow(coronal_window);
       iren_sagittal = ui->qvtkWidget_2->GetInteractor();
       iren_sagittal->SetInteractorStyle(sagittal_style);
       ui->qvtkWidget_3->SetRenderWindow(window);
       iren_3d=ui->qvtkWidget_3->GetInteractor();
       iren_3d->SetInteractorStyle(switch_style);
       ui->qvtkWidget_4->SetRenderWindow(sagittal_window);
       iren_coronal = ui->qvtkWidget_4->GetInteractor();
       iren_coronal->SetInteractorStyle(coronal_style);


}


void MainWindow::on_actionOpen_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open stl file", QDir::homePath(), "STL Files(*.stl)");
}

void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    ui->horizontalSlider->setMaximum(100);
    ui->horizontalSlider->setSingleStep(1);
    ui->horizontalSlider_2->update();
    axial_view->SetDisplayExtent(0,511,0,511,position,position);
    axial_view->Update();
    ui->qvtkWidget->GetRenderWindow()->Render();
}

void MainWindow::on_horizontalSlider_2_sliderMoved(int position)
{
    ui->horizontalSlider_2->setMaximum(511);
    ui->horizontalSlider_2->setSingleStep(1);
    ui->horizontalSlider_2->update();
    sagittal_view->SetDisplayExtent(position,position,0,511,0,100);
    sagittal_view->Update();
    ui->qvtkWidget_4->GetRenderWindow()->Render();
}

void MainWindow::on_horizontalSlider_3_sliderMoved(int position)
{
    //ui->horizontalSlider_3
    ui->horizontalSlider_3->setMaximum(511);
    ui->horizontalSlider_3->setSingleStep(1);
    coronal_view->SetDisplayExtent(0,511,position,position,0,100);
    coronal_view->Update();
    ui->qvtkWidget_2->GetRenderWindow()->Render();

}


void MainWindow::on_pushButton_5_pressed()
{
    skin_actor->VisibilityOn();
    bone_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
}

void MainWindow::on_pushButton_6_pressed()
{
    skin_actor->VisibilityOff();
    muscle_actor->VisibilityOff();
    bone_renderer->Render();
    bone_extract->SetValue(0,300);
    ui->qvtkWidget_3->GetRenderWindow()->Render();

}

void MainWindow::on_pushButton_7_pressed()
{
    muscle_actor->VisibilityOn();
    bone_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
}

void MainWindow::on_pushButton_pressed()
{
    bone_renderer->ResetCamera();
    bone_renderer->Render();
    axial_renderer->ResetCamera();
    axial_renderer->Render();
    sagittal_renderer->ResetCamera();
    sagittal_renderer->Render();
    coronal_renderer->ResetCamera();
    coronal_renderer->Render();
    ui->qvtkWidget->GetRenderWindow()->Render();
    ui->qvtkWidget_2->GetRenderWindow()->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
    ui->qvtkWidget_4->GetRenderWindow()->Render();

}

void MainWindow::on_pushButton_2_pressed()
{

  stl_actor->VisibilityOn();
  bone_renderer->Render();
  ui->qvtkWidget_3->GetRenderWindow()->Render();

}


void MainWindow::on_pushButton_8_clicked()
{
    stl_actor->VisibilityOff();
    bone_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();

}

void MainWindow::on_pushButton_3_clicked()
{
    bone->VisibilityOn();
    bone_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
}

void MainWindow::on_pushButton_9_clicked()
{
    bone->VisibilityOff();
    bone_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
}

void MainWindow::on_pushButton_4_clicked()
{
    cursor_actor->VisibilityOn();
    bone_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
}

void MainWindow::on_pushButton_10_clicked()
{
    cursor_actor->VisibilityOff();
    bone_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
}

void MainWindow::on_actionInstruction_triggered()
{
    QMessageBox::information(this, "Instruction", "Welcome in Hip Stem Viewer! \n \n Click 't' to turn trackball mode, or 'j' to turn joistick mode. \n Click 'a' buton to turn actor mode on, or 'c' to turn camera mode on. \n \n"
                                                  "Orthogonal Views: Press Shift with left mouse to pan the camera. \nScroll mouse to change dolly.");
}

void MainWindow::on_doubleSpinBox_4_valueChanged(double arg1)
{
    ui->doubleSpinBox_4->setMinimum(0.0);
    ui->doubleSpinBox_4->setMaximum(1.0);
    ui->doubleSpinBox_4->setSingleStep(0.1);
    bone->GetProperty()->SetOpacity(arg1);
    bone_renderer->Render();
    skin_actor->GetProperty()->SetOpacity(arg1);
    bone_renderer->Render();
    muscle_actor->GetProperty()->SetOpacity(arg1);
    bone_renderer->Render();
    ui->qvtkWidget->GetRenderWindow()->Render();
}


void MainWindow::on_pushButton_11_clicked()
{
    cursor_actor->SetPosition(stl_actor->GetPosition());
    bone_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
    ui->qvtkWidget_3->update();
}

void MainWindow::on_doubleSpinBox_valueChanged(double arg1)
{
    ui->doubleSpinBox->setMinimum(-500.0);
    ui->doubleSpinBox->setMaximum(500.0);
    ui->doubleSpinBox->setSingleStep(5.0);
    ui->doubleSpinBox->update();
    stl_actor->SetPosition(arg1, y, z);
    cursor_actor->SetPosition(arg1, y, z);
    sagittal_cursor->SetPosition(y, z, -arg1);
    coronal_cursor->SetPosition(arg1, z, y);
    axial_cursor->SetPosition(arg1, y, z);
    x=arg1;
    bone_renderer->Render();
    sagittal_renderer->Render();
    axial_renderer->Render();
    coronal_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
    ui->qvtkWidget_4->GetRenderWindow()->Render();
    ui->qvtkWidget_2->GetRenderWindow()->Render();
    ui->qvtkWidget->GetRenderWindow()->Render();
}

void MainWindow::on_doubleSpinBox_2_valueChanged(double arg2)
{
    ui->doubleSpinBox_2->setMinimum(-500.0);
    ui->doubleSpinBox_2->setMaximum(500.0);
    ui->doubleSpinBox_2->setSingleStep(5.0);
    ui->doubleSpinBox_2->update();
    stl_actor->SetPosition(x, arg2, z);
    cursor_actor->SetPosition(x, arg2, z);
    sagittal_cursor->SetPosition(arg2, z, -x);
    coronal_cursor->SetPosition(x, z, arg2);
    axial_cursor->SetPosition(x, arg2, z);
    y=arg2;
    bone_renderer->Render();
    sagittal_renderer->Render();
    axial_renderer->Render();
    coronal_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
    ui->qvtkWidget_4->GetRenderWindow()->Render();
    ui->qvtkWidget_2->GetRenderWindow()->Render();
    ui->qvtkWidget->GetRenderWindow()->Render();
}

void MainWindow::on_doubleSpinBox_3_valueChanged(double arg3)
{
    ui->doubleSpinBox_3->setMinimum(-500.0);
    ui->doubleSpinBox_3->setMaximum(500.0);
    ui->doubleSpinBox_3->setSingleStep(5.0);
    ui->doubleSpinBox_3->update();
    stl_actor->SetPosition(x, y, arg3);
    cursor_actor->SetPosition(x, y, arg3);
    sagittal_cursor->SetPosition(y, -arg3, -x);
    coronal_cursor->SetPosition(x,-arg3, y);
    axial_cursor->SetPosition(x, y, arg3);
    z=arg3;
    bone_renderer->Render();
    sagittal_renderer->Render();
    axial_renderer->Render();
    coronal_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
    ui->qvtkWidget_4->GetRenderWindow()->Render();
    ui->qvtkWidget_2->GetRenderWindow()->Render();
    ui->qvtkWidget->GetRenderWindow()->Render();

}

void MainWindow::on_doubleSpinBox_7_valueChanged(double arg4)
{
    ui->doubleSpinBox_7->setMinimum(-180.0);
    ui->doubleSpinBox_7->setMaximum(180.0);
    ui->doubleSpinBox_7->setSingleStep(2.0);
    ui->doubleSpinBox_7->update();
    x_2 = arg4;
    stl_actor->SetOrientation(arg4, y_2, z_2);
    cursor_actor->SetOrientation(stl_actor->GetOrientation());
    bone_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
}

void MainWindow::on_doubleSpinBox_5_valueChanged(double arg5)
{
    ui->doubleSpinBox_5->setMinimum(-180.0);
    ui->doubleSpinBox_5->setMaximum(180.0);
    ui->doubleSpinBox_5->setSingleStep(2.0);
    ui->doubleSpinBox_5->update();
    y_2 = arg5;
    stl_actor->SetOrientation(x_2, arg5, z_2);
    cursor_actor->SetOrientation(stl_actor->GetOrientation());
    bone_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
}

void MainWindow::on_doubleSpinBox_6_valueChanged(double arg6)
{
    ui->doubleSpinBox_6->setMinimum(-180.0);
    ui->doubleSpinBox_6->setMaximum(180.0);
    ui->doubleSpinBox_6->setSingleStep(2.0);
    ui->doubleSpinBox_6->update();
    z_2 = arg6;
    stl_actor->SetOrientation(x_2, y_2, arg6);
    cursor_actor->SetOrientation(stl_actor->GetOrientation());
    bone_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
}

void MainWindow::on_horizontalScrollBar_sliderMoved(int position)
{
    ui->horizontalScrollBar->setMaximum(1500);
    ui->horizontalScrollBar->setSingleStep(100);
    bone_extract->SetValue(0,position);
    bone_renderer->Render();
    ui->qvtkWidget_3->GetRenderWindow()->Render();
}

void MainWindow::on_actionOpen_STL_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open stl file", QDir::homePath(), "STL Files(*.stl)");
}

void MainWindow::on_actionExit_2_triggered()
{
    MainWindow::close();
}
