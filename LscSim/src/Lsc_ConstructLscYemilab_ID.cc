#include "CupSim/CupPMTSD.hh" // for "sensitive detector"
#include "CupSim/CupParam.hh"
#include "CupSim/CupScintSD.hh"
#include "CupSim/Cup_PMT_LogicalVolume.hh"     // for making PMT assemblies
#include "LscSim/LscDetectorConstruction.hh" // the DetectorConstruction class header

#include "G4Material.hh"
#include "G4Element.hh"
#include "G4OpticalSurface.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Trap.hh"
#include "G4Torus.hh"
#include "G4RotationMatrix.hh"
#include "G4UnionSolid.hh"

#include "G4LogicalVolume.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4VPhysicalVolume.hh"

#include "globals.hh"
#include "Randomize.hh"                     // for G4UniformRand()

#include <fstream>
#include <sstream>

using namespace CLHEP;
using namespace std;

////////////////////////////////////////////////////////////////
// declaration of "private" static utility functions that we
// don't need in class definition
static void MakeID_PMT_Support(Cup_PMT_LogicalVolume *that,
			       G4Material *SupportMat,
			       G4Material *ExteriorMat
			       );


////////////////////////////////////////////////////////////////
void LscDetectorConstruction::ConstructLscYemilab_ID()
{
  // -- database
  CupParam &db ( CupParam::GetDB() );

  // -- retrieve various geometry parameters
  G4double buffer_tank_height= db["buffer_tank_height"];
  G4double buffer_tank_radius= db["buffer_tank_radius"];
  G4double buffer_tank_thickness= db["buffer_tank_thickness"];

  // -- get pointer to physCavern, so we can put stuff in the pit
  G4VPhysicalVolume* BufferInteriorPhys
    = GetPhysicalVolumeByName("BufferInteriorPhys");
  if (BufferInteriorPhys == NULL) {
    G4Exception(" ", " ", JustWarning, "Could not find BufferInteriorPhys!  Cannot build ID.");
  }
  
  // -- open pmt coordinates file, using LscDATA environment variable if set
  //G4std::ifstream whereInnerPMT;
  ifstream whereInnerPMT;
  const char *basic_fn= "pmtcoordinates_ID.dat"; 
  if ( getenv("LscDATA") != NULL )
    whereInnerPMT.open( (G4String(getenv("LscDATA"))
			 +"/"+G4String(basic_fn)).c_str() );
  // print error message on failure of file open
  if (whereInnerPMT.fail()) {
    G4cerr << "Error, " << basic_fn << " could not be opened.\n";
    if ( getenv("LscDATA") == NULL)
      G4cerr << "LscDATA environment variable is not set, so I was looking"
	" for " << basic_fn << " in the current directory." << G4endl;
    else
      G4cerr << "I was looking for it in the LscDATA directory, "
	     << getenv("LscDATA") << G4endl;
    G4Exception(" ", " ", JustWarning, "Error, pmt coordinates file could not be opened.\n");
  }
  // read max number of pmts
  int maxIDPMTNo;
  whereInnerPMT >> maxIDPMTNo;
  if (whereInnerPMT.fail()) {
    G4cerr << "Error, initial integer could not be read from pmt coordinates file.\n";
    G4Exception(" ", " ", JustWarning, "Error, initial integer could not be read from pmt coordinates file.\n");
  }
  
  // --- PMT sensitive detector
  G4SDManager* fSDman = G4SDManager::GetSDMpointer();  
  CupPMTSD* pmtSDInner= new CupPMTSD("/cupdet/pmt/inner", maxIDPMTNo, 0, 10);
  fSDman->AddNewDetector(pmtSDInner);


  // --- make the fundamental inner  PMT assembly
  Cup_PMT_LogicalVolume* _logiInnerPMT10
    = new Cup_10inch_LogicalVolume
    ( "InnerPMT",
      //_mineralOil,
      _water,
      _glass,
      Photocathode_opsurf,
      PMT_Vac,
      _stainless,  // dynode material
      ( db["omit_pmt_masks"] != 0.0 ?
	NULL :       // no mask
	_blackAcryl // physical mask on tubes to block non-sensitive areas
	),
      pmtSDInner,  // sensitive detector hook
      whichPmtStyle);
  MakeID_PMT_Support(_logiInnerPMT10,
		     _water,     // support material
		     _water);   // external material
		     //_mineralOil);   // external material
  

  // --- make the inner PMTs 
  G4RotationMatrix* _rotInnerPMT; 

  G4int region_a,region_b,region_c;
  G4double cood_x, cood_y, cood_z, pmt_size;
  G4int InnerPMTno;
  G4double angle_x,angle_z;
  char PMTname[64];  
  G4ThreeVector new_x,new_y,new_z;

  G4double PMT_rotation_factor_for_cylindrical_ID=
    db.GetWithDefault("PMT_rotation_factor_for_cylindrical_ID", 0.0);

  G4double angle_tank_corner = atan2(buffer_tank_height/2,buffer_tank_radius);
  
  // --- loop reading coordinates and making PMTs
  while ( whereInnerPMT.good() ) {
    // get a line from the file
    char linebuffer[128];
    whereInnerPMT.getline( linebuffer, sizeof(linebuffer)-1 );
    if ( whereInnerPMT.fail() )
      break;
    
    // skip blank lines and lines beginning with '#'
    if (linebuffer[0] == '#' || linebuffer[0] == '\0')
      continue;
    
    // put the line in an istrstream for convenient parsing
    //G4std::istrstream lineStream(linebuffer);
    istringstream lineStream(linebuffer);
    
    // parse out region, coordinates,
    region_a= region_b= region_c= -1;
    cood_x= cood_y= cood_z= 0.0;
    pmt_size= -911.;
    lineStream >> region_a >> region_b >> region_c
	       >> cood_x >> cood_y >> cood_z >> pmt_size;
    
    // check for bad data 
    if (lineStream.fail() || region_a < 0 || region_b < 0 || region_c < 0
	|| (cood_x == 0. && cood_y == 0. && cood_z == 0.)
	|| pmt_size == -911. ) {
      G4cerr << "BAD DATA in PMT file:  line=\"" << linebuffer << "\"\n";
      G4cerr.flush();
      continue;
    }
    
    // skip if pmt_size == 0
    if (pmt_size <= 0.0)
      continue;
    
    // a possible DAQ numbering 
    InnerPMTno = region_c;
    
    // name this PMT
    sprintf(PMTname,"physInnerPMT%d",InnerPMTno);
  

    // calculate angles and positions
    // we want PMTS to point normal to the surface they're mounted on
    // if rotation_factor==0.0, we want them to point to the center if
    // rotation_factor==1.0, and we want something in between for in-between
    // values.
    G4double r  = sqrt(cood_x*cood_x +cood_y*cood_y + cood_z*cood_z);
    G4double dx = -cood_x/r;
    G4double dy = -cood_y/r;
    G4double dz = -cood_z/r;
    
    angle_z = atan2(dx,dy);
    angle_x = atan2(dz,sqrt(dx*dx+dy*dy));

    PMT_rotation_factor_for_cylindrical_ID = 0.0; // EJ: to point normal to the surface they're mounted on

    if ( fabs(angle_x) < angle_tank_corner) {
      // side PMTs
      angle_x *= PMT_rotation_factor_for_cylindrical_ID;
    }
    else {
      // top or bottom PMTs
      double normal_angle= (cood_z > 0.0 ? -M_PI/2 : M_PI/2);
      angle_x = normal_angle + (angle_x-normal_angle)*PMT_rotation_factor_for_cylindrical_ID;
    }
    
    double normal_angle= (cood_z > 0.0 ? -M_PI/2 : M_PI/2);

    _rotInnerPMT = new G4RotationMatrix();
    _rotInnerPMT -> rotateZ(angle_z);
    _rotInnerPMT -> rotateX(M_PI/2.0-angle_x);

    cood_x = cood_x;
    cood_y = cood_y;
    cood_z = cood_z;

    G4ThreeVector pmtpos( cood_x, cood_y, cood_z );


    // creating the new PVPlacement automatically adds it to the
    //  singleton G4PhysicalVolumeStore()
    // ****************************************************************
    // * Use the constructor that specifies the PHYSICAL mother, since
    // * each PMT occurs only once in one physical volume.  This saves
    // * the GeometryManager some work. -GHS.
    // ****************************************************************
    if ( db["omit_id_pmts"] == 0.0 ) {
      new G4PVPlacement(_rotInnerPMT,
			pmtpos,
			PMTname,
			_logiInnerPMT10,
			BufferInteriorPhys,    // physical parent
			false,
			InnerPMTno);
    }
  }
  whereInnerPMT.close();
  // (all done setting inner PMTs from file)

}

////////////////////////////////////////////////////////////////
// definition of "private" static utility functions that we
// don't need in class definition
static void MakeID_PMT_Support(Cup_PMT_LogicalVolume *that,
			       G4Material *SupportMat,
			       G4Material *ExteriorMat)
{
  // ... this should build the PMT support geometry
  // ... empty function for now
}
