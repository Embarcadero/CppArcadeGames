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

#define DEBUG_MODE

#include "uGame.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "uMainMenu"
#pragma link "uInstructions"
#pragma link "uLevelComplete"
#pragma link "uGameOver"
#pragma link "uHighScores"
#pragma link "uSettings"
#pragma link "uGamepad"
#pragma resource "*.fmx"
TGameForm *GameForm;

template<class T> inline void destroy(T*& p) { delete p; p = nullptr; }
#if defined(_PLAT_IOS) || defined(_PLAT_ANDROID)
namespace std {
// https://stackoverflow.com/questions/17902405/how-to-implement-make-unique-function-in-c11
	template<typename T, typename... Args>
	std::unique_ptr<T> make_unique(Args&&... args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
}
#endif

//---------------------------------------------------------------------------
__fastcall TGameForm::TGameForm(TComponent* Owner)
	: TForm(Owner)
{
}

void __fastcall TGameForm::ShowMsgBox( String S )
{
  TDialogServiceAsync::MessageDialog(S, TMsgDlgType::mtInformation, TMsgDlgButtons() << TMsgDlgBtn::mbOK, TMsgDlgBtn::mbOK, 0, [&](const TModalResult AResult)->void {});
}

bool __fastcall TGameForm::HandleAppEvent( TApplicationEvent AAppEvent, TObject* AContext )
{
  switch ( AAppEvent )
  {
	//TApplicationEvent::FinishedLaunching: Log('Finished Launching');
	case TApplicationEvent::BecameActive:
	{
	  if ( ( CurrentStage == GAMEPLAY ) && ( GameLoop->Enabled == false ) )
	  {
		StartGameLoop();
		PauseBitmapListAnimations( false );
	  }
	}
	break;
	//TApplicationEvent::WillBecomeInactive: Log('Will Become Inactive');

	case TApplicationEvent::EnteredBackground:
	{
	  if ( ( CurrentStage == GAMEPLAY ) && ( GameLoop->Enabled == true ) )
	  {
		PauseBitmapListAnimations( true );
		StopGameLoop();
	  }
	}
	break;
	//TApplicationEvent::WillBecomeForeground: Log('Will Become Foreground');
	//TApplicationEvent::WillTerminate: Log('Will Terminate');
	//TApplicationEvent::LowMemory: Log('Low Memory');
	//TApplicationEvent::TimeChange: Log('Time Change');
	//TApplicationEvent::OpenURL: Log('Open URL');
  }
  return true;
}


void __fastcall TGameForm::PauseBitmapListAnimations( bool Value )
{
  int I = 0;
  for ( int stop = 0, I = ExplosionList->Count - 1; I >= stop; I--)
  {
	auto *BLAObj = (TRectangle*)( ExplosionList->Objects[I] );
	((TBitmapListAnimation*)( BLAObj->TagObject ))->Pause = Value;
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
#if defined(_PLAT_IOS) || defined(_PLAT_MACOS)
 return 0.75;
#else
 return 1;
#endif
}


void __fastcall TGameForm::PlayMusic( )
{
  if (MusicPlayer->Media)
  {
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


void __fastcall TGameForm::ShowHighScores( )
{
  HighScoresFrame->InitFrame();
  SetStage( HIGH_SCORES );
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


void __fastcall TGameForm::AddScore( int I )
{
  PlayerData->Score = PlayerData->Score + I;
  ScoreLBL->Text = IntToStr( PlayerData->Score );
}


TRectangle* __fastcall TGameForm::GetPoolObj( TStringList *Pool )
{
  TRectangle *result = nullptr;
  if ( Pool->Count > 0 )
  {
    result = (TRectangle*)( Pool->Objects[Pool->Count - 1] );
    Pool->Delete( Pool->Count - 1 );
  }
  else
  {
    result = new TRectangle( nullptr );
  }
  return result;
}


void __fastcall TGameForm::SetPoolObj( TStringList *Pool, String Name, TRectangle *Obj )
{
  Pool->AddObject( Name, Obj );
  Obj->Parent = nullptr;
}


TRectangle* __fastcall TGameForm::SpawnProj( TRectangle *Source, float X, float Y, float Angle )
{
  auto *R = (TRectangle*)GetPoolObj( ProjPool );
  R->Width = 5;
  R->Height = 5;
  R->RotationAngle = Angle - 90;
  R->Stroke->Kind = TBrushKind::None;
  R->Fill->Bitmap->WrapMode = TWrapMode::TileStretch;
  R->Fill->Kind = TBrushKind::Bitmap;
  R->Fill->Bitmap->Bitmap->Assign( Projectile->Fill->Bitmap->Bitmap );
  float RAngle = R->RotationAngle * PI / 180;
 // set the center X,Y and then travel 25 out from the center on the angle
  R->Position->X = X + ( Source->Width / 2 ) - (  R->Width / 2 ) + 25 * Cos( RAngle );
  R->Position->Y = Y + ( Source->Height / 2 ) - ( R->Height / 2 ) + 25 * Sin( RAngle );
  R->Tag = PROJ_SPEED;
  R->TagFloat = 0;
  R->Parent = this;
  R->SendToBack();
  return R;
}


TRectangle* __fastcall TGameForm::SpawnEnemy( float X, float Y, float Angle, int Speed )
{
  auto *R = (TRectangle*)GetPoolObj( EnemyPool );
  R->Width = 50;
  R->Height = 50;
  R->RotationAngle = Angle;
  R->Position->X = X;
  R->Position->Y = Y;
  R->Tag = Speed;
  R->TagFloat = 0;
  R->Stroke->Kind = TBrushKind::None;
  R->Fill->Bitmap->WrapMode = TWrapMode::TileStretch;
  R->Fill->Kind = TBrushKind::Bitmap;
  R->Fill->Bitmap->Bitmap->Assign( Enemy->Fill->Bitmap->Bitmap );
  if ( Speed > 0 )
	R->Fill->Bitmap->Bitmap->FlipHorizontal();
  R->Parent = this;
  R->SendToBack();
  return R;
}


TRectangle* __fastcall TGameForm::SpawnEnemyProj( TRectangle *Source, float X, float Y, float Angle )
{
  auto *R = (TRectangle*)GetPoolObj( ProjPool );
  R->Width = 5;
  R->Height = 5;
  R->RotationAngle = Angle - 90;
  R->Stroke->Kind = TBrushKind::None;
  R->Fill->Bitmap->WrapMode = TWrapMode::TileStretch;
  R->Fill->Kind = TBrushKind::Bitmap;
  R->Fill->Bitmap->Bitmap->Assign( EnemyProjectile->Fill->Bitmap->Bitmap );
  float RAngle = R->RotationAngle * PI / 180;
 // set the center X,Y and then travel 25 out from the center on the angle
  R->Position->X = X + ( Source->Width / 2 ) - ( R->Width / 2 ) + 25 * Cos( RAngle );
  R->Position->Y = Y + ( Source->Height / 2 ) - ( R->Height / 2 ) + 25 * Sin( RAngle );
  R->Tag = ENEMYPROJ_SPEED;
  R->TagFloat = 0;
  R->Parent = this;
  R->SendToBack();
  return R;
}


TRectangle* __fastcall TGameForm::SpawnRock( float X, float Y, int Size )
{
  TRectangle *RockGraphic = nullptr;
  auto *R = (TRectangle*)GetPoolObj( RockPool );
  R->Stroke->Kind = TBrushKind::None;
  R->Fill->Kind = TBrushKind::None;
  R->RotationAngle = Random( 360 );
  switch ( Size )
  {
	case 1:
	{
	  R->Width = 100;
	  R->Height = 100;
	  R->TagString = 'l';
	  R->Tag = ROCK_SPEED;
	}
	break;
	case 2:
	{
	  R->Width = 50;
	  R->Height = 50;
	  R->TagString = 'm';
	  R->Tag = ROCK_SPEED + 1;
	}
	break;
	case 3:
	{
	  R->Width = 20;
	  R->Height = 20;
	  R->TagString = 's';
	  R->Tag = ROCK_SPEED + 2;
	}
	break;
  }
  R->Position->X = X;
  R->Position->Y = Y;
  if ( !R->TagObject )
  {
	RockGraphic = new TRectangle( R );
	RockGraphic->Parent = R;
  }
  else
  {
	RockGraphic = (TRectangle*)R->TagObject;
  }
  RockGraphic->Width = R->Width;
  RockGraphic->Height = R->Height;
  RockGraphic->Position->X = ( R->Width / 2 ) - ( RockGraphic->Width / 2 );
  RockGraphic->Position->Y = ( R->Height / 2 ) - ( RockGraphic->Height / 2 );
  RockGraphic->Stroke->Kind = TBrushKind::None;
  RockGraphic->Fill->Bitmap->WrapMode = TWrapMode::TileStretch;
  RockGraphic->Fill->Kind = TBrushKind::Bitmap;
  switch ( Random( 3 ) )
  {
	case 0:
	  RockGraphic->Fill->Bitmap->Bitmap->Assign( Asteroid1->Fill->Bitmap->Bitmap );
	break;
	case 1:
	  RockGraphic->Fill->Bitmap->Bitmap->Assign( Asteroid2->Fill->Bitmap->Bitmap );
	break;
	case 2:
	  RockGraphic->Fill->Bitmap->Bitmap->Assign( Asteroid3->Fill->Bitmap->Bitmap );
	break;
  }
  if (!R->TagObject)
  {
	R->TagObject = RockGraphic;
  }
  R->TagFloat = 0;
  R->Parent = this;
  R->SendToBack();
  return R;
}


TRectangle* __fastcall TGameForm::SpawnCollectItem( float X, float Y, int Size )
{
  TRectangle *CollectGraphic = nullptr;
  auto *R = (TRectangle*)GetPoolObj( CollectPool );
  R->Stroke->Kind = TBrushKind::None;
  R->Fill->Kind = TBrushKind::None;
  R->RotationAngle = Random( 360 );
  R->Width = 50;
  R->Height = 50;
  R->Position->X = X;
  R->Position->Y = Y;
  if (!R->TagObject)
  {
	CollectGraphic = new TRectangle( R );
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
  TBitmapListAnimation *AE = nullptr;
  auto *R = (TRectangle*)GetPoolObj( ExplosionPool );
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
	AE = (TBitmapListAnimation*)R->TagObject;
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


void __fastcall TGameForm::ShowHUD( )
{
  Ship->Visible = true;
  HUDLayout->Visible = true;
  #ifdef DEBUG_MODE
  FPSLBL->Visible = true;
  #endif
  HUDLayout->BringToFront();
}


void __fastcall TGameForm::HideHUD( )
{
  Ship->Visible = false;
  Thruster->Visible = false;
  HUDLayout->Visible = false;
  FPSLBL->Visible = false;
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
      HideHUD();
      StopMusic();
    }
    break;
    case INSTRUCTIONS:
    {
      MainMenuFrame->Visible = false;
      GameOverFrame->Visible = false;
      InstructionsFrame->Visible = true;
      HideHUD();
      StopMusic();
    }
    break;
    case GAMEPLAY:
    {
      InstructionsFrame->Visible = false;
      LevelCompleteFrame->Visible = false;
      ShowHUD();
	  PlayMusic();
    }
    break;
    case LEVEL_COMPLETE:
    {
      LevelCompleteFrame->Visible = true;
      HideHUD();
    }
    break;
    case GAME_OVER:
    {
      HighScoresFrame->Visible = false;
      GameOverFrame->Visible = true;
      HideHUD();
    }
    break;
    case HIGH_SCORES:
    {
      MainMenuFrame->Visible = false;
      GameOverFrame->Visible = false;
      HighScoresFrame->Visible = true;
	  HighScoresFrame->BringToFront();
      HideHUD();
    }
    break;
    case SETTINGS:
	{
      MainMenuFrame->Visible = false;
      SettingsFrame->Visible = true;
      HideHUD();
      StopMusic();
    }
    break;
    case GAMEPAD:
    {
      MainMenuFrame->Visible = false;
      GamepadFrame->Visible = true;
      HideHUD();
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


bool __fastcall TGameForm::IntersectCircle( TRectangle *R1, TRectangle *R2 )
{
  const float Distance = R1->Position->Point.Distance( R2->Position->Point );
  if ( Distance < ( ( R1->Width / 2 ) + ( R2->Width / 2 ) ) )
  {
	return true;
  } else {
	return false;
  }
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
  if ( AccY > - 4 )
  {
    AccUp->Visible = true;
    AccUpButtonDown = true;
  }
  else
  {
    AccUp->Visible = false;
    AccUpButtonDown = false;
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
  TRectangle *ProjObj = nullptr;
  TRectangle *RockObj = nullptr;
  TRectangle *ExplosionObj = nullptr;
  float RockObjCenterX = 0.0, RockObjCenterY = 0.0;
  TRectangle *EnemyObj = nullptr;
  TRectangle *EnemyProjObj = nullptr;
  TRectangle *CollectObj = nullptr;
  unsigned int Time = 0;
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
  if ( ( RockList->Count == 0 ) && ( EnemyList->Count == 0 ) )
  {
    LevelComplete();
    return;
  }
  if ( CanSwapXY == true )
  {
    SwapXY();
    CanSwapXY = false;
  }

  // process accelerometer data
  if ( MotionSensor->Active == true )
  {
    ProcessAccelerometer();
  }

  // Handle player movement and firing
  PlayerData->FireInterval = PlayerData->FireInterval + 1;
  if ( LeftButtonDown || AccLeftButtonDown )
    Ship->RotationAngle = Ship->RotationAngle - PlayerData->RotationSpeed;
  if ( RightButtonDown || AccRightButtonDown )
    Ship->RotationAngle = Ship->RotationAngle + PlayerData->RotationSpeed;
  if ( ForwardButtonDown || AccUpButtonDown )
  {
	PlayerData->SpeedX = PlayerData->SpeedX + ( PlayerData->Thrust * Sin( Ship->RotationAngle * ( PI / 180 ) ) );
	PlayerData->SpeedY = PlayerData->SpeedY + ( PlayerData->Thrust * Cos( Ship->RotationAngle * ( PI / 180 ) ) );
  }
  else
  {
    PlayerData->SpeedX = PlayerData->SpeedX * PlayerData->Decay;
    PlayerData->SpeedY = PlayerData->SpeedY * PlayerData->Decay;
  }
  if ( FireButtonDown )
  {
    FirePlayerProj();
  }
  if ( ForwardButtonDown || AccUpButtonDown )
  {
	Thruster->Visible = true;
  }
  else
  {
	Thruster->Visible = false;
  }
  PlayerData->CurrentSpeed = Sqrt( ( PlayerData->SpeedX * PlayerData->SpeedX ) + ( PlayerData->SpeedY * PlayerData->SpeedY ) );
  if ( PlayerData->CurrentSpeed > PlayerData->MaxSpeed )
  {
    PlayerData->SpeedX = PlayerData->SpeedX * ( PlayerData->MaxSpeed / PlayerData->CurrentSpeed );
    PlayerData->SpeedY = PlayerData->SpeedY * ( PlayerData->MaxSpeed / PlayerData->CurrentSpeed );
  }
  Ship->Position->X = Ship->Position->X + PlayerData->SpeedX;
  Ship->Position->Y = Ship->Position->Y - PlayerData->SpeedY;
  if ( Ship->ParentedRect.CenterPoint().X >= ( ScreenLayout->Width + ( Ship->Width / 2 ) ) )
  {
	Ship->Position->X = ( ScreenLayout->Position->X + 1 ) - ( Ship->Width / 2 );
  }
  if ( Ship->ParentedRect.CenterPoint().Y >= ( ScreenLayout->Height + ( Ship->Height / 2 ) ) )
  {
	Ship->Position->Y = ( ScreenLayout->Position->Y + 1 ) - ( Ship->Height / 2 );
  }
  if ( Ship->ParentedRect.CenterPoint().X <= ( ScreenLayout->Position->X - ( Ship->Width / 2 ) ) )
  {
    Ship->Position->X = ( ScreenLayout->Width - 1 );
  }
  if ( Ship->ParentedRect.CenterPoint().Y <= ( ScreenLayout->Position->Y - ( Ship->Height / 2 ) ) )
  {
    Ship->Position->Y = ( ScreenLayout->Height - 1 );
  }

  // Handle explosions
  if ( ExplosionList->Count > 0 )
    for ( int stop = 0, I = ExplosionList->Count - 1; I >= stop; I--)
	{
      ExplosionObj = (TRectangle*)( ExplosionList->Objects[I] );
      ExplosionObj->TagFloat = ExplosionObj->TagFloat + 0.1;
	  if ( ExplosionObj->TagFloat > (double)ExplosionObj->Tag )
      {
          //ProjList.Objects[I] := nil;
		SetPoolObj( ExplosionPool, ExplosionList->Strings[I], ExplosionObj );
        ExplosionList->Delete( I );
      }
    }

  // Handle player projectiles
  if ( ProjList->Count > 0 )
    for ( int stop = 0, I = ProjList->Count - 1; I >= stop; I--)
    {
      ProjObj = (TRectangle*)( ProjList->Objects[I] );
	  ProjAngle = ProjObj->RotationAngle * PI / 180;
	  ProjObj->Position->X = ProjObj->Position->X + (double)ProjObj->Tag * Cos( ProjAngle );
	  ProjObj->Position->Y = ProjObj->Position->Y + (double)ProjObj->Tag * Sin( ProjAngle );
	  if ( RockList->Count > 0 )
        for ( int stop = RockList->Count - 1, II = 0; II <= stop; II++)
        {
          RockObj = (TRectangle*)( RockList->Objects[II] );
          if ( IntersectRect( RockObj->ParentedRect, ProjObj->ParentedRect ) )
          {
            RockObj->TagFloat = RockObj->TagFloat + 1;
			ProjObj->TagFloat = PlayerData->ProjDuration + 1;
			break;
          }
        }
      if ( EnemyList->Count > 0 )
        for ( int stop = EnemyList->Count - 1, II = 0; II <= stop; II++)
        {
          EnemyObj = (TRectangle*)( EnemyList->Objects[II] );
          if ( IntersectRect( EnemyObj->ParentedRect, ProjObj->ParentedRect ) )
          {
            EnemyObj->TagFloat = EnemyObj->TagFloat + 1;
            ProjObj->TagFloat = PlayerData->ProjDuration + 1;
			break;
          }
        }
	  if ( ProjObj->ParentedRect.CenterPoint().X >= ( ScreenLayout->Width + ( ProjObj->Width / 2 ) ) )
      {
		ProjObj->Position->X = ( ScreenLayout->Position->X + 1 ) - ( ProjObj->Width / 2 );
      }
	  if ( ProjObj->ParentedRect.CenterPoint().Y >= ( ScreenLayout->Height + ( ProjObj->Height / 2 ) ) )
      {
		ProjObj->Position->Y = ( ScreenLayout->Position->Y + 1 ) - ( ProjObj->Height / 2 );
      }
	  if ( ProjObj->ParentedRect.CenterPoint().X <= ( ScreenLayout->Position->X - ( ProjObj->Width / 2 ) ) )
      {
        ProjObj->Position->X = ( ScreenLayout->Width - 1 );
	  }
      if ( ProjObj->ParentedRect.CenterPoint().Y <= ( ScreenLayout->Position->Y - ( ProjObj->Height / 2 ) ) )
      {
        ProjObj->Position->Y = ( ScreenLayout->Height - 1 );
      }
      ProjObj->TagFloat = ProjObj->TagFloat + 0.1;
      if ( ProjObj->TagFloat > PlayerData->ProjDuration )
      {
		SetPoolObj( ProjPool, ProjList->Strings[I], ProjObj );
        ProjList->Delete( I );
      }
    }

  // Handle rocks
  if ( RockList->Count > 0 )
    for ( int stop = 0, I = RockList->Count - 1; I >= stop; I--)
    {
      RockObj = (TRectangle*)( RockList->Objects[I] );
      if ( PlayerData->Invulnerable == 0 )
        if ( IntersectRect( RockObj->ParentedRect, Ship->ParentedRect ) )
          if ( IntersectCircle( RockObj, Ship ) )
          {
            PlayerHit();
            RockObj->TagFloat = RockObj->TagFloat + 1;
          }
      if ( RockObj->TagFloat > 0 )
	  {
		RockObjCenterX = RockObj->Position->X + ( RockObj->Width / 2 );
		RockObjCenterY = RockObj->Position->Y + ( RockObj->Height / 2 );
        CreateExplosion( RockObjCenterX, RockObjCenterY );
        if ( RockObj->TagString == 'l' )
        {
          AddScore( 100 );
          RockList->AddObject( "", SpawnRock( RockObjCenterX, RockObjCenterY, 2 ) );
          RockList->AddObject( "", SpawnRock( RockObjCenterX, RockObjCenterY, 2 ) );
        }
        else
          if ( RockObj->TagString == 'm' )
          {
            AddScore( 50 );
            RockList->AddObject( "", SpawnRock( RockObjCenterX, RockObjCenterY, 3 ) );
			RockList->AddObject( "", SpawnRock( RockObjCenterX, RockObjCenterY, 3 ) );
          }
          else
          {
            AddScore( 20 );
          }
		SetPoolObj( RockPool, RockList->Strings[I], RockObj );
        RockList->Delete( I );
      }
      else
	  {
		((TRectangle*)(RockObj->TagObject))->RotationAngle = ((TRectangle*)(RockObj->TagObject))->RotationAngle+1;
		RockAngle = RockObj->RotationAngle * PI / 180;
		RockObj->Position->X = RockObj->Position->X + (double)RockObj->Tag * Cos( RockAngle );
		RockObj->Position->Y = RockObj->Position->Y + (double)RockObj->Tag * Sin( RockAngle );
		if ( RockObj->ParentedRect.CenterPoint().X >= ( ScreenLayout->Width + ( RockObj->Width / 2 ) ) )
		{
		  RockObj->Position->X = ( ScreenLayout->Position->X + 1 ) - ( RockObj->Width / 2 );
		}
		if ( RockObj->ParentedRect.CenterPoint().Y >= ( ScreenLayout->Height + ( RockObj->Height / 2 ) ) )
		{
		  RockObj->Position->Y = ( ScreenLayout->Position->Y + 1 ) - ( RockObj->Height / 2 );
		}
		if ( RockObj->ParentedRect.CenterPoint().X <= ( ScreenLayout->Position->X - ( RockObj->Width / 2 ) ) )
		{
		  RockObj->Position->X = ( ScreenLayout->Width - 1 );
		}
		if ( RockObj->ParentedRect.CenterPoint().Y <= ( ScreenLayout->Position->Y - ( RockObj->Height / 2 ) ) )
		{
		  RockObj->Position->Y = ( ScreenLayout->Height - 1 );
		}
	  }
	}

   // Handle enemy movement and firing
  if ( EnemyList->Count > 0 )
	for ( int stop = 0, I = EnemyList->Count - 1; I >= stop; I--)
	{
	  EnemyObj = (TRectangle*)( EnemyList->Objects[I] );
	  EnemyAngle = EnemyObj->RotationAngle * PI / 180;
	  EnemyObj->Position->X = EnemyObj->Position->X + (double)EnemyObj->Tag * Cos( EnemyAngle );
	  EnemyObj->Position->Y = EnemyObj->Position->Y + (double)EnemyObj->Tag * Sin( EnemyAngle );
	  if ( PlayerData->Invulnerable == 0 )
		if ( IntersectRect( Ship->ParentedRect, EnemyObj->ParentedRect ) )
		{
		  PlayerHit();
		  EnemyObj->TagFloat = EnemyObj->TagFloat + 1;
		}
	  if ( Random( 150 ) == 1 )
	  {
		EnemyProjList->AddObject( "", SpawnEnemyProj( EnemyObj, EnemyObj->Position->X, EnemyObj->Position->Y, RandomRange( - 180, 180 ) ) );
		PlaySound( ALIEN_SFX );
	  }
	  if ( ( EnemyObj->TagFloat > 0 ) || ( EnemyObj->Position->X > ScreenLayout->Width ) || ( EnemyObj->Position->Y > ScreenLayout->Height ) || ( EnemyObj->Position->X < ScreenLayout->Position->X ) || ( EnemyObj->Position->Y < ScreenLayout->Position->Y ) )
	  {
		if ( EnemyObj->TagFloat > 0 )
		{
		  CreateExplosion( EnemyObj->Position->X + ( EnemyObj->Width / 2 ), EnemyObj->Position->Y + ( EnemyObj->Height / 2 ) );
		  AddScore( 500 );
		}
		SetPoolObj( EnemyPool, EnemyList->Strings[I], EnemyObj );
		EnemyList->Delete( I );
	  }
	}

  // Handle enemy projectiles
  if ( EnemyProjList->Count > 0 )
    for ( int stop = 0, I = EnemyProjList->Count - 1; I >= stop; I--)
    {
	  EnemyProjObj = (TRectangle*)( EnemyProjList->Objects[I] );
	  EnemyProjAngle = EnemyProjObj->RotationAngle * PI / 180;
	  EnemyProjObj->Position->X = EnemyProjObj->Position->X + (double)EnemyProjObj->Tag * Cos( EnemyProjAngle );
	  EnemyProjObj->Position->Y = EnemyProjObj->Position->Y + (double)EnemyProjObj->Tag * Sin( EnemyProjAngle );
      if ( PlayerData->Invulnerable == 0 )
        if ( IntersectRect( Ship->ParentedRect, EnemyProjObj->ParentedRect ) )
        {
          PlayerHit();
          EnemyProjObj->TagFloat = EnemyProjObj->TagFloat + 1;
        }
      if ( ( EnemyProjObj->TagFloat > 0 ) || ( EnemyProjObj->Position->X > ScreenLayout->Width ) || ( EnemyProjObj->Position->Y > ScreenLayout->Height ) || ( EnemyProjObj->Position->X < ScreenLayout->Position->X ) || ( EnemyProjObj->Position->Y < ScreenLayout->Position->Y ) )
      {
		SetPoolObj( EnemyProjPool, EnemyProjList->Strings[I], EnemyProjObj );
        EnemyProjList->Delete( I );
      }
    }

    // Spawn enemies
  if ( EnemyList->Count < 2 )
  {
    if ( Random( 100 ) == 1 )
    {
      if ( Random( 2 ) == 1 )
      {
		EnemyList->AddObject( "", SpawnEnemy( ScreenLayout->Width - 1, RandomRange( 1, (int)( ScreenLayout->Height ) ), 0, ENEMY_SPEED * - 1 ) );
      }
	  else
      {
		EnemyList->AddObject( "", SpawnEnemy( ScreenLayout->Position->X + 1, RandomRange( 1, (int)( ScreenLayout->Height ) ), 0, ENEMY_SPEED ) );
      }
      PlaySound( ALIEN_SFX );
    }
  }

  // Handle collectable items
  if ( CollectList->Count > 0 )
    for ( int stop = 0, I = CollectList->Count - 1; I >= stop; I--)
    {
	  CollectObj = (TRectangle*)( CollectList->Objects[I] );
	  CollectAngle = CollectObj->RotationAngle * PI / 180;
	  CollectObj->Position->X = CollectObj->Position->X + (double)CollectObj->Tag * Cos( CollectAngle );
	  CollectObj->Position->Y = CollectObj->Position->Y + (double)CollectObj->Tag * Sin( CollectAngle );
	  if ( CollectObj->ParentedRect.CenterPoint().X >= ( ScreenLayout->Width + ( CollectObj->Width / 2 ) ) )
	  {
		CollectObj->Position->X = ( ScreenLayout->Position->X + 1 ) - ( CollectObj->Width / 2 );
	  }
	  if ( CollectObj->ParentedRect.CenterPoint().Y >= ( ScreenLayout->Height + ( CollectObj->Height / 2 ) ) )
      {
		CollectObj->Position->Y = ( ScreenLayout->Position->Y + 1 ) - ( CollectObj->Height / 2 );
      }
      if ( CollectObj->ParentedRect.CenterPoint().X <= ( ScreenLayout->Position->X - ( CollectObj->Width / 2 ) ) )
      {
        CollectObj->Position->X = ( ScreenLayout->Width - 1 );
      }
	  if ( CollectObj->ParentedRect.CenterPoint().Y <= ( ScreenLayout->Position->Y - ( CollectObj->Height / 2 ) ) )
	  {
        CollectObj->Position->Y = ( ScreenLayout->Height - 1 );
      }
      if ( IntersectRect( Ship->ParentedRect, CollectObj->ParentedRect ) )
      {
        AddScore( 5000 );
        CollectObj->TagFloat = COLLECTITEM_DURATION + 1;
        PlaySound( COLLECT_SFX );
      }
      CollectObj->TagFloat = CollectObj->TagFloat + 0.1;
      if ( CollectObj->TagFloat > COLLECTITEM_DURATION )
      {
		SetPoolObj( CollectPool, CollectList->Strings[I], CollectObj );
        CollectList->Delete( I );
      }
    }

    // Spawn collectable
  if ( CollectList->Count < 1 )
  {
    if ( Random( 100 ) == 1 )
    {
	  CollectList->AddObject( "", SpawnCollectItem( RandomRange( 1, (int)( ScreenLayout->Width - 1 ) ), RandomRange( 1, (int)( ScreenLayout->Height - 1 ) ), 0 ) );
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
    AccUp->Visible = false;
    AccUpButtonDown = false;
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
  PlayerData->Health = PlayerData->Health - 1;
  DisplayHealth( PlayerData->Health );
  CreateExplosion( Ship->Position->X + ( Ship->Width / 2 ), Ship->Position->Y + ( Ship->Height / 2 ) );
  if ( PlayerData->Health <= 0 )
  {
    CreateExplosion( Ship->Position->X + RandomRange( 1, (int)( Ship->Width ) ), Ship->Position->Y + RandomRange( 1, (int)( Ship->Height ) ) );
    PlayerData->Lives = PlayerData->Lives - 1;
    PlayerData->Health = PLAYER_HEALTH;
    Health1->Visible = true;
    Health2->Visible = true;
    Health3->Visible = true;
    PlayerData->SpeedX = 0;
    PlayerData->SpeedY = 0;
    PlayerData->CurrentSpeed = 0;
    DisplayLives( PlayerData->Lives );
	if ( PlayerData->Lives > 0 )
    {
      PlayerData->Invulnerable = PlayerData->InvulnerableInterval;
      Ship->Opacity = Ship->Opacity - 0.25;
      InvulnerableTimer->Enabled = true;
      CenterPlayer();
      PlaySound( FAIL_SFX );
    }
  }
}


void __fastcall TGameForm::DisplayLives( int Lives )
{
  Lives1->Visible = Lives > 0;
  Lives2->Visible = Lives > 1;
  Lives3->Visible = Lives > 2;
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


void __fastcall TGameForm::DisplayHealth( int Health )
{
  Health1->Visible = Health > 0;
  Health2->Visible = Health > 1;
  Health3->Visible = Health > 2;
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


void __fastcall TGameForm::HighScoresFrameContinueBTNClick( TObject *Sender )
{
  CloseHighScores();
}


void __fastcall TGameForm::FireBTNClick( TObject *Sender )
{
  FirePlayerProj();
}


void __fastcall TGameForm::FireBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  FireButtonDown = true;
}


void __fastcall TGameForm::FireBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  FireButtonDown = false;
}


void __fastcall TGameForm::SaveSettings( )
{
  std::unique_ptr<TMemIniFile> IniFile(new TMemIniFile( DataFilePath + "Settings.ini" ));
  IniFile->WriteBool( "Settings", "MusicEnabled", MusicEnabled );
  IniFile->WriteBool( "Settings", "SoundEnabled", SoundEnabled );
  IniFile->WriteBool( "Settings", "GyroEnabled", GyroEnabled );
  IniFile->WriteBool( "Settings", "HostEnabled", HostEnabled );
  IniFile->WriteBool( "Settings", "FullScreenEnabled", FullScreenEnabled );
  IniFile->UpdateFile();
}


void __fastcall TGameForm::LoadSettings( )
{
  std::unique_ptr<TMemIniFile> IniFile(new TMemIniFile( DataFilePath + "Settings.ini" ));
  MusicEnabled = IniFile->ReadBool( "Settings", "MusicEnabled", true );
  if ( MusicEnabled == false )
    MusicOffLine->Visible = true;
  SoundEnabled = IniFile->ReadBool( "Settings", "SoundEnabled", true );
  if ( SoundEnabled == false )
	SoundOffLine->Visible = true;
  GyroEnabled = IniFile->ReadBool( "Settings", "GyroEnabled", false );
  if ( GyroEnabled == false )
    GyroOffLine->Visible = true;
  HostEnabled = IniFile->ReadBool( "Settings", "HostEnabled", false );
  FullScreenEnabled = IniFile->ReadBool( "Settings", "FullScreenEnabled", false );
  if ( FullScreenEnabled == true )
	GameForm->FullScreen = true;
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
	result = false;
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
  IFMXApplicationEventService *FMXApplicationEventService = nullptr;
  IFMXScreenService *ScreenSvc = nullptr;
  TPointF Size;
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

  if ( TPlatformServices::Current->SupportsPlatformService( __uuidof( IFMXScreenService), (void *)&( ScreenSvc ) ) )
  {
	Size = ScreenSvc->GetScreenSize();
  }
  if ( RoundTo( Size.X , 0) < RoundTo( GameForm->Fill->Bitmap->Bitmap->Height , 0) )
	GameForm->Fill->Bitmap->Bitmap->Resize( RoundTo( (double) Size.X, 0 ) * GameForm->Fill->Bitmap->Bitmap->Width / GameForm->Fill->Bitmap->Bitmap->Height , RoundTo( Size.X, 0 ) );
  GameForm->Fill->Bitmap->WrapMode = TWrapMode::Tile;
  ScreenOrientation = GetScreenOrientation();
  if ( ( ScreenOrientation == TScreenOrientation::Portrait ) || ( ScreenOrientation == TScreenOrientation::InvertedPortrait ) )
  {
    GameForm->Fill->Bitmap->Bitmap->Rotate( - 90 );
  }

  TMessageListenerMethod OrientationChangedPointer = &(this->OrientationChanged);
  OrientationChangedId = TMessageManager::DefaultManager->SubscribeToMessage( __classid(TOrientationChangedMessage), OrientationChangedPointer );


  if ( TPlatformServices::Current->SupportsPlatformService( __uuidof( IFMXApplicationEventService), (void *)&( FMXApplicationEventService ) ) )
	FMXApplicationEventService->SetApplicationEventHandler( HandleAppEvent );
  LoadSettings();
  AudioManager = std::make_unique<TAudioManager>();
  RegisterSound( DataFilePath + "fire.wav" );
  RegisterSound( DataFilePath + "explosion.wav" );
  RegisterSound( DataFilePath + "fail.wav" );
  RegisterSound( DataFilePath + "win.wav" );
  RegisterSound( DataFilePath + "alien.wav" );
  RegisterSound( DataFilePath + "collect.wav" );
  SetStage( MAIN_MENU );
}


void __fastcall TGameForm::FormDestroy( TObject *Sender )
{
  SaveSettings();
  TMessageManager::DefaultManager->Unsubscribe(__classid( TOrientationChangedMessage ), OrientationChangedId );
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
		  FireDownEvent(Sender);
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
		  FireUpEvent(Sender);
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
    ForwardButtonDown = false;
    FireButtonDown = false;
  }
  }
}


void __fastcall TGameForm::ForwardBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  UpDownEvent(Sender);
}


void __fastcall TGameForm::ForwardBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  UpUpEvent(Sender);
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


void __fastcall TGameForm::InvulnerableTimerTimer( TObject *Sender )
{
  PlayerData->Invulnerable = PlayerData->Invulnerable - 1;
  if ( PlayerData->Invulnerable <= 0 )
  {
    InvulnerableTimer->Enabled = false;
    Ship->Opacity = Ship->Opacity + 0.25;
  }
}

void __fastcall TGameForm::FireDownEvent( TObject *Sender )
{
  FireButtonDown = true;
}

void __fastcall TGameForm::FireUpEvent( TObject *Sender )
{
  FireButtonDown = false;
}

void __fastcall TGameForm::LeftDownEvent( TObject *Sender )
{
  RightButtonDown = false;
  LeftButtonDown = true;
}

void __fastcall TGameForm::LeftUpEvent( TObject *Sender )
{
  LeftButtonDown = false;
}

void __fastcall TGameForm::RightDownEvent( TObject *Sender )
{
  LeftButtonDown = false;
  RightButtonDown = true;
}

void __fastcall TGameForm::RightUpEvent( TObject *Sender )
{
  RightButtonDown = false;
}

void __fastcall TGameForm::UpDownEvent( TObject *Sender )
{
  ForwardButtonDown = true;
}

void __fastcall TGameForm::UpUpEvent( TObject *Sender )
{
  ForwardButtonDown = false;
}


void __fastcall TGameForm::LeftActionDownExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  LeftDownEvent(Sender);
}


void __fastcall TGameForm::LeftActionUpExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
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

void __fastcall TGameForm::FireActionDownExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  TThread::Synchronize(nullptr, [&](){
		switch (CurrentStage)
		{
		case MAIN_MENU:
			StartGame();
			break;

		case INSTRUCTIONS:
			InitAndPlayGame();
			break;

		case GAMEPLAY:
			  FireDownEvent(nullptr);
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
  });
}

void __fastcall TGameForm::RightActionDownExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  RightDownEvent(Sender);
}


void __fastcall TGameForm::RightActionUpExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  RightUpEvent(Sender);
}


void __fastcall TGameForm::FireActionUpExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  FireUpEvent(Sender);
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


void __fastcall TGameForm::RightBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  RightDownEvent(Sender);
}


void __fastcall TGameForm::RightBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  RightUpEvent(Sender);
}


void __fastcall TGameForm::FirePlayerProj( )
{
  if ( PlayerData->FireInterval > PlayerData->FireSpeed )
  {
    ProjList->AddObject( "", SpawnProj( Ship, Ship->Position->X, Ship->Position->Y, Ship->RotationAngle ) );
    PlayerData->FireInterval = 0;
    PlaySound( FIRE_SFX );
  }
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
    AccUp->Visible = false;
    AccUpButtonDown = false;
    AccLeft->Visible = false;
    AccLeftButtonDown = false;
    AccRight->Visible = false;
    AccRightButtonDown = false;
  }
  if ( PlayerData->Invulnerable > 0 )
    InvulnerableTimer->Enabled = true;
  GameLoop->Enabled = true;
}


void __fastcall TGameForm::StopGameLoop( )
{
  if ( GyroEnabled )
    MotionSensor->Active = false;
  InvulnerableTimer->Enabled = false;
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
  RockList = new TStringList();
  RockPool = new TStringList();
  ProjList = new TStringList();
  ProjPool = new TStringList();
  ExplosionList = new TStringList();
  ExplosionPool = new TStringList();
  EnemyList = new TStringList();
  EnemyPool = new TStringList();
  EnemyProjList = new TStringList();
  EnemyProjPool = new TStringList();
  CollectList = new TStringList();
  CollectPool = new TStringList();
  CleanedUp = false;
  Ship->RotationAngle = 0;
  CenterPlayer();
}


void __fastcall TGameForm::InitPlayer( )
{
  PlayerData = new TPlayerData(this);
  PlayerData->Health = PLAYER_HEALTH;
  PlayerData->Lives = PLAYER_LIVES;
  PlayerData->SpeedX = 0;
  PlayerData->SpeedY = 0;
  PlayerData->Level = 1;
  PlayerData->FireSpeed = 5;
  PlayerData->FireInterval = PlayerData->FireSpeed;
  PlayerData->CurrentSpeed = 0;
  PlayerData->MaxSpeed = 10;
  PlayerData->Decay = 0.97;
  PlayerData->Thrust = 0.5;
  PlayerData->RotationSpeed = 10;
  PlayerData->InvulnerableInterval = 5;
  PlayerData->ProjDuration = ( ( Max( ScreenLayout->Width, ScreenLayout->Height ) / 2 ) / PROJ_SPEED ) / 10;
  if ( LivesSet == false )
  {
    BitmapToRectangle( Ship->Fill->Bitmap->Bitmap, Lives1 );
    BitmapToRectangle( Ship->Fill->Bitmap->Bitmap, Lives2 );
    BitmapToRectangle( Ship->Fill->Bitmap->Bitmap, Lives3 );
    BitmapToRectangle( HealthBar->Fill->Bitmap->Bitmap, Health1 );
    BitmapToRectangle( HealthBar->Fill->Bitmap->Bitmap, Health2 );
    BitmapToRectangle( HealthBar->Fill->Bitmap->Bitmap, Health3 );
  }
  DisplayLives( PlayerData->Lives );
  DisplayHealth( PlayerData->Health );
  DisplayScore();
  CenterPlayer();
  Ship->TagObject = PlayerData;
}


void __fastcall TGameForm::BitmapToRectangle( TBitmap *B, TRectangle *R )
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
  Ship->Position->X = ( ScreenLayout->Width / 2 ) - ( Ship->Width / 2 );
  Ship->Position->Y = ( ScreenLayout->Height / 2 ) - ( Ship->Height / 2 );
}


void __fastcall TGameForm::CleanupGame( bool Continue )
{
  if ( CleanedUp == false )
  {
	RockList->OwnsObjects = true;
	RockPool->OwnsObjects = true;
	ProjList->OwnsObjects = true;
	ProjPool->OwnsObjects = true;
	ExplosionList->OwnsObjects = true;
	ExplosionPool->OwnsObjects = true;
	EnemyList->OwnsObjects = true;
	EnemyPool->OwnsObjects = true;
	EnemyProjList->OwnsObjects = true;
	EnemyProjPool->OwnsObjects = true;
	CollectList->OwnsObjects = true;
	CollectPool->OwnsObjects = true;
	if ( Continue == false ) {
	  destroy(PlayerData);
	}
	destroy(ProjList);
	destroy(ProjPool);
	destroy(ExplosionList);
	destroy(ExplosionPool);
	destroy(RockList);
	destroy(RockPool);
	destroy(EnemyList);
	destroy(EnemyPool);
	destroy(EnemyProjList);
	destroy(EnemyProjPool);
	destroy(CollectList);
	destroy(CollectPool);
	CleanedUp = true;
  }
}


void __fastcall TGameForm::PlayGame( )
{
  int I = 0;
  SetStage( GAMEPLAY );
  for ( int stop = PlayerData->Level + 2, I = 1; I <= stop; I++)
  {
    switch ( RandomRange( 1, 4 ) )
    {
      case 1:
        RockList->AddObject( "", SpawnRock( Random( (int)( ScreenLayout->Width ) ), 0, 1 ) );
      break;
      case 2:
        RockList->AddObject( "", SpawnRock( 0, Random( (int)( ScreenLayout->Height ) ), 1 ) );
      break;
      case 3:
        RockList->AddObject( "", SpawnRock( Random( (int)( ScreenLayout->Width ) ), ScreenLayout->Height, 1 ) );
      break;
      case 4:
        RockList->AddObject( "", SpawnRock( ScreenLayout->Width, Random( (int)( ScreenLayout->Height ) ), 1 ) );
      break;
    }
  }
  StartGameLoop();
}


void __fastcall TGameForm::ContinueGame( )
{
  CleanupGame( true );
  InitGame();
  PlayerData->SpeedX = 0;
  PlayerData->SpeedY = 0;
  PlayerData->CurrentSpeed = 0;
  PlayerData->Level = PlayerData->Level + 1;
  PlayGame();
}


void __fastcall TGameForm::LevelFail( )
{
  StopGameLoop();
  PlayerData->SpeedX = 0;
  PlayerData->SpeedY = 0;
  PlayerData->CurrentSpeed = 0;
  if ( PlayerData->Invulnerable > 0 )
  {
    Ship->Opacity = Ship->Opacity + 0.25;
    PlayerData->Invulnerable = 0;
  }
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


void __fastcall TGameForm::SwapListXY( TStringList *List )
{
  int I = 0;
  float TempX = 0.0, TempY = 0.0;
  if ( List->Count > 0 )
    for ( int stop = 0, I = List->Count - 1; I >= stop; I--)
	{
	  TempX = ((TRectangle*)( List->Objects[I] ))->Position->X;
	  TempY = ((TRectangle*)( List->Objects[I] ))->Position->Y;
	  ((TRectangle*)( List->Objects[I] ))->Position->X = TempY;
	  ((TRectangle*)( List->Objects[I] ))->Position->Y = TempX;
    }
}


void __fastcall TGameForm::SwapXY( )
{
  float TempX = 0.0, TempY = 0.0;
  Ship->Anchors = TAnchors();
  TempX = Ship->Position->X;
  TempY = Ship->Position->Y;
  Ship->Position->X = TempY;
  Ship->Position->Y = TempX;
  Ship->Align = TAlignLayout::None;
  Ship->Anchors << TAnchorKind::akLeft  << TAnchorKind::akTop;

  SwapListXY( ProjList );
  SwapListXY( RockList );
  SwapListXY( EnemyList );
  SwapListXY( EnemyProjList );
  SwapListXY( ExplosionList );
  SwapListXY( CollectList );
}


void __fastcall TGameForm::OrientationChanged(System::TObject * const Sender, System::Messaging::TMessageBase * const Msg)
{
  TScreenOrientation NewScreenOrientation;
  if ( CurrentStage == GAMEPLAY )
  {
    NewScreenOrientation = GetScreenOrientation();
	if ( ( ( NewScreenOrientation == TScreenOrientation::Portrait ) || ( NewScreenOrientation == TScreenOrientation::InvertedPortrait ) ) && ( ( ScreenOrientation == TScreenOrientation::Landscape ) || ( ScreenOrientation == TScreenOrientation::InvertedLandscape ) ) )
	{
      CanSwapXY = true;
      GameForm->Fill->Bitmap->Bitmap->Rotate( 90 );
    }
    else
	  if ( ( ( NewScreenOrientation == TScreenOrientation::Landscape ) || ( NewScreenOrientation == TScreenOrientation::InvertedLandscape ) ) && ( ( ScreenOrientation == TScreenOrientation::Portrait ) || ( ScreenOrientation == TScreenOrientation::InvertedPortrait ) ) )
      {
        CanSwapXY = true;
        GameForm->Fill->Bitmap->Bitmap->Rotate( - 90 );
	  }
  }
}

void __fastcall TGameForm::ExitDialog(TObject *Sender)
{
	TDialogServiceAsync::MessageDialog("Exit the game?", TMsgDlgType::mtInformation,
	  TMsgDlgButtons() << TMsgDlgBtn::mbYes << TMsgDlgBtn::mbNo,
	  TMsgDlgBtn::mbYes, 0,
	  [&](const TModalResult AResult)->void {
			switch (AResult) {
			  case mrYes:
				{
					TThread::Synchronize(nullptr,[&](){
					  LevelFail();
					  CleanupGame(false);
					  ShowMainMenu();
					});
				}
				break;
			  case mrNo:
				{
				 // pressed no
				}
				break;
			}
	  });
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
  IFMXScreenService *ScreenSvc = nullptr;
  if ( TPlatformServices::Current->SupportsPlatformService( __uuidof(IFMXScreenService), (void *)&ScreenSvc ) )
  {
	result = ScreenSvc->GetScreenOrientation();
  }
  return result;
}

void __fastcall RegisterRenderingSetup( )
{
  IFMXRenderingSetupService *SetupService = nullptr;
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
  //enable the GPU on Windows
  //GlobalUseGPUCanvas := True;
  uGame_initialization();
}
};
static uGame_unit _uGame_unit;

//---------------------------------------------------------------------------

