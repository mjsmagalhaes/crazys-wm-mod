/*
 * Copyright 2009, 2010, The Pink Petal Development Team.
 * The Pink Petal Devloment Team are defined as the game's coders
 * who meet on http://pinkpetal.org     // old site: http://pinkpetal .co.cc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "cJobManager.h"
#include "cBrothel.h"
#include "cClinic.h"
#include "cMovieStudio.h"
#include "cCustomers.h"
#include "cRng.h"
#include "cInventory.h"
#include "sConfig.h"
#include "cRival.h"
#include <sstream>
#include "CLog.h"
#include "cTrainable.h"
#include "cTariff.h"
#include "cGold.h"
#include "cGangs.h"
#include "cMessageBox.h"
#include "libintl.h"

extern cRng g_Dice;
extern CLog g_LogFile;
extern cCustomers g_Customers;
extern cInventory g_InvManager;
extern cBrothelManager g_Brothels;
extern cMovieStudioManager g_Studios;
extern cGangManager g_Gangs;
extern cMessageQue g_MessageQue;
extern cPlayer* The_Player;

// `J` Job Movie Studio - Actress
bool cJobManager::WorkFilmBondage(sGirl* girl, sBrothel* brothel, bool Day0Night1, string& summary)
{
	int actiontype = ACTION_WORKMOVIE;
	// No film crew.. then go home	// `J` this will be taken care of in building flow, leaving it in for now
	if (g_Studios.GetNumGirlsOnJob(0, JOB_CAMERAMAGE, SHIFT_NIGHT) == 0 || g_Studios.GetNumGirlsOnJob(0, JOB_CRYSTALPURIFIER, SHIFT_NIGHT) == 0)
	{
		girl->m_Events.AddMessage("There was no crew to film the scene, so she took the day off", IMGTYPE_PROFILE, EVENT_NOWORK);
		return false;
	}

	stringstream ss;
	string girlName = girl->m_Realname;
	int wages = 50, tips = 0;
	int enjoy = 0;
	double jobperformance = JP_FilmBondage(girl, false);

	g_Girls.UnequipCombat(girl);	// not for actress (yet)

#if 0
	//OLD version
	ss << girlName << " worked as an actress filming BDSM scenes.\n\n";

	int roll = g_Dice.d100();
	if (roll <= 10 && g_Girls.DisobeyCheck(girl, ACTION_WORKMOVIE, brothel))
	{
		ss << "She refused to get beaten on film today.\n";
		girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_NOWORK);
		return true;
	}
	else if (roll <= 10) { enjoy -= g_Dice % 3 + 1;	ss << "She did not enjoy getting tied up and hurt today.\n\n"; }
	else if (roll >= 90) { enjoy += g_Dice % 3 + 1;	ss << "She had a great time getting spanked and whipped.\n\n"; }
	else /*            */{ enjoy += g_Dice % 2;		ss << "She had just another day in the dungeon.\n\n"; }
	jobperformance = enjoy * 2;
#else
	int bonus = 0;
	int roll = g_Dice.d100();


	if (g_Girls.GetStat(girl, STAT_HEALTH) < 40)
	{
		ss << gettext("The crew refused to film a dungeon scene with ") << girlName << gettext(" because she is not healthy enough.\n\"We are NOT filming snuff.\"");
		girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_NOWORK);
		return true;
	}
	else if (girl->is_pregnant())
	{
		ss << gettext("The crew refused to do a BDSM scene with ") << girlName << gettext(" due to her pregnancy.");
		girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_NOWORK);
		return true;
	}
	else if (girl->has_trait( "Mind Fucked"))
	{
		ss << gettext("Mind fucked ") << girlName << gettext(" seemed at home in the dungeon, and started collecting together tools she should be punished with.\n");
		bonus += 10;
	}
	else if (girl->has_trait( "Masochist"))
	{
		ss << gettext("Masochist ") << girlName << gettext(" was pleased to in the dungeon. It is her place.\n");
		bonus += 6;
	}
	else if (roll <= 10 && g_Girls.DisobeyCheck(girl, ACTION_WORKMOVIE, brothel))
	{
		ss << girlName << gettext(" was horrified and refused to be beaten and sexually tortured in this ");
		if (girl->is_slave())
		{
			if (The_Player->disposition() > 30)  // nice
			{
				ss << gettext("\"monstrous\" place.\nShe was starting to panic, so you allowed her the day off.\n");
				g_Girls.UpdateStat(girl, STAT_PCLOVE, 2);
				g_Girls.UpdateStat(girl, STAT_PCHATE, -1);
				girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_NOWORK);
				return true;
			}
			else if (The_Player->disposition() > -30) //pragmatic
			{
				ss << gettext("\"monstrous\" place.\nShe was starting to panic, so you ordered your men to grab her and bind her for action.\n");
				g_Girls.UpdateStat(girl, STAT_PCLOVE, -1);
				g_Girls.UpdateStat(girl, STAT_PCHATE, 2);
				g_Girls.UpdateStat(girl, STAT_PCFEAR, 2);
				The_Player->disposition(-1);
				enjoy -= 2;
			}
			else if (The_Player->disposition() > -30)
			{
				ss << gettext("\"monstrous\" place.\nShe was starting to panic so you ordered your men to quickly grab, strip and bind her. Finally, ");
				if (girl->has_trait( "Pierced Nipples"))
				{
					ss << gettext("noticing her pierced nipples");
					if (girl->has_trait( "Pierced Clit"))
					{
						ss << gettext(" and clit");
					}
					ss << gettext(", ");
				}
				else if (girl->has_trait( "Pierced Clit")) ss << gettext("noticing her clittoral piercing, ");
				ss << gettext("you personally selected some 'fun tools' for your actor, instructing him to train your slave in humility and obedience.\n\"Master her. Intimately.\"");
				g_Girls.UpdateStat(girl, STAT_PCLOVE, -4);
				g_Girls.UpdateStat(girl, STAT_PCHATE, +5);
				g_Girls.UpdateStat(girl, STAT_PCFEAR, +5);
				The_Player->disposition(-2);
				enjoy -= 6;
			}
		}
		else // not a slave
		{
			ss << " \"monstrous\" place.\n";
			girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_NOWORK);
			return true;
		}
	}
	else ss << girlName << gettext(" was taken for bondage and torture scenes in your dungeon.\n\n");

	//Qual
	if (jobperformance >= 350)
	{
		ss << gettext("It was an excellent scene.");
		bonus += 12;
	}
	else if (jobperformance >= 245)
	{
		ss << gettext("It was mostly an excellent scene.");
		bonus += 6;
	}
	else if (jobperformance >= 185)
	{
		ss << gettext("Overall, it was an solid scene.");
		bonus += 4;
	}
	else if (jobperformance >= 145)
	{
		ss << gettext("Overall, it wasn't a bad scene.");
		bonus += 2;
	}
	else if (jobperformance >= 100)
	{
		ss << gettext("It wasn't a great scene.");
		bonus++;
	}
	else
	{
		ss << gettext("It was a poor scene.");
	}
	ss << gettext("\n");


	//Enjoyed? If she's deranged, she'd should have enjoyed it.
	if (girl->has_trait( "Mind Fucked"))
	{
		enjoy += 16;
		ss << gettext("Being completely mind fucked, ") << girlName << gettext(" actually gets off on this.\n");
	}
	else if (girl->has_trait( "Masochist"))
	{
		enjoy += 10;
		ss << girlName << gettext(" enjoys this. It's what she deserves.\n");
	}
	else if (girl->has_trait( "Broken Will") || girl->has_trait( "Dependant"))
	{
		enjoy += 5;
		ss << girlName << gettext(" accepts this. It is Master's will.\n");
	}
	else if (girl->has_trait( "Iron Will") || girl->has_trait( "Fearless"))
	{
		enjoy -= 5;
		ss << girlName << gettext(" endures in stoic silence, determined not to let you see her suffer.\n");
	}
	else if (girl->has_trait( "Nymphomaniac"))
	{
		enjoy += 2;
		ss << girlName << gettext(" doesn't much like the pain, but loves the sex and attention.\n");
	}

	//Feedback enjoyment
	if (enjoy > 10)
	{
		ss << gettext("She won't say it, but you suspect she secretly gets off on the attention, sin and degradation.\n\n");
	}
	else if (enjoy > 0)
	{
		ss << gettext("She's only a little traumatised.\n\n");
	}
	else
	{
		ss << gettext("From the way she's crouched, rocking in a corner and silently weeping... you figure she didn't enjoy this.\n\n");
	}
#endif

	if (g_Girls.CheckVirginity(girl))
	{
		g_Girls.LoseVirginity(girl);	// `J` updated for trait/status
		jobperformance += 50;
		ss << "She is no longer a virgin.\n";
	}
	sCustomer* Cust = new sCustomer; g_Customers.GetCustomer(Cust, brothel); Cust->m_Amount = 1;
	if (Cust->m_IsWoman)	// FemDom
	{
		jobperformance += 20;
		/* */if (girl->has_trait("Lesbian"))	jobperformance += 20;
		else if (girl->has_trait("Straight"))	jobperformance -= 20;
	}
	else
	{
		if (!girl->calc_pregnancy(Cust, false, 0.75))
			g_MessageQue.AddToQue(girl->m_Realname + " has gotten pregnant", 0);
		/* */if (girl->has_trait("Lesbian"))	jobperformance -= 10;
		else if (girl->has_trait("Straight"))	jobperformance += 10;
	}

	//Evil pays more and costs about the same...
	int finalqual = g_Studios.AddScene(girl, JOB_FILMBONDAGE, bonus);
	ss << "Her scene is valued at: " << finalqual << " gold.\n";

	girl->m_Events.AddMessage(ss.str(), IMGTYPE_BDSM, Day0Night1);

	// work out the pay between the house and the girl
	if (girl->is_slave() && !cfg.initial.slave_pay_outofpocket())
	{
		wages = 0;	// You own her so you don't have to pay her.
	}
	else
	{
		wages += finalqual * 2;
	}
	girl->m_Tips = max(0, tips);
	girl->m_Pay = max(0, wages);

	// Improve stats
	int xp = 10, skill = 3;

	if (girl->has_trait( "Quick Learner"))		{ skill += 1; xp += 3; }
	else if (girl->has_trait( "Slow Learner"))	{ skill -= 1; xp -= 3; }

	g_Girls.UpdateStat(girl, STAT_EXP, xp);
	g_Girls.UpdateSkill(girl, SKILL_PERFORMANCE, g_Dice%skill);
	g_Girls.UpdateSkill(girl, SKILL_BDSM, g_Dice%skill + 1);

	g_Girls.UpdateEnjoyment(girl, ACTION_SEX, enjoy);
	g_Girls.UpdateEnjoyment(girl, ACTION_WORKMOVIE, enjoy);
	//gain
	g_Girls.PossiblyGainNewTrait(girl, "Fake Orgasm Expert", 50, ACTION_SEX, "She has become quite the faker.", Day0Night1);
	g_Girls.PossiblyGainNewTrait(girl, "Masochist", 65, ACTION_SEX, girlName + " has turned into a Masochist from filming so many BDSM scenes.", Day0Night1);
	g_Girls.PossiblyGainNewTrait(girl, "Slut", 80, ACTION_SEX, girlName + " has turned into quite a slut.", Day0Night1);
	g_Girls.PossiblyGainNewTrait(girl, "Porn Star", 80, ACTION_WORKMOVIE, "She has performed in enough sex scenes that she has become a well known Porn Star.", Day0Night1);
	//lose

	delete Cust;

	//Evil job bonus-------------------------------------------------------
	//BONUS - evil jobs damage her body, break her spirit and make her hate you

	int MrEvil = g_Dice % 8, MrNasty = g_Dice % 8;
	MrEvil = (MrEvil + MrNasty) / 2;				//Should come out around 3 most of the time.

	g_Girls.UpdateStat(girl, STAT_CONFIDENCE, -MrEvil);
	g_Girls.UpdateStat(girl, STAT_SPIRIT, -MrEvil);
	g_Girls.UpdateStat(girl, STAT_DIGNITY, -MrEvil);
	g_Girls.UpdateStat(girl, STAT_PCLOVE, -MrEvil);
	g_Girls.UpdateStat(girl, STAT_PCHATE, MrEvil);
	g_Girls.UpdateStat(girl, STAT_PCFEAR, MrEvil);
	The_Player->disposition(-MrEvil);

	//----------------------------------------------------------------------
	return false;
}

double cJobManager::JP_FilmBondage(sGirl* girl, bool estimate)// not used
{
	double jobperformance =
		(g_Girls.GetStat(girl, STAT_CHARISMA) + g_Girls.GetStat(girl, STAT_BEAUTY)
		+ g_Girls.GetStat(girl, STAT_DIGNITY));

	if (!estimate)
	{
		int t = girl->tiredness() - 80;
		if (t > 0)
			jobperformance -= (t + 2) * (t / 3);
	}

	if (girl->has_trait( "Mind Fucked"))				jobperformance += 50;	//this is her home
	if (girl->has_trait( "Masochist"))				jobperformance += 30;	//she likes it
	if (girl->has_trait( "Twisted"))					jobperformance += 10;	//
	if (girl->has_trait( "Open Minded"))				jobperformance += 10;	//
	if (girl->has_trait( "Broken Will"))				jobperformance += 20;	//
	if (girl->has_trait( "Pierced Clit"))				jobperformance += 40;	//Piercings can be useful for this...
	if (girl->has_trait( "Pierced Nipples"))			jobperformance += 40;	//
	if (girl->has_trait( "Pierced Tongue"))			jobperformance += 20;	//
	if (girl->has_trait( "Pierced Nose"))				jobperformance += 10;	//
	if (girl->has_trait( "Pierced Navel"))			jobperformance += 5;	//
	if (girl->has_trait( "Princess"))					jobperformance += 30;	//High-Status degraded
	if (girl->has_trait( "Queen"))					jobperformance += 30;	//
	if (girl->has_trait( "Goddess"))					jobperformance += 30;	//
	if (girl->has_trait( "Angel"))					jobperformance += 30;	//
	if (girl->has_trait( "Noble"))					jobperformance += 20;	//
	if (girl->has_trait( "Elegant"))					jobperformance += 10;	//
	if (girl->has_trait( "Branded on the Forehead"))	jobperformance += 5;	//Degraded
	if (girl->has_trait( "Branded on the Ass"))		jobperformance += 5;	//
	if (g_Girls.CheckVirginity(girl))						jobperformance += 20;	//
	if (girl->has_trait( "Strong Gag Reflex"))		jobperformance += 10;	//Degrading
	if (girl->has_trait( "Gag Reflex"))				jobperformance += 5;	//
	if (girl->has_trait( "No Gag Reflex"))			jobperformance += 5;	//
	if (girl->has_trait( "Deep Throat"))				jobperformance += 10;	//Capable
	if (girl->has_trait( "Tsundere"))					jobperformance += 15;	//Mistreated customers can see haughty girl knocked down
	if (girl->has_trait( "Yandere"))					jobperformance += 15;	//
	if (girl->has_trait( "Actress"))					jobperformance += 25;	//Can play it up for crowd

	if (girl->has_trait( "Iron Will"))				jobperformance -= 30;	//Refuses to react
	if (girl->has_trait( "Incorporeal"))				jobperformance -= 60;	//Can't be hurt
	if (girl->has_trait( "Construct"))				jobperformance -= 60;	//Can't be hurt
	if (girl->has_trait( "Half-Construct"))			jobperformance -= 50;	//Can't be hurt
	if (girl->has_trait( "Skeleton"))					jobperformance -= 80;	//Can't be hurt
	if (girl->has_trait( "Undead"))					jobperformance -= 80;	//Can't be hurt
	if (girl->has_trait( "Zombie"))					jobperformance -= 80;	//Can't be hurt
	if (girl->has_trait( "Fragile"))					jobperformance -= 80;	//Too quickly damaged
	if (girl->has_trait( "Delicate"))					jobperformance -= 80;	//Too quickly damaged

	return jobperformance;
}
