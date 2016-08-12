#ifndef uGameOverH
#define uGameOverH


#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Objects.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>

class TFrameGameOver: public TFrame {
__published:	// IDE-managed Components
  TButton *MainMenuBTN;
  TButton *MoreGamesBTN;
  TButton *PlayAgainBTN;
  TRectangle *Rectangle1;
  TLabel *Label1;
  private:
    /* Private declarations */
  public:
    /* Public declarations */
	__fastcall TFrameGameOver(TComponent* Owner);
};

//---------------------------------------------------------------------------
extern PACKAGE TFrameGameOver *FrameGameOver;
//---------------------------------------------------------------------------
#endif //  uGameOverH
