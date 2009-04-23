/****************************************************************************
 *
 * $Id: calibrate2dGrid.cpp,v 1.7 2008-06-17 08:08:23 asaunier Exp $
 *
 * Copyright (C) 1998-2006 Inria. All rights reserved.
 *
 * This software was developed at:
 * IRISA/INRIA Rennes
 * Projet Lagadic
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * http://www.irisa.fr/lagadic
 *
 * This file is part of the ViSP toolkit
 *
 * This file may be distributed under the terms of the Q Public License
 * as defined by Trolltech AS of Norway and appearing in the file
 * LICENSE included in the packaging of this file.
 *
 * Licensees holding valid ViSP Professional Edition licenses may
 * use this file in accordance with the ViSP Commercial License
 * Agreement provided with the Software.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Contact visp@irisa.fr if any conditions of this licensing are
 * not clear to you.
 *
 * Description:
 * Camera calibration with any calibration grid.
 *
 * Authors:
 * Anthony Saunier
 *
 *****************************************************************************/


/*!
  \example calibrate2dGrid.cpp
  \brief Tool for camera calibration.

  This example is an implementation of a camera calibration tool with
  a non linear method based on virtual visual servoing. It uses
  several images of a unique calibration grid. 

  The calibration grid used here is available in
  ViSP-images/calibration/grid2d.{fig,pdf} or in
  ./example/calibration/grid2d.fig (.fig files comes from Xfig tool).

  The calibration grid is a 6*6 dots grid where dots
  centers are spaced by 0.03 meter.  You can obviously use another
  calibration grid changing its parameters in the program.  Then you
  have to grab some images of this grid (you can use grab examples of
  ViSP to do it), save them as PGM files and precise their names with
  the -p option.
*/
#include <visp/vpConfig.h>
#include <visp/vpDebug.h>
#include <visp/vpParseArgv.h>
#include <visp/vpIoTools.h>

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

#include <visp/vpImage.h>
#include <visp/vpImageIo.h>
#include <visp/vpCalibration.h>
#include <visp/vpDisplayX.h>
#include <visp/vpDisplayGDI.h>
#include <visp/vpDisplayGTK.h>
#include <visp/vpDisplayD3D.h>
#include <visp/vpMouseButton.h>

#include <visp/vpPose.h>
#include <visp/vpDot.h>
#include <visp/vpDot2.h>
#include <visp/vpPixelMeterConversion.h>
#include <visp/vpMeterPixelConversion.h>
// List of allowed command line options
#define GETOPTARGS  "di:p:hf:g:n:s:l:c"

/*!

  Print the program options.

  \param name : Program name.
  \param badparam : Bad parameter name.
  \param ipath : Input image path.
  \param ppath : Personal image path.
  \param gray : Gray level precision.
  \param first : First image.
  \param nimages : Number of images to manipulate.
  \param step : Step between two images.
  \param lambda : Gain of the virtual visual servoing.

 */
void usage(const char *name,const char *badparam, std::string ipath, std::string ppath,
      double gray, unsigned first, unsigned nimages, unsigned step, double lambda)
{
  fprintf(stdout, "\n\
  Read images of a calibration grid from the disk and \n\
  calibrate the camera used for grabbing it.\n\
  Each image corresponds to a PGM file.\n\
  The calibration grid used here is available in : \n\
      ViSP-images/calibration/grid2d.{fig,pdf} or \n\
      ./example/calibration/grid2d.fig\n\
  This is a 6*6 dots calibration grid where dots centers \n\
  are spaced by 0.03 meter. You can obviously use another \n\
  calibration grid changing its parameters in the program.\n\
  Then you have to grab some images of this grid (you can use \n\
  grab examples of ViSP to do it), save them as PGM files and\n\
  precise their names with the -p option.\n\
\n\
SYNOPSIS\n\
  %s [-i <test image path>] [-p <personal image path>]\n\
     [-g <gray level precision>] [-f <first image>] \n\
     [-n <number of images>] [-s <step>] [-l lambda] \n\
     [-c] [-d] [-h]\n\
 ", name);

 fprintf(stdout, "\n\
 OPTIONS:                                               Default\n\
  -i <test image path>                                %s\n\
     Set image input path.\n\
     From this path read \"ViSP-images/calibration/grid36-%%02d.pgm\"\n\
     images and the calibration grid data. \n\
     These images come from ViSP-images-x.y.z.tar.gz\n\
     available on the ViSP website.\n\
     Setting the VISP_INPUT_IMAGE_PATH environment\n\
     variable produces the same behaviour than using\n\
     this option.\n\
 \n\
  -p <personal image path>                             %s\n\
     Specify a personal sequence containing images \n\
     to process.\n\
     By image sequence, we mean one file per image.\n\
     The following image file formats PNM (PGM P5, PPM P6)\n\
     are supported. The format is selected by analysing \n\
     the filename extension.\n\
     Example : \"/Temp/ViSP-images/calibration/grid36-%%02d.pgm\"\n\
     %%02d is for the image numbering.\n\
 \n\
  -g <gray level precision>                             %f\n\
     Specify a gray level precision to detect dots.\n\
     A number between 0 and 1.\n\
     precision of the gray level of the dot. \n\
     It is a double precision float witch \n\
     value is in ]0,1]. 1 means full precision, \n\
     whereas values close to 0 show a very bad \n\
     precision.\n\
 \n\
  -f <first image>                                     %u\n\
     First image number of the sequence.\n\
 \n\
  -n <number of images>                                %u\n\
     Number of images used to compute calibration.\n\
 \n\
  -s <step>                                            %u\n\
     Step between two images.\n\
 \n\
  -l <lambda>                                          %f\n\
     Gain of the virtual visual servoing.\n\
 \n\
  -d                                             \n\
     Disable the image display. This can be usefull \n\
     for automatic tests using crontab under Unix or \n\
     using the task manager under Windows.\n\
 \n\
  -c\n\
     Disable the mouse click.\n\
     If the image display is disabled (using -d)\n\
     this option is without effect.\n\
\n\
  -h\n\
     Print the help.\n\n",
    ipath.c_str(),ppath.c_str(), gray ,first, nimages, step,lambda);

  if (badparam)
    fprintf(stdout, "\nERROR: Bad parameter [%s]\n", badparam);
}
/*!

  Set the program options.

  \param argc : Command line number of parameters.
  \param argv : Array of command line parameters.
  \param ipath : Input image path.
  \param ppath : Personal image path.
  \param gray : gray level precision.
  \param first : First image.
  \param nimages : Number of images to display.
  \param step : Step between two images.
  \param lambda : Gain of the virtual visual servoing.
  \param display : Set as true, activates the image display. This is
  the default configuration. When set to false, the display is
  disabled. This can be usefull for automatic tests using crontab
  under Unix or using the task manager under Windows.

  \param click : Set as false, disable the mouse click.

  \return false if the program has to be stopped, true otherwise.

*/
bool getOptions(int argc,const char **argv, std::string &ipath, std::string &ppath,
    double &gray, unsigned &first, unsigned &nimages, unsigned &step,
    double &lambda, bool &display, bool &click)
{
  const char *optarg;
  int c;
  while ((c = vpParseArgv::parse(argc, argv, GETOPTARGS, &optarg)) > 1) {

    switch (c) {
    case 'd': display = false; break;
    case 'i': ipath = optarg; break;
    case 'p': ppath = optarg; break;
    case 'g': gray = atof(optarg);break;
    case 'f': first = (unsigned) atoi(optarg); break;
    case 'n': nimages = (unsigned) atoi(optarg); break;
    case 's': step = (unsigned) atoi(optarg); break;
    case 'l': lambda = atof(optarg); break;
    case 'c': click = false; break;
    case 'h': usage(argv[0], NULL, ipath, ppath,gray, first, nimages, step, lambda);
      return false; break;

    default:
      usage(argv[0], optarg, ipath, ppath, gray,first, nimages, step, lambda);
      return false; break;
    }
  }

  if ((c == 1) || (c == -1)) {
    // standalone param or error
    usage(argv[0], NULL, ipath, ppath,gray, first, nimages, step, lambda);
    std::cerr << "ERROR: " << std::endl;
    std::cerr << "  Bad argument " << optarg << std::endl << std::endl;
    return false;
  }

  return true;
}

#if (defined(VISP_HAVE_X11) || defined(VISP_HAVE_GTK) || defined(VISP_HAVE_GDI) || defined(VISP_HAVE_D3D9))


int main(int argc, const char ** argv)
{
///////////////////////////////////////////
//---------PARAMETERS--------------------
  
  // set the calibration method
  vpCalibration::vpCalibrationMethodType calibMethod = vpCalibration::CALIB_VIRTUAL_VS_DIST ;
  // set the camera intrinsic parameters
  // see more details about the model in vpCameraParameters
  double px = 600 ;
  double py = 600 ;
  double u0 = 0;
  double v0 = 0;
  vpCameraParameters cam(px,py,u0,v0) ;
  //set tracking dots parameters
  double sizePrecision = 0.5 ;
  //Calibration grid parameters////
  double Lx = 0.03; //distance between points along x axis
  double Ly = 0.03; //distance between points along y axis
  unsigned int sizeX = 6; //size of the calibration grid along x axis 
  unsigned int sizeY = 6; //size of the calibration grid along y axis
  unsigned int nbpt = sizeX*sizeY; //number of points in the calibration grid
  //set the 3D coordinates of points used to compute the initial pose
  const unsigned int nptPose = 4; //number of init dots by image
  vpPoint P[nptPose];
  //plan xOy
     P[0].setWorldCoordinates(Lx,Ly, 0 ) ;
     P[1].setWorldCoordinates(Lx,4*Ly, 0 ) ;
     P[2].setWorldCoordinates(3*Lx,4*Ly, 0 ) ;
     P[3].setWorldCoordinates(4*Lx,Ly, 0 ) ;
     
  // Calibration grid data
     vpList<double> LoX,LoY,LoZ; //3D coordinates of the calibration dots
//   char gridname[FILENAME_MAX] = "./grid2d.dat";
//   if(vpCalibration::readGrid(gridname,nbpt,LoX,LoY,LoZ)!=0){
//     std::cout << "Can't read : " << gridname << std::endl;
//     std::cout << "Calibration grid to use : " ;
//     std::cin >> gridname ;
//     if(vpCalibration::readGrid(gridname,nbpt,LoX,LoY,LoZ)!=0){
//       vpCERROR << "Can't read " << gridname << std::endl;
//       exit(-1);
//     }
//   }
     LoX.front();
     LoY.front();
     LoZ.front();
     for (unsigned int i=0 ; i < sizeX ; i++){
       for(unsigned int j=0 ; j < sizeY ; j++){
         LoX.addRight(i*Lx) ;
         LoY.addRight(j*Ly) ;
         LoZ.addRight(0) ;
       }
     }
  
//---------------------------------------------------
///////////////////////////////////////////////////////
  std::string env_ipath;
  std::string opt_ipath;
  std::string ipath;
  std::string opt_ppath;
  std::string dirname;
  std::string filename;
  std::string filename_out;
  char comment[FILENAME_MAX];
  double opt_gray = 0.7;
  unsigned opt_first = 1;
  unsigned opt_nimages = 4;
  unsigned opt_step = 1;
  double opt_lambda = 0.5;
  bool opt_display = true;
  bool opt_click = true;
  bool save = false;
  double dotSize;
  // Get the VISP_IMAGE_PATH environment variable value
  char *ptenv = getenv("VISP_INPUT_IMAGE_PATH");
  if (ptenv != NULL)
    env_ipath = ptenv;

  // Set the default input path
  if (! env_ipath.empty())
    ipath = env_ipath;

  // Read the command line options
  if (getOptions(argc, argv, opt_ipath, opt_ppath,opt_gray,opt_first, opt_nimages,
     opt_step, opt_lambda, opt_display, opt_click) == false) {
    return (-1);
  }

  if (!opt_display)
    opt_click = false; // turn off the waiting

  // Get the option values
  if (!opt_ipath.empty())
    ipath = opt_ipath;

  // Compare ipath and env_ipath. If they differ, we take into account
  // the input path comming from the command line option
  if (opt_ipath.empty() && opt_ppath.empty()) {
    if (ipath != env_ipath) {
      std::cout << std::endl
     << "WARNING: " << std::endl;
      std::cout << "  Since -i <visp image path=" << ipath << "> "
     << "  is different from VISP_INPUT_IMAGE_PATH=" << env_ipath << std::endl
     << "  we skip the environment variable." << std::endl;
    }
  }

  // Test if an input path is set
  if (opt_ipath.empty() && env_ipath.empty() && opt_ppath.empty() ){
    usage(argv[0], NULL, ipath, opt_ppath, opt_gray, opt_first, opt_nimages,
          opt_step, opt_lambda);
    std::cerr << std::endl
   << "ERROR:" << std::endl;
    std::cerr << "  Use -i <visp image path> option or set VISP_INPUT_IMAGE_PATH "
   << std::endl
   << "  environment variable to specify the location of the " << std::endl
   << "  image path where test images are located." << std::endl
   << "  Use -p <personal image path> option if you want to "<<std::endl
   << "  use personal images." << std::endl
         << std::endl;

    return(-1);
  }
  
  // Declare an image, this is a gray level image (unsigned char)
  // it size is not defined yet, it will be defined when the image will
  // read on the disk
  vpImage<unsigned char> I ;

  unsigned iter = opt_first;
  std::ostringstream s;
  char cfilename[FILENAME_MAX];

  if (opt_ppath.empty()){
  
  
    // Warning :
    // the image sequence is not provided with the ViSP package
    // therefore the program will return you an error :
    //  !!    vpImageIoPnm.cpp: readPGM(#210) :couldn't read file
    //        ViSP-images/calibration/grid36-01.pgm
    //  !!    vpDotExample.cpp: main(#95) :Error while reading the image
    //  terminate called after throwing an instance of 'vpImageException'
    //
    //  The sequence is available on the visp www site
    //  http://www.irisa.fr/lagadic/visp/visp.html
    //  in the download section. It is named "ViSP-images.tar.gz"
  
    // Set the path location of the image sequence
    dirname = ipath + vpIoTools::path("/ViSP-images/calibration/");
  
    // Build the name of the image file
  
    s.setf(std::ios::right, std::ios::adjustfield);
    s << "grid36-" << std::setw(2) << std::setfill('0') << iter << ".pgm";
    filename = dirname + s.str();
  }
  else {

    sprintf(cfilename,opt_ppath.c_str(), iter) ;
    filename = cfilename;
  }
  // Read the PGM image named "filename" on the disk, and put the
  // bitmap into the image structure I.  I is initialized to the
  // correct size
  //
  // exception readPGM may throw various exception if, for example,
  // the file does not exist, or if the memory cannot be allocated
  try{
    vpImageIo::readPGM(I, filename) ;
  }
  catch(...)
  {
    // an exception is throwned if an exception from readPGM has been catched
    // here this will result in the end of the program
    // Note that another error message has been printed from readPGM
    // to give more information about the error
    std::cerr << std::endl
   << "ERROR:" << std::endl;
    std::cerr << "  Cannot read " << filename << std::endl;
    std::cerr << "  Check your -i " << ipath << " option, " << std::endl
   << "  or your -p " << opt_ppath << " option " <<std::endl
   << "  or VISP_INPUT_IMAGE_PATH environment variable"
   << std::endl;
    return(-1);
  }


  // We determine and store the calibration parameters for each image.
  vpCalibration* table_cal;
  table_cal = new vpCalibration[opt_nimages];
  unsigned int niter = 0;

  while (iter < opt_first + opt_nimages*opt_step) {
    try {
      // set the new image name

      if (opt_ppath.empty()){
        s.str("");
        s << "grid36-" << std::setw(2) << std::setfill('0') << iter<< ".pgm";
        filename = dirname + s.str();
      }
      else {
        sprintf(cfilename, opt_ppath.c_str(), iter) ;
        filename = cfilename;
      }
      filename_out = filename + ".txt";

      std::cout << "read : " << filename << std::endl;
      // read the image
      vpImageIo::readPGM(I, filename);
    
    u0 = I.getWidth()/2;
    v0 = I.getHeight()/2;

    cam.setPrincipalPoint(u0,v0);
      
#if defined VISP_HAVE_GDI
    vpDisplayGDI display; 
#elif defined VISP_HAVE_GTK
    vpDisplayGTK display;
#elif defined VISP_HAVE_X11
    vpDisplayX display;
#elif defined VISP_HAVE_D3D9
    vpDisplayD3D display;
#endif
      

    if (opt_display) {
      // Display the image

    try{
      // Display size is automatically defined by the image (I) size
      display.init(I, 100, 100,"Calibration initialization") ;
      // Display the image
      // The image class has a member that specify a pointer toward
      // the display that has been initialized in the display declaration
      // therefore is is no longuer necessary to make a reference to the
      // display variable.
      vpDisplay::display(I) ;
      vpDisplay::flush(I) ;
    }
    catch(...){
      vpERROR_TRACE("Error while displaying the image") ;
      delete [] table_cal;
      return(-1);
    }
    }


    // here we track dots on the calibration grid
    vpDot2 d[nptPose] ;
    unsigned int u_click[nptPose], v_click[nptPose];
    try{
      for(unsigned int i=0;i<nptPose;i++) {
        // by using setGraphics, we request to see the edges of the dot
        // in red on the screen.
        // It uses the overlay image plane.
        // The default of this setting is that it is time consumming
    
        d[i].setGraphics(true) ;
        d[i].setGrayLevelPrecision(opt_gray);
        d[i].setSizePrecision(sizePrecision);
            
        // tracking is initalized
        // if no other parameters are given to the iniTracking(..) method
        // a right mouse click on the dot is expected
        // dot location can also be specified explicitely in the initTracking
        // method  : d.initTracking(I,u,v)  where u is the column index and v is
        // the row index
        if (opt_click) {
          vpDisplay::display(I);
          std::printf("click in the dot %d of coordinates\nx=%f y=%f z=%f \n",
          i+1 ,P[i].get_oX(),P[i].get_oY(),P[i].get_oZ());
          std::sprintf(comment,"click in the dot %d",i+1 );
          vpDisplay::displayCharString(I,10,10,&comment[0],vpColor::blue);
          for(unsigned int j = 0;j<i;j++)
            d[j].display(I) ;
        // flush the display buffer
          vpDisplay::flush(I);
          try{
		        d[i].initTracking(I) ;
          }
          catch(...){
          }
        }
        else{
          d[i].initTracking(I,u_click[i],v_click[i]);
        }  
        // an expcetion is thrown by the track method if
        //  - dot is lost
        //  - the number of pixel is too small
        //  - too many pixels are detected (this is usual when a "big" specularity
        //    occurs. The threshold can be modified using the
        //    setNbMaxPoint(int) method
        if (opt_display) {
          d[i].display(I) ;
        // flush the display buffer        
        vpDisplay::flush(I) ;
        }
      }
    }
    catch(vpException e){
      vpERROR_TRACE("Error while tracking dots") ;
      vpCTRACE << e;
      delete [] table_cal;
      return(-1);
    }

    // --------------------------------------------------------
    // Now will compute the pose
    //
    // The pose will be contained in an homogeneous matrix cMo
    vpHomogeneousMatrix cMo ;

    // We need a structure that content both the 3D coordinates of the point
    // in the object frame and the 2D coordinates of the point expressed in meter
    // the vpPoint class is ok for that

    //The vpCalibration class mainly contents a list of points (X,Y,Z,u,v)
    vpCalibration calib; 
    calib.clearPoint(); 

    // The vpPose class mainly contents a list of vpPoint (that is (X,Y,Z, x, y) )
    vpPose pose ;
    //  the list of point is cleared (if that's not done before)
    pose.clearPoint() ;
    // we set the 3D points coordinates (in meter !) in the object/world frame
    


    // pixel-> meter conversion
    for (unsigned int i=0 ; i < nptPose ; i++){
      // conversion in meter is achieved using
      // x = (u-u0)/px
      // y = (v-v0)/py
      // where px, py, u0, v0 are the intrinsic camera parameters
      double x=0.,y=0. ;
      vpPixelMeterConversion::convertPoint(cam,
					   d[i].get_u(), d[i].get_v(),
					   x,y)  ;
      P[i].set_x(x) ;
      P[i].set_y(y) ;
    }

    // The pose structure is build, we put in the point list the set of point
    // here both 2D and 3D world coordinates are known
    for (unsigned int i=0 ; i < nptPose ; i++){
      pose.addPoint(P[i]) ; // and added to the pose computation point list

      //and added to the local calibration points list
      calib.addPoint(P[i].get_oX(),P[i].get_oY(),P[i].get_oZ(),
                      d[i].get_u(),d[i].get_v());
    
    }
    // compute the initial pose using Lagrange method followed by a non linear
    // minimisation method

    // Pose by Lagrange it provides an initialization of the pose
    pose.computePose(vpPose::LAGRANGE, cMo) ;
  
    // the pose is now refined using the virtual visual servoing approach
    // Warning: cMo needs to be initialized otherwise it may diverge
    pose.computePose(vpPose::VIRTUAL_VS, cMo) ;
    //pose.display(I,cMo,cam, 0.05, vpColor::blue) ;
    vpHomogeneousMatrix cMoTmp = cMo;
    vpCameraParameters camTmp = cam;
    //compute local calibration to match the calibration grid with the image
    try{
    calib.computeCalibration(vpCalibration::CALIB_VIRTUAL_VS,cMoTmp,camTmp,false);
    }
    catch(...){
      if(opt_click){
        vpDisplay::display(I);    
        vpDisplay::displayCharString(I,10,10,"Pose computation failed",
                                     vpColor::red);
        vpDisplay::displayCharString(I,22,10,
                                     "A left click to define other dots.",
                                     vpColor::blue);
        vpDisplay::displayCharString(I,34,10,
                                     "A middle click to don't care of this pose.",
                                     vpColor::blue);
        vpDisplay::flush(I) ;
        std::cout << "\nPose computation failed." << std::endl;
        std::cout << "A left click to define other dots." << std::endl;
        std::cout << "A middle click to don't care of this pose." << std::endl;
        vpMouseButton::vpMouseButtonType button;
        unsigned int r,c;
        vpDisplay::getClick(I,r,c,button) ;
        switch(button){
          case 1 :
            std::cout << "Left click has been pressed." << std::endl;
            continue;      
          case 3 :
            std::cout << "Right click has been pressed." << std::endl;
            continue;
          case 2 :
            std::cout << "Middle click has been pressed." << std::endl;
            iter += opt_step ;
            niter++;
            continue;
        }
      }
      else{
        iter += opt_step ;
        niter++;
        continue;
      }
    }
    if (opt_display) {
      // display the computed pose
      vpDisplay::display(I) ;
      for(unsigned int j = 0;j<nptPose;j++)
        d[j].display(I) ;
      pose.display(I,cMoTmp,camTmp, 0.05, vpColor::red) ;
      vpDisplay::flush(I) ;
      if(opt_click){
        vpDisplay::displayCharString(I,10,10,
                                     "A left click to display grid.",
                                     vpColor::blue);
        vpDisplay::displayCharString(I,22,10,
                                     "A right click to define other dots.",
                                     vpColor::blue);
        vpDisplay::flush(I) ;
        std::cout << "\nA a left click to display grid." << std::endl;
        std::cout << "A right click to define other dots." << std::endl;
        vpMouseButton::vpMouseButtonType button;
        unsigned int r,c;
        vpDisplay::getClick(I,r,c,button) ;
        switch(button){
        case 1 :
          std::cout << "Left click has been pressed." << std::endl;
          break;
        case 2 :
          std::cout << "Middle click has been pressed." << std::endl;
          continue;
        case 3 :
          std::cout << "Right click has been pressed." << std::endl;
          continue;
        }
      }
      vpDisplay::display(I) ;
      vpDisplay::flush(I) ;
    }
    dotSize = 0;
    for(unsigned i =0 ; i<nptPose ;i++){
      dotSize += d[i].getWidth()+d[i].getHeight();
    }
    dotSize /= nptPose;  
        
    //now we detect all dots of the grid
    vpDot2* md = new vpDot2[nbpt];
    for(unsigned int i=0;i<nbpt;i++){

      // by using setGraphics, we request to see the contour of the dot
      // in red on the screen.
      md[i].setGraphics(false);
      md[i].setSizePrecision(sizePrecision);
      md[i].setGrayLevelPrecision(opt_gray);
    }
    
    vpDisplay::display(I) ;

    // --------------------------------------------------------
    // Now we will compute the calibration
    //

    // We need a structure that content both the 3D coordinates of the point
    // in the object frame and the 2D coordinates of the point expressed in meter
    // the vpPoint class is ok for that
    vpPoint* mP=new vpPoint[nbpt]  ;

    // The vpPose class mainly contents a list of vpPoint (that is (X,Y,Z, x, y) )
    //  the list of point is cleared (if that's not done before)
    table_cal[niter].clearPoint() ;

    // we set the 3D points coordinates (in meter !) in the object/world frame
    //xOy plan
    LoX.front();
    LoY.front();
    LoZ.front();
    for(unsigned int i = 0 ; i < nbpt ; i++){
        mP[i].setWorldCoordinates(LoX.value(),LoY.value(),LoZ.value()) ; // (X,Y,Z)
        LoX.next();
        LoY.next();
        LoZ.next();
    }
    // pixel-> meter conversion
    bool* valid = new bool[nbpt];
    for (unsigned int i=0 ; i < nbpt ; i++){
      vpColVector _cP, _p ;
      double u=0.,v=0. ;
      valid[i] = true;
      mP[i].changeFrame(cMoTmp,_cP) ;
      mP[i].projection(_cP,_p) ;
      vpMeterPixelConversion::convertPoint(camTmp,_p[0],_p[1], u, v);
      if(10<u && u<I.getWidth()-10 && 10<v && v<I.getHeight()-10){
        try {
          md[i].initTracking(I,(unsigned int)u,(unsigned int)v,(unsigned int)dotSize);
          vpRect bbox = md[i].getBBox();
          if(bbox.getLeft()<5 || bbox.getRight()>(double)I.getWidth()-5 ||
              bbox.getTop()<5 || bbox.getBottom()>(double)I.getHeight()-5||
              vpMath::abs(u-md[i].get_u())>10 ||vpMath::abs(v-md[i].get_v())>10)
            valid[i] = false;
          // u[i]. v[i] are expressed in pixel
          // conversion in meter
          double x=0.,y=0. ;
          vpPixelMeterConversion::convertPoint(camTmp,
                md[i].get_u(), md[i].get_v(),
                x,y)  ;
          mP[i].set_x(x) ;
          mP[i].set_y(y) ;
          if (opt_display) {
            if(valid[i]){
             md[i].display(I,vpColor::red);
             mP[i].display(I,cMoTmp,camTmp) ;
            }
          }
        }
        catch(...){
          valid[i] = false;
        }
      }
      else {valid[i] = false;}
    }

    // The calibration structure is build, we put in the point list the set of point
    // here both 2D and 3D world coordinates are known
    // and added to the calibration computation point list.


    //we put the pose matrix in the current calibration structure
//     table_cal[niter].cMo = cMo ; //.setIdentity();//
    if(save == true)
      table_cal[niter].writeData(filename_out.c_str());
    if (opt_click) {
        
        vpDisplay::displayCharString(I,10,10,"A left click to validate this pose.",
                                     vpColor::blue);
        vpDisplay::displayCharString(I,22,10,
                                     "A right click to retry.",
                                     vpColor::blue);
        vpDisplay::displayCharString(I,34,10,
                                     "A middle click to don't care of this pose.",
                                     vpColor::blue);
        vpDisplay::flush(I) ;
   
        std::cout << "\nA left click to validate this pose." << std::endl;
        std::cout << "A right click to retry." << std::endl;
        std::cout << "A middle click to don't care of this pose." << std::endl;
        vpMouseButton::vpMouseButtonType button;
        unsigned int r,c;
        vpDisplay::getClick(I,r,c,button) ;
        switch(button){
        case 1 : //left
          std::cout << "\nLeft click has been pressed." << std::endl;
        break;
        case 2 : //middle
          std::cout << "Middle click has been pressed." << std::endl;
          for (unsigned int i=0 ; i < nbpt ; i++)
            valid[i]=false;
        break;
        case 3 : //right
          std::cout << "Right click has been pressed." << std::endl;
        continue;
        }
    }
    //Add valid points in the calibration structure
    for (unsigned int i=0 ; i < nbpt ; i++){
      if(valid[i]){

        table_cal[niter].addPoint(mP[i].get_oX(),mP[i].get_oY(),mP[i].get_oZ(),
            md[i].get_u(),md[i].get_v()) ;
      }
    }
      
    //we free the memory
    delete [] mP;
    delete [] md;
    delete [] valid;

    niter++ ;
    }
    catch(...) {
      return(-1) ;
    }
    iter += opt_step ;
  }
  vpCalibration::setLambda(opt_lambda);
  // Calibrate by a non linear method based on virtual visual servoing
  vpCalibration::computeCalibrationMulti(calibMethod,opt_nimages,table_cal,cam,true) ;

   //CALIB_VIRTUAL_VS 1
   //CALIB_VIRTUAL_VS_DIST  2

  // Compute Tsai calibration for extrinsic parameters estimation

  iter = opt_first;
  niter = 0;
  //Print calibration results for each image
  while (iter < opt_first + opt_nimages*opt_step) {
    try {
      // set the new image name

      if (opt_ppath.empty()){
        s.str("");
        s << "grid36-" << std::setw(2) << std::setfill('0') << iter<< ".pgm";
        filename = dirname + s.str();
      }
      else {
        sprintf(cfilename, opt_ppath.c_str(), iter) ;
        filename = cfilename;
      }

      std::cout << "read : " << filename << std::endl;
      // read the image
      vpImageIo::readPGM(I, filename);
      if(table_cal[niter].get_npt()!=0){
        std::cout << "\nCompute standard deviation for pose " << niter <<std::endl;
        double deviation, deviation_dist ;
        table_cal[niter].computeStdDeviation(deviation,deviation_dist);
        std::cout << "deviation for model without distortion : "
                  << deviation << std::endl;
        std::cout << "deviation for model with distortion : "
                  << deviation_dist << std::endl;
        //Display results
      }
      else{
        std::cout << "This image has not been used!" << std::endl;
      }       
#if defined VISP_HAVE_GDI
      vpDisplayGDI display;
#elif defined VISP_HAVE_GTK
      vpDisplayGTK display;
#elif defined VISP_HAVE_X11  
      vpDisplayX display;

#elif defined VISP_HAVE_D3D9
      vpDisplayD3D display;
#endif

      if (opt_display) {
        // Display the image

        try{
          // Display size is automatically defined by the image (I) size
          display.init(I, 100, 100,"Calibration results") ;
          // Display the image
          // The image class has a member that specify a pointer toward
          // the display that has been initialized in the display declaration
          // therefore is is no longuer necessary to make a reference to the
          // display variable.
          vpDisplay::display(I) ;
        }
        catch(...){
          vpERROR_TRACE("Error while displaying the image") ;
          delete [] table_cal;
          return(-1);
        }
	    //Display the data of the calibration (center of the dots)
        table_cal[niter].displayData(I) ;
	    //Display grid : estimated center of dots using camera parameters
        table_cal[niter].displayGrid(I) ;
        vpDisplay::flush(I) ;
        if(opt_click){
          vpDisplay::displayCharString(I,10,10,"A click to continue...",
                                       vpColor::blue);
          vpDisplay::flush(I) ;
          std::cout << "\nA click to continue..." << std::endl;
          vpDisplay::getClick(I) ;
        }
      }
      niter++;
    }
    catch(...) {
      delete [] table_cal;
      return(-1) ;
    }
    iter += opt_step ;
  }


  delete [] table_cal;
  return(0);
}



#else // (defined (VISP_HAVE_GTK) || defined(VISP_HAVE_GDI)...)

int
main()
{
  vpTRACE("X11 or GTK or GDI or D3D functionnality is not available...") ;
}
#endif // (defined (VISP_HAVE_GTK) || defined(VISP_HAVE_GDI)...)

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */
