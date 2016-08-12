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
TGameForm* GameForm;

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
  for ( int stop = Enemies->ControlCollection->Count - 1, I = 0; I <= stop; I++)
  {
	auto *EnemyObj = (TRectangle*)( (TGridPanelLayout::TControlItem*)( Enemies->ControlCollection->Items[I] )->Control );
	((TBitmapListAnimation*)( EnemyObj->TagObject ))->Pause = Value;
  }
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
	MusicPlayer->CurrentTime = 0;
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
	result = (TRectangle *)( Pool->Objects[Pool->Count - 1] );
	// Pool->Objects[Pool->Count-1] := nil;
	Pool->Delete( Pool->Count - 1 );
  }
  else
  {
	result = new TRectangle(this);
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
  R->Width = 25;
  R->Height = 3;
  R->RotationAngle = Angle - 90;
  R->Stroke->Kind = TBrushKind::None;
  R->Fill->Bitmap->WrapMode = TWrapMode::TileStretch;
  R->Fill->Kind = TBrushKind::Bitmap;
  R->Fill->Bitmap->Bitmap->Assign( Projectile->Fill->Bitmap->Bitmap );
  float RAngle = R->RotationAngle * PI / 180;
  // set the center X,Y and then travel 25 out from the center on the angle
  R->Position->X = X + ( Source->Width / 2 ) - ( R->Width / 2 ) + 25 * Cos( RAngle );
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
  R->Width = Enemy->Width;
  R->Height = Enemy->Height;
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


void __fastcall TGameForm::InitEnemies( )
{
  TBitmapListAnimation *AE = nullptr;
  TBitmapListAnimation *AnimateEnemy = nullptr;
  bool SmallEnemies = false;
  if ( Enemies->Width > ScreenLayout->Width )
  {
	SmallEnemies = true;
  }
  else
	SmallEnemies = false;
  if ( Enemies->ControlCollection->Count > 0 )
  {
	for ( int I = 0; I < Enemies->ControlCollection->Count; I++)
	{
	  auto *EnemyObj = (TRectangle *) Enemies->ControlCollection->Items[I]->Control;
	  if ( ( SmallEnemies == true ) && ( ( I == 4 ) || ( I == 5 ) || ( I == 6 ) || ( I == 11 ) || ( I == 12 ) || ( I == 13 ) || ( I == 18 ) || ( I == 19 ) || ( I == 20 ) ) )
	  {
		EnemyObj->Visible = false;
		EnemyObj->TagFloat = 1;
	  }
	  else
	  {
		EnemyObj->Visible = true;
		EnemyObj->TagFloat = 0;
	  }
	  EnemyObj->Tag = 0;
	  EnemyObj->Fill->Kind = TBrushKind::Bitmap;
	  EnemyObj->Fill->Bitmap->WrapMode = TWrapMode::Tile;
	  if (!EnemyObj->TagObject)
	  {
		AE = new TBitmapListAnimation( EnemyObj );
		if ( ( I >= 0 ) && ( I <= 6 ) )
		{
		  AnimateEnemy = AnimateEnemy3;
		}
		if ( ( I >= 7 ) && ( I <= 13 ) )
		{
		  AnimateEnemy = AnimateEnemy2;
		}
		if ( ( I >= 14 ) && ( I <= 20 ) )
		{
		  AnimateEnemy = AnimateEnemy1;
		}
		AE->AnimationBitmap->Assign( AnimateEnemy->AnimationBitmap );
		AE->AnimationCount = AnimateEnemy->AnimationCount;
		AE->AnimationRowCount = AnimateEnemy->AnimationRowCount;
		AE->PropertyName = AnimateEnemy->PropertyName;
		AE->Duration = AnimateEnemy->Duration;
		AE->Loop = AnimateEnemy->Loop;
		EnemyObj->TagObject = AE;
		AE->Parent = EnemyObj;
	  }
	  else
	  {
		AE = (TBitmapListAnimation *)EnemyObj->TagObject;
	  }
	  AE->Start();
	  if ( ( SmallEnemies == true ) && ( ( I == 4 ) || ( I == 5 ) || ( I == 6 ) || ( I == 11 ) || ( I == 12 ) || ( I == 13 ) || ( I == 18 ) || ( I == 19 ) || ( I == 20 ) ) )
	  {
		EnemyDestroyedCount = EnemyDestroyedCount + 1;
	  }
	  else
	  {
		EnemyList->AddObject( "", EnemyObj );
	  }
	}
  }
  BottomMostEnemyY = Enemies->Position->Y + Enemies->Height;
}


TRectangle* __fastcall TGameForm::SpawnEnemyProj( TRectangle *Source, float X, float Y, float Angle )
{
  auto *R = (TRectangle*)GetPoolObj( ProjPool );
  R->Parent = this;
  R->Width = 27;
  R->Height = 8;
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
  Enemies->Visible = true;
  WallLayout->Visible = true;
  #ifdef DEBUG_MODE
  FPSLBL->Visible = true;
  #endif
  HUDLayout->BringToFront();
}


void __fastcall TGameForm::HideHUD( )
{
  Enemies->Visible = false;
  WallLayout->Visible = false;
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
  return Distance < ( ( Max( R1->Width, R1->Height ) / 2 ) + ( Max( R2->Width, R2->Height ) / 2 ) );
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
  float ProjAngle = 0.0, EnemyAngle = 0.0, EnemyProjAngle = 0.0, CollectAngle = 0.0;
  TRectangle *ProjObj = nullptr;
  TRectangle *WallObj = nullptr;
  TRectangle *ExplosionObj = nullptr;
  TRectF 	   EnemyRect;
  TRectangle *EnemyObj = nullptr;
  TRectangle *EnemyProjObj = nullptr;
  TRectangle *CollectObj = nullptr;
  unsigned int Time = 0;
  float EnemySpeed = 0.0;
  float LeftMostEnemyX = 0.0, RightMostEnemyX = 0.0;
  float TmpBottomMostEnemyY = 0.0;

  #ifdef DEBUG_MODE
  Time = TThread::GetTickCount();
  #endif
  // Check for game over
  if ( ( PlayerData->Lives <= 0 ) || ( BottomMostEnemyY > ScreenLayout->Height ) )
  {
	LevelFail();
	GameOver();
	return;
  }

  // Check for level complete
  if ( EnemyDestroyedCount >= 21 )
  {
	LevelComplete();
	return;
  }
  else
  {
	EnemySpeed = Max( EnemyDistance * ( PlayerData->Level * 0.1 ), 1.0 );
  }

  // process accelerometer data
  if ( MotionSensor->Active == true )
  {
	ProcessAccelerometer();
  }

  // Handle player movement and firing
  PlayerData->FireInterval = PlayerData->FireInterval + 1;
  if ( ( LeftButtonDown || AccLeftButtonDown ) && ( Ship->Position->X > ScreenLayout->Position->X ) )
	Ship->Position->X = Ship->Position->X - 5;
  if ( ( RightButtonDown || AccRightButtonDown ) && ( Ship->Position->X + Ship->Width < ScreenLayout->Position->X + ScreenLayout->Width ) )
	Ship->Position->X = Ship->Position->X + 5;
  if ( FireButtonDown || AccUpButtonDown )
  {
	FirePlayerProj();
  }
  if ( LeftButtonDown || AccLeftButtonDown || RightButtonDown || AccRightButtonDown )
  {
	Thruster->Visible = true;
  }
  else
  {
	Thruster->Visible = false;
  }


  // Handle explosions
  if ( ExplosionList->Count > 0 )
	for ( I = 0; I < ExplosionList->Count; I++ )
	{
	  ExplosionObj = (TRectangle  *)(ExplosionList->Objects[I]);
	  ExplosionObj->TagFloat = ExplosionObj->TagFloat + 0.1;
	  if ( ExplosionObj->TagFloat > (double)ExplosionObj->Tag )
	  {
		// ProjList->Objects[I] := nil;
		SetPoolObj( ExplosionPool, ExplosionList->Names[I], ExplosionObj );
		ExplosionList->Delete( I );
	  }
	}

  // Handle player projectiles
  if ( ProjList->Count > 0 )
	for ( I = 0; I < ProjList->Count; I++ )
	{
	  ProjObj = (TRectangle *)(ProjList->Objects[I]);
	  ProjAngle = ProjObj->RotationAngle * PI / 180;
	  ProjObj->Position->X = ProjObj->Position->X + (double)ProjObj->Tag * Cos( ProjAngle );
	  ProjObj->Position->Y = ProjObj->Position->Y + (double)ProjObj->Tag * Sin( ProjAngle );
	  if ( EnemyList->Count > 0 )
		for ( II = 0; II < EnemyList->Count; II++ )
		{
		  EnemyObj = (TRectangle *)( EnemyList->Objects[II] );
		  if ( EnemyObj->Tag != 0 )
		  {
			EnemyRect = EnemyObj->ParentedRect;
		  }
		  else
			EnemyRect = EnemyObj->AbsoluteRect;
		  if ( IntersectRect( EnemyRect, ProjObj->AbsoluteRect ) )
		  // if RockObj->PointInObject(ProjObj->Position->X,ProjObj->Position->Y) then
		  {
			EnemyObj->TagFloat = EnemyObj->TagFloat + 1;
			ProjObj->TagFloat = PlayerData->ProjDuration + 1;
			break;
		  }
		}
	  for ( II = 0; II < WallList->Count; II++ )
	  {
		WallObj = (TRectangle *)( WallList->Objects[II] );
		if ( WallObj->TagFloat < 4 )
		{
		  if ( IntersectRect( WallObj->AbsoluteRect, ProjObj->AbsoluteRect ) )
		  {
			WallObj->TagFloat = WallObj->TagFloat + 1;
			if ( WallObj->TagFloat >= 4 )
			{
			  WallObj->Visible = false;
			}
			ProjObj->TagFloat = ProjObj->TagFloat + 1;
			CreateExplosion( ProjObj->Position->X + ( ProjObj->Width / 2 ), ProjObj->Position->Y + ( ProjObj->Height / 2 ) );
			break;
		  }
		}
	  }
	  if ( CollectList->Count > 0 )
		for ( II = 0; II < CollectList->Count; II++ )
		{
		  CollectObj = (TRectangle*)( CollectList->Objects[II] );
		  if ( IntersectRect( CollectObj->ParentedRect, ProjObj->ParentedRect ) )
		  // if IntersectRect(CollectObj->AbsoluteRect, ProjObj->AbsoluteRect) then
		  // if RockObj->PointInObject(ProjObj->Position->X,ProjObj->Position->Y) then
		  {
			ProjObj->TagFloat = ProjObj->TagFloat + 1;
			CollectObj->TagFloat = COLLECTITEM_DURATION + 1;
			AddScore( 5000 );
			PlaySound( COLLECT_SFX );
			break;
		  }
		}
	  if ( ( ProjObj->TagFloat > 0 ) || ( ProjObj->Position->X > ScreenLayout->Width ) || ( ProjObj->Position->Y > ScreenLayout->Height ) || ( ProjObj->Position->X < ScreenLayout->Position->X ) || ( ProjObj->Position->Y < ScreenLayout->Position->Y ) )
	  {
		// ProjList->Objects[I] := nil;
		SetPoolObj( ProjPool, ProjList->Names[I], ProjObj );
		ProjList->Delete( I );
	  }
	}

  // Handle enemy movement and firing
  LeftMostEnemyX = ScreenLayout->Width;
  RightMostEnemyX = 0;
  if ( EnemyList->Count > 0 )
	for ( I = 0; I < EnemyList->Count; I++ )
	{
	  EnemyObj = (TRectangle*)( EnemyList->Objects[I] );
	  if ( EnemyObj->Tag != 0 )
	  {
		EnemyAngle = EnemyObj->RotationAngle * PI / 180;
		EnemyObj->Position->X = EnemyObj->Position->X + (double)EnemyObj->Tag * Cos( EnemyAngle );
		EnemyObj->Position->Y = EnemyObj->Position->Y + (double)EnemyObj->Tag * Sin( EnemyAngle );
	  }
	  if ( PlayerData->Invulnerable == 0 )
		if ( IntersectRect( Ship->AbsoluteRect, EnemyObj->AbsoluteRect ) )
		{
		  PlayerHit();
		  EnemyObj->TagFloat = EnemyObj->TagFloat + 1;
		}
	  for ( II = 0; II < WallList->Count; II++)
	  {
		WallObj = (TRectangle*)( WallList->Objects[II] );
		if ( WallObj->TagFloat < 4 )
		{
		  if ( IntersectRect( WallObj->AbsoluteRect, EnemyObj->AbsoluteRect ) )
		  {
			WallObj->TagFloat = WallObj->TagFloat + 4;
			if ( WallObj->TagFloat >= 4 )
			{
			  WallObj->Visible = false;
			}
			EnemyObj->TagFloat = EnemyObj->TagFloat + 1;
			break;
		  }
		}
	  }
	  if ( RandomRange( 1, 500 ) == 1 )
	  {
		if ( EnemyObj->Tag != 0 )
		{
		  EnemyProjList->AddObject( "", SpawnEnemyProj( EnemyObj, EnemyObj->Position->X, EnemyObj->Position->Y, 180 ) );
		}
		else
		{
		  EnemyProjList->AddObject( "", SpawnEnemyProj( EnemyObj, Enemies->Position->X + EnemyObj->Position->X, Enemies->Position->Y + EnemyObj->Position->Y, 180 ) );
		}
		PlaySound( ALIEN_SFX );
	  }
	  if ( ( EnemyObj->TagFloat > 0 ) || ( EnemyObj->Position->X > ScreenLayout->Width ) || ( EnemyObj->Position->Y > ScreenLayout->Height ) || ( EnemyObj->Position->X < ScreenLayout->Position->X ) || ( EnemyObj->Position->Y < ScreenLayout->Position->Y ) )
	  {
		// EnemyList->Objects[I] := nil;
		if ( EnemyObj->TagFloat > 0 )
		{
		  if ( EnemyObj->Tag != 0 )
		  {
			CreateExplosion( EnemyObj->Position->X + ( EnemyObj->Width / 2 ), EnemyObj->Position->Y + ( EnemyObj->Height / 2 ) );
			AddScore( 500 );
		  }
		  else
		  {
			EnemyDestroyedCount = EnemyDestroyedCount + 1;
			CreateExplosion( Enemies->Position->X + EnemyObj->Position->X + ( EnemyObj->Width / 2 ), Enemies->Position->Y + EnemyObj->Position->Y + ( EnemyObj->Height / 2 ) );
			AddScore( 100 );
		  }
		}
		if ( EnemyObj->Tag != 0 )
		{
		  SetPoolObj( EnemyPool, EnemyList->Names[I], EnemyObj );
		  EnemyList->Delete( I );
		  FlyingSaucer = FlyingSaucer - 1;
		}
		else
		{
		  EnemyObj->Visible = false;
		  ((TBitmapListAnimation *)EnemyObj->TagObject)->Stop();
		  EnemyList->Delete( I );
		}
	  }
	  else
	  {
		if ( EnemyObj->Tag == 0 )
		{
		  if ( EnemyObj->Position->X > RightMostEnemyX )
		  {
			RightMostEnemyX = Enemies->Position->X + GetTheMostRightEnemyPosition( EnemyList ) + EnemyObj->Width;
		  }
		  if ( EnemyObj->Position->X < LeftMostEnemyX )
		  {
			LeftMostEnemyX = EnemyObj->Position->X;
		  }
		  TmpBottomMostEnemyY = Enemies->Position->Y + EnemyObj->Position->Y + EnemyObj->Height;
		  if ( TmpBottomMostEnemyY > BottomMostEnemyY )
		  {
			BottomMostEnemyY = TmpBottomMostEnemyY;
		  }
		}
	  }
	}

  // Handle enemy projectiles
  if ( EnemyProjList->Count > 0 )
	for ( I = 0; I < EnemyProjList->Count; I++)
	{
	  EnemyProjObj = (TRectangle*)( EnemyProjList->Objects[I] );
	  EnemyProjAngle = EnemyProjObj->RotationAngle * PI / 180;
	  EnemyProjObj->Position->X = EnemyProjObj->Position->X + (double)EnemyProjObj->Tag * Cos( EnemyProjAngle );
	  EnemyProjObj->Position->Y = EnemyProjObj->Position->Y + (double)EnemyProjObj->Tag * Sin( EnemyProjAngle );
	  for ( int stop = WallList->Count - 1, II = 0; II <= stop; II++)
	  {
		WallObj = (TRectangle*)( WallList->Objects[II] );
		if ( WallObj->TagFloat < 4 )
		{
		  if ( WallObj->PointInObject( EnemyProjObj->Position->X, EnemyProjObj->Position->Y ) )
		  {
			WallObj->TagFloat = WallObj->TagFloat + 1;
			if ( WallObj->TagFloat >= 4 )
			{
			  WallObj->Visible = false;
			}
			EnemyProjObj->TagFloat = EnemyProjObj->TagFloat + 1;
			break;
		  }
		}
	  }
	  if ( PlayerData->Invulnerable == 0 )
		if ( IntersectRect( Ship->AbsoluteRect, EnemyProjObj->AbsoluteRect ) )
		// if IntersectCircle(Ship, EnemyProjObj) then
		{
		  PlayerHit();
		  EnemyProjObj->TagFloat = EnemyProjObj->TagFloat + 1;
		}
	  if ( ( EnemyProjObj->TagFloat > 0 ) || ( EnemyProjObj->Position->X > ScreenLayout->Width ) || ( EnemyProjObj->Position->Y > ScreenLayout->Height ) || ( EnemyProjObj->Position->X < ScreenLayout->Position->X ) || ( EnemyProjObj->Position->Y < ScreenLayout->Position->Y ) )
	  {
		// EnemyProjList->Objects[I] := nil;
		SetPoolObj( EnemyProjPool, EnemyProjList->Names[I], EnemyProjObj );
		EnemyProjList->Delete( I );
	  }
	}

  // Spawn enemies
  if ( FlyingSaucer < 1 )
  {
	if ( Random( 1000 ) == 1 )
	{
	  if ( Random( 2 ) == 1 )
	  {
		EnemyList->AddObject( "", SpawnEnemy( ScreenLayout->Width - 1, RandomRange( 1, int( ScreenLayout->Height / 8 ) ), 0, - ENEMY_SPEED ) );
	  }
	  else
	  {
		EnemyList->AddObject( "", SpawnEnemy( ScreenLayout->Position->X + 1, RandomRange( 1, int( ScreenLayout->Height / 8 ) ), 0, ENEMY_SPEED ) );
	  }
	  PlaySound( ALIEN_SFX );
	  FlyingSaucer = FlyingSaucer + 1;
	}
  }

  // Handle collectable items
  if ( CollectList->Count > 0 )
	for ( I = 0; I < CollectList->Count; I++ )
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
	  CollectObj->TagFloat = CollectObj->TagFloat + 0.1;
	  if ( CollectObj->TagFloat > COLLECTITEM_DURATION )
	  {
		// EnemyProjList->Objects[I] := nil;
		SetPoolObj( CollectPool, CollectList->Names[I], CollectObj );
		CollectList->Delete( I );
	  }
	}

  // Spawn collectable
  if ( CollectList->Count < 1 )
  {
	if ( Random( 1000 ) == 1 )
	{
	  CollectList->AddObject( "", SpawnCollectItem( RandomRange( 1, int( ScreenLayout->Width - 1 ) ), RandomRange( 1, int( ( ScreenLayout->Height / 4 ) - 1 ) ), 0 ) );
	}
  }

  // move enemy grid
  if ( Enemies->Tag == 0 )
  {
	Enemies->Position->X = Enemies->Position->X + EnemySpeed;
  }
  else
  {
	Enemies->Position->X = Enemies->Position->X - EnemySpeed;
  }
  if ( ( RightMostEnemyX ) > ( ScreenLayout->Width - 5 ) )
  {
	Enemies->Tag = 1;
	Enemies->Position->Y = Enemies->Position->Y + EnemyDropHeight;
	EnemyDistance = EnemyDistance + 1;
  }
  if ( ( Enemies->Position->X ) < ( ScreenLayout->Position->X - LeftMostEnemyX + 5 ) )
  {
	Enemies->Tag = 0;
	Enemies->Position->Y = Enemies->Position->Y + EnemyDropHeight;
	EnemyDistance = EnemyDistance + 1;
  }

  // Handle music loop
  if ( ( MusicPlayer->CurrentTime >= MusicPlayer->Duration ) || ( MusicPlayer->State == TMediaState::Stopped ) )
  {
	MusicPlayer->CurrentTime = 0;
	MusicPlayer->Stop();
	PlayMusic();
  }
  #ifdef DEBUG_MODE
  FPSLBL->Text = IntToStr(int(TThread::GetTickCount() - Time)) + " ms";
  #endif
}


float __fastcall TGameForm::GetTheMostRightEnemyPosition( TStringList *EnemyList )
{
  int I = 0;
  float X = 0.0;
  for ( I = 0; I < EnemyList->Count; I++)
  {
	auto *pObj = (TRectangle *)(EnemyList->Objects[I]);
	if ( ( pObj->Position->X > X ) && ( pObj->Tag == 0 ) )
	  X = pObj->Position->X;
  }
  return X;
}


float __fastcall TGameForm::GetTheMostTopEnemyPosition( TStringList *EnemyList )
{
  float Y = 0.0;
  for ( int I = 0; I < EnemyList->Count; I++)
  {
	auto *pObj = (TRectangle *)(EnemyList->Objects[I]);
	if ( ( pObj->Position->Y > Y ) && ( pObj->Tag == 0 ) )
	  Y = pObj->Position->Y;
  }
  return Y;
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


float __fastcall TGameForm::GetTargetAngle( float TargetX, float TargetY, float OriginX, float OriginY )
{
  float Radians = ArcTan2( TargetY - OriginY, TargetX - OriginX );
  return Radians / ( PI / 180 ) + 90;
}


void __fastcall TGameForm::PlayerHit( )
{
  PlayerData->Health = PlayerData->Health - 1;
  DisplayHealth( PlayerData->Health );
  CreateExplosion( Ship->Position->X + ( Ship->Width / 2 ), Ship->Position->Y + ( Ship->Height / 2 ) );
  if ( PlayerData->Health <= 0 )
  {
	CreateExplosion( Ship->Position->X + RandomRange( 1, int( Ship->Width ) ), Ship->Position->Y + RandomRange( 1, int( Ship->Height ) ) );
	PlayerData->Lives = PlayerData->Lives - 1;
	PlayerData->Health = PLAYER_HEALTH;
	Health1->Visible = true;
	Health2->Visible = true;
	Health3->Visible = true;
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


void __fastcall TGameForm::FireBTNClick( TObject *Sender )
{
  FirePlayerProj();
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
  std::unique_ptr<TMemIniFile> IniFile(new TMemIniFile( SettingsFilePath + "Settings.ini" ));
  IniFile->WriteBool( "Settings", "MusicEnabled", MusicEnabled );
  IniFile->WriteBool( "Settings", "SoundEnabled", SoundEnabled );
  IniFile->WriteBool( "Settings", "GyroEnabled", GyroEnabled );
  IniFile->WriteBool( "Settings", "HostEnabled", HostEnabled );
  IniFile->WriteBool( "Settings", "FullScreenEnabled", FullScreenEnabled );
  IniFile->UpdateFile();
}


void __fastcall TGameForm::LoadSettings( )
{
  std::unique_ptr<TMemIniFile> IniFile(new TMemIniFile( SettingsFilePath + "Settings.ini" ));
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


void __fastcall TGameForm::FormCreate( TObject *Sender )
{
  IFMXApplicationEventService *FMXApplicationEventService = nullptr;
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
  // CleanupGame(False);
  SaveSettings();
  TMessageManager::DefaultManager->Unsubscribe( __classid(TOrientationChangedMessage), OrientationChangedId );
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
		case GAMEPLAY:
		case LEVEL_COMPLETE:
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
  CenterPlayer();
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


void __fastcall TGameForm::InvulnerableTimerTimer( TObject *Sender )
{
  PlayerData->Invulnerable = PlayerData->Invulnerable - 1;
  if ( PlayerData->Invulnerable <= 0 )
  {
	InvulnerableTimer->Enabled = false;
	Ship->Opacity = Ship->Opacity + 0.25;
  }
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
  FireButtonDown = true;
}

void __fastcall TGameForm::UpUpEvent( TObject *Sender )
{
  FireButtonDown = false;
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
			  UpDownEvent(Sender);
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


void __fastcall TGameForm::FireActionUpExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  UpUpEvent(Sender);
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
  if ( ( PlayerData->FireInterval > PlayerData->FireSpeed ) && ( ProjList->Count < 1 ) )
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
  if ( PlayerData->Invulnerable > 0 )
	InvulnerableTimer->Enabled = true;
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
  int I = 0;
  TRectangle* WallObj = nullptr;
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
  WallList = new TStringList();
  FlyingSaucer = 0;
  EnemyDestroyedCount = 0;
  EnemyDistance = 1;
  EnemyDropHeight = int( ScreenLayout->Height / 100 );
  Enemies->Position->X = 32;
  Enemies->Position->Y = 8;
  InitEnemies();
  if ( WallLayout->ControlCollection->Count > 0 )
  {
	for ( int stop = WallLayout->ControlCollection->Count - 1, I = 0; I <= stop; I++)
	{
	  WallObj = (TRectangle*)WallLayout->ControlCollection->Items[I]->Control;
	  WallObj->Visible = true;
	  WallObj->Tag = 0;
	  WallObj->TagFloat = 0;
	  WallObj->Fill->Kind = TBrushKind::Bitmap;
	  WallObj->Fill->Bitmap->WrapMode = TWrapMode::TileStretch;
	  WallObj->Fill->Bitmap->Bitmap->Assign( Wall->Fill->Bitmap->Bitmap );
	  WallList->AddObject( "", WallObj );
	}
  }
  CleanedUp = false;
  Ship->RotationAngle = 0;
  Ship->Opacity = 1;
  CenterPlayer();
}


void __fastcall TGameForm::InitPlayer( )
{
  PlayerData = new TPlayerData(this);
  PlayerData->Health = PLAYER_HEALTH;
  PlayerData->Lives = PLAYER_LIVES;
  PlayerData->Level = 1;
  PlayerData->FireSpeed = 5;
  PlayerData->FireInterval = PlayerData->FireSpeed;
  PlayerData->InvulnerableInterval = 2;
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
  auto *animation = (TBitmapListAnimation *)Sender;
  ((TRectangle *)animation->Owner)->Visible = false;
}


void __fastcall TGameForm::CenterPlayer( )
{
  Ship->Position->X = ( ScreenLayout->Width / 2 ) - ( Ship->Width / 2 );
  Ship->Position->Y = ScreenLayout->Height - Ship->Height - 10;
}


void __fastcall TGameForm::CleanupGame( bool Continue )
{
  int I = 0;
  if ( CleanedUp == false )
  {
	if (EnemyList)
	{
	  for ( int stop = 0, I = EnemyList->Count - 1; I >= stop; I--)
	  {
		auto *EnemyObj = (TRectangle*)( EnemyList->Objects[I] );
		if ( EnemyObj->Tag != 0 )
		{
		  SetPoolObj( EnemyPool, EnemyList->Names[I], EnemyObj );
		  EnemyList->Delete( I );
		}
	  }
	}
	ProjList->OwnsObjects = true;
	ProjPool->OwnsObjects = true;
	ExplosionList->OwnsObjects = true;
	ExplosionPool->OwnsObjects = true;
  // Enemies in the Enemies grid are re-used.
  // EnemyList->OwnsObjects = true;
	EnemyPool->OwnsObjects = true;
	EnemyProjList->OwnsObjects = true;
	EnemyProjPool->OwnsObjects = true;
	CollectList->OwnsObjects = true;
	CollectPool->OwnsObjects = true;
  // Objects in the wall grid are reused.
  // WallList->OwnsObjects = true;
	if ( Continue == false ) {
	  destroy(PlayerData);
	}

	destroy(ProjList);
	destroy(ExplosionList);
	destroy(ExplosionPool);
	destroy(EnemyList);
	destroy(EnemyPool);
	destroy(EnemyProjList);
	destroy(EnemyProjPool);
	destroy(CollectList);
	destroy(CollectPool);
	destroy(WallList);
	CleanedUp = true;
  }
}


void __fastcall TGameForm::PlayGame( )
{
  SetStage( GAMEPLAY );
  WallLayout->Visible = true;
  Enemies->Visible = true;
  StartGameLoop();
}


void __fastcall TGameForm::ContinueGame( )
{
  CleanupGame( true );
  InitGame();
  PlayerData->Level = PlayerData->Level + 1;
  PlayGame();
}


void __fastcall TGameForm::LevelFail( )
{
  StopGameLoop();
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
	  auto *pObj = (TRectangle*)(List->Objects[I]);
	  TempX = pObj->Position->X;
	  TempY = pObj->Position->Y;
	  pObj->Position->X = TempY;
	  pObj->Position->Y = TempX;
	}
}


void __fastcall TGameForm::SwapXY( )
{
  CenterPlayer();
  SwapListXY( ProjList );
  //SwapListXY(EnemyList);
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
	if ( ( NewScreenOrientation == TScreenOrientation::Portrait ) && ( ScreenOrientation == TScreenOrientation::Landscape ) )
	{
	  SwapXY();
	}
	else
	  if ( ( NewScreenOrientation == TScreenOrientation::Landscape ) && ( ScreenOrientation == TScreenOrientation::Portrait ) )
	  {
		SwapXY();
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
  IFMXScreenService* ScreenSvc = nullptr;
  if ( TPlatformServices::Current->SupportsPlatformService( __uuidof(IFMXScreenService), (void *)&ScreenSvc ) )
  {
	result = ScreenSvc->GetScreenOrientation();
  }
  return result;
}

void __fastcall RegisterRenderingSetup( ) //!
{
  IFMXRenderingSetupService* SetupService = nullptr;
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
  // enables the GPU on Windows
  //GlobalUseGPUCanvas = true;

  uGame_initialization();
}
};
static uGame_unit _uGame_unit;
//---------------------------------------------------------------------------

