#ifndef uLevelCompleteH
#define uLevelCompleteH


#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Objects.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>



class TFrameLevelComplete: public TFrame {
__published:	// IDE-managed Components
  TLabel *Label1;
  TButton *ContinueBTN;
  TRectangle *Rectangle1;
  TText *InfoText;
  TGridPanelLayout*	GridPanelLayout1;
  TRectangle*	CollectItem1;
  TRectangle*	CollectItem2;
  TRectangle*	CollectItem3;
  private:
    /* Private declarations */
  public:
    /* Public declarations */
	__fastcall TFrameLevelComplete(TComponent* Owner);
};

//---------------------------------------------------------------------------
extern PACKAGE TFrameLevelComplete *FrameLevelComplete;
//---------------------------------------------------------------------------
#endif //  uLevelCompleteH
