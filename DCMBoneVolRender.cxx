// Inspired by VTK/Examples/Cxx/Medical4.cxx
// Reads a DICOM volume dataset and displays it via volume rendering, only bone is shown.
// Lower density bone is rendered with 20% opacity to allow the visualization of air-cells in the skull. 
// This allows the user to visualize the pneumatization of the patient's temporal bones

#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkDICOMImageReader.h>
#include <vtkNamedColors.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include <array>

int main (int argc, char *argv[])
{
  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << "file.mhd" << endl;
    return EXIT_FAILURE;
  }

  vtkSmartPointer<vtkNamedColors> colors =
    vtkSmartPointer<vtkNamedColors>::New();

  std::array<unsigned char , 4> bkg{{51, 77, 102, 255}};
    colors->SetColor("BkgColor", bkg.data());

  // Create the renderer, the render window, and the interactor. The renderer
  // draws into the render window, the interactor enables mouse- and
  // keyboard-based interaction with the scene.
  vtkSmartPointer<vtkRenderer> ren =
    vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renWin =
    vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(ren);
  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  //DICOM Reader
	  // Read all the DICOM files in the specified directory.
  std::string folder = argv[1];
  vtkSmartPointer<vtkDICOMImageReader> reader =
	  vtkSmartPointer<vtkDICOMImageReader>::New();
  reader->SetDirectoryName(folder.c_str());
  reader->Update();

  // The volume will be displayed by ray-cast alpha compositing.
  // A ray-cast mapper is needed to do the ray-casting.
  vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapper =
    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
  volumeMapper->SetInputConnection(reader->GetOutputPort());

  // The color transfer function maps voxel intensities to colors.
  // It is modality-specific, and often anatomy-specific as well.
  // The goal is to one color for flesh (between 500 and 1000)
  // and another color for bone (1150 and over).
  vtkSmartPointer<vtkColorTransferFunction>volumeColor =
    vtkSmartPointer<vtkColorTransferFunction>::New();
  
  //Realistic colors
  //sRGB 255 generic anatomy colors									//converted to sRGB 1.0 colors 
  //https://www.slicer.org/wiki/Slicer3:2010_GenericAnatomyColors     https://tug.org/pracjourn/2007-4/walden/color.pdf
  //≈
  //D4≈right sphenoid bone	rgb(209, 185, 85)							= 0.81961,  0.72549,  0.33333
  //D3≈right frontal bone	rgb(203, 179, 77)							= 0.79608,  0.70196,  0.30196
  //D2≈right parietal bone	rgb(229, 204, 109)							= 0.89804,  0.80000,  0.42745
  //D1≈right temporal bone	rgb(255,243,152)							= 1.00000,  0.95294,  0.59608	 
  //Visualize with generic anatomy BONE colors
  //Comment this block and unblock the next to use exagerated colors
  volumeColor->AddRGBPoint(0, 0.0, 0.0, 0.0); //background
  volumeColor->AddRGBPoint(150, 0.81961, 0.72549, 0.33333); //D4 bone (fine trabecular) +150 to 350 HU
  volumeColor->AddRGBPoint(350, 0.81961, 0.72549, 0.33333); //Identical color because of the threshold
  volumeColor->AddRGBPoint(351, 0.79608, 0.70196, 0.30196); //D3 Bone (Porous trabecular bone) +350 to +850 HU
  volumeColor->AddRGBPoint(850, 0.79608, 0.70196, 0.30196); // 
  volumeColor->AddRGBPoint(851, 0.89804, 0.80000, 0.42745); //D2 Bone(Thick porous cortical bone) +850 to +1250 HU
  volumeColor->AddRGBPoint(1250,0.89804, 0.80000, 0.42745);
  volumeColor->AddRGBPoint(1251,1.00000, 0.95294, 0.59608); //D1 Bone(Dense cortical bone) > 1250
  
  /* Comment previous block and uncomment this one to use exagerated colors
  //D4≈blood	rgb(216,101,79)											= 0.84706  0.39608  0.30980
  //D3≈skull	rgb(241,213,144)										= 0.94510  0.83529  0.56471
  //D2≈bone		rgb(241,214,145)										= 0.94510  0.83922  0.56863
  //D1≈teeth	rgb(255,250,220)										= 1.00000  0.98039  0.86275
  volumeColor->AddRGBPoint(150, 0.84706  0.39608  0.30980); //D4 bone (fine trabecular) +150 to 350 HU
  volumeColor->AddRGBPoint(350, 0.84706  0.39608  0.30980); //Identical color because of the threshold
  volumeColor->AddRGBPoint(351, 0.94510  0.83529  0.56471); //D3 Bone (Porous trabecular bone) +350 to +850 HU
  volumeColor->AddRGBPoint(850, 0.94510  0.83529  0.56471); // 
  volumeColor->AddRGBPoint(851, 0.94510  0.83922  0.56863); //D2 Bone(Thick porous cortical bone) +850 to +1250 HU
  volumeColor->AddRGBPoint(1250,0.94510  0.83922  0.56863);
  volumeColor->AddRGBPoint(1251,1.00000  0.98039  0.86275); //D1 Bone(Dense cortical bone) > 1250
  */
  // The opacity transfer function is used to control the opacity
  // of different tissue types.
  vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity =
    vtkSmartPointer<vtkPiecewiseFunction>::New(); 
  volumeScalarOpacity->AddPoint(0,   0.00);
  volumeScalarOpacity->AddPoint(150, 0.25);
  volumeScalarOpacity->AddPoint(350, 0.50);
  volumeScalarOpacity->AddPoint(850, 1);
  
  // The gradient opacity function is used to decrease the opacity
  // in the "flat" regions of the volume while maintaining the opacity
  // at the boundaries between tissue types.  The gradient is measured
  // as the amount by which the intensity changes over unit distance.
  // For most medical data, the unit distance is 1mm.
  // Piecewise Function recommended by Users Guide for 8-bit unsigned data
  // meaning a byte-long image (8-bits)
 //volumeGradientOpacity->AddPoint(0, 0.0); // 0 is background
 //volumeGradientOpacity->AddPoint(3, 0); //
 //volumeGradientOpacity->AddPoint(6, 1.0); 
 //volumeGradientOpacity->AddPoint(255, 1.0); // 1 is the maximum
 // CT is traditionally unsigned int (16-bits) ranging from 0 to 65535.	
    vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity =
    vtkSmartPointer<vtkPiecewiseFunction>::New();
 volumeGradientOpacity->AddPoint(0, 0.0); 
 volumeGradientOpacity->AddPoint(765, 0); 
 volumeGradientOpacity->AddPoint(1530, 1.0);   
	//PENDING TEST
 
  // The VolumeProperty attaches the color and opacity functions to the
  // volume, and sets other volume properties.  The interpolation should
  // be set to linear to do a high-quality rendering.  The ShadeOn option
  // turns on directional lighting, which will usually enhance the
  // appearance of the volume and make it look more "3D".  However,
  // the quality of the shading depends on how accurately the gradient
  // of the volume can be calculated, and for noisy data the gradient
  // estimation will be very poor.  The impact of the shading can be
  // decreased by increasing the Ambient coefficient while decreasing
  // the Diffuse and Specular coefficient.  To increase the impact
  // of shading, decrease the Ambient and increase the Diffuse and Specular.
  vtkSmartPointer<vtkVolumeProperty> volumeProperty =
    vtkSmartPointer<vtkVolumeProperty>::New();
  volumeProperty->SetColor(volumeColor);
  volumeProperty->SetScalarOpacity(volumeScalarOpacity);
  volumeProperty->SetGradientOpacity(volumeGradientOpacity);
  volumeProperty->SetInterpolationTypeToLinear();
  volumeProperty->ShadeOn();
/* ORIGINAL PARAMETERs  
  volumeProperty->SetAmbient(0.4);
  volumeProperty->SetDiffuse(0.6);
  volumeProperty->SetSpecular(0.2);
  */
  //Updated parameters
  volumeProperty->SetAmbient(0.1);
  volumeProperty->SetDiffuse(1);
  volumeProperty->SetSpecular(0.3);

  // The vtkVolume is a vtkProp3D (like a vtkActor) and controls the position
  // and orientation of the volume in world coordinates.
  vtkSmartPointer<vtkVolume> volume =
    vtkSmartPointer<vtkVolume>::New();
  volume->SetMapper(volumeMapper);
  volume->SetProperty(volumeProperty);

  // Finally, add the volume to the renderer
  ren->AddViewProp(volume);

  // Set up an initial view of the volume.  The focal point will be the
  // center of the volume, and the camera position will be 400mm to the
  // patient's left (which is our right). Seeing right temporal bone. 
  // Change to + 400 to start seeing the left temporal bone.
  vtkCamera *camera = ren->GetActiveCamera();
  double *c = volume->GetCenter();
  camera->SetViewUp (0, 0, -1); 
  camera->SetPosition (c[0], c[1] - 400, c[2]);
  camera->SetFocalPoint (c[0], c[1], c[2]);
  camera->Azimuth(90);
  camera->Elevation(-20.0);

  // Set a background color for the renderer
  ren->SetBackground(colors->GetColor3d("BkgColor").GetData());

  // Increase the size of the render window
  //renWin->SetSize(640, 480); Original
  renWin->SetSize(1024, 720); //HD size render

  // Interact with the data.
  iren->Start();

  return EXIT_SUCCESS;
}
