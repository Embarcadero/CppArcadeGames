#ifndef uInstructionsH
#define uInstructionsH


#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Objects.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>

class TFrameInstructions: public TFrame {
__published:	// IDE-managed Components
  TRectangle *Rectangle1;
  TButton *ContinueBTN;
  TText *Text1;
  TLabel *Label1;
  private:
    /* Private declarations */
  public:
    /* Public declarations */
	__fastcall TFrameInstructions(TComponent* Owner);
};

//---------------------------------------------------------------------------
extern PACKAGE TFrameInstructions *FrameInstructions;
//---------------------------------------------------------------------------
#endif //  uInstructionsH