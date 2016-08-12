//---------------------------------------------------------------------------

#ifndef uGameH
#define uGameH
//---------------------------------------------------------------------------
#include <System.SysUtils.hpp>
#include <System.Types.hpp>
#include <System.UITypes.hpp>
#include <System.Classes.hpp>
#include <System.Variants.hpp>
#include <System.IniFiles.hpp>
#include <System.Messaging.hpp>
#include <System.IOUtils.hpp>
#include <FMX.Types.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Graphics.hpp>
#include <FMX.Dialogs.hpp>
#include <FMX.Objects.hpp>
#include <FMX.Layouts.hpp>
#include <System.Math.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Ani.hpp>
#include <FMX.Media.hpp>
#include <FMX.Platform.hpp>
#include <FMX.Filter.Effects.hpp>
#include <FMX.Effects.hpp>
#include <IPPeerClient.hpp>
#include <IPPeerServer.hpp>
#include <System.Sensors.hpp>
#include <System.Sensors.Components.hpp>
#include <System.Actions.hpp>
#include <FMX.ActnList.hpp>
#include <System.Tether.Manager.hpp>
#include <System.Tether.AppProfile.hpp>
#include <FMX.DialogService.ASync.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdTCPConnection.hpp>
#include <IdTCPClient.hpp>
#include <FMX.Controls.Presentation.hpp>
#include "AudioManager.h"
#include "uMainMenu.h"
#include "uInstructions.h"
#include "uLevelComplete.h"
#include "uGameOver.h"
#include "uHighScores.h"
#include "uSettings.h"
#include "uGamepad.h"
#include <memory>
//---------------------------------------------------------------------------

class TPlayerData: public TFmxObject {
  public:
  __fastcall virtual TPlayerData(System::Classes::TComponent* AOwner): TFmxObject(AOwner)
  {
	SpeedX= SpeedY= Thrust= Decay= CurrentSpeed= MaxSpeed= RotationSpeed = 0.0;
	Lives= Health= Invulnerable= InvulnerableInterval= Score= Level= FireSpeed= FireInterval = 0;
	ProjDuration = 0.0;
  };
  __fastcall virtual  ~TPlayerData(void){};

  float SpeedX;
  float SpeedY;
  float Thrust;
  float Decay;
  float CurrentSpeed;
  float MaxSpeed;
  float RotationSpeed;
  int Lives;
  int Health;
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
	TLayout *ScreenLayout;
	TLayout *HUDLayout;
	TCircle *AccUp;
	TRectangle *ForwardBTN;
	TCircle *AccLeft;
	TCircle *AccRight;
	TLayout *AssetLayout;
	TRectangle *Asteroid1;
	TRectangle *Enemy;
	TRectangle *Projectile;
	TRectangle *Explosion;
	TBitmapListAnimation *AnimateExplosion;
	TRectangle *EnemyProjectile;
	TRectangle *CollectItem;
	TBitmapListAnimation *AnimateCollectItem;
	TRectangle *HealthBar;
	TBitmapListAnimation *BitmapListAnimation1;
	TRectangle *Asteroid2;
	TRectangle *Asteroid3;
	TRectangle *FireBTN;
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
	TRectangle *GyroBTN;
	TLine *GyroOffLine;
	TRectangle *Ship;
	TRectangle *Thruster;
	TTimer *GameLoop;
	TFrameInstructions *InstructionsFrame;
	TFrameGameOver *GameOverFrame;
	TFrameLevelComplete *LevelCompleteFrame;
	TTimer *InvulnerableTimer;
	TStyleBook *StyleBook;
	TMediaPlayer *MusicPlayer;
	TFrameSettings *SettingsFrame;
	TFrameHighScores *HighScoresFrame;
	TFrameMainMenu *MainMenuFrame;
	TFrameGamepad *GamepadFrame;
	TTetheringManager *TetheringManager;
	TTetheringAppProfile *TetheringAppProfile;
	TMotionSensor *MotionSensor;
	TTimer *DelayedSettings;
    TIdTCPClient *IdTCPClient1;
	TActionList *ActionList;
	TAction* LeftActionDown;
	TAction* LeftActionUp;
	TAction* RightActionDown;
	TAction* RightActionUp;
	TAction* UpActionDown;
	TAction* UpActionUp;
	TAction* FireActionDown;
	TAction* FireActionUp;

  void __fastcall GameLoopTimer( TObject *Sender );
  void __fastcall RightBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall RightBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall LeftBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall LeftBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall FormMouseMove( TObject *Sender, TShiftState Shift, float X, float Y );
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
  void __fastcall ForwardBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall ForwardBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall HighScoresFrameContinueBTNClick( TObject *Sender );
  void __fastcall MainMenuFrameHighScoresBTNClick( TObject *Sender );
  void __fastcall FormKeyDown( TObject *Sender, WORD& Key, Char& KeyChar, TShiftState Shift );
  void __fastcall FormKeyUp( TObject *Sender, WORD& Key, Char& KeyChar, TShiftState Shift );
  void __fastcall LevelCompleteFrameContinueBTNClick( TObject *Sender );
  void __fastcall FormCreate( TObject *Sender );
  void __fastcall AnimateExplosionFinish( TObject *Sender );
  void __fastcall MusicBTNClick( TObject *Sender );
  void __fastcall SoundBTNClick( TObject *Sender );
  void __fastcall GamepadFrameCloseBTNClick( TObject *Sender );
  void __fastcall LeftActionDownExecute( TObject *Sender );
  void __fastcall LeftActionUpExecute( TObject *Sender );
  void __fastcall RightActionDownExecute( TObject *Sender );
  void __fastcall RightActionUpExecute( TObject *Sender );
  void __fastcall UpActionDownExecute( TObject *Sender );
  void __fastcall UpActionUpExecute( TObject *Sender );
  void __fastcall SettingsFrameMainMenuBTNClick( TObject *Sender );
  void __fastcall FireActionDownExecute( TObject *Sender );
  void __fastcall FireActionUpExecute( TObject *Sender );
  void __fastcall SettingsFrameFullScreenSwitchSwitch( TObject *Sender );
  void __fastcall SettingsFrameHostSwitchSwitch( TObject *Sender );
  void __fastcall MainMenuFrameGamepadBTNClick( TObject *Sender );
  void __fastcall MainMenuFrameSettingsBTNClick( TObject *Sender );
  void __fastcall GyroBTNClick( TObject *Sender );
  void __fastcall DelayedSettingsTimer( TObject *Sender );
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);

private:	// User declarations
  bool LeftButtonDown;
  bool RightButtonDown;
  bool ForwardButtonDown;
  bool FireButtonDown;
  bool AccLeftButtonDown;
  bool AccRightButtonDown;
  bool AccUpButtonDown;
  TStringList *RockList;
  TStringList *ProjList;
  TStringList *ExplosionList;
  TStringList *EnemyList;
  TStringList *EnemyProjList;
  TStringList *CollectList;
  TPlayerData *PlayerData;
  TStringList *RockPool;
  TStringList *ProjPool;
  TStringList *ExplosionPool;
  TStringList *EnemyPool;
  TStringList *EnemyProjPool;
  TStringList *CollectPool;
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
  bool CanSwapXY;
  bool CleanedUp;
  TScreenOrientation ScreenOrientation;
  int OrientationChangedId;
  bool NetworkConnected;
  bool NetworkChecking;
  void __fastcall SwapListXY( TStringList* List );
  void __fastcall SwapXY( );
//  void __fastcall OrientationChanged( const TObject *Sender, const TMessage* Msg );
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
  void __fastcall MusicToggle( );
  void __fastcall SoundToggle( );
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
  void __fastcall LeftDownEvent(TObject *Sender);
  void __fastcall LeftUpEvent(TObject *Sender);
  void __fastcall RightDownEvent(TObject *Sender);
  void __fastcall RightUpEvent(TObject *Sender);
  void __fastcall UpDownEvent(TObject *Sender);
  void __fastcall UpUpEvent(TObject *Sender);
  void __fastcall CenterPlayer( );
  void __fastcall AddScore( int I );
  void __fastcall DisplayLives( int Lives );
  void __fastcall DisplayHealth( int Health );
  void __fastcall DisplayScore( );
  void __fastcall PlayerHit( );
  TRectangle* __fastcall SpawnProj( TRectangle *Source, float X, float Y, float Angle );
  TRectangle* __fastcall SpawnRock( float X, float Y, int Size );
  TRectangle* __fastcall SpawnCollectItem( float X, float Y, int Size );
  TRectangle* __fastcall SpawnEnemy( float X, float Y, float Angle, int Speed );
  TRectangle* __fastcall SpawnEnemyProj( TRectangle *Source, float X, float Y, float Angle );
  TRectangle* __fastcall SpawnExplosion( float X, float Y, float Angle );
  void __fastcall CreateExplosion( float X, float Y );
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
  void __fastcall SetStage( int Stage );
  void __fastcall SaveSettings( );
  void __fastcall LoadSettings( );
  void __fastcall GyroToggle( );
  void __fastcall ProcessAccelerometer( );
  bool __fastcall CheckNetworkState( );
  void __fastcall PollNetworkState( );
  void __fastcall ToggleAppTethering( bool State );
public:		// User declarations
  bool __fastcall HandleAppEvent( TApplicationEvent AAppEvent, TObject* AContext );
  void __fastcall ShowMsgBox( String S );
  void __fastcall ExitDialog(TObject *Sender);
  void PollNetworkStateAnonThread();
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
    // Music

const char MUSIC_FILENAME[] = "music.mp3";
    // Movement Speeds

const int PROJ_SPEED = 12;
const int PLAYER_SPEED = 5;
const int ENEMYPROJ_SPEED = 10;
const int ENEMY_SPEED = 7;
const int ROCK_SPEED = 2;
const int COLLECTITEM_SPEED = 1;
const int COLLECTITEM_DURATION = 6;
    // Network Connections

const char NETWORK_ADDRESS[] = "www.embarcadero.com";
const char NO_NETWORK_MESSAGE[] = "A network connection is needed for this feature and it was not detected.";

const double PI = 3.1415926;

extern PACKAGE TGameForm *GameForm;
//---------------------------------------------------------------------------
#endif
