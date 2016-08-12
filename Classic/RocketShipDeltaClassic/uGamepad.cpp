//---------------------------------------------------------------------------

// This software is Copyright (c) 2016 Embarcadero Technologies, Inc.
// You may only use this software if you are an authorized licensee
// of Delphi, C++Builder or RAD Studio (Embarcadero Products).
// This software is considered a Redistributable as defined under
// the software license agreement that comes with the Embarcadero Products
// and is subject to that software license agreement.

//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "uGamepad.h"

#pragma package(smart_init)
#pragma resource "*.fmx"
TFrameGamepad *FrameGamepad;
//---------------------------------------------------------------------------
__fastcall TFrameGamepad::TFrameGamepad(TComponent* Owner)
	: TFrame(Owner)
{
}

void __fastcall TFrameGamepad::CheckRemoteProfiles( )
{
  //int I = 0;
  if ( GPTetheringManager->RemoteProfiles->Count > 0 )
  {
	for ( int stop = GPTetheringManager->RemoteProfiles->Count - 1, I = 0; I <= stop; I++)
	  if ( GPTetheringManager->RemoteProfiles->Items[I].ProfileText == TetheringName )
        IdConnectCount++;
    if ( IdConnectCount > 0 )
    {
	  GPTetheringAppProfile->Connect( GPTetheringManager->RemoteProfiles->Items[0] );
	  Label1->Text = "You are connected with " + GPTetheringManager->RemoteProfiles->Items[0].ProfileText;
    }
    FIsConnected = true;
  }
  else
  {
	Label1->Text = "You are not connected.";
    FIsConnected = false;
  }
  ProgressBar->Visible = False;
  GPTimer->Enabled = False;
}


void __fastcall TFrameGamepad::GPTetheringManagerEndManagersDiscovery( const TObject *Sender, const TTetheringManagerInfoList *RemoteManagers )
{
  int I = 0;
  if ( RemoteManagers->Count > 0 )
  {
	for ( int stop = RemoteManagers->Count - 1, I = 0; I <= stop; I++)
	  if ( const_cast<TTetheringManagerInfoList *>(RemoteManagers)->Items[I].ManagerText == "TetheringManager" )
		GPTetheringManager->PairManager( const_cast<TTetheringManagerInfoList *>(RemoteManagers)->Items[I] );
  }
}


void __fastcall TFrameGamepad::GPTetheringManagerEndProfilesDiscovery( const TObject *Sender, const TTetheringProfileInfoList RemoteProfiles )
{
  CheckRemoteProfiles();
}


void __fastcall TFrameGamepad::GPTetheringManagerRemoteManagerShutdown( const TObject *Sender, const String ManagerIdentifier )
{
  CheckRemoteProfiles();
}


void __fastcall TFrameGamepad::ConnectBTNClick( TObject *Sender )
{
  int I = 0;
  ProgressBar->Visible = True;
  ProgressBar->Value = 0;
  GPTimer->Enabled = True;

  IdConnectCount = 0;
  for ( int stop = 0, I = GPTetheringManager->PairedManagers->Count - 1; I >= stop; I--)
	GPTetheringManager->UnPairManager( GPTetheringManager->PairedManagers->Items[I] );
  GPTetheringManager->DiscoverManagers();
}


void __fastcall TFrameGamepad::ButtonAction( int idaction )
{
  if ( FIsConnected == false )
    return;
  switch ( idaction )
  {
    case LEFTACTIONDOWN_C:
	  GPTetheringAppProfile->RunRemoteActionAsync( GPTetheringManager->RemoteProfiles->Items[0], "LeftActionDown" );
    break;
    case LEFTACTIONUP_C:
	  GPTetheringAppProfile->RunRemoteActionAsync( GPTetheringManager->RemoteProfiles->Items[0], "LeftActionUp" );
    break;
    case RIGHTACTIONDOWN_C:
	  GPTetheringAppProfile->RunRemoteActionAsync( GPTetheringManager->RemoteProfiles->Items[0], "RightActionDown" );
    break;
    case RIGHTACTIONUP_C:
	  GPTetheringAppProfile->RunRemoteActionAsync( GPTetheringManager->RemoteProfiles->Items[0], "RightActionUp" );
    break;
    case UPACTIONDOWN_C:
	  GPTetheringAppProfile->RunRemoteActionAsync( GPTetheringManager->RemoteProfiles->Items[0], "UpActionDown" );
    break;
    case UPACTIONUP_C:
	  GPTetheringAppProfile->RunRemoteActionAsync( GPTetheringManager->RemoteProfiles->Items[0], "UpActionUp" );
    break;
    case DOWNACTIONDOWN_C:
	  GPTetheringAppProfile->RunRemoteActionAsync( GPTetheringManager->RemoteProfiles->Items[0], "DownActionDown" );
    break;
    case DOWNACTIONUP_C:
	  GPTetheringAppProfile->RunRemoteActionAsync( GPTetheringManager->RemoteProfiles->Items[0], "DownActionUp" );
    break;
    case FIREACTIONDOWN_C:
	  GPTetheringAppProfile->RunRemoteActionAsync( GPTetheringManager->RemoteProfiles->Items[0], "FireActionDown" );
    break;
    case FIREACTIONUP_C:
	  GPTetheringAppProfile->RunRemoteActionAsync( GPTetheringManager->RemoteProfiles->Items[0], "FireActionUp" );
    break;
    case WARPACTIONCLICK_C:
	  GPTetheringAppProfile->RunRemoteActionAsync( GPTetheringManager->RemoteProfiles->Items[0], "WarpActionClick" );
    break;
    case BOMBACTIONCLICK_C:
	  GPTetheringAppProfile->RunRemoteActionAsync( GPTetheringManager->RemoteProfiles->Items[0], "BombActionClick" );
    break;
  }
}


void __fastcall TFrameGamepad::LeftBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  ButtonAction( LEFTACTIONDOWN_C );
}


void __fastcall TFrameGamepad::LeftBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  ButtonAction( LEFTACTIONUP_C );
}


void __fastcall TFrameGamepad::RightBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  ButtonAction( RIGHTACTIONDOWN_C );
}


void __fastcall TFrameGamepad::RightBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  ButtonAction( RIGHTACTIONUP_C );
}


void __fastcall TFrameGamepad::UpBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  ButtonAction( UPACTIONDOWN_C );
}


void __fastcall TFrameGamepad::UpBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  ButtonAction( UPACTIONUP_C );
}


void __fastcall TFrameGamepad::DownBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  ButtonAction( DOWNACTIONDOWN_C );
}


void __fastcall TFrameGamepad::DownBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  ButtonAction( DOWNACTIONUP_C );
}


void __fastcall TFrameGamepad::FireBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  ButtonAction( FIREACTIONDOWN_C );
}


void __fastcall TFrameGamepad::FireBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  ButtonAction( FIREACTIONUP_C );
}


void __fastcall TFrameGamepad::WarpBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  ButtonAction( WARPACTIONCLICK_C );
}


void __fastcall TFrameGamepad::BombBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  ButtonAction( BOMBACTIONCLICK_C );
}


void __fastcall TFrameGamepad::HandleKeyDown( TObject *Sender, WORD& Key, Char& KeyChar, TShiftState Shift )
{
  switch ( KeyChar )
  {
    case 'Z': case 'z': case ' ':
      ButtonAction( FIREACTIONDOWN_C );
    break;
    case 'X': case 'x':
      ButtonAction( BOMBACTIONCLICK_C );
    break;
    case 'C': case 'c':
      ButtonAction( WARPACTIONCLICK_C );
    break;
    case 'W': case 'w':
      ButtonAction( UPACTIONDOWN_C );
    break;
    case 'S': case 's':
      ButtonAction( DOWNACTIONDOWN_C );
    break;
    case 'A': case 'a':
      ButtonAction( LEFTACTIONDOWN_C );
    break;
    case 'D': case 'd':
      ButtonAction( RIGHTACTIONDOWN_C );
    break;
  }
  switch ( Key )
  {
    case vkUp:
      ButtonAction( UPACTIONDOWN_C );
    break;
	case vkDown:
      ButtonAction( DOWNACTIONDOWN_C );
    break;
    case vkLeft:
      ButtonAction( LEFTACTIONDOWN_C );
    break;
    case vkRight:
      ButtonAction( RIGHTACTIONDOWN_C );
    break;
  }
}


void __fastcall TFrameGamepad::HandleKeyUp( TObject *Sender, WORD& Key, Char& KeyChar, TShiftState Shift )
{
  switch ( KeyChar )
  {
    case 'Z': case 'z': case ' ':
      ButtonAction( FIREACTIONUP_C );
    break;
    // 'X','x'   :
    // 'C','c'   :

    case 'W': case 'w':
      ButtonAction( UPACTIONUP_C );
    break;
    case 'S': case 's':
      ButtonAction( DOWNACTIONUP_C );
    break;
    case 'A': case 'a':
      ButtonAction( LEFTACTIONUP_C );
	break;
    case 'D': case 'd':
      ButtonAction( RIGHTACTIONUP_C );
    break;
  }
  switch ( Key )
  {
    case vkUp:
      ButtonAction( UPACTIONUP_C );
    break;
    case vkDown:
      ButtonAction( DOWNACTIONUP_C );
    break;
    case vkLeft:
      ButtonAction( LEFTACTIONUP_C );
    break;
    case vkRight:
      ButtonAction( RIGHTACTIONUP_C );
    break;
  }
}

void __fastcall TFrameGamepad::ClearButtons( )
{
  ButtonAction( FIREACTIONUP_C );
  ButtonAction( UPACTIONUP_C );
  ButtonAction( DOWNACTIONUP_C );
  ButtonAction( LEFTACTIONUP_C );
  ButtonAction( RIGHTACTIONUP_C );
}

void __fastcall TFrameGamepad::GPTimerTimer(TObject *Sender)
{
if (ProgressBar->Value==100) { ProgressBar->Value = 0; }
ProgressBar->Value = ProgressBar->Value + 20;
}
//---------------------------------------------------------------------------

