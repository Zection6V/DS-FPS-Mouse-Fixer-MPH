#include "InputMapping.h"

#include "AppState.h"
#include "InputUtils.h"
#include "MacroActions.h"

#define X g_State.center.x
#define Y g_State.center.y
#define DX g_State.distanceFromBoarderSides
#define DY g_State.distanceFromBoarderTop
#define R g_Config.rightBound
#define B g_Config.bottomBound

const std::array<InputMappingDef, NUM_MAPPINGS> g_Mappings = {{
    {"Shoot",                MacroActions::DoNothing, 0x01, MacroType::Single},
    {"Mouse Manual Reset",   InputUtils::ResetPos,    0x04, MacroType::Single},
    {"Swap To Main Weapon",  [](){ MacroActions::Tap(X - DX*2, g_Config.topBound + DY/2); }, 0x06, MacroType::Single},
    {"Swap To Missiles",     [](){ MacroActions::Tap(X, g_Config.topBound + DY/2); }, 0x05, MacroType::Single},
    {"Swap To Third Weapon", [](){ MacroActions::Tap(X + DX*2, g_Config.topBound + DY/2); }, 0x52, MacroType::Single},
    {"Zoom",                 MacroActions::DoNothing, 0x02, MacroType::Single},
    {"Special Weapon 1",     [](){ MacroActions::Drag(R - DX*2, g_Config.topBound + DY, X - DX*1.5, g_Config.topBound + DY); }, 0x31, MacroType::Single},
    {"Special Weapon 2",     [](){ MacroActions::Drag(R - DX*2, g_Config.topBound + DY, X - DX*1.5, Y - DY/4); }, 0x32, MacroType::Single},
    {"Special Weapon 3",     [](){ MacroActions::Drag(R - DX*2, g_Config.topBound + DY, X - DX, Y + DY/1.5); }, 0x33, MacroType::Single},
    {"Special Weapon 4",     [](){ MacroActions::Drag(R - DX*2, g_Config.topBound + DY, X + DX/2, B - DY/1.2); }, 0x34, MacroType::Single},
    {"Special Weapon 5",     [](){ MacroActions::Drag(R - DX*2, g_Config.topBound + DY, X + DX*2, B - DY/1.5); }, 0x35, MacroType::Single},
    {"Special Weapon 6",     [](){ MacroActions::Drag(R - DX*2, g_Config.topBound + DY, R - DX*1.8, B - DY/2); }, 0x36, MacroType::Single},
    {"Scan Vision",          [](){ MacroActions::Tap(X, B - DY/2, 500); }, 0x56, MacroType::Single},
    {"Ball",                 [](){ MacroActions::Tap(R - DX*1.5, B - DY/2); }, 0xA2, MacroType::Single},
    {"Screen Tap Jump",      MacroActions::DoNothing, 0xA3, MacroType::Single},
    {"Ok (menu)",            [](){ MacroActions::Tap(X, B - DY*1.2); }, 0x46, MacroType::Single},
    {"Yes (menu)",           [](){ MacroActions::Tap(X - DX*2, B - DY*1.2); }, 0x05, MacroType::Single},
    {"No (menu)",            [](){ MacroActions::Tap(X + DX*2, B - DY*1.2); }, 0x06, MacroType::Single},
    {"Left (menu)",          [](){ MacroActions::Tap(X - DX*2.5, B - DY*1.2); }, 0x5A, MacroType::Single},
    {"Right (menu)",         [](){ MacroActions::Tap(X + DX*2.5, B - DY*1.2); }, 0x58, MacroType::Single},
    {"Boost Ball (Swipe)",   MacroActions::BoostBall_MPH, 0xA0, MacroType::Continuous},
    {"Boost Ball (Trigger)", MacroActions::DoNothing,     0xA0, MacroType::TriggerLoop},
}};

#undef X
#undef Y
#undef DX
#undef DY
#undef R
#undef B
