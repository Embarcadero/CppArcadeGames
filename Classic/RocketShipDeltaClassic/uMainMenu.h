//---------------------------------------------------------------------------

#ifndef uMainMenuH
#define uMainMenuH


#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Objects.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>


class TFrameMainMenu: public TFrame {
__published:	// IDE-managed Components
  TRectangle *Rectangle1;
  TButton *PlayBTN;
  TButton *HighScoresBTN;
  TImage *LogoImage;
  TRectangle *Rectangle2;
  TLayout *Layout1;
  TButton *GamepadBTN;
  TButton *SettingsBTN;
  TButton *MoreGamesBTN;
  private:
    /* Private declarations */
  public:
    /* Public declarations */
	__fastcall TFrameMainMenu(TComponent* Owner);
};

//---------------------------------------------------------------------------
extern PACKAGE TFrameMainMenu *FrameMainMenu;
//---------------------------------------------------------------------------
#endif //  uMainMenuH