//---------------------------------------------------------------------------

#ifndef uGameH
#define uGameH
//---------------------------------------------------------------------------

#include <System.SysUtils.hpp>
#include <System.Types.hpp>
#include <System.Uitypes.hpp>
#include <System.Classes.hpp>
#include <System.Variants.hpp>
#include <System.IniFiles.hpp>
#include <System.Messaging.hpp>
#include <System.Ioutils.hpp>
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
#include <System.Sensors.hpp>
#include <System.Sensors.Components.hpp>
#include <IPPeerClient.hpp>
#include <IPPeerServer.hpp>
#include <System.Actions.hpp>
#include <FMX.ActnList.hpp>
#include <System.Tether.Manager.hpp>
#include <System.Tether.AppProfile.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdTCPConnection.hpp>
#include <IdTCPClient.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.DialogService.Async.hpp>
#include "AudioManager.h"
#include "uGameOver.h"
#include "uGamepad.h"
#include "uHighScores.h"
#include "uInstructions.h"
#include "uLevelComplete.h"
#include "uMainMenu.h"
#include "uSettings.h"

//---------------------------------------------------------------------------

class TPlayerData: public TFmxObject{
  public:
  __fastcall virtual TPlayerData(System::Classes::TComponent* AOwner): TFmxObject(AOwner)   {;};
  __fastcall virtual  ~TPlayerData(void){};// { this->TFmxObject::~TFmxObject();};

  float SpeedX;
  float SpeedY;
  float Thrust;
  int Lives;
  int Score;
  int Level;
  int Stars;

  int Fuel;
  float VerticalVelocity;
  float HorizontalVelocity;
  bool Landed;
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
	TTimer *GameLoop;
	TLayout *ScreenLayout;
	TLayout *AssetLayout;
	TStyleBook *StyleBook;
	TRectangle *Explosion;
	TBitmapListAnimation *AnimateExplosion;
	TRectangle *CollectItem;
	TBitmapListAnimation *AnimateCollectItem;
	TRectangle *LeftBTN;
	TRectangle *RightBTN;
	TRectangle *Lives1;
	TRectangle *Lives2;
	TRectangle *Lives3;
	TLabel *ScoreLBL;
	TRectangle *FireBTN;
	TLabel *FPSLBL;
	TRectangle *SoundBTN;
	TLine *SoundOffLine;
	TRectangle *MusicBTN;
	TLine *MusicOffLine;
	TLayout *HUDLayout;
	TMediaPlayer *MusicPlayer;
	TLabel *FuelLBL;
	TLabel *AngleLBL;
	TLabel *VVLBL;
	TRectangle *CliffRect;
	TRectangle *Ship;
	TRectangle *Thruster;
	TRectangle *GyroBTN;
	TLine *GyroOffLine;
	TRectangle *InterfaceBackground;
	TRectangle *LandingPad;
	TCircle *AccLeft;
	TCircle *AccRight;
	TCircle *AccThrustCircle;
	TCircle *AboveCircle;
	TFrameMainMenu *MainMenuFrame;
	TFrameSettings *SettingsFrame;
	TFrameGameOver *GameOverFrame;
	TFrameGamepad *GamepadFrame;
	TFrameInstructions *InstructionsFrame;
	TFrameHighScores *HighScoresFrame;
	TFrameLevelComplete *LevelCompleteFrame;
	TTetheringManager *TetheringManager;
	TTetheringAppProfile *TetheringAppProfile;
	TMotionSensor *MotionSensor;
	TActionList *ActionList;
	TAction *LeftActionDown;
	TAction *LeftActionUp;
	TAction *RightActionDown;
	TAction *RightActionUp;
	TAction *UpActionDown;
	TAction *UpActionUp;
	TAction *DownActionDown;
	TAction *DownActionUp;
	TAction *FireActionDown;
	TAction *FireActionUp;
	TAction *WarpActionClick;
	TAction *BombActionClick;
	TIdTCPClient *IdTCPClient1;
	TTimer *DelayedSettings;
  void __fastcall GameLoopTimer( TObject *Sender );
  void __fastcall RightBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall RightBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall LeftBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall LeftBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall FormMouseMove( TObject *Sender, TShiftState Shift, float X, float Y );
  void __fastcall FireBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall FireBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall FormDestroy( TObject *Sender );
  void __fastcall MainMenuFramePlayBTNClick( TObject *Sender );
  void __fastcall InstructionsFrameContinueBTNClick( TObject *Sender );
  void __fastcall GameOverFramePlayAgainBTNClick( TObject *Sender );
  void __fastcall GameOverFrameMainMenuBTNClick( TObject *Sender );
  void __fastcall GameOverFrameMoreGamesBTNClick( TObject *Sender );
  void __fastcall MainMenuFrameMoreGamesBTNClick( TObject *Sender );
  void __fastcall HighScoresFrameContinueBTNClick( TObject *Sender );
  void __fastcall MainMenuFrameHighScoresBTNClick( TObject *Sender );
  void __fastcall FormKeyDown( TObject *Sender, WORD& Key, Char& KeyChar, TShiftState Shift );
  void __fastcall FormKeyUp( TObject *Sender, WORD& Key, Char& KeyChar, TShiftState Shift );
  void __fastcall LevelCompleteFrameContinueBTNClick( TObject *Sender );
  void __fastcall FormCreate( TObject *Sender );
  void __fastcall AnimateExplosionFinish( TObject *Sender );
  void __fastcall MusicBTNClick( TObject *Sender );
  void __fastcall SoundBTNClick( TObject *Sender );
  void __fastcall GyroBTNClick( TObject *Sender );
  void __fastcall FormResize( TObject *Sender );
  void __fastcall GamepadFrameCloseBTNClick( TObject *Sender );
  void __fastcall SettingsFrameHostSwitchSwitch( TObject *Sender );
  void __fastcall SettingsFrameMainMenuBTNClick( TObject *Sender );
  void __fastcall MainMenuFrameSettingsBTNClick( TObject *Sender );
  void __fastcall MainMenuFrameGamepadBTNClick( TObject *Sender );
  void __fastcall LeftActionDownExecute( TObject *Sender );
  void __fastcall LeftActionUpExecute( TObject *Sender );
  void __fastcall RightActionDownExecute( TObject *Sender );
  void __fastcall RightActionUpExecute( TObject *Sender );
  void __fastcall UpActionDownExecute( TObject *Sender );
  void __fastcall UpActionUpExecute( TObject *Sender );
  void __fastcall FireActionDownExecute( TObject *Sender );
  void __fastcall FireActionUpExecute( TObject *Sender );
  void __fastcall SettingsFrameFullScreenSwitchSwitch( TObject *Sender );
  void __fastcall HUDLayoutMouseMove( TObject *Sender, TShiftState Shift, float X, float Y );
  void __fastcall HighScoresFrameOkayBTNClick( TObject *Sender );
  void __fastcall HighScoresFrameCancelBTNClick( TObject *Sender );
  void __fastcall DelayedSettingsTimer( TObject *Sender );
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
private:	// User declarations
  bool LeftButtonDown;
  bool RightButtonDown;
  bool FireButtonDown;
  bool AccLeftButtonDown;
  bool AccRightButtonDown;
  bool AccFireButtonDown;
  TStringList *GroundList;
  TStringList *ExplosionList;
  TStringList *CollectList;
  TPlayerData *PlayerData;
  TStringList *GroundPool;
  TStringList *ExplosionPool;
  TStringList *CollectPool;
  TAudioManager *AudioManager;
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
  float CatchLastPos;
  bool RScreenOut;
  bool LScreenOut;
  bool CleanedUp;
  bool NetworkConnected;
  bool NetworkChecking;
  TScreenOrientation ScreenOrientation;
  int OrientationChangedId;
//  void __fastcall OrientationChanged( const TObject *Sender, const TMessage* Msg );
  void __fastcall OrientationChanged(System::TObject * const Sender, System::Messaging::TMessageBase * const Msg);
  TScreenOrientation __fastcall GetScreenOrientation( );
  void __fastcall ReOrientgame( );
  void __fastcall StartGame( );
  void __fastcall PlayGame( );
  void __fastcall StartGameLoop( );
  void __fastcall StopGameLoop( );
  void __fastcall ShowMainMenu( );
  void __fastcall ShowHighScores( );
  void __fastcall ShowMoreGames( );
  void __fastcall ShowGamePad( );
  void __fastcall InitPlayer( );
  void __fastcall LevelComplete( );
  void __fastcall LevelFail( );
  void __fastcall GameOver( );
  void __fastcall InitAndPlayGame( );
  void __fastcall InitGame( );
  void __fastcall ShowHUD( int Stage );
  void __fastcall HideHUD( int Stage );
  void __fastcall ContinueGame( );
  void __fastcall CloseHighScores( );
  void __fastcall CleanupGame( bool Continue );
  void __fastcall ClearButtons( );
  void __fastcall LeftDownEvent(TObject *Sender);
  void __fastcall LeftUpEvent(TObject *Sender);
  void __fastcall RightDownEvent(TObject *Sender);
  void __fastcall RightUpEvent(TObject *Sender);
  void __fastcall UpDownEvent(TObject *Sender);
  void __fastcall UpUpEvent(TObject *Sender);
  void __fastcall CenterPlayer( );
  void __fastcall AddScore( int I );
  void __fastcall DisplayLives( int Lives );
  void __fastcall DisplayScore( );
  void __fastcall PlayerHit( );
  TRectangle* __fastcall SpawnCollectItem( float X, float Y, int Size );
  TRectangle* __fastcall SpawnExplosion( float X, float Y, float Angle );
  void __fastcall CreateExplosion( float X, float Y );
  TRectangle* __fastcall SpawnGround( float X, float Y, int GroundType, float LandingPadWidth );
  void __fastcall ProcessAccelerometer( );
  void __fastcall AssignInterfaceBackground( TRectangle* R );
  void __fastcall ResetGround( );
  void __fastcall BitmapToRectangle( TBitmap* B, TRectangle* R );
  bool __fastcall IntersectCircle( TRectangle R1, TRectangle R2 );
  float __fastcall GetTargetAngle( float TargetX, float TargetY, float OriginX, float OriginY );
  TRectangle* __fastcall GetPoolObj( TStringList* Pool );
  void __fastcall SetPoolObj( TStringList* Pool, String Name, TRectangle* Obj );
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
  void __fastcall ShowSettings( );
  void __fastcall SaveAndExitSettings( );
  bool __fastcall CheckNetworkState( );
  void __fastcall PollNetworkState( );
  void __fastcall ToggleAppTethering( bool State );

public:		// User declarations
  bool __fastcall HandleAppEvent( TApplicationEvent AAppEvent, TObject* AContext );
  void __fastcall ShowMsgBox( String S );
  void __fastcall ShowMsgBoxResultProc(TObject* Sender, const TModalResult AResult);
  void __fastcall ExitDialog(TObject *Sender);
  void __fastcall ExitGameMsgBoxResultProc(TObject* Sender, const TModalResult AResult);
  void PollNetworkStateAnonThread();
  void __fastcall LevelExitProc();
  void __fastcall FireActionDownThread();
  void __fastcall FireActionUpThread();
	__fastcall TGameForm(TComponent* Owner);
};

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
const int PLAYER_LIVES = 3;
const int PLAYER_FUEL = 500; // lower to increase fuel

const double PLAYER_THRUST = 0.15;
const int PLAYER_LANDING_VELOCITY = 4; // try 2 for a harder landing

const double GAME_GRAVITY = 0.05;
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

const int COLLECTITEM_SPEED = 0;
  // Network Connections

const char NETWORK_ADDRESS[] = "www.embarcadero.com";
const char NO_NETWORK_MESSAGE[] = "A network connection is needed for this feature and it was not detected.";

const double PI = 3.1415926;
//---------------------------------------------------------------------------
extern PACKAGE TGameForm *GameForm;
//---------------------------------------------------------------------------
#endif

