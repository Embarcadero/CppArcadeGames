//---------------------------------------------------------------------------

#ifndef uSettingsH
#define uSettingsH


#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Objects.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>


class TFrameSettings: public TFrame {
__published:	// IDE-managed Components
  TRectangle *BackgroundRect;
  TLabel *HeaderLBL;
  TButton *MainMenuBTN;
  TLayout *ControllerHostLayout;
  TSwitch *HostSwitch;
  TLabel *HostLBL;
  TLayout *GyroLayout;
  TSwitch *GyroSwitch;
  TLabel *GyroLBL;
  TLayout *MusicLayout;
  TSwitch *MusicSwitch;
  TLabel *MusicLBL;
  TLayout *SoundLayout;
  TSwitch *SoundSwitch;
  TLabel *SoundLBL;
  TLayout *FullScreenLayout;
  TSwitch *FullScreenSwitch;
  TLabel *FullScreenLBL;
  private:
    /* Private declarations */
  public:
    /* Public declarations */
	__fastcall TFrameSettings(TComponent* Owner);
};

//---------------------------------------------------------------------------
extern PACKAGE TFrameSettings *FrameSettings;
//---------------------------------------------------------------------------
#endif //  uSettingsH