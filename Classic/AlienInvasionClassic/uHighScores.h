#ifndef uHighScoresH
#define uHighScoresH


#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <Data.Bind.Components.hpp>
#include <Data.Bind.DBScope.hpp>
#include <Data.Bind.EngExt.hpp>
#include <Data.DB.hpp>
#include <FireDAC.Comp.Client.hpp>
#include <FireDAC.Comp.DataSet.hpp>
#include <FireDAC.Comp.UI.hpp>
#include <FireDAC.DApt.hpp>
#include <FireDAC.DApt.Intf.hpp>
#include <FireDAC.DatS.hpp>
#include <FireDAC.FMXUI.Wait.hpp>
#include <FireDAC.Phys.hpp>
#include <FireDAC.Phys.Intf.hpp>
#include <FireDAC.Phys.SQLite.hpp>
#include <FireDAC.Phys.SQLiteDef.hpp>
#include <FireDAC.Stan.Async.hpp>
#include <FireDAC.Stan.Def.hpp>
#include <FireDAC.Stan.Error.hpp>
#include <FireDAC.Stan.ExprFuncs.hpp>
#include <FireDAC.Stan.Intf.hpp>
#include <FireDAC.Stan.Option.hpp>
#include <FireDAC.Stan.Param.hpp>
#include <FireDAC.Stan.Pool.hpp>
#include <FireDAC.UI.Intf.hpp>
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
#include <FireDAC.Phys.SQLiteWrapper.Stat.hpp>

class TFrameHighScores: public TFrame {
__published:	// IDE-managed Components
  TFDConnection *FireScoresList;
  TFDTable *FDTableHighScores;
  TFDQuery *FDQueryDelete;
  TFDQuery *FDQueryInsert;
  TFDPhysSQLiteDriverLink *FDPhysSQLiteDriverLink1;
  TFDGUIxWaitCursor *FDGUIxWaitCursor1;
  TBindSourceDB *BindSourceDB1;
  TBindingsList *BindingsList1;
  TButton *ContinueBTN;
  TRectangle *Rectangle1;
  TListView *ListView1;
  TLinkFillControlToField *LinkFillControlToField2;
  TWideMemoField *FDTableHighScoresName;
  TIntegerField *FDTableHighScoresScore;
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