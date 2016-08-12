//---------------------------------------------------------------------------

// This software is Copyright (c) 2016 Embarcadero Technologies, Inc.
// You may only use this software if you are an authorized licensee
// of Delphi, C++Builder or RAD Studio (Embarcadero Products).
// This software is considered a Redistributable as defined under
// the software license agreement that comes with the Embarcadero Products
// and is subject to that software license agreement.

//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "uHighScores.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TFrameHighScores *FrameHighScores;
//---------------------------------------------------------------------------
__fastcall TFrameHighScores::TFrameHighScores(TComponent* Owner)
	: TFrame(Owner)
{
}


void __fastcall TFrameHighScores::CancelBTNClick( TObject* Sender )
{
  CloseInputBox();
}


void __fastcall TFrameHighScores::FireScoresListAfterConnect( TObject* Sender )
{
  FireScoresList->ExecSQL( "CREATE TABLE IF NOT EXISTS HighScores (Name TEXT NOT NULL, Score INTEGER)" );
}


void __fastcall TFrameHighScores::FireScoresListBeforeConnect( TObject* Sender )
{
  FireScoresList->Params->Values["Database"] = System::Ioutils::TPath::GetDocumentsPath() + PathDelim + "HighScores.s3db";
}


void __fastcall TFrameHighScores::AddScore( String Name, int Score )
{
  if ( Name.Trim() != "" )
  {
    SaveScore( Name, Score );
  }
  else
  {
    InputEdit->Tag = Score;
    InputBGRect->Visible = true;
	InputBGRect->BringToFront();
    InputBoxRect->Visible = true;
	InputBoxRect->BringToFront();
  }
}


void __fastcall TFrameHighScores::SaveScore( String Name, int Score )
{
  if ( Name.Trim() != "" )
  {
    FDQueryInsert->ParamByName( "Name" )->AsString = Name;
    FDQueryInsert->ParamByName( "Score" )->AsInteger = Score;
    FDQueryInsert->ExecSQL( );
	FDTableHighScores->Refresh();
	LinkFillControlToField2->BindList->FillList();
  }
}


void __fastcall TFrameHighScores::InitFrame( )
{
  if ( Loaded == false )
  {
    try
    {
      // For unidirectional dataset, don't refill automatically when dataset is activated
      // because dataset is reactivated everytime use DataSet->First.
      LinkFillControlToField2->AutoActivate = false;
      LinkFillControlToField2->AutoFill = false;
      FireScoresList->Connected = true;
      FDTableHighScores->Active = true;
	  LinkFillControlToField2->BindList->FillList();
    }
    catch( Exception & E )
    {
    {
      ShowMessage( E.Message );
    }
    }
    Loaded = true;
  }
}


void __fastcall TFrameHighScores::CloseInputBox( )
{
  InputBGRect->Visible = false;
  InputBoxRect->Visible = false;
}


void __fastcall TFrameHighScores::OkayBTNClick( TObject* Sender )
{
  CloseInputBox();
  SaveScore( InputEdit->Text, InputEdit->Tag );
}
