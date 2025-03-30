#include "SandBox/Config/SBScene.cpp"
#include "SandBox/Config/SBRenderer.cpp"
#include "SandBox/SBImpl.h"
/*
 *           ______             _         __      _____
 *          |  ____|           (_)        \ \    / /__ \
 *          | |__   _ __   __ _ _ _ __   __\ \  / /   ) |
 *          |  __| | '_ \ / _` | | '_ \ / _ \ \/ /   / /
 *          | |____| | | | (_| | | | | |  __/\  /   / /_
 *          |______|_| |_|\__, |_|_| |_|\___| \/   |____|
 *                         __/ |
 *                        |___/
 *
*/
int main (void) {
    SandBox::SBImpl game;
    game.initSandBoxInfo();
    game.runSandBox();
    game.destroySandBox();
    return 0;
}