#ifndef uGamepadH
#define uGamepadH


#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Objects.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <IPPeerClient.hpp>
#include <IPPeerServer.hpp>
#include <System.Tether.AppProfile.hpp>
#include <System.Tether.Manager.hpp>

class TFrameGamepad: public TFrame {
__published:	// IDE-managed Components
  TRectangle *BackgroundRect;
  TLabel *Label1;
  TTetheringAppProfile *GPTetheringAppProfile;
  TRectangle *FireBTN;
  TRectangle *LeftBTN;
  TRectangle *RightBTN;
  TLayout *Layout1;
  TRectangle *CloseBTN;
  TRectangle *ConnectBTN;
  TTetheringManager *GPTetheringManager;
  TLayout *Layout2;
  TProgressBar *ProgressBar;
  TTimer *GPTimer;
  void __fastcall LeftBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall LeftBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall UpBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall UpBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall RightBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall RightBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall FireBTNMouseDown( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall FireBTNMouseUp( TObject *Sender, TMouseButton Button, TShiftState Shift, float X, float Y );
  void __fastcall GPTetheringManagerEndManagersDiscovery( const TObject *Sender, const TTetheringManagerInfoList *RemoteManagers );
  void __fastcall GPTetheringManagerEndProfilesDiscovery( const TObject *Sender, const TTetheringProfileInfoList RemoteProfiles );
  void __fastcall GPTetheringManagerRemoteManagerShutdown( const TObject *Sender, const String ManagerIdentifier );
  void __fastcall ConnectBTNClick( TObject* Sender );
	void __fastcall GPTimerTimer(TObject *Sender);
  private:
    /* Private declarations */
  bool FIsConnected;
  int IdConnectCount;
  void __fastcall CheckRemoteProfiles( );
  void __fastcall ButtonAction( int idaction );
  public:
    /* Public declarations */
  String TetheringName;
	__fastcall TFrameGamepad(TComponent* Owner);
  void __fastcall ClearButtons( );
  void __fastcall HandleKeyDown( TObject* Sender, WORD& Key, Char& KeyChar, TShiftState Shift );
  void __fastcall HandleKeyUp( TObject* Sender, WORD& Key, Char& KeyChar, TShiftState Shift );
};


const int LEFTACTIONDOWN_C = 11;
const int LEFTACTIONUP_C = 12;
const int RIGHTACTIONDOWN_C = 21;
const int RIGHTACTIONUP_C = 22;
const int UPACTIONDOWN_C = 31;
const int UPACTIONUP_C = 32;
const int FIREACTIONDOWN_C = 51;
const int FIREACTIONUP_C = 52;

//---------------------------------------------------------------------------
extern PACKAGE TFrameGamepad *FrameGamepad;
//---------------------------------------------------------------------------
#endif //  uGamepadH