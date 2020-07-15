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

#define DEBUG_MODE

#include "uGame.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "uGameOver"
#pragma link "uGamepad"
#pragma link "uHighScores"
#pragma link "uInstructions"
#pragma link "uLevelComplete"
#pragma link "uMainMenu"
#pragma link "uSettings"
#pragma resource "*.fmx"
TGameForm *GameForm;

template<class T> inline void destroy(T*& p) { delete p; p = NULL; }

//---------------------------------------------------------------------------
__fastcall TGameForm::TGameForm(TComponent* Owner)
	: TForm(Owner)
{
}

void __fastcall TGameForm::ShowMsgBox( String S )
{
  TDialogServiceAsync::MessageDialog(S, TMsgDlgType::mtInformation, TMsgDlgButtons() << TMsgDlgBtn::mbOK, TMsgDlgBtn::mbOK, 0, ShowMsgBoxResultProc);
}

void __fastcall TGameForm::ShowMsgBoxResultProc(TObject* Sender, const TModalResult AResult)
{

}

bool __fastcall TGameForm::HandleAppEvent( TApplicationEvent AAppEvent, TObject* AContext )
{
  switch ( AAppEvent )
  {
	// TApplicationEvent::FinishedLaunching: Log('Finished Launching');
	case TApplicationEvent::BecameActive:
	{
	  if ( ( CurrentStage == GAMEPLAY ) && ( GameLoop->Enabled == false ) )
	  {
		StartGameLoop();
		PauseBitmapListAnimations( false );
	  }
	}
	break;
	// TApplicationEvent::WillBecomeInactive: Log('Will Become Inactive');

	case TApplicationEvent::EnteredBackground:
	{
	  if ( ( CurrentStage == GAMEPLAY ) && ( GameLoop->Enabled == true ) )
	  {
		PauseBitmapListAnimations( true );
		StopGameLoop();
	  }
	}
	break;
	// TApplicationEvent::WillBecomeForeground: Log('Will Become Foreground');
	// TApplicationEvent::WillTerminate: Log('Will Terminate');
	// TApplicationEvent::LowMemory: Log('Low Memory');
	// TApplicationEvent::TimeChange: Log('Time Change');
	// TApplicationEvent::OpenURL: Log('Open URL');
  }
  return true;
}


void __fastcall TGameForm::PauseBitmapListAnimations( bool Value )
{
  int I = 0;
  for ( int stop = 0, I = ExplosionList->Count - 1; I >= stop; I--)
  {
	TRectangle *BLAObj = (TRectangle*)ExplosionList->Objects[I];
	((TBitmapListAnimation*)BLAObj->TagObject)->Pause = Value;
  }
}


void __fastcall TGameForm::PlaySound( int Index )
{
  if ( SoundEnabled )
	AudioManager->PlaySound( Index );
}


int __fastcall TGameForm::RegisterSound( String Filename )
{
  if ( FileExists( Filename ) )
	return AudioManager->AddSound( Filename );
  else
	return -1;
}


float __fastcall TGameForm::GetMaxVolume( )
{
#if defined(IOS) || defined(MACOS)
 return 0.75;
#else
 return 1;
#endif
}


void __fastcall TGameForm::PlayMusic( )
{
  if (MusicPlayer->Media)
  {
	// MusicPlayer.CurrentTime := 0;
	MusicPlayer->Play();
	if ( MusicEnabled )
	{
	  if ( MusicPlayer->Volume != GetMaxVolume() )
		MusicPlayer->Volume = GetMaxVolume();
    }
    else
    {
	  if ( MusicPlayer->Volume != 0 )
        MusicPlayer->Volume = 0;
    }
  }
}


void __fastcall TGameForm::StopMusic( )
{
  if ( MusicPlayer->FileName != "" )
  {
	if ( MusicPlayer->Volume != 0 )
      MusicPlayer->Volume = 0;
  }
}

void __fastcall TGameForm::LeftDownEvent( TObject *Sender )
{
  if ( RScreenOut == true )
	RScreenOut = false;
  RightButtonDown = false;
  LeftButtonDown = true;
}

void __fastcall TGameForm::LeftUpEvent( TObject *Sender )
{
  LeftButtonDown = false;
}

void __fastcall TGameForm::RightDownEvent( TObject *Sender )
{
  if ( LScreenOut == true )
	LScreenOut = false;
  LeftButtonDown = false;
  RightButtonDown = true;
}

void __fastcall TGameForm::RightUpEvent( TObject *Sender )
{
  RightButtonDown = false;
}

void __fastcall TGameForm::UpDownEvent( TObject *Sender )
{
  if ( RScreenOut == true )
	RScreenOut = false;
  if ( LScreenOut == true )
	LScreenOut = false;
  FireButtonDown = true;
}

void __fastcall TGameForm::UpUpEvent( TObject *Sender )
{
  FireButtonDown = false;
}

void __fastcall TGameForm::UpActionDownExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  UpDownEvent(Sender);
}

void __fastcall TGameForm::UpActionUpExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  UpUpEvent(Sender);
}

void __fastcall TGameForm::ShowHighScores( )
{
  HighScoresFrame->InitFrame();
  SetStage( HIGH_SCORES );
}


void __fastcall TGameForm::AddScore( int I )
{
  PlayerData->Score = PlayerData->Score + I;
  ScoreLBL->Text = IntToStr( PlayerData->Score );
}


TRectangle* __fastcall TGameForm::GetPoolObj( TStringList* Pool )
{
  TRectangle *result = NULL;
  if ( Pool->Count > 0 )
  {
	result = (TRectangle*)( Pool->Objects[Pool->Count - 1] );
    // Pool.Objects[Pool.Count-1] := nil;
	Pool->Delete( Pool->Count - 1 );
  }
  else
  {
	result = new TRectangle(this);
  }
  return result;
}


void __fastcall TGameForm::SetPoolObj( TStringList* Pool, String Name, TRectangle* Obj )
{
  Pool->AddObject( Name, Obj );
  Obj->Parent = NULL;
}


TRectangle* __fastcall TGameForm::SpawnGround( float X, float Y, int GroundType, float LandingPadWidth )
{
  TRectangle *R = GetPoolObj( GroundPool );
  R->Parent = ScreenLayout;
  if ( GroundType == 1 )
  {
	R->Width = LandingPadWidth + RandomRange( 20, 40 );
	R->Height = RandomRange( 100, 150 );
	R->XRadius = 0;
	R->YRadius = 0;
	BitmapToRectangle( LandingPad->Fill->Bitmap->Bitmap, R );
	R->TagString = 'P';
  }
  else
  {
	// R.Width := RandomRange(15, 75);
	R->Height = RandomRange( 50, 125 );
	R->Width = R->Height;
	R->RotationAngle = 45;
	R->XRadius = 20;
	R->YRadius = 20;
	R->Fill->Kind = TBrushKind::Solid;
	R->Fill->Color = 0xFF6F5336;
	R->TagString = 'G';
  }
  R->Position->X = X;
  R->Position->Y = Y - R->Height;
  R->Stroke->Kind = TBrushKind::None;
  R->Tag = 5;
  R->SendToBack();
  return R;
}


TRectangle* __fastcall TGameForm::SpawnCollectItem( float X, float Y, int Size )
{
  TRectangle *CollectGraphic = NULL;
  TRectangle *R = GetPoolObj( CollectPool );
  R->Stroke->Kind = TBrushKind::None;
  R->Fill->Kind = TBrushKind::None;
  R->RotationAngle = Random( 360 );
  R->Width = 50;
  R->Height = 50;
  R->Position->X = X;
  R->Position->Y = Y;
  if (!R->TagObject)
  {
	CollectGraphic = new TRectangle(R);
	CollectGraphic->Parent = R;
  }
  else
  {
	CollectGraphic = (TRectangle*)R->TagObject;
  }
  CollectGraphic->Width = R->Width;
  CollectGraphic->Height = R->Height;
  CollectGraphic->Position->X = ( R->Width / 2 ) - ( CollectGraphic->Width / 2 );
  CollectGraphic->Position->Y = ( R->Height / 2 ) - ( CollectGraphic->Height / 2 );
  CollectGraphic->RotationAngle = ( 360 - R->RotationAngle );
  CollectGraphic->Stroke->Kind = TBrushKind::None;
  CollectGraphic->Fill->Bitmap->WrapMode = TWrapMode::TileStretch;
  CollectGraphic->Fill->Kind = TBrushKind::Bitmap;
  CollectGraphic->Fill->Bitmap->Bitmap->Assign( CollectItem->Fill->Bitmap->Bitmap );
  if (!R->TagObject)
  {
	R->TagObject = CollectGraphic;
  }
  R->Tag = COLLECTITEM_SPEED;
  R->TagFloat = 0;
  R->Parent = this;
  R->SendToBack();
  return R;
}


TRectangle* __fastcall TGameForm::SpawnExplosion( float X, float Y, float Angle )
{
  TBitmapListAnimation *AE = NULL;
  TRectangle *R = GetPoolObj( ExplosionPool );
  R->Parent = this;
  R->Width = 75;
  R->Height = 75;
  R->RotationAngle = Angle - 90;
  R->Stroke->Kind = TBrushKind::None;
  R->Fill->Kind = TBrushKind::Bitmap;
  R->Fill->Bitmap->WrapMode = TWrapMode::Tile;
  if (!R->TagObject)
  {
	AE = new TBitmapListAnimation( R );
	AE->AnimationBitmap->Assign( AnimateExplosion->AnimationBitmap );
	AE->AnimationCount = AnimateExplosion->AnimationCount;
	AE->AnimationRowCount = AnimateExplosion->AnimationRowCount;
	AE->PropertyName = AnimateExplosion->PropertyName;
	AE->Trigger = AnimateExplosion->Trigger;
	R->TagObject = AE;
	AE->Parent = R;
	AE->OnFinish = AnimateExplosionFinish;
  }
  else
  {
	AE = (TBitmapListAnimation*)( R->TagObject );
  }
  AE->Start();
  R->Position->X = X - ( R->Width / 2 );
  R->Position->Y = Y - ( R->Height / 2 );
  R->Tag = 2;
  R->TagFloat = 0;
  R->Visible = true;
  R->SendToBack();
  return R;
}


void __fastcall TGameForm::CreateExplosion( float X, float Y )
{
  ExplosionList->AddObject( "", SpawnExplosion( X, Y, 0 ) );
  PlaySound( EXPLOSION_SFX );
}


void __fastcall TGameForm::ShowHUD( int Stage )
{
  Ship->Visible = true;
  HUDLayout->Visible = true;
  #ifdef DEBUG_MODE
  FPSLBL->Visible = true;
  #endif
}


void __fastcall TGameForm::HideHUD( int Stage )
{
  if ( Stage != LEVEL_COMPLETE )
	Ship->Visible = false;
  Thruster->Visible = false;
  HUDLayout->Visible = false;
  FPSLBL->Visible = false;
}


void __fastcall TGameForm::AssignInterfaceBackground( TRectangle* R )
{
  BitmapToRectangle( InterfaceBackground->Fill->Bitmap->Bitmap, R );
}


void __fastcall TGameForm::SetStage( int Stage )
{

  switch ( Stage )
  {
    case MAIN_MENU:
	{
	  MainMenuFrame->Visible = true;
	  GameOverFrame->Visible = false;
	  InstructionsFrame->Visible = false;
	  HighScoresFrame->Visible = false;
	  LevelCompleteFrame->Visible = false;
	  SettingsFrame->Visible = false;
	  GamepadFrame->Visible = false;
	  HideHUD( Stage );
	  StopMusic();
    }
    break;
    case INSTRUCTIONS:
	{
	  MainMenuFrame->Visible = false;
	  GameOverFrame->Visible = false;
	  AssignInterfaceBackground( InstructionsFrame->Rectangle1 );
	  InstructionsFrame->Visible = true;
	  HideHUD( Stage );
      StopMusic();
	}
    break;
    case GAMEPLAY:
    {
	  InstructionsFrame->Visible = false;
	  LevelCompleteFrame->Visible = false;
      ShowHUD( Stage );
	  PlayMusic();
	}
	break;
    case LEVEL_COMPLETE:
	{
	  AssignInterfaceBackground( LevelCompleteFrame->Rectangle1 );
	  LevelCompleteFrame->Visible = true;
	  if ( PlayerData->Stars > 0 )
      {
		LevelCompleteFrame->CollectItem1->Fill->Bitmap->Bitmap->Assign( CollectItem->Fill->Bitmap->Bitmap );
		LevelCompleteFrame->CollectItem1->Visible = true;
      }
      else
		LevelCompleteFrame->CollectItem1->Visible = false;
      if ( PlayerData->Stars > 1 )
      {
		LevelCompleteFrame->CollectItem2->Fill->Bitmap->Bitmap->Assign( CollectItem->Fill->Bitmap->Bitmap );
		LevelCompleteFrame->CollectItem2->Visible = true;
      }
      else
		LevelCompleteFrame->CollectItem2->Visible = false;
      if ( PlayerData->Stars > 2 )
      {
		LevelCompleteFrame->CollectItem3->Fill->Bitmap->Bitmap->Assign( CollectItem->Fill->Bitmap->Bitmap );
		LevelCompleteFrame->CollectItem3->Visible = true;
      }
      else
		LevelCompleteFrame->CollectItem3->Visible = false;
      HideHUD( Stage );
    }
	break;
    case GAME_OVER:
    {
	  HighScoresFrame->Visible = false;
	  AssignInterfaceBackground( GameOverFrame->Rectangle1 );
	  GameOverFrame->Visible = true;
      HideHUD( Stage );
    }
	break;
    case HIGH_SCORES:
    {
	  MainMenuFrame->Visible = false;
	  GameOverFrame->Visible = false;
	  AssignInterfaceBackground( HighScoresFrame->Rectangle1 );
	  HighScoresFrame->Visible = true;
	  HighScoresFrame->BringToFront();
	  HideHUD( Stage );
    }
    break;
    case SETTINGS:
    {
	  MainMenuFrame->Visible = false;
	  SettingsFrame->Visible = true;
      HideHUD( Stage );
	  StopMusic();
    }
    break;
    case GAMEPAD:
    {
	  MainMenuFrame->Visible = false;
	  GamepadFrame->Visible = true;
      HideHUD( Stage );
      StopMusic();
    }
    break;
  }
  LastStage = CurrentStage;
  CurrentStage = Stage;
}


void __fastcall TGameForm::SettingsFrameFullScreenSwitchSwitch( TObject *Sender )
{
  if ( FullScreenEnabled )
  {
    FullScreenEnabled = false;
    GameForm->FullScreen = false;
  }
  else
  {
    FullScreenEnabled = true;
    GameForm->FullScreen = true;
  }
}


void __fastcall TGameForm::ToggleAppTethering( bool State )
{
  if ( State == true )
  {
	try
    {
      TetheringManager->Enabled = true;
	  TetheringAppProfile->Enabled = true;
    }
    catch( Exception & E )
	{
	  ShowMsgBox( "Enabling app tethering caused an error: " + E.Message );
	}
  }
  else
  {
    try
	{
	  TetheringManager->Enabled = false;
	  TetheringAppProfile->Enabled = false;
	}
    catch( Exception & E )
    {
	  ShowMsgBox( "Disabling app tethering caused an error: " + E.Message );
	}
  }
}


void __fastcall TGameForm::SettingsFrameHostSwitchSwitch( TObject *Sender )
{
  if ( HostEnabled )
  {
    HostEnabled = false;
    ToggleAppTethering( false );
  }
  else
  {
    HostEnabled = true;
    if ( NetworkConnected == true )
    {
      ToggleAppTethering( true );
    }
    else
    {
      ShowMsgBox( NO_NETWORK_MESSAGE );
    }
  }
}


void __fastcall TGameForm::SettingsFrameMainMenuBTNClick( TObject *Sender )
{
  SaveAndExitSettings();
}


void __fastcall TGameForm::SaveAndExitSettings( )
{
  HostEnabled = SettingsFrame->HostSwitch->IsChecked;
  GyroEnabled = SettingsFrame->GyroSwitch->IsChecked;
  FullScreenEnabled = SettingsFrame->FullScreenSwitch->IsChecked;
  SoundEnabled = SettingsFrame->SoundSwitch->IsChecked;
  MusicEnabled = SettingsFrame->MusicSwitch->IsChecked;
  SaveSettings();
  ShowMainMenu();
}


bool __fastcall TGameForm::IntersectCircle( TRectangle R1, TRectangle R2 )
{
  const float Distance = R1.Position->Point.Distance( R2.Position->Point );
  return Distance < ( ( R1.Width / 2 ) + ( R2.Width / 2 ) );
}


void __fastcall TGameForm::ProcessAccelerometer( )
{
  float AccX = 0.0, AccY = 0.0;

  if (!MotionSensor->Sensor) {
   return;
  }

  if ( ( GetScreenOrientation() == TScreenOrientation::Portrait ) || ( GetScreenOrientation() == TScreenOrientation::InvertedPortrait ) )
  {
	AccX = ( MotionSensor->Sensor->AccelerationX * 10 );
	AccY = ( MotionSensor->Sensor->AccelerationY * 10 );
  }
  else
  {
    AccY = ( MotionSensor->Sensor->AccelerationX * 10 );
    AccX = ( MotionSensor->Sensor->AccelerationY * 10 );
  }
  if ( AccY > - 7 )
  {
    if ( RScreenOut == true )
      RScreenOut = false;
    if ( LScreenOut == true )
      LScreenOut = false;
    AccThrustCircle->Visible = true;
    AccFireButtonDown = true;
  }
  else
  {
    AccThrustCircle->Visible = false;
    AccFireButtonDown = false;
  }
  if ( ( GetScreenOrientation() == TScreenOrientation::Portrait ) || ( GetScreenOrientation() == TScreenOrientation::InvertedPortrait ) )
  {

    // right
    if ( AccX > 2 )
    {
      AccRight->Visible = true;
      AccRightButtonDown = true;
    }
    else
	{
      AccRight->Visible = false;
      AccRightButtonDown = false;
    }

    // left
    if ( AccX < - 2 )
    {
      AccLeft->Visible = true;
      AccLeftButtonDown = true;
    }
    else
    {
      AccLeft->Visible = false;
      AccLeftButtonDown = false;
    }
  }
  else
  {

    // left
    if ( AccX > 2 )
    {
      AccLeft->Visible = true;
	  AccLeftButtonDown = true;
    }
    else
    {
      AccLeft->Visible = false;
      AccLeftButtonDown = false;
	}

    // right
    if ( AccX < - 2 )
    {
      AccRight->Visible = true;
      AccRightButtonDown = true;
    }
    else
    {
      AccRight->Visible = false;
      AccRightButtonDown = false;
    }
  }
}


void __fastcall TGameForm::GameLoopTimer( TObject *Sender )
{
  int I = 0, II = 0;
  float ProjAngle = 0.0, RockAngle = 0.0, EnemyAngle = 0.0, EnemyProjAngle = 0.0, CollectAngle = 0.0;
  TRectangle *ProjObj = NULL;
  TRectangle *RockObj = NULL;
  TRectangle *ExplosionObj = NULL;
  float RockObjCenterX = 0.0, RockObjCenterY = 0.0;
  TRectangle *EnemyObj = NULL;
  TRectangle *EnemyProjObj = NULL;
  TRectangle *CollectObj = NULL;
  unsigned int Time = 0;
  float RScreenLimit = 0.0;
  float ShipAngle = 0.0;
  TRectangle* GroundObj = NULL;
  #ifdef DEBUG_MODE
  Time = TThread::GetTickCount();
  #endif
  // Check for game over
  if ( PlayerData->Lives <= 0 )
  {
    LevelFail();
	GameOver();
    return;
  }

  // Check for level complete
  if ( PlayerData->Landed == true )
  {
    AddScore( 1000 + PlayerData->Fuel );
    LevelComplete();
    return;
  }

  // process accelerometer data
  if ( MotionSensor->Active == true )
  {
    ProcessAccelerometer();
  }

  // process inputs
  if ( ( LeftButtonDown || AccLeftButtonDown ) && ( Ship->RotationAngle > - 90 ) )
    Ship->RotationAngle = Ship->RotationAngle - 2;
  if ( ( RightButtonDown || AccRightButtonDown ) && ( Ship->RotationAngle < 90 ) )
    Ship->RotationAngle = Ship->RotationAngle + 2;
  if ( FireButtonDown || AccFireButtonDown )
  {
    if ( PlayerData->Fuel > 0 )
    {
      PlayerData->VerticalVelocity = PlayerData->VerticalVelocity - PLAYER_THRUST;
      PlayerData->HorizontalVelocity = PlayerData->HorizontalVelocity + PLAYER_THRUST;
      PlayerData->Fuel = Max( 0, PlayerData->Fuel - 1 );
      PlaySound( FIRE_SFX );
      Thruster->Visible = true;
    }
    else
    {
      Thruster->Visible = false;
    }
  }
  else
  {
	Thruster->Visible = false;
  }

  // move player
  PlayerData->VerticalVelocity = PlayerData->VerticalVelocity + GAME_GRAVITY;
  PlayerData->HorizontalVelocity = Max( 0.0, PlayerData->HorizontalVelocity - GAME_GRAVITY );
  ShipAngle = ( Ship->RotationAngle ) * ( PI / 180 );
  //change movement
  if ( ( Ship->Position->X > ScreenLayout->Position->X ) && ( Ship->Position->X + Ship->Width < ScreenLayout->Position->X + ScreenLayout->Width ) )
  {
    if ( ( LScreenOut == false ) && ( RScreenOut == false ) )
	{
      Ship->Position->X = Ship->Position->X + PlayerData->HorizontalVelocity * Sin( ShipAngle );
      if ( Ship->Position->X < 0 )
      {
        Ship->Position->X = - 1;
        LScreenOut = true;
        CatchLastPos = Ship->Position->X;
      }
        //ship never reach ScreenLayout.Width, make right screen limit area
      RScreenLimit = 35;
      if ( Ship->Position->X > ( ScreenLayout->Width - RScreenLimit ) )
      {
          //make ship position little bit outside right screen limit area
        Ship->Position->X = ( ScreenLayout->Width - ( RScreenLimit - 1 ) );
        RScreenOut = true;
        CatchLastPos = Ship->Position->X;
      }
    }
  }
  else
  {
    if ( Ship->Position->X > ScreenLayout->Position->X )
    {
      if ( RScreenOut == true )
        Ship->Position->X = CatchLastPos;
      else
        Ship->Position->X = ( Ship->Position->X - PlayerData->HorizontalVelocity ) + PlayerData->HorizontalVelocity * Sin( ShipAngle );
    }
    else
    {
	  if ( LScreenOut == true )
        Ship->Position->X = CatchLastPos;
	  else
        Ship->Position->X = ( Ship->Position->X + PlayerData->HorizontalVelocity ) + PlayerData->HorizontalVelocity * Sin( ShipAngle );
    }
  }
  Ship->Position->Y = Ship->Position->Y + PlayerData->VerticalVelocity;
  if ( ( Ship->Position->Y + Ship->Height ) < 0 )
  {
    if ( AboveCircle->Visible == false )
      AboveCircle->Visible = true;
  }
  else
  {
    if ( AboveCircle->Visible == true )
    {
      AboveCircle->Visible = false;
    }
  }

  // handle HUD elements
  AboveCircle->Position->X = Ship->Position->X;
  FuelLBL->Text = "Fuel: " + IntToStr( PlayerData->Fuel );
  AngleLBL->Text = "Angle: " + FormatFloat( "0.00", Ship->RotationAngle );
  VVLBL->Text = "V. Velocity: " + FormatFloat( "0.00", PlayerData->VerticalVelocity );
  if ( ( Ship->RotationAngle > - 10 ) && ( Ship->RotationAngle < 10 ) )
  {
	if ( AngleLBL->TextSettings->FontColor != TAlphaColor(TAlphaColorRec::Aqua) )
	  AngleLBL->TextSettings->FontColor = TAlphaColor(TAlphaColorRec::Aqua);
  }
  else
  {
	if ( AngleLBL->TextSettings->FontColor != TAlphaColor(TAlphaColorRec::White) )
	  AngleLBL->TextSettings->FontColor = TAlphaColor(TAlphaColorRec::White);
  }
  if ( PlayerData->VerticalVelocity < PLAYER_LANDING_VELOCITY )
  {
	if ( VVLBL->TextSettings->FontColor != TAlphaColor(TAlphaColorRec::Aqua) )
	  VVLBL->TextSettings->FontColor = TAlphaColor(TAlphaColorRec::Aqua);
  }
  else
  {
	if ( VVLBL->TextSettings->FontColor != TAlphaColor(TAlphaColorRec::White) )
	  VVLBL->TextSettings->FontColor = TAlphaColor(TAlphaColorRec::White);
  }

  // process ground collisions and landing
  if ( GroundList->Count > 0 )
    for ( int stop = GroundList->Count - 1, I = 0; I <= stop; I++)
    {
	  GroundObj = (TRectangle*) GroundList->Objects[I] ;
	  if ( IntersectRect( GroundObj->BoundsRect, Ship->BoundsRect ) )
      {
        if ( ( Ship->RotationAngle > - 10 ) && ( Ship->RotationAngle < 10 ) && ( PlayerData->VerticalVelocity < PLAYER_LANDING_VELOCITY ) && ( Ship->Position->X > GroundObj->Position->X ) && ( ( Ship->Position->X + Ship->Width ) < ( GroundObj->Position->X + GroundObj->Width ) ) && ( GroundObj->TagString == 'P' ) )
        {
          PlayerData->Landed = true;
        }
        else
        {
          PlayerHit();
		}
      }
    }
  if ( Ship->Position->Y > ScreenLayout->Height )
  {
    PlayerHit();
  }

  // Handle explosions
  if ( ExplosionList->Count > 0 )
    for ( int stop = 0, I = ExplosionList->Count - 1; I >= stop; I--)
    {
	  ExplosionObj = (TRectangle*) ExplosionList->Objects[I];
      ExplosionObj->TagFloat = ExplosionObj->TagFloat + 0.1;
	  if ( ExplosionObj->TagFloat > (double)ExplosionObj->Tag )
      {
        // ProjList.Objects[I] := nil;
		SetPoolObj( ExplosionPool, ExplosionList->Strings[I], ExplosionObj );
        ExplosionList->Delete( I );
      }
	}

  // Handle collectable items
  if ( CollectList->Count > 0 )
    for ( int stop = 0, I = CollectList->Count - 1; I >= stop; I--)
    {
	  CollectObj = (TRectangle*) CollectList->Objects[I] ;
	  CollectAngle = CollectObj->RotationAngle * PI / 180;
	  CollectObj->Position->X = CollectObj->Position->X + (double)CollectObj->Tag * Cos( CollectAngle );
	  CollectObj->Position->Y = CollectObj->Position->Y + (double)CollectObj->Tag * Sin( CollectAngle );
	  if ( CollectObj->BoundsRect.CenterPoint().X >= ( ScreenLayout->Width + ( CollectObj->Width / 2 ) ) )
      {
		CollectObj->Position->X = ( ScreenLayout->Position->X + 1 ) - ( CollectObj->Width / 2 );
	  }
	  if ( CollectObj->BoundsRect.CenterPoint().Y >= ( ScreenLayout->Height + ( CollectObj->Height / 2 ) ) )
	  {
		CollectObj->Position->Y = ( ScreenLayout->Position->Y + 1 ) - ( CollectObj->Height / 2 );
	  }
	  if ( CollectObj->BoundsRect.CenterPoint().X <= ( ScreenLayout->Position->X - ( CollectObj->Width / 2 ) ) )
      {
        CollectObj->Position->X = ( ScreenLayout->Width - 1 );
      }
	  if ( CollectObj->BoundsRect.CenterPoint().Y <= ( ScreenLayout->Position->Y - ( CollectObj->Height / 2 ) ) )
      {
        CollectObj->Position->Y = ( ScreenLayout->Height - 1 );
      }
      if ( IntersectRect( Ship->BoundsRect, CollectObj->BoundsRect ) )
      {
        AddScore( 100 * PlayerData->Level );
        CollectObj->TagFloat = CollectObj->TagFloat + 1;
        PlayerData->Stars = PlayerData->Stars + 1;
        PlaySound( COLLECT_SFX );
      }
      if ( CollectObj->TagFloat > 0 )
      {
		SetPoolObj( CollectPool, CollectList->Strings[I], CollectObj );
        CollectList->Delete( I );
      }
    }

  // Handle music loop
  if ( ( MusicPlayer->CurrentTime >= MusicPlayer->Duration ) || ( MusicPlayer->State == TMediaState::Stopped ) )
  {
    MusicPlayer->CurrentTime = 0;
    //MusicPlayer.Stop;
    PlayMusic();
  }
  #ifdef DEBUG_MODE
  FPSLBL->Text = IntToStr((int) (TThread::GetTickCount() - Time) ) + " ms";
  #endif
}


float __fastcall TGameForm::GetTargetAngle( float TargetX, float TargetY, float OriginX, float OriginY )
{
  float Radians = ArcTan2( TargetY - OriginY, TargetX - OriginX );
  return Radians / ( PI / 180 ) + 90;
}


void __fastcall TGameForm::GyroBTNClick( TObject *Sender )
{
  GyroToggle();
}


void __fastcall TGameForm::GyroToggle( )
{
  if ( GyroEnabled )
  {
    GyroEnabled = false;
    GyroOffLine->Visible = true;
    MotionSensor->Active = false;
    AccFireButtonDown = false;
    AccThrustCircle->Visible = false;
    AccLeftButtonDown = false;
    AccLeft->Visible = false;
    AccLeftButtonDown = false;
    AccRight->Visible = false;
    AccRightButtonDown = false;
  }
  else
  {
    GyroEnabled = true;
    GyroOffLine->Visible = false;
    MotionSensor->Active = true;
  }
}


void __fastcall TGameForm::PlayerHit( )
{
  CreateExplosion( Ship->Position->X + ( Ship->Width / 2 ), Ship->Position->Y + ( Ship->Height / 2 ) );
  Ship->Visible = false;
  PlayerData->Lives = PlayerData->Lives - 1;
  PlayerData->SpeedX = 0;
  PlayerData->SpeedY = 0;
  PlayerData->VerticalVelocity = 0;
  PlayerData->HorizontalVelocity = 10;
  Ship->RotationAngle = 45;
  DisplayLives( PlayerData->Lives );
  if ( PlayerData->Lives > 0 )
  {
    CenterPlayer();
    PlaySound( FAIL_SFX );
  }
}


void __fastcall TGameForm::DisplayScore( )
{
  ScoreLBL->Text = IntToStr( PlayerData->Score );
}


void __fastcall TGameForm::DelayedSettingsTimer( TObject *Sender )
{
  if ( FileExists( DataFilePath + MUSIC_FILENAME ) & ( MusicPlayer->FileName == "" ) )
  {
	try
    {
	  MusicPlayer->FileName = DataFilePath + MUSIC_FILENAME;
    }
	catch( Exception & E )
    {
	  ShowMsgBox( "Music failed to load: " + E.Message );
	}
  }
  if ( HostEnabled == true )
  {
	if ( NetworkConnected == true )
    {
      ToggleAppTethering( true );
    }
  }
  else
  {
    ToggleAppTethering( false );
  }
  DelayedSettings->Enabled = false;
}


void __fastcall TGameForm::DisplayLives( int Lives )
{
  Lives1->Visible = Lives > 0;
  Lives2->Visible = Lives > 1;
  Lives3->Visible = Lives > 2;
}


void __fastcall TGameForm::GameOverFrameMainMenuBTNClick( TObject *Sender )
{
  CleanupGame( false );
  ShowMainMenu();
}


void __fastcall TGameForm::GameOverFrameMoreGamesBTNClick( TObject *Sender )
{
  ShowMoreGames();
}


void __fastcall TGameForm::GameOverFramePlayAgainBTNClick( TObject *Sender )
{
  CleanupGame( false );
  StartGame();
}


void __fastcall TGameForm::GamepadFrameCloseBTNClick( TObject *Sender )
{
  if ( HostEnabled )
  {
    if ( NetworkConnected == true )
	{
      ToggleAppTethering( true );
    }
  }
  SetStage( MAIN_MENU );
}


void __fastcall TGameForm::CloseHighScores( )
{
  HighScoresFrame->CloseInputBox();
  if ( LastStage == MAIN_MENU )
    SetStage( MAIN_MENU );
  else
    SetStage( GAME_OVER );
}


void __fastcall TGameForm::ShowGamePad( )
{
  PollNetworkState();
  if ( NetworkConnected )
  {
    if ( HostEnabled )
    {
	  ToggleAppTethering( false );
    }
	GamepadFrame->TetheringName = TetheringAppProfile->Text;
	GamepadFrame->GPTetheringManager->Enabled = true;
	GamepadFrame->GPTetheringAppProfile->Enabled = true;
	SetStage( GAMEPAD );
  }
  else
  {
    ShowMsgBox( NO_NETWORK_MESSAGE );
  }
}


void __fastcall TGameForm::HighScoresFrameCancelBTNClick( TObject *Sender )
{
  HighScoresFrame->CancelBTNClick( Sender );
}


void __fastcall TGameForm::HighScoresFrameContinueBTNClick( TObject *Sender )
{
  CloseHighScores();
}


void __fastcall TGameForm::HighScoresFrameOkayBTNClick( TObject *Sender )
{
  HighScoresFrame->OkayBTNClick( Sender );
}


void __fastcall TGameForm::HUDLayoutMouseMove( TObject *Sender, TShiftState Shift, float X, float Y )
{
  ClearButtons();
}

void __fastcall TGameForm::FireActionDownThread()
{
	  switch (CurrentStage)
	  {
		case MAIN_MENU:
			StartGame();
		  break;
		case INSTRUCTIONS:
			InitAndPlayGame();
		  break;
		case GAMEPLAY:
			UpDownEvent(NULL);
		  break;
		case LEVEL_COMPLETE:
			ContinueGame();
		  break;
		case GAME_OVER:
			CleanupGame(False);
			StartGame();
		  break;
		case HIGH_SCORES:
			CloseHighScores();
		  break;
		case SETTINGS:
			SaveAndExitSettings();
		  break;
		case GAMEPAD:
		  break;
		default:
		  break;
		}
}

void __fastcall TGameForm::FireActionDownExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  TThread::Synchronize(NULL, FireActionDownThread);
}

void __fastcall TGameForm::FireActionUpExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  UpUpEvent(Sender);
}


void __fastcall TGameForm::FireBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  UpDownEvent(Sender);
}


void __fastcall TGameForm::FireBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  UpUpEvent(Sender);
}


void __fastcall TGameForm::SaveSettings( )
{
  TMemIniFile *IniFile = new TMemIniFile( DataFilePath + "Settings.ini" );
  IniFile->WriteBool( "Settings", "MusicEnabled", MusicEnabled );
  IniFile->WriteBool( "Settings", "SoundEnabled", SoundEnabled );
  IniFile->WriteBool( "Settings", "GyroEnabled", GyroEnabled );
  IniFile->WriteBool( "Settings", "HostEnabled", HostEnabled );
  IniFile->WriteBool( "Settings", "FullScreenEnabled", FullScreenEnabled );
  IniFile->UpdateFile();
  delete IniFile;
}


void __fastcall TGameForm::LoadSettings( )
{
  TMemIniFile *IniFile = new TMemIniFile( DataFilePath + "Settings.ini" );
  MusicEnabled = IniFile->ReadBool( "Settings", "MusicEnabled", true );
  if ( MusicEnabled == false )
	MusicOffLine->Visible = true;
  SoundEnabled = IniFile->ReadBool( "Settings", "SoundEnabled", true );
  if ( SoundEnabled == false )
	SoundOffLine->Visible = true;
  GyroEnabled = IniFile->ReadBool( "Settings", "GyroEnabled", true );
  if ( GyroEnabled == false )
	GyroOffLine->Visible = true;
  HostEnabled = IniFile->ReadBool( "Settings", "HostEnabled", false );
  FullScreenEnabled = IniFile->ReadBool( "Settings", "FullScreenEnabled", false );
  if ( FullScreenEnabled == true )
	GameForm->FullScreen = true;
  delete IniFile;
  PollNetworkState();
  DelayedSettings->Enabled = true;
}


bool __fastcall TGameForm::CheckNetworkState( )
{
  bool result = false;
  try
  {
	if ( IdTCPClient1->Connected() == false )
	{
	  IdTCPClient1->ReadTimeout = 2000;
	  IdTCPClient1->ConnectTimeout = 2000;
	  IdTCPClient1->Host = NETWORK_ADDRESS;
	  IdTCPClient1->Port = 80;
	  IdTCPClient1->Connect();
	  IdTCPClient1->Disconnect();
	}
	result = true;
  }
  catch( Exception & E )
  {
  {
	result = false;
  }
  }
  return result;
}


void TGameForm::PollNetworkStateAnonThread()
{
  bool AResult = false;
  try
  {
	AResult = GameForm->CheckNetworkState();

	if (AResult==true)
	  NetworkConnected = true;
	else
	  NetworkConnected = false;
	NetworkChecking = false;
  }
  catch( Exception & E )
  {
	// do something if there is an exception
	NetworkConnected = false;
	NetworkChecking = false;
   }
}

void __fastcall TGameForm::PollNetworkState( )
{
  if ( NetworkChecking == false )
  {
	NetworkChecking = true;

	TThread::CreateAnonymousThread(AnonymousLambda(&this->PollNetworkStateAnonThread))->Start();

	//Application->ProcessMessages;
  }
}


void __fastcall TGameForm::FormCreate( TObject *Sender )
{
  IFMXApplicationEventService *FMXApplicationEventService = NULL;
  AssetLayout->Visible = false;

#if defined(_PLAT_MSWINDOWS)
  SettingsFilePath = ExtractFilePath(ParamStr(0));
#else
  SettingsFilePath = System::Ioutils::TPath::GetDocumentsPath() +
	System::Sysutils::PathDelim;
#endif
#if defined(_PLAT_MSWINDOWS)
  DataFilePath = ExtractFilePath(ParamStr(0));
#else
#if (defined(_PLAT_MACOS) && !defined(_PLAT_IOS))
  DataFilePath = System::Ioutils::TPath::GetHomePath() +
	System::Sysutils::PathDelim;
#else
  DataFilePath = System::Ioutils::TPath::GetDocumentsPath() +
	System::Sysutils::PathDelim;
#endif
#endif

  ScreenOrientation = GetScreenOrientation();

  TMessageListenerMethod OrientationChangedPointer = &(this->OrientationChanged);
  OrientationChangedId = TMessageManager::DefaultManager->SubscribeToMessage( __classid(TOrientationChangedMessage), OrientationChangedPointer );
  if ( TPlatformServices::Current->SupportsPlatformService( __uuidof(IFMXApplicationEventService), (void *)&FMXApplicationEventService ) )
	FMXApplicationEventService->SetApplicationEventHandler( HandleAppEvent );
  LoadSettings();
  AudioManager = new TAudioManager;
  RegisterSound( DataFilePath + "thruster.wav" );
  RegisterSound( DataFilePath + "explosion.wav" );
  RegisterSound( DataFilePath + "fail.wav" );
  RegisterSound( DataFilePath + "win.wav" );
  RegisterSound( DataFilePath + "alien.wav" );
  RegisterSound( DataFilePath + "collect.wav" );
  SetStage( MAIN_MENU );
}


void __fastcall TGameForm::FormDestroy( TObject *Sender )
{
  // CleanupGame(False);
  destroy(AudioManager);
  SaveSettings();
  TMessageManager::DefaultManager->Unsubscribe(__classid( TOrientationChangedMessage ), OrientationChangedId );
}

void __fastcall TGameForm::LevelExitProc()
{
  LevelFail();
  CleanupGame(False);
  ShowMainMenu();
}

void __fastcall TGameForm::ExitGameMsgBoxResultProc(TObject* Sender, const TModalResult AResult) //! ??method is not exists in pas-file
{
	switch (AResult) {
	  case mrYes:
		{
			TThread::Synchronize(NULL,LevelExitProc);
		}
		break;
	  case mrNo:
		{
		 // pressed no
		}
		break;
	}
}

void __fastcall TGameForm::FormKeyDown( TObject *Sender, WORD& Key, Char& KeyChar, TShiftState Shift )
{
  switch ( CurrentStage )
  {
	case GAMEPLAY:
	{
	  switch ( KeyChar )
	  {
		case 'Z': case 'z': case ' ':
		{
		  UpDownEvent(Sender);
		}
		break;
		case 'W': case 'w':
		{
		  UpDownEvent(Sender);
		}
		break;
		case 'A': case 'a':
		{
		  LeftDownEvent(Sender);
		}
		break;
		case 'D': case 'd':
		{
		  RightDownEvent(Sender);
		}
		break;
	  }
	  switch ( Key )
	  {
		case vkUp:
		{
		  UpDownEvent(Sender);
		}
		break;
		case vkLeft:
		{
		  LeftDownEvent(Sender);
		}
		break;
		case vkRight:
		{
		  RightDownEvent(Sender);
		}
		break;
	  }
	  if (Key!=vkHardwareBack) { Key = 0; }
	}
	break;
	case GAMEPAD:
	  GamepadFrame->HandleKeyDown( Sender, Key, KeyChar, Shift );
	break;
  }
  switch ( Key )
  {
	case vkHardwareBack:
	{
	  switch ( CurrentStage )
	  {
		case MAIN_MENU:
		{
			  // allow default functionality
		}
		break;
		case INSTRUCTIONS:
		{
		  Key = 0;
		  ShowMainMenu();
		}
		break;
		case GAMEPLAY: case LEVEL_COMPLETE:
		{
		  Key = 0;
		  ExitDialog(Sender);
		}
		break;
		case GAME_OVER:
		{
		  Key = 0;
		  CleanupGame( false );
		  ShowMainMenu();
		}
		break;
		case HIGH_SCORES:
		{
		  Key = 0;
		  CloseHighScores();
		}
		break;
	  }
	}
	break;
  }
}


void __fastcall TGameForm::FormKeyUp( TObject *Sender, WORD& Key, Char& KeyChar, TShiftState Shift )
{
  switch ( CurrentStage )
  {
	case GAMEPLAY:
	{
	  switch ( KeyChar )
	  {
		case 'Z': case 'z': case ' ':
		{
		  UpUpEvent(Sender);
		}
		break;
		case 'W': case 'w':
		{
		  UpUpEvent(Sender);
		}
		break;
		case 'A': case 'a':
		{
		  LeftUpEvent(Sender);
		}
		break;
		case 'D': case 'd':
		{
		  RightUpEvent(Sender);
		}
		break;
	  }
	  switch ( Key )
	  {
		case vkUp:
		{
		  UpUpEvent(Sender);
		}
		break;
		case vkLeft:
		{
		  LeftUpEvent(Sender);
		}
		break;
		case vkRight:
		{
		  RightUpEvent(Sender);
		}
		break;
	  }
	  if (Key!=vkHardwareBack) { Key = 0; }
	}
	break;
	case GAMEPAD:
	  GamepadFrame->HandleKeyUp( Sender, Key, KeyChar, Shift );
	break;
  }
}


void __fastcall TGameForm::FormMouseMove( TObject *Sender, TShiftState Shift, float X, float Y )
{
  ClearButtons();
}


void __fastcall TGameForm::FormResize( TObject *Sender )
{
  ReOrientgame();
}


void __fastcall TGameForm::ClearButtons( )
{
  switch ( CurrentStage )
  {
	case GAMEPAD:
	{
	  GamepadFrame->ClearButtons();
	}
	break;
  default:
  {
	LeftButtonDown = false;
	RightButtonDown = false;
	FireButtonDown = false;
  }
  }
}


void __fastcall TGameForm::InstructionsFrameContinueBTNClick( TObject *Sender )
{
  InitAndPlayGame();
}


void __fastcall TGameForm::InitAndPlayGame( )
{
  InitGame();
  InitPlayer();
  PlayGame();
}


void __fastcall TGameForm::LeftActionDownExecute( TObject *Sender )
{
  LeftDownEvent(Sender);
}


void __fastcall TGameForm::LeftActionUpExecute( TObject *Sender )
{
  LeftUpEvent(Sender);
}


void __fastcall TGameForm::LeftBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  LeftDownEvent(Sender);
}


void __fastcall TGameForm::LeftBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  LeftUpEvent(Sender);
}


void __fastcall TGameForm::MainMenuFrameGamepadBTNClick( TObject *Sender )
{
  ShowGamePad();
}


void __fastcall TGameForm::MainMenuFrameHighScoresBTNClick( TObject *Sender )
{
  ShowHighScores();
}


void __fastcall TGameForm::MainMenuFrameMoreGamesBTNClick( TObject *Sender )
{
  ShowMoreGames();
}


void __fastcall TGameForm::MainMenuFramePlayBTNClick( TObject *Sender )
{
  StartGame();
}


void __fastcall TGameForm::MainMenuFrameSettingsBTNClick( TObject *Sender )
{
  ShowSettings();
}


void __fastcall TGameForm::MusicBTNClick( TObject *Sender )
{
  MusicToggle();
}


void __fastcall TGameForm::MusicToggle( )
{
  if ( MusicEnabled )
  {
	MusicEnabled = false;
	MusicOffLine->Visible = true;
	StopMusic();
  }
  else
  {
	MusicEnabled = true;
	MusicOffLine->Visible = false;
	if ( CurrentStage != SETTINGS )
	  PlayMusic();
  }
}


void __fastcall TGameForm::RightActionDownExecute( TObject *Sender )
{
  RightDownEvent(Sender);
}


void __fastcall TGameForm::RightActionUpExecute( TObject *Sender )
{
  RightUpEvent(Sender);
}


void __fastcall TGameForm::RightBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  RightDownEvent(Sender);
}


void __fastcall TGameForm::RightBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  RightUpEvent(Sender);
}


void __fastcall TGameForm::StartGame( )
{
  if ( NetworkConnected == true )
  {
	ToggleAppTethering( true );
  }
  try
  {
	if ( GamepadFrame->GPTetheringManager->Enabled == true )
	  GamepadFrame->GPTetheringManager->Enabled = false;
	if ( GamepadFrame->GPTetheringAppProfile->Enabled == true )
	  GamepadFrame->GPTetheringAppProfile->Enabled = false;
  }
  catch( Exception & E )
  {
  }
  SetStage( INSTRUCTIONS );
}


void __fastcall TGameForm::StartGameLoop( )
{
  if ( GyroEnabled ) {
	MotionSensor->Active = true;
	GyroOffLine->Visible = false;
  } else {
	GyroOffLine->Visible = true;
    AccFireButtonDown = false;
    AccThrustCircle->Visible = false;
    AccLeftButtonDown = false;
    AccLeft->Visible = false;
    AccLeftButtonDown = false;
    AccRight->Visible = false;
    AccRightButtonDown = false;
  }
  GameLoop->Enabled = true;
}


void __fastcall TGameForm::StopGameLoop( )
{
  if ( GyroEnabled )
	MotionSensor->Active = false;
  GameLoop->Enabled = false;
  ClearButtons();
}


void __fastcall TGameForm::ShowMainMenu( )
{
  SetStage( MAIN_MENU );
}


void __fastcall TGameForm::ShowMoreGames( )
{
  // launch http://www.embarcadero.com/
}


void __fastcall TGameForm::ShowSettings( )
{
  PollNetworkState();
  SettingsFrame->HostSwitch->IsChecked = HostEnabled;
  SettingsFrame->GyroSwitch->IsChecked = GyroEnabled;
  SettingsFrame->FullScreenSwitch->IsChecked = FullScreenEnabled;
  SettingsFrame->SoundSwitch->IsChecked = SoundEnabled;
  SettingsFrame->MusicSwitch->IsChecked = MusicEnabled;
  SetStage( SETTINGS );
}


void __fastcall TGameForm::SoundBTNClick( TObject *Sender )
{
  SoundToggle();
}


void __fastcall TGameForm::SoundToggle( )
{
  if ( SoundEnabled )
  {
	SoundEnabled = false;
	SoundOffLine->Visible = true;
  }
  else
  {
	SoundEnabled = true;
	SoundOffLine->Visible = false;
  }
}


void __fastcall TGameForm::LevelComplete( )
{
  StopGameLoop();
  PlaySound( WIN_SFX );
  LevelCompleteFrame->InfoText->Text = "Score: " + IntToStr( PlayerData->Score ) + "\x0d\x0a";
  SetStage( LEVEL_COMPLETE );
}


void __fastcall TGameForm::LevelCompleteFrameContinueBTNClick( TObject *Sender )
{
  ContinueGame();
}


void __fastcall TGameForm::InitGame( )
{
  GroundList = new TStringList;
  GroundPool = new TStringList;
  ExplosionList = new TStringList;
  ExplosionPool = new TStringList;
  CollectList = new TStringList;
  CollectPool = new TStringList;
  CleanedUp = false;
  Ship->RotationAngle = 0;
  CenterPlayer();
}


void __fastcall TGameForm::InitPlayer( )
{
  PlayerData = new TPlayerData(this);
  PlayerData->Lives = PLAYER_LIVES;
  PlayerData->Stars = 0;
  PlayerData->SpeedX = 0;
  PlayerData->SpeedY = 0;
  PlayerData->Level = 1;
  PlayerData->Thrust = 0.5;
  PlayerData->Fuel = (int)( ScreenLayout->Width * ScreenLayout->Height / PLAYER_FUEL );
  PlayerData->VerticalVelocity = 0;
  PlayerData->HorizontalVelocity = 10;
  PlayerData->Landed = false;
  if ( LivesSet == false )
  {
	BitmapToRectangle( Ship->Fill->Bitmap->Bitmap, Lives1 );
	BitmapToRectangle( Ship->Fill->Bitmap->Bitmap, Lives2 );
	BitmapToRectangle( Ship->Fill->Bitmap->Bitmap, Lives3 );
  }
  DisplayLives( PlayerData->Lives );
  DisplayScore();
  CenterPlayer();
  Ship->TagObject = PlayerData;
  Ship->RotationAngle = 90;
}


void __fastcall TGameForm::BitmapToRectangle( TBitmap* B, TRectangle* R )
{
  R->Fill->Bitmap->WrapMode = TWrapMode::TileStretch;
  R->Fill->Kind = TBrushKind::Bitmap;
  R->Fill->Bitmap->Bitmap->Assign( B );
}


void __fastcall TGameForm::AnimateExplosionFinish( TObject *Sender )
{
  TBitmapListAnimation *animation = (TBitmapListAnimation *)Sender;
  ((TRectangle *)animation->Owner)->Visible = false;
}


void __fastcall TGameForm::CenterPlayer( )
{
  Ship->Position->X = ScreenLayout->Position->X + 20;
  Ship->Position->Y = ScreenLayout->Position->Y + 20;
  Ship->Visible = true;
  RScreenOut = false;
  LScreenOut = false;
}


void __fastcall TGameForm::CleanupGame( bool Continue )
{
  if ( CleanedUp == false )
  {
	GroundList->OwnsObjects = true;
	GroundPool->OwnsObjects = true;
	ExplosionList->OwnsObjects = true;
	ExplosionPool->OwnsObjects = true;
	CollectList->OwnsObjects = true;
	CollectPool->OwnsObjects = true;
	if ( Continue == false ) {
	  destroy(PlayerData);
	}
	destroy(GroundList);
	destroy(GroundPool);
	destroy(ExplosionList);
	destroy(ExplosionPool);
	destroy(CollectList);
	destroy(CollectPool);
	CleanedUp = true;
  }
}


void __fastcall TGameForm::PlayGame( )
{
  int I = 0;
  float NextX = 0.0, NextY = 0.0;
  TRectangle *R = NULL;
  int GroundType = 0;
  SetStage( GAMEPLAY );

  // Spawn collectable
  for ( int stop = 3, I = 1; I <= stop; I++)
  {
	CollectList->AddObject( "", SpawnCollectItem( RandomRange( 1, (int)( ScreenLayout->Width - CollectItem->Width ) ), RandomRange( 1, (int)( ScreenLayout->Height / 4 ) ), 0 ) );
  }

  // spawn ground and landing pads
  for ( int stop = 100, I = 0; I <= stop; I++)
  {
	if ( GroundList->Count > 0 )
	{
	  R = (TRectangle*) GroundList->Objects[GroundList->Count - 1] ;
	  NextX = R->Position->X + R->Width;
	  NextY = ScreenLayout->Position->Y + ScreenLayout->Height;
	  if ( NextX > ScreenLayout->Width )
		break;
	}
	else
	{
	  NextX = ScreenLayout->Position->X;
	  NextY = ScreenLayout->Position->Y + ScreenLayout->Height;
	}
	GroundType = Random( 5 );

	// ensure there is at least one landing pad
	if ( I == 4 )
	  GroundType = 1;
	GroundList->AddObject( "", SpawnGround( NextX, NextY, GroundType, Ship->Width ) );
  }
  CliffRect->BringToFront();
  Ship->RotationAngle = 45;
  StartGameLoop();
}


void __fastcall TGameForm::ContinueGame( )
{
  CleanupGame( true );
  InitGame();
  PlayerData->SpeedX = 0;
  PlayerData->SpeedY = 0;
  PlayerData->Level = PlayerData->Level + 1;
  PlayerData->Stars = 0;
  PlayerData->HorizontalVelocity = 10;
  PlayerData->VerticalVelocity = 0;
  PlayerData->Fuel = (int)( ScreenLayout->Width * ScreenLayout->Height / ( PLAYER_FUEL + ( PLAYER_FUEL * PlayerData->Level * 0.1 ) ) );
  PlayerData->Landed = false;
  PlayGame();
}


void __fastcall TGameForm::LevelFail( )
{
  StopGameLoop();
  PlayerData->SpeedX = 0;
  PlayerData->SpeedY = 0;
  PlaySound( FAIL_SFX );
}


void __fastcall TGameForm::GameOver( )
{
  SetStage( GAME_OVER );
  Application->ProcessMessages();
  HighScoresFrame->InitFrame();
  HighScoresFrame->AddScore( "", PlayerData->Score );
  SetStage( HIGH_SCORES );
}


void __fastcall TGameForm::ResetGround( )
{
  int I = 0;
  if ( GroundList->Count > 0 )
	for ( int stop = GroundList->Count - 1, I = 0; I <= stop; I++)
	{
	  TRectangle *GroundObj = (TRectangle*) GroundList->Objects[I];
	  GroundObj->Position->Y = ( ScreenLayout->Position->Y + ScreenLayout->Height ) - GroundObj->Height;
	}
}


void __fastcall TGameForm::ReOrientgame( )
{
  if ( CurrentStage == GAMEPLAY )
  {
	ResetGround();
	if ( Ship->Position->X > ScreenLayout->Width )
	{
	  Ship->Position->X = ScreenLayout->Width - Ship->Width - 25;
	}
  }
}


void __fastcall TGameForm::OrientationChanged(System::TObject * const Sender, System::Messaging::TMessageBase * const Msg)
{
  ReOrientgame();
}

void __fastcall TGameForm::ExitDialog(TObject *Sender)
{
			TDialogServiceAsync::MessageDialog("Exit the game?", TMsgDlgType::mtInformation,
			  TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo,
			  TMsgDlgBtn::mbYes, 0,
			  ExitGameMsgBoxResultProc
			  );
}

void __fastcall TGameForm::FormCloseQuery(TObject *Sender, bool &CanClose)
{
#if !defined(DEBUG_MODE)
#if defined(_PLAT_MSWINDOWS) || defined(_PLAT_MACOS)
		switch (CurrentStage)
		{
		case GAMEPLAY:
			  ExitDialog(Sender);
			  CanClose = false;
			break;
		}
#endif
#endif
}

TScreenOrientation __fastcall TGameForm::GetScreenOrientation( )
{
  TScreenOrientation result = TScreenOrientation::Portrait;
  IFMXScreenService *ScreenSvc = NULL;
  if ( TPlatformServices::Current->SupportsPlatformService( __uuidof(IFMXScreenService), (void *)&ScreenSvc ) )
  {
	result = ScreenSvc->GetScreenOrientation();
  }
  return result;
}

void __fastcall RegisterRenderingSetup( )
{
  IFMXRenderingSetupService *SetupService = NULL;
  if ( TPlatformServices::Current->SupportsPlatformService( __uuidof(IFMXRenderingSetupService), (void *)&SetupService ) )
  {
	SetupService->Subscribe(new TRenderingSetupCallbackRef());
  }
  // There is also SetupService->Unsubscribe, which removes the hook.
}

void uGame_initialization()
{
  RegisterRenderingSetup();
  Randomize();
}

class uGame_unit
{
public:
uGame_unit()
{
  // enables Metal API on iOS and macOS
  GlobalUseMetal = True;

  // enables the GPU on Windows
  //GlobalUseGPUCanvas := True;
  uGame_initialization();
}
};
static uGame_unit _uGame_unit;


//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

