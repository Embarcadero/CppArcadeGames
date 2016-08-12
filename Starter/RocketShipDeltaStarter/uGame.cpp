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
#pragma link "uGameOver"
#pragma link "uHighScores"
#pragma link "uInstructions"
#pragma link "uLevelComplete"
#pragma link "uSettings"
#pragma link "uMainMenu"
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
  for ( int stop = EnemyList->Count - 1, I = 0; I <= stop; I++)
  {
	auto *EnemyObj = (TRectangle*)( EnemyList->Objects[I] );
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


int __fastcall TGameForm::RegisterSound( String FileName )
{
  if ( FileExists( FileName ) )
	return AudioManager->AddSound( FileName );
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
  SettingsFrame->HostSwitch->IsChecked = HostEnabled;
  SettingsFrame->GyroSwitch->IsChecked = GyroEnabled;
  SettingsFrame->FullScreenSwitch->IsChecked = FullScreenEnabled;
  SettingsFrame->SoundSwitch->IsChecked = SoundEnabled;
  SettingsFrame->MusicSwitch->IsChecked = MusicEnabled;
  SetStage( SETTINGS );
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
  TRectangle *Projectile = nullptr;
  auto *R = (TRectangle*)( GetPoolObj( ProjPool ) );
  R->Width = 200;
  R->Height = 3;
  R->RotationAngle = Angle - 90;
  R->Stroke->Kind = TBrushKind::None;
  R->Fill->Bitmap->WrapMode = TWrapMode::TileStretch;
  R->Fill->Kind = TBrushKind::Bitmap;
  switch ( RandomRange( 1, 7 ) )
  {
	case 1:
	  Projectile = Projectile1;
	break;
	case 2:
	  Projectile = Projectile2;
	break;
	case 3:
	  Projectile = Projectile3;
	break;
	case 4:
	  Projectile = Projectile4;
	break;
	case 5:
	  Projectile = Projectile5;
	break;
	case 6:
	  Projectile = Projectile6;
	break;
  default:
	Projectile = Projectile1;
  }
  R->Fill->Bitmap->Bitmap->Assign( Projectile->Fill->Bitmap->Bitmap );
  float RAngle = R->RotationAngle * PI / 180;
  // set the center X,Y and then travel 100 out from the center on the angle
  R->Position->X = X + ( Source->Width / 2 ) - ( R->Width / 2 ) + 100 * Cos( RAngle );
  R->Position->Y = Y + ( Source->Height / 2 ) - ( R->Height / 2 ) + 100 * Sin( RAngle );
  R->Tag = PROJ_SPEED;
  R->TagFloat = 0;
  R->Parent = MapLayout1;
  R->BringToFront();
  return R;
}


TRectangle* __fastcall TGameForm::SpawnPerson( float X, float Y, float Angle, int Speed )
{
  auto *R = (TRectangle*)( GetPoolObj( PeoplePool ) );
  R->Width = Person->Width;
  R->Height = Person->Height;
  R->RotationAngle = Angle;
  R->Position->X = X;
  R->Position->Y = Y - R->Height - 5;
  R->Tag = Speed;
  R->TagFloat = 0;
  R->Stroke->Kind = TBrushKind::None;
  R->Fill->Bitmap->WrapMode = TWrapMode::TileStretch;
  R->Fill->Kind = TBrushKind::Bitmap;
  R->Fill->Bitmap->Bitmap->Assign( Person->Fill->Bitmap->Bitmap );
  R->Parent = MapLayout1;
  R->BringToFront();
  return R;
}


TRectangle* __fastcall TGameForm::SpawnPersonOnMinimap( TRectangle *Rect )
{
  auto *R = (TRectangle*)( GetPoolObj( PeoplePoolForMinimap ) );
  R->Fill->Color = 0xFFFFFF00;
  R->Fill->Kind = TBrushKind::Solid;
  R->Width = 6;
  R->Height = 6;
  R->Position->X = Rect->Position->X / ( MapLayout1->Width / MiniMapScaledLayout->Width );
  R->Position->Y = Rect->Position->Y / ( MapLayout1->Height / MiniMapScaledLayout->Height );
  R->TagFloat = 0;
  R->Opacity = 1;
  R->Parent = MiniMapScaledLayout;
  R->BringToFront();
  return R;
}


TRectangle* __fastcall TGameForm::SpawnEnemy( float X, float Y, float Angle, int Speed )
{
  TRectangle *EnemyGraphic = nullptr;
  TBitmapListAnimation *AE = nullptr;
  TBitmapListAnimation *AnimateEnemy = nullptr;
  auto *R = (TRectangle*)( GetPoolObj( EnemyPool ) );
  R->Parent = MapLayout1;
  R->Width = 50;
  R->Height = 50;
  R->RotationAngle = 0;
  R->Position->X = X;
  R->Position->Y = Y;
  R->Tag = Speed;
  R->TagFloat = 0;
  R->TagString = ENEMY_AI_TRAVEL;
  R->Stroke->Kind = TBrushKind::None;
  R->Fill->Kind = TBrushKind::None;
  if (!R->TagObject)
  {
	EnemyGraphic = new TRectangle( R );
	EnemyGraphic->Parent = R;
  }
  else
  {
	EnemyGraphic = (TRectangle*)R->TagObject;
  }
  EnemyGraphic->TagString = ENEMY_LEVEL1;
  EnemyGraphic->TagFloat = Angle;
  EnemyGraphic->Width = R->Width;
  EnemyGraphic->Height = R->Height;
  EnemyGraphic->Position->X = ( R->Width / 2 ) - ( EnemyGraphic->Width / 2 );
  EnemyGraphic->Position->Y = ( R->Height / 2 ) - ( EnemyGraphic->Height / 2 );
  EnemyGraphic->RotationAngle = ( 360 - R->RotationAngle );
  EnemyGraphic->Stroke->Kind = TBrushKind::None;
  EnemyGraphic->Fill->Bitmap->WrapMode = TWrapMode::TileStretch;
  EnemyGraphic->Fill->Kind = TBrushKind::Bitmap;
  EnemyGraphic->Fill->Bitmap->Bitmap->Assign( Enemy1->Fill->Bitmap->Bitmap );
  if (!R->TagObject)
  {
	R->TagObject = EnemyGraphic;
  }
  if (!EnemyGraphic->TagObject)
  {
	AE = new TBitmapListAnimation( EnemyGraphic );
	EnemyGraphic->TagObject = AE;
	AE->Parent = EnemyGraphic;
  }
  else
  {
	AE = (TBitmapListAnimation*)EnemyGraphic->TagObject;
  }
  AnimateEnemy = AnimateEnemy1;
  AE->AnimationBitmap->Assign( AnimateEnemy->AnimationBitmap );
  AE->AnimationCount = AnimateEnemy->AnimationCount;
  AE->AnimationRowCount = AnimateEnemy->AnimationRowCount;
  AE->PropertyName = AnimateEnemy->PropertyName;
  AE->Duration = AnimateEnemy->Duration;
  AE->Loop = AnimateEnemy->Loop;
  AE->Start();
  R->BringToFront();
  return R;
}


TRectangle* __fastcall TGameForm::SpawnEnemyOnMinimap( TRectangle *Rect )
{
  auto *R = (TRectangle*)( GetPoolObj( EnemyPoolForMinimap ) );
  R->Fill->Color = 0xFFADD8E6;
  R->Fill->Kind = TBrushKind::Solid;
  R->Width = 5;
  R->Height = 5;
  R->Position->X = Rect->Position->X / ( MapLayout1->Width / MiniMapScaledLayout->Width );
  R->Position->Y = Rect->Position->Y / ( MapLayout1->Height / MiniMapScaledLayout->Height );
  R->TagFloat = 0;
  R->Opacity = 1;
  R->Parent = MiniMapScaledLayout;
  R->BringToFront();
  return R;
}


void __fastcall TGameForm::UpgradeEnemy( TRectangle* R, int Speed )
{
  TBitmapListAnimation *AnimateEnemy = nullptr;
  auto *EnemyGraphic = (TRectangle*)( R->TagObject );
  EnemyGraphic->Fill->Bitmap->Bitmap->Assign( Enemy2->Fill->Bitmap->Bitmap );
  R->Tag = Speed;
  auto *AE = (TBitmapListAnimation*)( EnemyGraphic->TagObject );
  if ( EnemyGraphic->TagString == ENEMY_LEVEL1 )
  {
    AnimateEnemy = AnimateEnemy2;
    EnemyGraphic->TagString = ENEMY_LEVEL2;
  }
  else
    if ( EnemyGraphic->TagString == ENEMY_LEVEL2 )
    {
      R->Tag = Speed * 2;
      AnimateEnemy = AnimateEnemy3;
      EnemyGraphic->TagString = ENEMY_LEVEL3;
    }
    else
    {
      AnimateEnemy = AnimateEnemy3;
    }
  AE->AnimationBitmap->Assign( AnimateEnemy->AnimationBitmap );
  AE->AnimationCount = AnimateEnemy->AnimationCount;
  AE->AnimationRowCount = AnimateEnemy->AnimationRowCount;
  AE->PropertyName = AnimateEnemy->PropertyName;
  AE->Duration = AnimateEnemy->Duration;
  AE->Loop = AnimateEnemy->Loop;
  AE->Start();
}


TRectangle* __fastcall TGameForm::SpawnEnemyProj( TRectangle *Source, float X, float Y, float Angle )
{
  auto *R = (TRectangle*)( GetPoolObj( ProjPool ) );
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
  R->Parent = MapLayout1;
  R->BringToFront();
  return R;
}


TRectangle* __fastcall TGameForm::SpawnCollectItem( float X, float Y, int Size )
{
  TRectangle *CollectGraphic = nullptr;
  auto *R = (TRectangle*)( GetPoolObj( CollectPool ) );
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
  R->Parent = MapLayout1;
  R->BringToFront();
  return R;
}


TRectangle* __fastcall TGameForm::SpawnExplosion( float X, float Y, float Angle )
{
  TBitmapListAnimation *AE = nullptr;
  auto *R = (TRectangle*)( GetPoolObj( ExplosionPool ) );
  R->Parent = MapLayout1;
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
  R->BringToFront();
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

void __fastcall TGameForm::SettingsFrameHostSwitchSwitch( TObject *Sender )
{
  if ( HostEnabled )
  {
	HostEnabled = false;
  }
  else
  {
	HostEnabled = true;
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


bool __fastcall TGameForm::IntersectCircle( TRectangle *R1, TRectangle *R2 )
{
  const float Distance = R1->Position->Point.Distance( R2->Position->Point );
  return Distance < ( ( R1->Width / 2 ) + ( R2->Width / 2 ) );
}


bool __fastcall TGameForm::IsAttackDistance( TRectangle *R1, TRectangle *R2, float Range )
{
  const float Distance = R1->Position->Point.Distance( R2->Position->Point );
  return Distance < Range;
}


float __fastcall TGameForm::EnemySearchAndTargetPeople( TRectangle *EnemyObj )
{
  TRectangle *PersonObj = nullptr;
  int I = 0;
  float TmpDistance = 0.0, ClosestDistance = 0.0;
  float result = EnemyObj->RotationAngle;
  if ( PeopleList->Count > 0 )
  {
	ClosestDistance = MapLayout1->Width;
	for ( int stop = 0, I = PeopleList->Count - 1; I >= stop; I--)
	{
	  TmpDistance = EnemyObj->Position->Point.Distance( ((TRectangle*)( PeopleList->Objects[I] ))->Position->Point );
	  if ( TmpDistance < ClosestDistance )
	  {
		ClosestDistance = TmpDistance;
		PersonObj = (TRectangle*)( PeopleList->Objects[I] );
	  }
	}
	if (PersonObj)
	  result = GetTargetAngle( PersonObj->Position->X, PersonObj->Position->Y, EnemyObj->Position->X, EnemyObj->Position->Y );
	else
	  result = Random( 360 );
  }
  return result;
}


float __fastcall TGameForm::EnemyGetPlayerAngle( TRectangle* EnemyObj )
{
  return GetTargetAngle( Ship->Position->X, Ship->Position->Y, EnemyObj->Position->X, EnemyObj->Position->Y );
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
  if ( AccY < - 7 )
  {
    AccDown->Visible = true;
    AccDownButtonDown = true;
  }
  else
  {
    AccDown->Visible = false;
    AccDownButtonDown = false;
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
  float MoveX = 0.0, MoveY = 0.0;
  float ProjAngle = 0.0, EnemyAngle = 0.0, EnemyProjAngle = 0.0, CollectAngle = 0.0, PersonAngle = 0.0;
  TRectangle *ProjObj = nullptr;
  TRectangle *PersonObj = nullptr;
  TRectangle *PersonObjMini = nullptr;
  TRectangle *ExplosionObj = nullptr;
  TRectangle *EnemyObj = nullptr;
  TRectangle *EnemyObjMini = nullptr;
  TRectangle *EnemyProjObj = nullptr;
  TRectangle *CollectObj = nullptr;
  unsigned int Time = 0;
  float ScreenLayoutWidthDiv2 = 0.0, ScreenLayoutWidthDiv4 = 0.0;
  float RndWarp = 0.0;
  #ifdef DEBUG_MODE
  Time = TThread::GetTickCount();
  #endif
  // Check for game over
  if ( ( PlayerData->Lives <= 0 ) || ( PeopleList->Count == 0 ) )
  {
    LevelFail();
    GameOver();
    return;
  }

  // Check for level complete
  if ( PlayerData->EnemiesDestroyed > ( ENEMY_COUNT * PlayerData->Level ) )
  {
    LevelComplete();
    return;
  }
  if ( CanResetLayout == true )
  {
    ResetLayout();
    CanResetLayout = false;
  }
  ScreenLayoutWidthDiv2 = ScreenLayout->Width / 2;
  ScreenLayoutWidthDiv4 = ScreenLayout->Width / 4;

  // process accelerometer data
  if ( MotionSensor->Active == true )
  {
    ProcessAccelerometer();
  }

  // Handle player movement and firing
  PlayerData->FireInterval = PlayerData->FireInterval + 1;
  if ( UpButtonDown || AccUpButtonDown )
  {
    if ( PlayerData->SpeedY > PlayerData->MaxReverseSpeedY )
    {
      PlayerData->SpeedY = PlayerData->SpeedY - PlayerData->AccelerationY;
    }
  }
  if ( DownButtonDown || AccDownButtonDown )
  {
    if ( PlayerData->SpeedY < PlayerData->MaxSpeedY )
    {
      PlayerData->SpeedY = PlayerData->SpeedY + PlayerData->AccelerationY;
    }
  }
  if ( RightButtonDown || AccRightButtonDown )
  {
    if ( PlayerData->SpeedX < PlayerData->MaxSpeedX )
    {
      PlayerData->SpeedX = PlayerData->SpeedX + PlayerData->AccelerationX;
    }
    Ship->RotationAngle = 90;
  }
  if ( LeftButtonDown || AccLeftButtonDown )
  {
    if ( PlayerData->SpeedX > PlayerData->MaxReverseSpeedX )
    {
	  PlayerData->SpeedX = PlayerData->SpeedX - PlayerData->AccelerationX;
    }
    Ship->RotationAngle = - 90;
  }
  if ( FireButtonDown )
  {
    FirePlayerProj();
  }
  if ( UpButtonDown || AccUpButtonDown || DownButtonDown || AccDownButtonDown || LeftButtonDown || AccLeftButtonDown || RightButtonDown || AccRightButtonDown )
  {
	Thruster->Visible = true;
  }
  else
  {
	Thruster->Visible = false;
  }
  if ( PlayerData->CanWarp == true )
  {
    RndWarp = RandomRange( (int)( ScreenLayoutWidthDiv2 ), (int)( Map1Width - ( ScreenLayoutWidthDiv2 ) ) );
    MapLayout3->Position->X = 0 - ( ( RndWarp / 4 ) - ( ScreenLayoutWidthDiv2 ) + 5 );
    MapLayout2->Position->X = 0 - ( ( RndWarp / 2 ) - ( ScreenLayoutWidthDiv2 ) + 5 );
    MapLayout1->Position->X = ( 0 - ( RndWarp - ( ScreenLayoutWidthDiv2 ) + 5 ) );
	Ship->Position->X = abs( MapLayout1->Position->X ) + ( ScreenLayoutWidthDiv2 - ( Ship->Width / 2 ) );
    PlaySound( WARP_SFX );
    PlayerData->CanWarp = false;
  }
  PlayerData->SpeedX = PlayerData->SpeedX * 0.98;
  PlayerData->SpeedY = PlayerData->SpeedY * 0.98;
  MoveX = PlayerData->SpeedX;
  MoveY = PlayerData->SpeedY;
  if ( Ship->Position->X < ( MapLayout1->Position->X + ScreenLayoutWidthDiv2 ) )
  {
    MoveX = MoveX + 15;
    PlayerData->SpeedX = 0;
  }
  else
    if ( Ship->Position->X > ( MapLayout1->Width - ScreenLayoutWidthDiv2 - 25 ) )
    {
      MoveX = MoveX - 15;
      PlayerData->SpeedX = 0;
    }
  if ( Ship->Position->Y < 0 )
  {
	Ship->Position->Y = 0;
  }
  if ( Ship->Position->Y > ( ScreenLayout->Height - Ship->Height ) )
  {
    Ship->Position->Y = ScreenLayout->Height - Ship->Height;
  }
  ShipMini->Position->X = Ship->Position->X / ( MapLayout1->Width / MiniMapScaledLayout->Width );
  ShipMini->Position->Y = Ship->Position->Y / ( MapLayout1->Height / MiniMapScaledLayout->Height );
  Ship->Position->X = Ship->Position->X + MoveX;
  Ship->Position->Y = Ship->Position->Y + MoveY;

  // update map
  MapLayout1->Position->X = MapLayout1->Position->X - MoveX;
  MapLayout2->Position->X = MapLayout2->Position->X - ( MoveX / 2 );
  MapLayout3->Position->X = MapLayout3->Position->X - ( MoveX / 4 );

  // Handle explosions
  if ( ExplosionList->Count > 0 )
    for ( int stop = 0, I = ExplosionList->Count - 1; I >= stop; I--)
    {
      ExplosionObj = (TRectangle*)( ExplosionList->Objects[I] );
      ExplosionObj->TagFloat = ExplosionObj->TagFloat + 0.1;
	  if ( ExplosionObj->TagFloat > (double)ExplosionObj->Tag )
      {
        SetPoolObj( ExplosionPool, ExplosionList->Strings[I], ExplosionObj );
        ExplosionList->Delete( I );
      }
    }

  // Handle player projectiles
  if ( ProjList->Count > 0 )
	for ( int stop = 0, I = ProjList->Count - 1; I >= stop; I--)
    {
      ProjObj = (TRectangle*)( ProjList->Objects[I] );
      if ( ProjObj->TagFloat > PlayerData->ProjDuration )
      {
        SetPoolObj( ProjPool, ProjList->Strings[I], ProjObj );
        ProjList->Delete( I );
      }
      else
      {
		ProjAngle = ProjObj->RotationAngle * PI / 180;
		ProjObj->Position->X = ProjObj->Position->X + (double)ProjObj->Tag * Cos( ProjAngle );
		ProjObj->Position->Y = ProjObj->Position->Y + (double)ProjObj->Tag * Sin( ProjAngle );
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
        if ( PeopleList->Count > 0 )
          for ( int stop = PeopleList->Count - 1, II = 0; II <= stop; II++)
          {
            PersonObj = (TRectangle*)( PeopleList->Objects[II] );
            if ( IntersectRect( PersonObj->ParentedRect, ProjObj->ParentedRect ) )
            {
              PersonObj->TagFloat = PersonObj->TagFloat + 1;
			  ProjObj->TagFloat = PlayerData->ProjDuration + 1;
              break;
            }
          }
        ProjObj->TagFloat = ProjObj->TagFloat + 0.1;
      }
    }

  // Handle enemy movement and firing
  if ( EnemyList->Count > 0 )
    for ( int stop = 0, I = EnemyList->Count - 1; I >= stop; I--)
    {
      EnemyObj = (TRectangle*)( EnemyList->Objects[I] );
      EnemyObjMini = (TRectangle*)( EnemyListForMinimap->Objects[I] );
      if ( EnemyObj->TagString == ENEMY_AI_RUN )
      {
		((TRectangle*)( EnemyObj->TagObject ))->TagFloat = 90;
        EnemyObj->TagString = ENEMY_AI_RUNNING;
      }
      else
        if ( EnemyObj->TagString == ENEMY_AI_TRAVEL )
        {
          if ( IsAttackDistance( EnemyObj, Ship, ScreenLayoutWidthDiv4 ) )
          {
            EnemyObj->TagString = ENEMY_AI_ATTACK;
          }
          else
            if ( Random( 150 ) == 1 )
              EnemyObj->TagString = ENEMY_AI_SEARCH;
        }
        else
		  if ( EnemyObj->TagString == ENEMY_AI_SEARCH )
          {
			((TRectangle*)( EnemyObj->TagObject ))->TagFloat = EnemySearchAndTargetPeople( EnemyObj ) - 90;
            EnemyObj->TagString = ENEMY_AI_TARGET;
          }
          else
            if ( EnemyObj->TagString == ENEMY_AI_UPGRADE )
            {
              EnemyObj->TagString = ENEMY_AI_ATTACK;
            }
            else
              if ( EnemyObj->TagString == ENEMY_AI_ATTACK )
              {
                if ( ! IsAttackDistance( EnemyObj, Ship, ScreenLayout->Width ) )
                {
                  EnemyObj->TagString = ENEMY_AI_TRAVEL;
                }
                else
				  ((TRectangle*)( EnemyObj->TagObject ))->TagFloat = EnemyGetPlayerAngle( EnemyObj ) - 90;
              }
      EnemyAngle = (double)( ((TRectangle*)( EnemyObj->TagObject ))->TagFloat ) * PI / 180;
	  EnemyObj->Position->X = EnemyObj->Position->X + (double)EnemyObj->Tag * Cos( EnemyAngle );
	  EnemyObj->Position->Y = EnemyObj->Position->Y + (double)EnemyObj->Tag * Sin( EnemyAngle );
      if ( PlayerData->Invulnerable == 0 )
        if ( IntersectRect( Ship->ParentedRect, EnemyObj->ParentedRect ) )
        {
          PlayerHit();
          EnemyObj->TagFloat = EnemyObj->TagFloat + 1;
        }
      if ( ( EnemyObj->TagString == ENEMY_AI_TRAVEL ) || ( EnemyObj->TagString == ENEMY_AI_SEARCH ) || ( EnemyObj->TagString == ENEMY_AI_TARGET ) )
        if ( PeopleList->Count > 0 )
		  for ( int stop = PeopleList->Count - 1, II = 0; II <= stop; II++)
          {
            PersonObj = (TRectangle*)( PeopleList->Objects[II] );
            if ( IntersectRect( PersonObj->ParentedRect, EnemyObj->ParentedRect ) & ( PersonObj->TagString == PEOPLE_STATE_NONE ) )
            {
              PersonObj->TagObject = EnemyObj;
              PersonObj->TagString = PEOPLE_STATE_CAPTURED;
              EnemyObj->TagString = ENEMY_AI_RUN;
              break;
            }
          }
      if ( EnemyObj->Position->Point.Distance( Ship->Position->Point ) < ( ScreenLayoutWidthDiv2 ) )
      {
        if ( Random( 150 ) == 1 )
        {
          EnemyProjList->AddObject( "", SpawnEnemyProj( EnemyObj, EnemyObj->Position->X, EnemyObj->Position->Y, GetTargetAngle( Ship->Position->X, Ship->Position->Y, EnemyObj->Position->X, EnemyObj->Position->Y ) ) );
          PlaySound( ALIEN_SFX );
        }
      }
      if ( CanLaunchBomb )
        if ( IntersectRect( EnemyObj->AbsoluteRect, ScreenLayout->AbsoluteRect ) )
        {
          EnemyObj->TagFloat = EnemyObj->TagFloat + 1;
        }
      if ( EnemyObj->Position->X < ( ScreenLayoutWidthDiv2 ) - 10 )
      {
        EnemyObj->Position->X = EnemyObj->Position->X + 5;
      }
      if ( EnemyObj->Position->X > Map1Width - ( ScreenLayoutWidthDiv2 ) + 10 )
      {
        EnemyObj->Position->X = EnemyObj->Position->X - 5;
	  }
      if ( EnemyObj->Position->X > MapLayout1->Width )
      {
        EnemyObj->Position->X = MapLayout1->Width - EnemyObj->Width - 10;
      }
      if ( EnemyObj->Position->Y > MapLayout1->Height )
      {
        EnemyObj->Position->Y = MapLayout1->Height - EnemyObj->Height - 10;
      }
	  EnemyObjMini->Position->X = EnemyObj->Position->X / ( MapLayout1->Width / MiniMapScaledLayout->Width );
	  EnemyObjMini->Position->Y = EnemyObj->Position->Y / ( MapLayout1->Height / MiniMapScaledLayout->Height );
      if ( ( EnemyObj->Position->Y >= ( MapLayout1->Height - ( EnemyObj->Height + 10 ) ) ) || ( EnemyObj->Position->Y <= MapLayout1->Position->Y ) )
      {
		((TRectangle*)( EnemyObj->TagObject ))->TagFloat = ((TRectangle*)( EnemyObj->TagObject ))->TagFloat + 180;
        if ( ( EnemyObj->Position->Y <= MapLayout1->Position->Y ) && ( EnemyObj->TagString == ENEMY_AI_RUNNING ) )
        {
          EnemyObj->TagString = ENEMY_AI_UPGRADE;
          UpgradeEnemy( EnemyObj, (int)( ENEMY_SPEED * 1.5 ) );
        }
      }
      if ( EnemyObj->TagFloat > 0 )
      {
        if ( EnemyObj->TagFloat > 0 )
        {
          PlayerData->EnemiesDestroyed = PlayerData->EnemiesDestroyed + 1;
          CreateExplosion( EnemyObj->Position->X + ( EnemyObj->Width / 2 ), EnemyObj->Position->Y + ( EnemyObj->Height / 2 ) );
          AddScore( 150 );
        }
        EnemyObj->TagString = ENEMY_AI_NONE;
        SetPoolObj( EnemyPool, EnemyList->Strings[I], EnemyObj );
		SetPoolObj( EnemyPoolForMinimap, EnemyListForMinimap->Strings[I], EnemyObjMini );
		EnemyList->Delete( I );
        EnemyListForMinimap->Delete( I );
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
      if ( EnemyProjObj->Position->X < ScreenLayout->Width - 10 )
      {
        EnemyProjObj->TagFloat = EnemyProjObj->TagFloat + 1;
      }
      if ( EnemyProjObj->Position->X > Map1Width - ScreenLayout->Width + 10 )
      {
        EnemyProjObj->TagFloat = EnemyProjObj->TagFloat + 1;
      }
      if ( ( EnemyProjObj->TagFloat > 0 ) || ( EnemyProjObj->Position->X > MapLayout1->Width ) || ( EnemyProjObj->Position->Y > MapLayout1->Height ) || ( EnemyProjObj->Position->X < MapLayout1->Position->X ) || ( EnemyProjObj->Position->Y < MapLayout1->Position->Y ) )
      {
        SetPoolObj( EnemyProjPool, EnemyProjList->Strings[I], EnemyProjObj );
        EnemyProjList->Delete( I );
	  }
    }

  // Spawn enemies
  if ( EnemyList->Count < 10 )
  {
    if ( Random( 30 ) == 1 )
    {
      if ( PlayerData->EnemiesSpawned <= ( ENEMY_COUNT * PlayerData->Level ) )
      {
        EnemyList->AddObject( "", SpawnEnemy( RandomRange( (int)( ScreenLayoutWidthDiv2 ), (int)( MapLayout1->Width - ScreenLayoutWidthDiv2 ) ), RandomRange( 1, (int)( MapLayout1->Height / 2 ) ), RandomRange( 1, 360 ), ENEMY_SPEED ) );
        EnemyListForMinimap->AddObject( "", SpawnEnemyOnMinimap( (TRectangle*)( EnemyList->Objects[EnemyList->Count - 1] ) ) );
        PlaySound( ALIEN_SFX );
        PlayerData->EnemiesSpawned = PlayerData->EnemiesSpawned + 1;
      }
    }
  }

  // Handle people
  if ( PeopleList->Count > 0 )
    for ( int stop = 0, I = PeopleList->Count - 1; I >= stop; I--)
    {
      PersonObj = (TRectangle*)( PeopleList->Objects[I] );
      PersonObjMini = (TRectangle*)( PeopleListForMinimap->Objects[I] );
      if ( PersonObj->TagString == PEOPLE_STATE_CAPTURED )
      {
        EnemyObj = (TRectangle*)( PersonObj->TagObject );
        if ( ( EnemyObj->TagString == ENEMY_AI_RUN ) || ( EnemyObj->TagString == ENEMY_AI_RUNNING ) )
        {
          PersonObj->Position->X = EnemyObj->Position->X + ( EnemyObj->Width / 2 - ( PersonObj->Width / 2 ) );
          PersonObj->Position->Y = EnemyObj->Position->Y + EnemyObj->Height;
		}
        else
          if ( EnemyObj->TagString == ENEMY_AI_UPGRADE )
          {
            PersonObj->TagString = PEOPLE_STATE_NONE;
			PersonObj->TagObject = nullptr;
            PersonObj->TagFloat = PersonObj->TagFloat + 1;
          }
          else
          {
            PersonObj->TagString = PEOPLE_STATE_NONE;
            PersonObj->TagObject = nullptr;
          }
      }
      else
      {
		PersonAngle = 90 * PI / 180;
		PersonObj->Position->X = PersonObj->Position->X + (double)PersonObj->Tag * Cos( PersonAngle );
		PersonObj->Position->Y = PersonObj->Position->Y + (double)PersonObj->Tag * Sin( PersonAngle );
      }
      if ( PersonObj->Position->Y > ( MapLayout1->Height - ( PersonObj->Height ) ) )
      {
        PersonObj->Position->Y = ( MapLayout1->Height - ( PersonObj->Height ) );
      }
      if ( PersonObj->Position->X < ( ScreenLayoutWidthDiv2 ) - 10 )
      {
        PersonObj->Position->X = PersonObj->Position->X + 5;
      }
      if ( PersonObj->Position->X > Map1Width - ( ScreenLayoutWidthDiv2 ) + 10 )
      {
        PersonObj->Position->X = PersonObj->Position->X - 5;
	  }
      if ( PersonObj->Position->X > MapLayout1->Width )
      {
        PersonObj->Position->X = MapLayout1->Width - PersonObj->Width - 10;
      }
      if ( PersonObj->Position->Y > MapLayout1->Height )
      {
        PersonObj->Position->Y = MapLayout1->Height - PersonObj->Height - 10;
      }
	  PersonObjMini->Position->X = PersonObj->Position->X / ( MapLayout1->Width / MiniMapScaledLayout->Width );
	  PersonObjMini->Position->Y = PersonObj->Position->Y / ( MapLayout1->Height / MiniMapScaledLayout->Height );
      if ( PersonObj->TagFloat > 0 )
      {
        SetPoolObj( PeoplePool, PeopleList->Strings[I], PersonObj );
		SetPoolObj( PeoplePoolForMinimap, PeopleListForMinimap->Strings[I], PersonObjMini );
        PeopleList->Delete( I );
        PeopleListForMinimap->Delete( I );
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
	  if ( CollectObj->ParentedRect.CenterPoint().X >= ( MapLayout1->Width + ( CollectObj->Width / 2 ) ) )
      {
		CollectObj->Position->X = ( MapLayout1->Position->X + 1 ) - ( CollectObj->Width / 2 );
	  }
	  if ( CollectObj->ParentedRect.CenterPoint().Y >= ( MapLayout1->Height + ( CollectObj->Height / 2 ) ) )
      {
		CollectObj->Position->Y = ( MapLayout1->Position->Y + 1 ) - ( CollectObj->Height / 2 );
      }
      if ( CollectObj->ParentedRect.CenterPoint().X <= ( MapLayout1->Position->X - ( CollectObj->Width / 2 ) ) )
      {
        CollectObj->Position->X = ( MapLayout1->Width - 1 );
      }
      if ( CollectObj->ParentedRect.CenterPoint().Y <= ( MapLayout1->Position->Y - ( CollectObj->Height / 2 ) ) )
      {
        CollectObj->Position->Y = ( MapLayout1->Height - 1 );
      }
      if ( IntersectRect( Ship->ParentedRect, CollectObj->ParentedRect ) )
      {
        AddScore( 5000 );
        CollectObj->TagFloat = COLLECTITEM_DURATION + 1;
        PlaySound( COLLECT_SFX );
      }
      if ( CollectObj->Position->X < ( ScreenLayoutWidthDiv2 ) - 10 )
      {
        CollectObj->Position->X = CollectObj->Position->X + 5;
      }
      if ( CollectObj->Position->X > Map1Width - ( ScreenLayoutWidthDiv2 ) + 10 )
      {
        CollectObj->Position->X = CollectObj->Position->X - 5;
      }
      if ( CollectObj->Position->X > MapLayout1->Width )
      {
        CollectObj->Position->X = MapLayout1->Width - CollectObj->Width - 10;
      }
	  if ( CollectObj->Position->Y > MapLayout1->Height )
      {
        CollectObj->Position->Y = MapLayout1->Height - CollectObj->Height - 10;
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
      CollectList->AddObject( "", SpawnCollectItem( RandomRange( (int)( ScreenLayoutWidthDiv2 ), (int)( MapLayout1->Width - ScreenLayoutWidthDiv2 ) ), RandomRange( 1, (int)( MapLayout1->Height - 1 ) ), 0 ) );
    }
  }

  // warp between portals
  if ( (Portal1) && (Portal2) )
  {
    if ( Portal1->TagFloat > 0 )
      Portal1->TagFloat = Portal1->TagFloat - 1;
    if ( Portal2->TagFloat > 0 )
      Portal2->TagFloat = Portal2->TagFloat - 1;
    if ( IntersectRect( Ship->ParentedRect, Portal1->ParentedRect ) )
    {
      if ( Portal1->TagFloat == 0 )
	  {
        Portal2->TagFloat = 30;
        WarpToObject( Portal2 );
      }
    }
    else
      if ( IntersectRect( Ship->ParentedRect, Portal2->ParentedRect ) )
      {
        if ( Portal2->TagFloat == 0 )
        {
          Portal1->TagFloat = 30;
          WarpToObject( Portal1 );
        }
      }
  }

  // bomb fire complete
  if ( CanLaunchBomb == true )
  {
    CanLaunchBomb = false;
    PlayerData->Bombs = PlayerData->Bombs - 1;
    DisplayBombs( PlayerData->Bombs );
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
    AccDown->Visible = false;
    AccDownButtonDown = false;
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
    DisplayLives( PlayerData->Lives );
    if ( PlayerData->Lives > 0 )
    {
	  PlayerData->Invulnerable = PlayerData->InvulnerableInterval;
      Ship->Opacity = Ship->Opacity - 0.25;
      InvulnerableTimer->Enabled = true;
      // CenterPlayerScreen;
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
  DelayedSettings->Enabled = false;
}


void __fastcall TGameForm::DisplayBombs( int Bombs )
{
  Bombs1->Visible = Bombs > 0;
  Bombs2->Visible = Bombs > 1;
  Bombs3->Visible = Bombs > 2;
}

void __fastcall TGameForm::FireDownEvent( TObject *Sender )
{
  FireButtonDown = true;
}

void __fastcall TGameForm::FireUpEvent( TObject *Sender )
{
  FireButtonDown = false;
}

void __fastcall TGameForm::DownDownEvent( TObject *Sender )
{
  UpButtonDown = false;
  DownButtonDown = true;
}

void __fastcall TGameForm::DownUpEvent( TObject *Sender )
{
  DownButtonDown = false;
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
  DownButtonDown = false;
  UpButtonDown = true;
}

void __fastcall TGameForm::UpUpEvent( TObject *Sender )
{
  UpButtonDown = false;
}

void __fastcall TGameForm::DownActionDownExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  DownDownEvent(Sender);
}

void __fastcall TGameForm::DownActionUpExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  DownUpEvent(Sender);
}

void __fastcall TGameForm::DownBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  DownDownEvent(Sender);
}

void __fastcall TGameForm::DownBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  DownUpEvent(Sender);
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


void __fastcall TGameForm::HighScoresFrameContinueBTNClick( TObject *Sender )
{
  CloseHighScores();
}


void __fastcall TGameForm::HUDLayoutMouseMove( TObject *Sender, TShiftState Shift, float X, float Y )
{
  ClearButtons();
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

void __fastcall TGameForm::FireActionUpExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  FireUpEvent(Sender);
}


void __fastcall TGameForm::FireBTNClick( TObject *Sender )
{
  FirePlayerProj();
}


void __fastcall TGameForm::FireBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  FireDownEvent(Sender);
}


void __fastcall TGameForm::FireBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  FireUpEvent(Sender);
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
  DelayedSettings->Enabled = true;
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
  RegisterSound( DataFilePath + "warp.wav" );
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
        case 'X': case 'x':
        {
		  LaunchPlayerBomb();
        }
        break;
        case 'C': case 'c':
        {
          PlayerWarp();
        }
        break;
        case 'W': case 'w':
		{
		  UpDownEvent(Sender);
		}
		break;
		case 'S': case 's':
		{
		  DownDownEvent(Sender);
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
		case vkDown:
		{
		  DownDownEvent(Sender);
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
	 //
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
        case INSTRUCTIONS: case SETTINGS:
        {
          Key = 0;
          ShowMainMenu();
        }
        break;
        case GAMEPLAY: case LEVEL_COMPLETE:
		{
          Key = 0;
		ExitDialog(Sender);
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
        case 'X': case 'x':
        {
              // LaunchPlayerBomb;
        }
        break;
        case 'C': case 'c':
        {
              // PlayerWarp;
        }
        break;
        case 'W': case 'w':
        {
		  UpUpEvent(Sender);
		}
		break;
		case 'S': case 's':
		{
		  DownUpEvent(Sender);
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
		case vkDown:
		{
		  DownUpEvent(Sender);
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
	 //
    break;
  }
}


void __fastcall TGameForm::FormMouseMove( TObject *Sender, TShiftState Shift, float X, float Y )
{
  ClearButtons();
}


void __fastcall TGameForm::FormResize( TObject *Sender )
{
  MapLayout1->Height = ScreenLayout->Height;
  CanResetLayout = true;
}


void __fastcall TGameForm::ResetPlasmaFence( )
{
  if (PlasmaFence1)
  {
	PlasmaFence1->Position->X = ( ScreenLayout->Width / 2 ) - PlasmaFence1->Width;
	PlasmaFence1->Height = MapLayout1->Height;
  }
  if (PlasmaFence2)
  {
	PlasmaFence2->Position->X = MapLayout1->Width - ( ScreenLayout->Width / 2 );
	PlasmaFence2->Height = MapLayout1->Height;
  }
  if (Portal1)
  {
	Portal1->Position->X = ( ScreenLayout->Width / 2 ) + 100;
	Portal1->Position->Y = ( ScreenLayout->Height / 2 ) - ( Portal1->Height / 2 );
  }
  if (Portal2)
  {
	Portal2->Position->X = MapLayout1->Width - ( ScreenLayout->Width / 2 ) - 100;
	Portal2->Position->Y = ( ScreenLayout->Height / 2 ) - ( Portal2->Height / 2 );
  }
}


void __fastcall TGameForm::ClearButtons( )
{
  switch ( CurrentStage )
  {
    case GAMEPAD:
    {
	 //
    }
	break;
   default:
   {
	 LeftButtonDown = false;
	 RightButtonDown = false;
	 UpButtonDown = false;
	 DownButtonDown = false;
	 FireButtonDown = false;
	 Thruster->Visible = false;
   }
  }
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

void __fastcall TGameForm::UpBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  UpDownEvent(Sender);
}


void __fastcall TGameForm::UpBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  UpUpEvent(Sender);
}


void __fastcall TGameForm::WarpActionClickExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  PlayerWarp();
}


void __fastcall TGameForm::WarpBTNClick( TObject *Sender )
{
  PlayerWarp();
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

void __fastcall TGameForm::LeftActionDownExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  LeftDownEvent(Sender);
}

void __fastcall TGameForm::LeftActionUpExecute( TObject* Sender )
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


void __fastcall TGameForm::ShowGamePad( )
{
  ShowMsgBox( NO_GAMEPAD );
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
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  RightDownEvent(Sender);
}

void __fastcall TGameForm::RightActionUpExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
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

void __fastcall TGameForm::LaunchPlayerBomb( )
{
  if ( PlayerData->Bombs > 0 )
    CanLaunchBomb = true;
}


void __fastcall TGameForm::PlayerWarp( )
{
  if ( PlayerData->CanWarp == false )
    PlayerData->CanWarp = true;
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
    AccDown->Visible = false;
    AccDownButtonDown = false;
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


void __fastcall TGameForm::CreateEnemies( )
{
  int I = 0;
  for ( int stop = 10, I = 0; I <= stop; I++)
  {
    EnemyPool->AddObject( "", new TRectangle( this ) );
  }
}


void __fastcall TGameForm::CreateExplosions( )
{
  int I = 0;
  for ( int stop = 10, I = 0; I <= stop; I++)
  {
    ExplosionPool->AddObject( "", new TRectangle( this ) );
  }
}


void __fastcall TGameForm::CreateEnemyProj( )
{
  int I = 0;
  for ( int stop = 10, I = 0; I <= stop; I++)
  {
    EnemyProjPool->AddObject( "", new TRectangle( this ) );
  }
}


void __fastcall TGameForm::CreateFencesAndPortals( )
{
  PlasmaFence1 = new TRectangle( nullptr );
  PlasmaFence1->Parent = MapLayout1;
  PlasmaFence1->Height = MapLayout1->Height;
  PlasmaFence1->Position->X = ( ScreenLayout->Width / 2 ) - PlasmaFence1->Width;
  PlasmaFence1->Stroke->Kind = TBrushKind::None;
  BitmapToRectangle( PlasmaFence->Fill->Bitmap->Bitmap, PlasmaFence1 );
  PlasmaFence2 = new TRectangle( nullptr );
  PlasmaFence2->Parent = MapLayout1;
  PlasmaFence2->Height = MapLayout1->Height;
  PlasmaFence2->Position->X = MapLayout1->Width - ( ScreenLayout->Width / 2 );
  PlasmaFence2->Stroke->Kind = TBrushKind::None;
  BitmapToRectangle( PlasmaFence->Fill->Bitmap->Bitmap, PlasmaFence2 );
  Portal1 = new TRectangle( nullptr );
  Portal1->Parent = MapLayout1;
  Portal1->Width = Portal->Fill->Bitmap->Bitmap->Width;
  Portal1->Height = Portal->Fill->Bitmap->Bitmap->Height;
  Portal1->Position->X = ( ScreenLayout->Width / 2 ) + 100;
  Portal1->Position->Y = ( ScreenLayout->Height / 2 ) - ( Portal1->Height / 2 );
  Portal1->Opacity = 0.75;
  Portal1->Stroke->Kind = TBrushKind::None;
  BitmapToRectangle( Portal->Fill->Bitmap->Bitmap, Portal1 );
  Portal2 = new TRectangle( nullptr );
  Portal2->Parent = MapLayout1;
  Portal2->Width = Portal->Fill->Bitmap->Bitmap->Width;
  Portal2->Height = Portal->Fill->Bitmap->Bitmap->Height;
  Portal2->Position->X = MapLayout1->Width - ( ScreenLayout->Width / 2 ) - 100;
  Portal2->Position->Y = ( ScreenLayout->Height / 2 ) - ( Portal2->Height / 2 );
  Portal2->Opacity = 0.75;
  Portal2->Stroke->Kind = TBrushKind::None;
  BitmapToRectangle( Portal->Fill->Bitmap->Bitmap, Portal2 );
}


void __fastcall TGameForm::CreatePeople( )
{
  int I = 0;
  for ( int stop = PEOPLE_COUNT, I = 0; I <= stop; I++)
  {
    PeopleList->AddObject( "", SpawnPerson( RandomRange( (int)( ScreenLayout->Width / 2 ), (int)( MapLayout1->Width - ( ScreenLayout->Width / 2 ) ) ), MapLayout1->Height, 0, 1 ) );
    PeopleListForMinimap->AddObject( "", SpawnPersonOnMinimap( (TRectangle*)( PeopleList->Objects[I] ) ) );
  }
}


void __fastcall TGameForm::InitGame( )
{
  ProjList = new TStringList();
  ProjPool = new TStringList();
  ExplosionList = new TStringList();
  ExplosionPool = new TStringList();
  EnemyList = new TStringList();
  EnemyListForMinimap = new TStringList();
  EnemyPool = new TStringList();
  EnemyPoolForMinimap = new TStringList();
  EnemyProjList = new TStringList();
  EnemyProjPool = new TStringList();
  CollectList = new TStringList();
  CollectPool = new TStringList();
  PeopleList = new TStringList();
  PeopleListForMinimap = new TStringList();
  PeoplePool = new TStringList();
  PeoplePoolForMinimap = new TStringList();
  CreateEnemies();
  CreateExplosions();
  CreateEnemyProj();
  CreatePeople();
  Map3Width = MapLayout3->Width;
  Map3Height = MapLayout3->Height;
  Map2Width = MapLayout2->Width;
  Map2Height = MapLayout2->Height;
  Map1Width = MapLayout1->Width;
  Map1Height = ScreenLayout->Height;
  MapLayout3->Position->X = 0 - ( ( Map3Width / 2 ) - ( ScreenLayout->Width / 2 ) + 5 );
  MapLayout3->Position->Y = 0 - ( ( Map3Height / 2 ) - ( ScreenLayout->Height / 2 ) + 5 );
  MapLayout2->Position->X = 0 - ( ( Map2Width / 2 ) - ( ScreenLayout->Width / 2 ) + 5 );
  MapLayout2->Position->Y = 0 - ( ( Map2Height / 2 ) - ( ScreenLayout->Height / 2 ) + 5 );
  MapLayout1->Position->X = 0 - ( ( Map1Width / 2 ) - ( ScreenLayout->Width / 2 ) + 5 );
  MapLayout1->Position->Y = 0 - ( ( Map1Height / 2 ) - ( ScreenLayout->Height / 2 ) + 5 );
  CreateFencesAndPortals();
  Ship->Parent = MapLayout1;
  CleanedUp = false;
  Ship->RotationAngle = 90;
  CenterPlayer();
}


void __fastcall TGameForm::InitPlayer( )
{
  PlayerData = new TPlayerData( this );
  PlayerData->Health = PLAYER_HEALTH;
  PlayerData->Lives = PLAYER_LIVES;
  PlayerData->Bombs = PLAYER_BOMBS;
  PlayerData->SpeedX = 0;
  PlayerData->SpeedY = 0;
  PlayerData->EnemiesDestroyed = 0;
  PlayerData->EnemiesSpawned = 0;
  PlayerData->MaxSpeedX = PLAYER_SPEED;
  PlayerData->MaxSpeedY = PLAYER_SPEED;
  PlayerData->MaxReverseSpeedX = PLAYER_SPEED * - 1;
  PlayerData->MaxReverseSpeedY = PLAYER_SPEED * - 1;
  PlayerData->AccelerationX = 1;
  PlayerData->AccelerationY = 0.5;
  PlayerData->Level = 1;
  PlayerData->FireSpeed = 5;
  PlayerData->FireInterval = PlayerData->FireSpeed;
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
    BitmapToRectangle( Bomb->Fill->Bitmap->Bitmap, Bombs1 );
    BitmapToRectangle( Bomb->Fill->Bitmap->Bitmap, Bombs2 );
    BitmapToRectangle( Bomb->Fill->Bitmap->Bitmap, Bombs3 );
  }
  DisplayLives( PlayerData->Lives );
  DisplayHealth( PlayerData->Health );
  DisplayScore();
  DisplayBombs( PlayerData->Bombs );
  MapLayout3->Position->Y = 0 - ( ( Map3Height / 2 ) - ( ScreenLayout->Height / 2 ) + 5 );
  MapLayout2->Position->Y = 0 - ( ( Map2Height / 2 ) - ( ScreenLayout->Height / 2 ) + 5 );
  MapLayout1->Height = ScreenLayout->Height;
  CenterPlayer();
  Ship->TagObject = PlayerData;
}


void __fastcall TGameForm::BitmapToRectangle( TBitmap *B, TRectangle *R )
{
  R->Fill->Bitmap->WrapMode = TWrapMode::TileStretch;
  R->Fill->Kind = TBrushKind::Bitmap;
  R->Fill->Bitmap->Bitmap->Assign( B );
}


void __fastcall TGameForm::BombActionClickExecute( TObject *Sender )
{
  // called from RunRemoteActionAsync. Requires TThread::Synchronize for UI changes.
  LaunchPlayerBomb();
}


void __fastcall TGameForm::BombBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y )
{
  LaunchPlayerBomb();
}


void __fastcall TGameForm::AnimateExplosionFinish( TObject *Sender )
{
  auto *animation = (TBitmapListAnimation *)Sender;
  ((TRectangle*)animation->Owner)->Visible = false;
}


void __fastcall TGameForm::CenterPlayer( )
{
  Ship->Position->X = ( MapLayout1->Width / 2 ) - ( Ship->Width / 2 );
  Ship->Position->Y = ( MapLayout1->Height / 2 ) - ( Ship->Height / 2 );
  ShipMini->Position->X = ( MiniMapScaledLayout->Width / 2 ) - ( ShipMini->Width / 2 );
  ShipMini->Position->Y = ( MiniMapScaledLayout->Height / 2 ) - ( ShipMini->Height / 2 );
}


void __fastcall TGameForm::WarpToObject( TRectangle *R )
{
  if (R)
  {
	MapLayout3->Position->X = 0 - ( ( R->Position->X / 4 ) - ( ScreenLayout->Width / 2 ) + 5 );
	MapLayout2->Position->X = 0 - ( ( R->Position->X / 2 ) - ( ScreenLayout->Width / 2 ) + 5 );
	MapLayout1->Position->X = ( 0 - ( R->Position->X - ( ScreenLayout->Width / 2 ) + 5 ) );
	Ship->Position->X = abs( MapLayout1->Position->X ) + ( ScreenLayout->Width / 2 - ( Ship->Width / 2 ) );
    Ship->Position->Y = R->Position->Y + ( R->Height / 2 ) - ( Ship->Height / 2 );
    PlaySound( WARP_SFX );
  }
}


void __fastcall TGameForm::CenterPlayerScreen( )
{
  MapLayout3->Position->X = 0 - ( ( Map3Width / 2 ) - ( ScreenLayout->Width / 2 ) + 5 );
  MapLayout2->Position->X = 0 - ( ( Map2Width / 2 ) - ( ScreenLayout->Width / 2 ) + 5 );
  MapLayout1->Position->X = 0 - ( ( Map1Width / 2 ) - ( ScreenLayout->Width / 2 ) + 5 );
  Ship->Position->X = ( MapLayout1->Width / 2 ) - ( Ship->Width / 2 );
  Ship->Position->Y = ( MapLayout1->Height / 2 ) - ( Ship->Height / 2 );
}


void __fastcall TGameForm::CleanupGame( bool Continue )
{
  if ( CleanedUp == false )
  {
	ProjList->OwnsObjects = true;
	ProjPool->OwnsObjects = true;
	ExplosionList->OwnsObjects = true;
	ExplosionPool->OwnsObjects = true;
	EnemyList->OwnsObjects = true;
	EnemyListForMinimap->OwnsObjects = true;
	EnemyPool->OwnsObjects = true;
	EnemyPoolForMinimap->OwnsObjects = true;
	EnemyProjList->OwnsObjects = true;
	EnemyProjPool->OwnsObjects = true;
	CollectList->OwnsObjects = true;
	CollectPool->OwnsObjects = true;
	PeopleList->OwnsObjects = true;
	PeopleListForMinimap->OwnsObjects = true;
	PeoplePool->OwnsObjects = true;
	PeoplePoolForMinimap->OwnsObjects = true;
	PlasmaFence1->DisposeOf();
	PlasmaFence1 = nullptr;
	PlasmaFence2->DisposeOf();
	PlasmaFence2 = nullptr;
	Portal1->DisposeOf();
	Portal1 = nullptr;
	Portal2->DisposeOf();
	Portal2 = nullptr;
	if ( Continue == false ) {
	  destroy(PlayerData);
	}
	destroy(ProjList);
	destroy(ProjPool);
	destroy(ExplosionList);
	destroy(ExplosionPool);
	destroy(EnemyList);
	destroy(EnemyListForMinimap);
	destroy(EnemyPool);
	destroy(EnemyPoolForMinimap);
	destroy(EnemyProjList);
	destroy(EnemyProjPool);
	destroy(CollectList);
	destroy(CollectPool);
	destroy(PeopleList);
	destroy(PeopleListForMinimap);
	destroy(PeoplePool);
	destroy(PeoplePoolForMinimap);
	CleanedUp = true;
  }
}


void __fastcall TGameForm::PlayGame( )
{
  SetStage( GAMEPLAY );
  StartGameLoop();
}


void __fastcall TGameForm::ContinueGame( )
{
  CleanupGame( true );
  InitGame();
  PlayerData->SpeedX = 0;
  PlayerData->SpeedY = 0;
  PlayerData->Level = PlayerData->Level + 1;
  PlayerData->EnemiesDestroyed = 0;
  PlayerData->EnemiesSpawned = 0;
  PlayGame();
}


void __fastcall TGameForm::LevelFail( )
{
  StopGameLoop();
  PlayerData->SpeedX = 0;
  PlayerData->SpeedY = 0;
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


void __fastcall TGameForm::ResetLayout( )
{
  MapLayout3->Position->Y = 0 - ( ( Map3Height / 2 ) - ( ScreenLayout->Height / 2 ) + 5 );
  MapLayout2->Position->Y = 0 - ( ( Map2Height / 2 ) - ( ScreenLayout->Height / 2 ) + 5 );
  MapLayout1->Height = ScreenLayout->Height;
  CenterPlayerScreen();
  ResetPeople();
  ResetPlasmaFence();
}


void __fastcall TGameForm::ResetPeople( )
{
  int I = 0;
  if (PeopleList)
    if ( PeopleList->Count > 0 )
      for ( int stop = PeopleList->Count - 1, I = 0; I <= stop; I++)
      {
		auto *PersonObj = (TRectangle*)( PeopleList->Objects[I] );
        PersonObj->Position->Y = ( ScreenLayout->Position->Y + ScreenLayout->Height ) - PersonObj->Height;
      }
}


void __fastcall TGameForm::OrientationChanged(System::TObject * const Sender, System::Messaging::TMessageBase * const Msg)
{
  if ( CurrentStage == GAMEPLAY )
  {
	MapLayout3->Position->Y = 0 - ( ( Map3Height / 2 ) - ( ScreenLayout->Height / 2 ) + 5 );
	MapLayout2->Position->Y = 0 - ( ( Map2Height / 2 ) - ( ScreenLayout->Height / 2 ) + 5 );
    MapLayout1->Height = ScreenLayout->Height;
    CanResetLayout = true;
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

void __fastcall RegisterRenderingSetup( )
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
  // enable the GPU on Windows
  //GlobalUseGPUCanvas = true;
  uGame_initialization();
}
};
static uGame_unit _uGame_unit;

//---------------------------------------------------------------------------

