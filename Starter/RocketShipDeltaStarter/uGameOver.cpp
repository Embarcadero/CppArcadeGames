//---------------------------------------------------------------------------

// This software is Copyright (c) 2016-2020 Embarcadero Technologies, Inc.
// You may only use this software if you are an authorized licensee
// of Delphi, C++Builder or RAD Studio (Embarcadero Products).
// This software is considered a Redistributable as defined under
// the software license agreement that comes with the Embarcadero Products
// and is subject to that software license agreement.

//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "uGameOver.h"

#pragma package(smart_init)
#pragma resource "*.fmx"
TFrameGameOver *FrameGameOver;
//---------------------------------------------------------------------------
__fastcall TFrameGameOver::TFrameGameOver(TComponent* Owner)
	: TFrame(Owner)
{
}