#ifndef uHighScoresH
#define uHighScoresH


#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <Fmx.Bind.DBEngExt.hpp>
#include <Fmx.Bind.Editors.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Edit.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.ListView.Adapters.Base.hpp>
#include <FMX.ListView.Appearances.hpp>
#include <FMX.ListView.hpp>
#include <FMX.ListView.Types.hpp>
#include <FMX.Objects.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <System.Bindings.Outputs.hpp>
#include <System.Rtti.hpp>
#include <System.Ioutils.hpp>
#include <Data.Bind.Components.hpp>
#include <Data.Bind.DBScope.hpp>
#include <Data.Bind.EngExt.hpp>

class TFrameHighScores: public TFrame {
__published:	// IDE-managed Components
  TBindSourceDB *BindSourceDB1;
  TBindingsList *BindingsList1;
  TButton *ContinueBTN;
  TRectangle *Rectangle1;
  TListView *ListView1;
  TLinkFillControlToField *LinkFillControlToField2;
  TLabel *Label1;
  TRectangle *InputBGRect;
  TRectangle *InputBoxRect;
  TLabel *Label2;
  TEdit *InputEdit;
  TGridPanelLayout *GridPanelLayout1;
  TButton *OkayBTN;
  TButton *CancelBTN;
  TLine *Line1;
  void __fastcall FireScoresListAfterConnect( TObject* Sender );
  void __fastcall FireScoresListBeforeConnect( TObject* Sender );
  void __fastcall OkayBTNClick( TObject* Sender );
  void __fastcall CancelBTNClick( TObject* Sender );
  private:
    /* Private declarations */
  bool Loaded;
  public:
    /* Public declarations */
	__fastcall TFrameHighScores(TComponent* Owner);
  void __fastcall AddScore( String Name, int Score );
  void __fastcall SaveScore( String Name, int Score );
  void __fastcall InitFrame( );
  void __fastcall CloseInputBox( );
};

//---------------------------------------------------------------------------
extern PACKAGE TFrameHighScores *FrameHighScores;
//---------------------------------------------------------------------------
#endif //  uHighScoresH