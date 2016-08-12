//---------------------------------------------------------------------------

#ifndef uGameH
#define uGameH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Ani.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Media.hpp>
#include <FMX.Objects.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.DialogService.ASync.hpp>
#include <FMX.ActnList.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>
#include <IPPeerClient.hpp>
#include <IPPeerServer.hpp>
#include <System.Actions.hpp>
#include <System.Sensors.Components.hpp>
#include <System.Sensors.hpp>
#include <System.Tether.AppProfile.hpp>
#include <System.Tether.Manager.hpp>
#include "uGameOver.h"
#include "uHighScores.h"
#include "uInstructions.h"
#include "uLevelComplete.h"
#include "uSettings.h"
#include "uGamepad.h"
#include "uMainMenu.h"
#include "AudioManager.h"
#include <memory>
//---------------------------------------------------------------------------

class TPlayerData: public TFmxObject {
  public:
  __fastcall virtual TPlayerData(System::Classes::TComponent* AOwner): TFmxObject(AOwner)
  {
   SpeedX= SpeedY= MaxSpeedX= MaxSpeedY= MaxReverseSpeedX= MaxReverseSpeedY= AccelerationX= AccelerationY = 0.0;
   EnemiesDestroyed= EnemiesSpawned = 0;
   CanWarp = false;
   Lives= Health= Bombs= Invulnerable= InvulnerableInterval= Score= Level= FireSpeed= FireInterval = 0;
   ProjDuration = 0.0;
  };
  __fastcall virtual  ~TPlayerData(void){};

  float SpeedX;
  float SpeedY;
  float MaxSpeedX;
  float MaxSpeedY;
  float MaxReverseSpeedX;
  float MaxReverseSpeedY;
  float AccelerationX;
  float AccelerationY;
  int EnemiesDestroyed;
  int EnemiesSpawned;
  bool CanWarp;
  int Lives;
  int Health;
  int Bombs;
  int Invulnerable;
  int InvulnerableInterval;
  int Score;
  int Level;
  int FireSpeed;
  int FireInterval;
  float ProjDuration;
};

//---------------------------------------------------------------------------
class TMyAnonymousProcedure : public TCppInterfacedObject<TProc>
{
public:
	typedef    void	(__closure *TMyFunction)();
	TMyAnonymousProcedure(TMyFunction AFunction){FFunction = AFunction;}
	void __fastcall Invoke(void){FFunction();}
private:
	TMyFunction	FFunction;
};
TMyAnonymousProcedure*	AnonymousLambda(TMyAnonymousProcedure::TMyFunction i)
{
	return new TMyAnonymousProcedure(i);
}
//---------------------------------------------------------------------------


class TGameForm : public TForm
{
__published:	// IDE-managed Components
	TLayout *MapLayout3;
	TRectangle *Rectangle3;
	TLayout *MapLayout2;
	TRectangle *Rectangle2;
	TLayout *MapLayout1;
	TRectangle *Rectangle1;
	TRectangle *MountainsRect;
	TLayout *ScreenLayout;
	TRectangle *Ship;
	TRectangle *Thruster;
	TTimer *GameLoop;
	TFrameInstructions *InstructionsFrame;
	TFrameGameOver *GameOverFrame;
	TFrameLevelComplete *LevelCompleteFrame;
	TTimer *InvulnerableTimer;
	TFrameHighScores *HighScoresFrame;
	TFrameMainMenu *MainMenuFrame;
	TFrameGamepad *GamepadFrame;
	TTetheringManager *TetheringManager;
	TTetheringAppProfile *TetheringAppProfile;
	TStyleBook *StyleBook;
	TMediaPlayer *MusicPlayer;
	TFrameSettings *SettingsFrame;
	TLayout *HUDLayout;
	TGridPanelLayout *MiniMapGrid;
	TLayout *MiniMapScaledLayout;
	TRectangle *MiniMapShadow;
	TRectangle *MiniMountains;
	TRectangle *ShipMini;
	TCircle *AccUp;
	TCircle *AccLeft;
	TCircle *AccDown;
	TCircle *AccRight;
	TRectangle *FireBTN;
	TRectangle *UpBTN;
	TLabel *FPSLBL;
	TRectangle *LeftBTN;
	TRectangle *Lives1;
	TRectangle *Lives2;
	TRectangle *Lives3;
	TRectangle *MusicBTN;
	TLine *MusicOffLine;
	TRectangle *RightBTN;
	TLabel *ScoreLBL;
	TRectangle *SoundBTN;
	TLine *SoundOffLine;
	TRectangle *Health1;
	TRectangle *Health2;
	TRectangle *Health3;
	TRectangle *DownBTN;
	TRectangle *BombBTN;
	TRectangle *WarpBTN;
	TLayout *AssetLayout;
	TRectangle *Projectile1;
	TRectangle *Explosion;
	TBitmapListAnimation *AnimateExplosion;
	TRectangle *EnemyProjectile;
	TRectangle *CollectItem;
	TRectangle *HealthBar;
	TRectangle *Person;
	TRectangle *Enemy1;
	TBitmapListAnimation *AnimateEnemy1;
	TRectangle *Enemy2;
	TBitmapListAnimation *AnimateEnemy2;
	TRectangle *Enemy3;
	TBitmapListAnimation *AnimateEnemy3;
	TRectangle *Bomb;
	TRectangle *PlasmaFence;
	TRectangle *Portal;
	TRectangle *Projectile2;
	TRectangle *Projectile3;
	TRectangle *Projectile4;
	TRectangle *Projectile5;
	TRectangle *Projectile6;
	TRectangle *GyroBTN;
	TLine *GyroOffLine;
	TRectangle *Bombs1;
	TRectangle *Bombs2;
	TRectangle *Bombs3;
	TMotionSensor *MotionSensor;
	TTimer *DelayedSettings;
	TIdTCPClient *IdTCPClient1;

	TActionList *ActionList;

	TAction *LeftActionDown;
	TAction *RightActionDown;
	TAction *UpActionDown;
	TAction *DownActionDown;
	TAction *LeftActionUp;
	TAction *RightActionUp;
	TAction *UpActionUp;
	TAction *DownActionUp;
	TAction *FireActionDown;
	TAction *FireActionUp;
	TAction *WarpActionClick;
	TAction *BombActionClick;

  void __fastcall GameLoopTimer( TObject *Sender );
  void __fastcall RightBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall RightBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall LeftBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall LeftBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall FireBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall FireBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall FormDestroy( TObject *Sender );
  void __fastcall FireBTNClick( TObject *Sender );
  void __fastcall MainMenuFramePlayBTNClick( TObject *Sender );
  void __fastcall InstructionsFrameContinueBTNClick( TObject *Sender );
  void __fastcall GameOverFramePlayAgainBTNClick( TObject *Sender );
  void __fastcall GameOverFrameMainMenuBTNClick( TObject *Sender );
  void __fastcall GameOverFrameMoreGamesBTNClick( TObject *Sender );
  void __fastcall MainMenuFrameMoreGamesBTNClick( TObject *Sender );
  void __fastcall InvulnerableTimerTimer( TObject *Sender );
  void __fastcall UpBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall UpBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall HighScoresFrameContinueBTNClick( TObject *Sender );
  void __fastcall MainMenuFrameHighScoresBTNClick( TObject *Sender );
  void __fastcall FormKeyDown( TObject *Sender, WORD& Key, Char& KeyChar, TShiftState Shift );
  void __fastcall FormKeyUp( TObject *Sender, WORD& Key, Char& KeyChar, TShiftState Shift );
  void __fastcall LevelCompleteFrameContinueBTNClick( TObject *Sender );
  void __fastcall FormCreate( TObject *Sender );
  void __fastcall AnimateExplosionFinish( TObject *Sender );
  void __fastcall MusicBTNClick( TObject *Sender );
  void __fastcall SoundBTNClick( TObject *Sender );
  void __fastcall DownBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall DownBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall BombBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall WarpBTNClick( TObject *Sender );
  void __fastcall FormResize( TObject *Sender );
  void __fastcall GyroBTNClick( TObject *Sender );
  void __fastcall LeftActionDownExecute( TObject *Sender );
  void __fastcall LeftActionUpExecute( TObject *Sender );
  void __fastcall RightActionDownExecute( TObject *Sender );
  void __fastcall RightActionUpExecute( TObject *Sender );
  void __fastcall UpActionDownExecute( TObject *Sender );
  void __fastcall UpActionUpExecute( TObject *Sender );
  void __fastcall DownActionDownExecute( TObject *Sender );
  void __fastcall DownActionUpExecute( TObject *Sender );
  void __fastcall SettingsFrameMainMenuBTNClick( TObject *Sender );
  void __fastcall FireActionDownExecute( TObject *Sender );
  void __fastcall FireActionUpExecute( TObject *Sender );
  void __fastcall BombActionClickExecute( TObject *Sender );
  void __fastcall WarpActionClickExecute( TObject *Sender );
  void __fastcall MainMenuFrameSettingsBTNClick( TObject *Sender );
  void __fastcall SettingsFrameHostSwitchSwitch( TObject *Sender );
  void __fastcall SettingsFrameFullScreenSwitchSwitch( TObject *Sender );
  void __fastcall MainMenuFrameGamepadBTNClick( TObject *Sender );
  void __fastcall GamepadFrameCloseBTNClick( TObject *Sender );
  void __fastcall FormMouseMove( TObject *Sender, TShiftState Shift, float X, float Y );
  void __fastcall HUDLayoutMouseMove( TObject *Sender, TShiftState Shift, float X, float Y );
  void __fastcall DelayedSettingsTimer( TObject *Sender );
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
private:	// User declarations
  bool LeftButtonDown;
  bool RightButtonDown;
  bool UpButtonDown;
  bool DownButtonDown;
  bool FireButtonDown;
  bool AccLeftButtonDown;
  bool AccRightButtonDown;
  bool AccUpButtonDown;
  bool AccDownButtonDown;
  bool CanLaunchBomb;
  TStringList *ProjList;
  TStringList *ExplosionList;
  TStringList *EnemyList;
  TStringList *EnemyListForMinimap;
  TStringList *EnemyProjList;
  TStringList *CollectList;
  TStringList *CollectPool;
  TStringList *PeopleList;
  TStringList *PeopleListForMinimap;
  TStringList *PeoplePool;
  TStringList *PeoplePoolForMinimap;
  TPlayerData *PlayerData;
  TStringList *ProjPool;
  TStringList *ExplosionPool;
  TStringList *EnemyPool;
  TStringList *EnemyPoolForMinimap;
  TStringList *EnemyProjPool;
  std::unique_ptr<TAudioManager> AudioManager;
  int CurrentStage;
  int LastStage;
  String DataFilePath;
  String SettingsFilePath;
  bool MusicEnabled;
  bool SoundEnabled;
  bool GyroEnabled;
  bool HostEnabled;
  bool FullScreenEnabled;
  bool LivesSet;
  bool CanResetLayout;
  bool CleanedUp;
  float Map1Width, Map1Height;
  float Map2Width, Map2Height;
  float Map3Width, Map3Height;
  TRectangle *PlasmaFence1, *PlasmaFence2;
  TRectangle *Portal1, *Portal2;
  TScreenOrientation ScreenOrientation;
  int OrientationChangedId;
  bool NetworkConnected;
  bool NetworkChecking;
  void __fastcall ResetPeople( );
  void __fastcall ResetPlasmaFence( );
  void __fastcall ResetLayout( );
  void __fastcall OrientationChanged(System::TObject * const Sender, System::Messaging::TMessageBase * const Msg);
  TScreenOrientation __fastcall GetScreenOrientation( );
  void __fastcall StartGame( );
  void __fastcall PlayGame( );
  void __fastcall StartGameLoop( );
  void __fastcall StopGameLoop( );
  void __fastcall ShowMainMenu( );
  void __fastcall ShowHighScores( );
  void __fastcall ShowMoreGames( );
  void __fastcall ShowSettings( );
  void __fastcall ShowGamePad( );
  void __fastcall SaveAndExitSettings( );
  void __fastcall InitAndPlayGame( );
  void __fastcall InitPlayer( );
  void __fastcall LevelComplete( );
  void __fastcall LevelFail( );
  void __fastcall GameOver( );
  void __fastcall InitGame( );
  void __fastcall ShowHUD( );
  void __fastcall HideHUD( );
  void __fastcall ContinueGame( );
  void __fastcall CloseHighScores( );
  void __fastcall CleanupGame( bool Continue );
  void __fastcall ClearButtons( );
  void __fastcall FirePlayerProj( );
  void __fastcall FireDownEvent(TObject *Sender);
  void __fastcall FireUpEvent(TObject *Sender);
  void __fastcall DownDownEvent(TObject *Sender);
  void __fastcall DownUpEvent(TObject *Sender);
  void __fastcall LeftDownEvent(TObject *Sender);
  void __fastcall LeftUpEvent(TObject *Sender);
  void __fastcall RightDownEvent(TObject *Sender);
  void __fastcall RightUpEvent(TObject *Sender);
  void __fastcall UpDownEvent(TObject *Sender);
  void __fastcall UpUpEvent(TObject *Sender);
  void __fastcall LaunchPlayerBomb( );
  void __fastcall CenterPlayer( );
  void __fastcall CenterPlayerScreen( );
  void __fastcall AddScore( int I );
  void __fastcall DisplayLives( int Lives );
  void __fastcall DisplayHealth( int Health );
  void __fastcall DisplayScore( );
  void __fastcall DisplayBombs( int Bombs );
  void __fastcall PlayerHit( );
  bool __fastcall IsAttackDistance( TRectangle *R1, TRectangle *R2, float Range );
  TRectangle* __fastcall SpawnProj( TRectangle *Source, float X, float Y, float Angle );
  TRectangle* __fastcall SpawnCollectItem( float X, float Y, int Size );
  TRectangle* __fastcall SpawnPerson( float X, float Y, float Angle, int Speed );
  TRectangle* __fastcall SpawnPersonOnMinimap( TRectangle *Rect );
  TRectangle* __fastcall SpawnEnemy( float X, float Y, float Angle, int Speed );
  TRectangle* __fastcall SpawnEnemyOnMinimap( TRectangle *Rect );
  void __fastcall UpgradeEnemy( TRectangle *R, int Speed );
  TRectangle* __fastcall SpawnEnemyProj( TRectangle *Source, float X, float Y, float Angle );
  TRectangle* __fastcall SpawnExplosion( float X, float Y, float Angle );
  void __fastcall CreateExplosion( float X, float Y );
  void __fastcall CreateEnemies( );
  void __fastcall CreateExplosions( );
  void __fastcall CreateEnemyProj( );
  void __fastcall CreateFencesAndPortals( );
  void __fastcall CreatePeople( );
  void __fastcall BitmapToRectangle( TBitmap *B, TRectangle *R );
  bool __fastcall IntersectCircle( TRectangle *R1, TRectangle *R2 );
  float __fastcall GetTargetAngle( float TargetX, float TargetY, float OriginX, float OriginY );
  TRectangle* __fastcall GetPoolObj( TStringList *Pool );
  void __fastcall SetPoolObj( TStringList *Pool, String Name, TRectangle *Obj );
  void __fastcall PauseBitmapListAnimations( bool Value );
  void __fastcall PlaySound( int Index );
  int __fastcall RegisterSound( String Filename );
  void __fastcall PlayMusic( );
  void __fastcall StopMusic( );
  float __fastcall GetMaxVolume( );
  void __fastcall GyroToggle( );
  void __fastcall SoundToggle( );
  void __fastcall MusicToggle( );
  void __fastcall SetStage( int Stage );
  void __fastcall SaveSettings( );
  void __fastcall LoadSettings( );
  float __fastcall EnemySearchAndTargetPeople( TRectangle *EnemyObj );
  float __fastcall EnemyGetPlayerAngle( TRectangle *EnemyObj );
  void __fastcall WarpToObject( TRectangle *R );
  void __fastcall PlayerWarp( );
  void __fastcall ProcessAccelerometer( );
  bool __fastcall CheckNetworkState( );
  void __fastcall PollNetworkState( );
  void __fastcall ToggleAppTethering( bool State );
public:		// User declarations
  bool __fastcall HandleAppEvent( TApplicationEvent AAppEvent, TObject* AContext );
  void __fastcall ShowMsgBox( String S );
  void __fastcall ExitDialog(TObject *Sender);
  void  PollNetworkStateAnonThread();
	__fastcall TGameForm(TComponent* Owner);
};
//---------------------------------------------------------------------------

//***************************************************************************

class TRenderingSetupCallbackRef : public TCppInterfacedObject<TRenderingSetupCallback>
{
public:
	virtual void __fastcall Invoke(System::TObject* const Sender, System::TObject* const Context, int &ColorBits, int &DepthBits, bool &Stencil, int &Multisamples)
	{
	  // Override OpenGL rendering setup to use custom values.
	  ColorBits = 16; // default is 24
	  DepthBits = 0; // default is 24
	  Stencil = false; // default is True
	  Multisamples = 0; // default depends on TForm.Quality or TForm3D.Multisample
	}
};

//***************************************************************************
  // Player
const int PLAYER_HEALTH = 3;
const int PLAYER_LIVES = 3;
const int PLAYER_BOMBS = 3;
  // Stages

const int MAIN_MENU = 0;
const int INSTRUCTIONS = 1;
const int GAMEPLAY = 2;
const int LEVEL_COMPLETE = 3;
const int GAME_OVER = 4;
const int HIGH_SCORES = 5;
const int SETTINGS = 6;
const int GAMEPAD = 7;
  // Sounds

const int FIRE_SFX = 0;
const int EXPLOSION_SFX = 1;
const int FAIL_SFX = 2;
const int WIN_SFX = 3;
const int ALIEN_SFX = 4;
const int COLLECT_SFX = 5;
const int WARP_SFX = 6;
  // Music

const char MUSIC_FILENAME[] = "music.mp3";
  // Movement Speeds

const int PROJ_SPEED = 36;
const int PLAYER_SPEED = 10;
const int ENEMYPROJ_SPEED = 10;
const int ENEMY_SPEED = 3;
const int COLLECTITEM_SPEED = 1;
const int COLLECTITEM_DURATION = 6;
  // Gameplay

const int PEOPLE_COUNT = 15;
const int ENEMY_COUNT = 20;
  // People States

const char PEOPLE_STATE_NONE[] = "";
const char PEOPLE_STATE_CAPTURED[] = "captured";
  // Enemy AI States

const char ENEMY_AI_NONE[] = "";
const char ENEMY_AI_RUN[] = "run";
const char ENEMY_AI_RUNNING[] = "running";
const char ENEMY_AI_TRAVEL[] = "travel";
const char ENEMY_AI_SEARCH[] = "search";
const char ENEMY_AI_TARGET[] = "target";
const char ENEMY_AI_UPGRADE[] = "upgrade";
const char ENEMY_AI_ATTACK[] = "attack";
  // Enemy Levels

const char ENEMY_LEVEL1[] = "level1";
const char ENEMY_LEVEL2[] = "level2";
const char ENEMY_LEVEL3[] = "level3";
  // Network Connections

const char NETWORK_ADDRESS[] = "www.embarcadero.com";
const char NO_NETWORK_MESSAGE[] = "A network connection is needed for this feature and it was not detected.";

const double PI = 3.1415926;

extern PACKAGE TGameForm *GameForm;
//---------------------------------------------------------------------------
#endif
