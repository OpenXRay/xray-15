

#include "mods.h"

#include "bonesdef.h"

#include "Maxscrpt.h"
#include "Strings.h"
#include "arrays.h"
#include "3DMath.h"
#include "Numbers.h"
#include "definsfn.h"


float WeightTableWindow::GetPrecision() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetFloat(skin_wt_precision);
		}
	else return 0.0f; 
	}

void WeightTableWindow::SetPrecision(float value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_precision,0,value);
		}
	}

int WeightTableWindow::GetFontSize() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_fontsize);
		}
	else return 0; 
	}

void WeightTableWindow::SetFontSize(int value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_fontsize,0,value);
		}
	}

BOOL WeightTableWindow::GetAffectSelectedOnly() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		if (GetJBMethod())
			return TRUE;
		else return mod->pblock_weighttable->GetInt(skin_wt_affectselected);
		}
	else return 0; 
	}

void WeightTableWindow::SetAffectSelectedOnly(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_affectselected,0,value);
		}
	}

BOOL WeightTableWindow::GetUpdateOnMouseUp() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_updateonmouseup);
		}
	else return 0; 
	}

void WeightTableWindow::SetUpdateOnMouseUp(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_updateonmouseup,0,value);
		}
	}


BOOL WeightTableWindow::GetAffectedBonesOnly() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_showaffectbones);
		}
	else return 0; 
	}

void WeightTableWindow::SetAffectedBonesOnly(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_showaffectbones,0,value);
		}
	}


BOOL WeightTableWindow::GetFlipFlopUI() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_flipui);
		}
	else return 0; 
	}

void WeightTableWindow::SetFlipFlopUI(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_flipui,0,value);
		}
	}


BOOL WeightTableWindow::GetShowAttributes() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_showattrib);
		}
	else return 0; 
	}

void WeightTableWindow::SetShowAttributes(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_showattrib,0,value);
		}
	}

BOOL WeightTableWindow::GetShowGlobals() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_showglobal);
		}
	else return 0; 
	}

void WeightTableWindow::SetShowGlobals(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_showglobal,0,value);
		}
	}

BOOL WeightTableWindow::GetReduceLabels() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_shortenlabel);
		}
	else return 0; 
	}

void WeightTableWindow::SetReduceLabels(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_shortenlabel,0,value);
		}
	}



BOOL WeightTableWindow::GetShowExclusion() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_showexclusion);
		}
	else return 0; 
	}

void WeightTableWindow::SetShowExclusion(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_showexclusion,0,value);
		}
	}


BOOL WeightTableWindow::GetShowLock() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_showlock);
		}
	else return 0; 
	}

void WeightTableWindow::SetShowLock(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_showlock,0,value);
		}
	}


int WeightTableWindow::GetTopBorder() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_tabley);
		}
	else return 0; 
	}

void WeightTableWindow::SetTopBorder(int value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_tabley,0,value);
		}
	}
/*
BOOL WeightTableWindow::IsLeftRightDragMode()
	{
	BOOL value = TRUE;
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->GetValue(skin_wt_dragleftright,0,value,FOREVER);
		}
	return value;
	}
*/
int WeightTableWindow::GetActiveSet()
	{
	int value = TRUE;
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->GetValue(skin_wt_currentvertexset,0,value,FOREVER);
		}
	return value;
	}

void WeightTableWindow::SetActiveSet(int set)
	{
	if ( (mod) && (mod->pblock_weighttable))
		{
		int cSet;
		mod->pblock_weighttable->GetValue(skin_wt_currentvertexset,0,cSet,FOREVER);
		if (set != cSet)
			{
			mod->pblock_weighttable->SetValue(skin_wt_currentvertexset,0,set);

			BOOL temp;			
			mod->pblock_weighttable->GetValue(skin_wt_showaffectbones,0,temp,FOREVER);
			mod->pblock_weighttable->SetValue(skin_wt_showaffectbones,0,temp);

			SetFocus(hWeightList);
			}
		}
	}

BOOL WeightTableWindow::GetJBMethod()
	{
	BOOL value = TRUE;
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->GetValue(skin_wt_jbuimethod,0,value,FOREVER);
		}
	return value;
	}
void WeightTableWindow::SetJBMethod(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_jbuimethod,0,value);
		}
	}

BOOL WeightTableWindow::GetShowMenu() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_showmenu);
		}
	else return 0; 
	}

void WeightTableWindow::SetShowMenu(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_showmenu,0,value);
		}
	}

BOOL WeightTableWindow::GetShowSetUI() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_showsetui);
		}
	else return 0; 
	}

void WeightTableWindow::SetShowSetUI(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_showsetui,0,value);
		}
	}

BOOL WeightTableWindow::GetShowOptionsUI() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_showoptionui);
		}
	else return 0; 
	}

void WeightTableWindow::SetShowOptionsUI(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_showoptionui,0,value);
		}
	}

BOOL WeightTableWindow::GetShowCopyPasteUI() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_showcopypasteui);
		}
	else return 0; 
	}

void WeightTableWindow::SetShowCopyPasteUI(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_showcopypasteui,0,value);
		}
	}



BOOL WeightTableWindow::GetDragLeftMode() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_dragleftright);
		}
	else return 0; 
	}

void WeightTableWindow::SetDragLeftMode(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_dragleftright,0,value);
		}
	}

BOOL WeightTableWindow::GetDebugMode() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_debugmode);
		}
	else return 0; 
	}

void WeightTableWindow::SetDebugMode(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_debugmode,0,value);
		}
	}

BOOL WeightTableWindow::GetShowMarker() 
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_showmarker);
		}
	else return 0; 
	}

void WeightTableWindow::SetShowMarker(BOOL value)  
	{ 
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_showmarker,0,value);
		}
	}


//5.1.01
BOOL WeightTableWindow::GetRightJustify()
{
	if ( (mod) && (mod->pblock_weighttable))
		{
		return mod->pblock_weighttable->GetInt(skin_wt_rightjustify);
		}
	else return 0; 

}
void WeightTableWindow::SetRightJustify(BOOL val)
{
	if ( (mod) && (mod->pblock_weighttable))
		{
		mod->pblock_weighttable->SetValue(skin_wt_rightjustify,0,val);
		}
}