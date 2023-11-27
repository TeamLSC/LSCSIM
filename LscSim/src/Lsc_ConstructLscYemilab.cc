#include "LscSim/LscDetectorConstruction.hh" // the DetectorConstruction class header
#include "LscSim/LscScintSD.hh" 

#include "CupSim/CupInputDataReader.hh"
#include "CupSim/CupParam.hh"

#include "G4Box.hh"
#include "G4Element.hh"
#include "G4IntersectionSolid.hh"
#include "G4Material.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4Sphere.hh"
#include "G4SubtractionSolid.hh"
#include "G4Tubs.hh"
#include "G4UnionSolid.hh"

#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4VisAttributes.hh"
#include "G4Tubs.hh"
#include "G4SDManager.hh"

using namespace CLHEP;

void LscDetectorConstruction::ConstructLscYemilab() {
    G4cout << "========================================" << G4endl;
    G4cout << "Construct Your Detector....." << G4endl;
    G4cout << "========================================" << G4endl;
    G4cout << G4endl;

    // -- database
    CupParam &db(CupParam::GetDB());
    // add spherical-geometry-specific parameters to parameter list
    if (getenv("LscDATA") != nullptr) {
        db.ReadFile((G4String(getenv("LscDATA")) + "/settings_lsc.dat").c_str());
    } else if (getenv("SOFTWARE_DIR") != nullptr) {
        db.ReadFile((G4String(getenv("SOFTWARE_DIR")) + "/LscSim/data/settings_lsc.dat").c_str());
    } else {
        db.ReadFile(G4String("./data/settings_lsc.dat").c_str());
    }

    // -- retrieve some parameters from database we'll need right away
    G4double tunnel_arch_radius   = db["tunnel_arch_radius"];
    G4double pit_depth            = db["pit_depth"];
    G4double pit_radius           = db["pit_radius"];
    G4double rock_shell_thickness = db["rock_shell_thickness"];
    G4cout << "rock shell thick= " << rock_shell_thickness << G4endl;

    G4double inner_veto_tank_height= db["inner_veto_tank_height"];
    G4double inner_veto_tank_radius= db["inner_veto_tank_radius"];
    G4double inner_veto_tank_thickness= db["inner_veto_tank_thickness"];

    G4double buffer_tank_height= db["buffer_tank_height"];
    G4double buffer_tank_radius= db["buffer_tank_radius"];
    G4double buffer_tank_thickness= db["buffer_tank_thickness"];

    // -- create world volume
    // calculate size of Rock shell logical and physical volumes
    //  G4double rock_half_width= tunnel_arch_radius;
    //  G4double rock_half_height= pit_depth/2.0 + tunnel_arch_radius;
    //  rock_half_height += rock_shell_thickness;
    G4double rock_half_width  = rock_shell_thickness;
    G4double rock_half_height = rock_shell_thickness;

    G4Box *solidRock          = new G4Box("Rock_solid",
                                 rock_half_width, // dx
                                 rock_half_width, // dy
                                 rock_half_height // dz
    );
    G4LogicalVolume *logiRock = new G4LogicalVolume(solidRock, _rock, "logiRock", 0, 0, 0);
    // logiRock-> SetVisAttributes (G4VisAttributes::Invisible);
    G4VisAttributes *visRock = new G4VisAttributes(G4Colour(0.0, 0.4, 0.4, 0.2));
    logiRock->SetVisAttributes(visRock);

    G4VPhysicalVolume *physRock =
        new G4PVPlacement(NULL, // rotation not allowed
                          G4ThreeVector(0, 0, 0), logiRock, "physRock", NULL, false, 0);

    // must set class variable "world_phys"
    world_phys = physRock;

    // --- "excavate" tunnel and pit
    G4Tubs *tunnel_solid= new G4Tubs("tunnel_solid",  // name
                                     0.0, // inner radius
                                     tunnel_arch_radius, // outer radius
                                     rock_half_width, // half-height
                                     0.*deg,
                                     180.*deg   // start and span angle of tube
                                     );
    G4Tubs *pit_solid= new G4Tubs("pit_solid",  // name
                                  0.0, // inner radius
                                  pit_radius, // outer radius
                                  pit_depth/2.0, // half-height
                                  0.*deg,
                                  360.*deg   // start and span angle of tube
                                  );
    G4RotationMatrix* tunnel_rotation= new G4RotationMatrix();
    tunnel_rotation->rotateX( -90.0*deg );
    G4BooleanSolid *cavern_solid =
      new G4UnionSolid( "cavern_solid",
                        pit_solid,
                        tunnel_solid,
                        tunnel_rotation,
                        G4ThreeVector(0,0,pit_depth/2.0) );
    G4LogicalVolume *logiCavern= new G4LogicalVolume( cavern_solid,
                                                     _air,
                                                     "logiCavern",
                                                     0,0,0);
    G4VisAttributes* visCavern = new G4VisAttributes(G4Colour(0.0,0.4,0.4,0.2));
    logiCavern -> SetVisAttributes(visCavern);

    G4VPhysicalVolume* physCavern = 
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logiCavern, "physCavern", logiRock, false, 0);


    ///////////////////////////////////////////////////////
    // -- make nested cylindrical tanks and fill volumes
    //Outer veto vessel
    G4Tubs* OuterVetoTankSolid=
      new G4Tubs("OuterVetoTankSolid",  // name
                 0.0, // inner radius
                 inner_veto_tank_radius+inner_veto_tank_thickness,
                 inner_veto_tank_height/2+inner_veto_tank_thickness, // outer radius
                 0.*deg, 360.*deg   // start and span angle of tube
                 );
    G4Tubs* OuterVetoInteriorSolid=
      new G4Tubs("OuterVetoInteriorSolid",  // name
                 0.0, // inner radius
                 inner_veto_tank_radius,
                 inner_veto_tank_height/2,
                 0.*deg, 360.*deg   // start and span angle of tube
                 );
    G4LogicalVolume *OuterVetoTankLogi= new G4LogicalVolume( OuterVetoTankSolid,
                                                    _stainless,
                                                    "OuterVetoTankLogical",
                                                    0,0,0);
    G4VisAttributes* stainlessVis= new G4VisAttributes(G4Colour(0.6,0.6,0.7,0.4));
    OuterVetoTankLogi -> SetVisAttributes(stainlessVis);
    G4VPhysicalVolume* OuterVetoTankPhys = new G4PVPlacement(0, // no rotation
                                                    G4ThreeVector(0,0,0),
                                                    "OuterVetoTankPhys",
                                                    OuterVetoTankLogi,
                                                    physCavern,
                                                    false,
                                                    0);
    G4LogicalVolume *OuterVetoInteriorLogi= new G4LogicalVolume(OuterVetoInteriorSolid,
                                                        _water,
                                                        "OuterVetoInteriorLogical",
                                                        0,0,0 );
    G4VisAttributes* waterVis=  new G4VisAttributes(G4Colour(0.0,0.0,0.7,0.3));
    OuterVetoInteriorLogi->SetVisAttributes(waterVis);
    G4VPhysicalVolume* OuterVetoInteriorPhys = new G4PVPlacement(0, // no rotation
                                                        G4ThreeVector(0,0,0),
                                                        "OuterVetoInteriorPhys",
                                                        OuterVetoInteriorLogi,
                                                        OuterVetoTankPhys,
                                                        false,
                                                        0);

    /////////////////
    //Buffer vessel
    G4Tubs* BufferTankSolid=
      new G4Tubs("BufferTankSolid",  // name
                 0.0, // inner radius
                 buffer_tank_radius+buffer_tank_thickness,
                 buffer_tank_height/2+buffer_tank_thickness, // outer radius
                 0.*deg, 360.*deg   // start and span angle of tube
                 );
    G4Tubs* BufferInteriorSolid=
      new G4Tubs("BufferInteriorSolid",  // name
                 0.0, // inner radius
                 buffer_tank_radius,
                 buffer_tank_height/2,
                 0.*deg, 360.*deg   // start and span angle of tube
                 );
    G4LogicalVolume *BufferTankLogi= new G4LogicalVolume(BufferTankSolid,
                                                    _water, //_stainless,
                                                    "BufferTankLogical",
                                                    0,0,0 );
    //G4VisAttributes* stainlessVis= new G4VisAttributes(G4Colour(0.6,0.6,0.7,0.4));
    BufferTankLogi -> SetVisAttributes(stainlessVis);
    G4VPhysicalVolume* BufferTankPhys = new G4PVPlacement(0, // no rotation
                                                    G4ThreeVector(0,0,0),
                                                    "BufferTankPhys",
                                                    BufferTankLogi,
                                                    OuterVetoInteriorPhys,
                                                    false,
                                                    0);
    G4LogicalVolume *BufferInteriorLogi= new G4LogicalVolume(BufferInteriorSolid,
                                                        _water, //_mineralOil,
                                                        "BufferInteriorLogical",
                                                        0,0,0 );
    G4VisAttributes* mineralOilVis= new G4VisAttributes(G4Colour(0.7,0.4,0.5, 0.1));
    BufferInteriorLogi->SetVisAttributes(mineralOilVis);
    G4VPhysicalVolume* BufferInteriorPhys = new G4PVPlacement(0, // no rotation
                                                        G4ThreeVector(0,0,0),
                                                        "BufferInteriorPhys",
                                                        BufferInteriorLogi,
                                                        BufferTankPhys,
                                                        false,
                                                        0);

    ///////////
    // Target
    G4double TargetRadius = 14000. * mm / 2.;
    G4double TargetHeight = 15000. * mm;

    G4VSolid *solidTarget = new G4Tubs("solidTarget", 0, TargetRadius, TargetHeight / 2, 0, twopi);

    G4LogicalVolume *logicTarget = new G4LogicalVolume(solidTarget, LS_LAB, "logicTarget");

    G4VPhysicalVolume *physTarget = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), "physTarget", logicTarget, BufferInteriorPhys, false, 0);

    G4VisAttributes *TargetVisAtt       = new G4VisAttributes(G4Colour::Blue());
    TargetVisAtt->SetVisibility(true);
    //TargetVisAtt->SetForceWireframe(true); // SetForceSolid(true);
    logicTarget->SetVisAttributes(TargetVisAtt);

    G4cout << "Setting Region..." << G4endl;
    G4Region *crystalsRegion = 0;
    if (crystalsRegion) delete crystalsRegion;
    crystalsRegion = new G4Region("crystals");
    crystalsRegion->AddRootLogicalVolume(logicTarget);

    ////////////////////
    // Logical surface 
    new G4LogicalBorderSurface("OuterVetoTank_logsurf1",
             OuterVetoInteriorPhys, OuterVetoTankPhys,
             Tyvek_opsurf);

    new G4LogicalBorderSurface("BufferTank_logsurf1",
             BufferInteriorPhys, BufferTankPhys,
             Tyvek_opsurf);


    ///////////////////////
    // Sensitive detector
    G4cout << "Setting Sensitive detectors for detectors..." << G4endl;
    G4cout << G4endl;

    G4String SDname;
    LscScintSD *TGSD  = new LscScintSD(SDname = "/lsc/TGSD", 1);
    G4SDManager *SDman = G4SDManager::GetSDMpointer();

    SDman->AddNewDetector(TGSD);
    logicTarget->SetSensitiveDetector(TGSD);
    G4cout << "Geometry setting is done..." << G4endl;
    G4cout << G4endl;


    // --- put in the Inner Detector PMTs, and details
    ConstructLscYemilab_ID();

}
