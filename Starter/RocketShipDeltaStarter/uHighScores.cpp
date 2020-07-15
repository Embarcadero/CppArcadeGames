//---------------------------------------------------------------------------

// This software is Copyright (c) 2016-2020 Embarcadero Technologies, Inc.
// You may only use this software if you are an authorized licensee
// of Delphi, C++Builder or RAD Studio (Embarcadero Products).
// This software is considered a Redistributable as defined under
// the software license agreement that comes with the Embarcadero Products
// and is subject to that software license agreement.

//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "uHighScores.h"
#include "uGame.h"

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
	std::unique_ptr<TMemIniFile> IniFile(new TMemIniFile( GameForm->SettingsFilePath + "HighScores.ini" ));
	IniFile->WriteInteger( "HighScores", Name + " [" + System::Sysutils::DateTimeToStr(Now()) + "]", Score );
	IniFile->UpdateFile();
	IniFile.reset();
	PopulateHighScores();
  }
}

int __fastcall StringListSortCompareV(TStringList *List, int Index1, int Index2)
{
 int A = StrToIntDef(List->ValueFromIndex[Index1], -1);
 int B = StrToIntDef(List->ValueFromIndex[Index2], -1);
 return B - A;
}

void __fastcall TFrameHighScores::PopulateHighScores( )
{
  int I = 0;
  std::unique_ptr<TStringList> SL(new TStringList());
	  std::unique_ptr<TMemIniFile> IniFile(new TMemIniFile( GameForm->SettingsFilePath + "HighScores.ini" ));
	  if (IniFile->SectionExists("HighScores")==true) {
	  ListView1->Items->Clear();
		  IniFile->ReadSectionValues("HighScores", SL.get());
		  SL->CustomSort(StringListSortCompareV);
		  for ( int stop = SL->Count - 1, I = 0; I <= stop; I++) {
			TListViewItem *LItem = ListView1->Items->Add();
			LItem->Text = SL->Names[I];
			LItem->Detail = SL->ValueFromIndex[I];
		  }
	  }
}

void __fastcall TFrameHighScores::InitFrame( )
{
  if ( Loaded == false )
  {
	try
	{
		PopulateHighScores();
	}
	catch( Exception & E )
	{
	  ShowMessage( E.Message );
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