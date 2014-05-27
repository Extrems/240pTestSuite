/* 
 * 240p Test Suite
 * Copyright (C)2014 Artemio Urbina (Wii GX)
 *
 * This file is part of the 240p Test Suite
 *
 * The 240p Test Suite is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The 240p Test Suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 240p Test Suite; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#include <stdio.h>
#include <stdlib.h>
#include "controller.h"
#include "video.h"
#include "image.h"
#include "font.h"
#include "options.h"
#include "help.h"
#include "menu.h"


#ifdef WII_VERSION
#include "CheckRegion.h"
char wiiregion[100];
#endif

u8 DrawMenu = 0;
char **HelpData = GENERALHELP;
u8 EndProgram = 0;

void ShowMenu()
{
	int 		sel = 1, close = 0;		
	ImagePtr	Back, cFB;	
	
	cFB = CopyFrameBufferToImage();	
	if(cFB)
	{
		cFB->alpha = 0x77;
	}
	Back = LoadImage(FLOATMENUIMG, 0);
	if(!Back)
		return;	
	
	Back->x = (dW - Back->w) / 2;
	Back->y = (dH - Back->h) / 2;
	
	Back->alpha = 0xaa;
	   
	while(!close && !EndProgram) 
	{		
		u8      r = 0xff;
		u8      g = 0xff;
		u8      b = 0xff;
		u8   	c = 1;				    					   
		u16     x = Back->x + 20;
		u16     y = Back->y + 10;
        u32     pressed = 0;
				
		StartScene();
		
		DrawImage(cFB);         
		DrawImage(Back);       

		DrawStringS(x, y, 0x00, 0xff, 0x00, VERSION_NUMBER); y += 3*fh; 		
		
		DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "Help"); y += fh; c++;				
		DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "Video"); y += fh; c++;		
		DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "Options"); y += fh; c++;		
        DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "Credits"); y += fh; c++;		
		DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "Close Menu"); y += 2* fh; c++;			
		DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "Exit 240p Suite"); y += 2* fh; 		
		
#ifdef WII_VERSION		
		u8 		battery;
		char 	level[30];
		
		battery = ControllerBattery(0);
		if(battery)
		{
			sprintf(level, "Wiimote batt: %d%%", battery);
			DrawStringS(x-10, y, battery < 25 ? 0xff : 0x00, 0x88, battery >= 25 ? 0xff : 0x00 , level); 
		}
#endif				
						
		EndScene();		
		
		ControllerScan();

		pressed = Controller_ButtonsDown(0);
		
		if ( pressed & PAD_BUTTON_UP )
	    {
		    sel --;
		    if(sel < 1)
			    sel = c;		
	    }
	    
	    if ( pressed & PAD_BUTTON_DOWN )
	    {
		    sel ++;
		    if(sel > c)
			    sel = 1;	
	    }			
			
		if (pressed & PAD_BUTTON_B || pressed & PAD_BUTTON_START) 		
			close = 1;	
	
		if (pressed & PAD_BUTTON_A)
		{     
			switch(sel)
			{			
					case 1:			
						HelpWindow(cFB);
						break;					
					case 2:		
						SelectVideoMode(cFB);
						Back->x = (dW - Back->w) / 2;
						Back->y = (dH - Back->h) / 2;
						break;
					case 3:		
						ChangeOptions(cFB);						
						break;
                    case 4:		
						DrawCredits(cFB);						
						break;
					case 5:
						close = 1;
						break;
					case 6:
						EndProgram = 1;
						close = 1;
						break;
					default:
						break;
			} 			            										
		}		
	}
	
	HelpData = GENERALHELP;
	FreeImage(&Back);
	FreeImage(&cFB);

	return;
}


void ChangeOptions(ImagePtr title)
{	
	int 		sel = 1, close = 0;	
	ImagePtr	back;
	
	back = LoadImage(HELPIMG, 1);
	if(!back)
		return;
		
	back->alpha = 0xaa;
		
	while(!close && !EndProgram) 
	{		
		u8      r = 0xff;
		u8      g = 0xff;
		u8      b = 0xff;
		u8   	c = 1;				    					   
		u16     x = 80;
		u16     y = 70;
		u16		OptPos = 140;
        u32     pressed = 0;
		char	intensity[80];
				
		StartScene();
		        
		DrawImage(title);        
		DrawImage(back);

		DrawStringS(x - 20, y, 0x00, 0xff, 0x00, "General Options"); y += 2*fh; 

		// option 1, Show region
#ifdef WII_VERSION				
		DrawStringS(x + OptPos, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, Options.ShowWiiRegion ? "ON" : "OFF");
		DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "Show WII region:"); y += fh; c++;		
#endif
		
		// option 2, Active 480p
		if(VIDEO_HaveComponentCable())
		{			
			DrawStringS(x + OptPos, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, Options.Activate480p ? "ON" : "OFF"); 					
			DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "Enable 480p modes:"); y += fh; c++;			
		}
		else
		{			
			DrawStringS(x + OptPos, y, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, Options.Activate480p ? "ON" : "OFF"); 					
			DrawStringS(x, y, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, "Enable 480p modes:"); y += fh; c++;			
		}

		// Option 3
		DrawStringS(x + OptPos, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, Options.EnablePAL ? "ON" : "OFF"); 					
		DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "Enable PAL Modes:"); y += fh; c++;						
		
		// option 4, Scanline intensity
		sprintf(intensity, "%d%%", GetScanlineIntensity());
		if(vmode == VIDEO_480P_SL)
		{
			DrawStringS(x + OptPos, y, r, sel == c ? 0 : g, sel == c ? 0 : b, intensity); 
			DrawStringS(x, y, r, sel == c ? 0 : g, sel == c ? 0 : b, "Scanline intensity:"); y += fh; c++;			
			
			// option 5, Scanline even/odd
			DrawStringS(x + OptPos, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, ScanlinesEven() ? "EVEN" : "ODD"); 					
			DrawStringS(x, y, r, sel == c ? 0 : g, sel == c ? 0 : b, "Scanlines:"); y += fh; c++;	
		}				
		else
		{
			DrawStringS(x + OptPos, y, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, intensity);
			DrawStringS(x, y, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, "Scanline intensity:"); y += fh; c++;			
			
			// option 5, Scanline even/odd
			DrawStringS(x + OptPos, y, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, ScanlinesEven() ? "EVEN" : "ODD"); 					
			DrawStringS(x, y, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, "Scanlines:"); y += fh; c++;	
		}
		
#ifdef WII_VERSION					
		// option 6, SFC CC
		DrawStringS(x + OptPos, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, Options.SFCClassicController ? "ON" : "OFF"); 					
		DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "SFC Classic Controller:"); y += fh; c++;			
#endif		
			
		// Option 7
		DrawStringS(x, y + 2* fh, r, sel == c ? 0 : g, sel == c ? 0 : b, "Back to Main Menu"); 		
				
#ifdef WII_VERSION					
		if(vmode == VIDEO_480P_SL && sel == 4)
		{
			if(ControllerType != ControllerGC && !Options.SFCClassicController)
				DrawStringS(x-40, y + 4*fh, r, g, b, "Adjust with + and - buttons"); 										
			else
				DrawStringS(x-40, y + 4*fh, r, g, b, "Adjust with L and R triggers"); 	
		}
#else
		if(vmode == VIDEO_480P_SL && sel == 3)	
			DrawStringS(x-40, y + 4*fh, r, g, b, "Adjust with L and R triggers"); 										
#endif


#ifdef WII_VERSION								
		if(vmode != VIDEO_480P_SL && (sel == 4 || sel == 5))
#else
		if(vmode != VIDEO_480P_SL && (sel == 3 || sel == 4))
#endif
			DrawStringS(x-40, y + 4*fh, r, g, b, "Scanlines are only available in\n480 Line Doubled mode"); 						
			
#ifdef WII_VERSION					
		if(sel == 6)	
			DrawStringS(x-40, y + 4*fh, r, g, b, "Change Classic Controller Button map:\n [HOME] -> [+] and [- +] -> [L R]"); 										
#endif
				
		EndScene();		
        
        ControllerScan();

		pressed = Controller_ButtonsDown(0);
		

		if ( pressed & PAD_BUTTON_UP )
	    {
		    sel --;
		    if(sel < 1)
			    sel = c;		
	    }
	    
	    if ( pressed & PAD_BUTTON_DOWN )
	    {
		    sel ++;
		    if(sel > c)
			    sel = 1;	
	    }			

#ifdef WII_VERSION			
		if ( pressed & PAD_TRIGGER_R && sel == 4)
	    {
			if(vmode == VIDEO_480P_SL)
				RaiseScanlineIntensity();
	    }
	    
	    if ( pressed & PAD_TRIGGER_L && sel == 4)
	    {
			if(vmode == VIDEO_480P_SL)
				LowerScanlineIntensity();
	    }			
#else
		if ( pressed & PAD_TRIGGER_R && sel == 3)
	    {
			if(vmode == VIDEO_480P_SL)
				RaiseScanlineIntensity();
	    }
	    
	    if ( pressed & PAD_TRIGGER_L && sel == 3)
	    {
			if(vmode == VIDEO_480P_SL)
				LowerScanlineIntensity();
	    }	
#endif
			
		if ( pressed & PAD_BUTTON_B ) 		
			close = 1;	
	
		if (pressed & PAD_BUTTON_A)
		{     
#ifdef WII_VERSION		
			switch(sel)
#else
			switch(sel + 1)
#endif
			{			
					case 1:
#ifdef WII_VERSION		
						Options.ShowWiiRegion = !Options.ShowWiiRegion;
						if(Options.ShowWiiRegion)
							GetWiiRegion();
#endif
						if(VIDEO_HaveComponentCable() && !(Options.Activate480p && vmode >= VIDEO_480P))
							Options.Activate480p = !Options.Activate480p;
						break;
					case 3:	
						Options.EnablePAL = !Options.EnablePAL;					
						break;				
					case 4:
						break;
					case 5:
						if(vmode == VIDEO_480P_SL)
							ToggleScanlineEvenOdd();
						break;
					case 6:
#ifdef WII_VERSION		
						Options.SFCClassicController = !Options.SFCClassicController;
						break;										
					case 7:
#endif					
						if(Controller_ButtonsHeld(0) & PAD_TRIGGER_L)
							Options.CorrectFor169 = !Options.CorrectFor169;
						else
							close = 1;
						break;
					default:
						break;
			} 			            										
		}		
	}
	
	FreeImage(&back);

	return;
}

void SelectVideoMode(ImagePtr title)
{
	int 		sel = 1, close = 0;		
	ImagePtr	back;
	
	back = LoadImage(HELPIMG, 1);
	if(!back)
		return;
		
	back->alpha = 0xaa;
	sel = vmode + 1;
	while(!close && !EndProgram) 
	{		
		u8      r = 0xff;
		u8      g = 0xff;
		u8      b = 0xff;
		u8   	c = 1;				    					   
		u16     x = 80;
		u16     y = 70;
        u32     pressed = 0;
				
		StartScene();
		        
		DrawImage(title);		
		DrawImage(back);        

		DrawStringS(x - 20, y, 0x00, 0xff, 0x00, "Please select the desired video mode"); y += 2*fh; 
		
		DrawStringS(x - 10, y + (vmode * fh), 0x00, 0xff, 0x00, ">"); 
		
		DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "240p"); y += fh; c++;
		DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "480i scaled 240p assets (NTSC)"); y += fh; c++;
		DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "480i mixed 480p/240p assets (1:1/NTSC)"); y += fh; c++;				
		
		if(Options.EnablePAL)
		{
			DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "288p"); y += fh; c++;
			DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "576i scaled 240p assets (PAL)"); y += fh; c++;			
			DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "576i mixed 480p/240p assets (1:1/PAL)"); y += fh; c++;			
		}
		else
		{
			DrawStringS(x, y, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, "288p"); y += fh; c++;
			DrawStringS(x, y, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, "576i scaled 240p assets (PAL)"); y += fh; c++;			
			DrawStringS(x, y, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, "576i mixed 480p/240p assets (1:1/PAL)"); y += fh; c++;			
		}			
		
		if(Options.Activate480p && VIDEO_HaveComponentCable())
		{
			DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "480p scaled 240p assets & scanlines"); y += fh; c++;
			DrawStringS(x, y, r, sel == c ? 0 : g,	sel == c ? 0 : b, "480p mixed 480p/240p assets (1:1)"); y += fh; c++;			
		}
		else
		{
			DrawStringS(x, y, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, "480p scaled 240p assets & scanlines"); y += fh; c++;
			DrawStringS(x, y, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, sel == c ? 0x77 : 0xAA, "480p mixed 480p/240p assets (1:1)"); y += fh; c++;			
		}			
			
		DrawStringS(x, y + fh, r, sel == c ? 0 : g, sel == c ? 0 : b, "Back to Main Menu"); 		
				
		EndScene();		
        
        ControllerScan();

		pressed = Controller_ButtonsDown(0);
		
		if ( pressed & PAD_BUTTON_UP )
	    {
		    sel --;
		    if(sel < 1)
			    sel = c;		
	    }
	    
	    if ( pressed & PAD_BUTTON_DOWN )
	    {
		    sel ++;
		    if(sel > c)
			    sel = 1;	
	    }			
			
		if ( pressed & PAD_BUTTON_B ) 		
			close = 1;	
	
		if (pressed & PAD_BUTTON_A)
		{     
			switch(sel)
			{			
					case 1:
						SetVideoMode(VIDEO_240P);
						SetupGX();
						break;
					case 2:
						SetVideoMode(VIDEO_480I_A240);
						SetupGX();
						break;
					case 3:
						SetVideoMode(VIDEO_480I);
						SetupGX();
						break;
					case 4:
						if(Options.EnablePAL)
						{
							SetVideoMode(VIDEO_288P);
							SetupGX();
						}
						break;
					case 5:
						if(Options.EnablePAL)
						{
							SetVideoMode(VIDEO_576I_A264);
							SetupGX();
						}
						break;
					case 6:
						if(Options.EnablePAL)
						{
							SetVideoMode(VIDEO_576I);
							SetupGX();
						}
						break;
					case 7:
						if(Options.Activate480p && VIDEO_HaveComponentCable())
						{
							SetVideoMode(VIDEO_480P_SL);
							SetupGX();
						}
						break;					
					case 8:
						if(Options.Activate480p && VIDEO_HaveComponentCable())
						{
							SetVideoMode(VIDEO_480P);
							SetupGX();
						}
						break;		
					case 9:
						close = 1;
						break;
					default:
						break;
			} 			            										
		}		
	}
	FreeImage(&back);

	return;
}

extern GXRModeObj *mvmode;
extern GXRModeObj *rmode;

void ShowVideoData()
{
	u16     x = 80;
	u16     y = 80;
	u32     pressed = 0, done = 0;
	char	data[100];
	
	/*
	u8 	aa
	u16 	efbHeight
	u16 	fbWidth
	u8 	field_rendering
	u8 	sample_pattern [12][2]
	u8 	vfilter [7]
	u16 	viHeight
	u32 	viTVMode
	u16 	viWidth
	u16 	viXOrigin
	u16 	viYOrigin
	u16 	xfbHeight
	u32 	xfbMode
	*/
	while(!done)
	{
		x = 40;
		y = 40;
		
		StartScene();
		
		sprintf(data, "Preferred viTVMode: %X", mvmode->viTVMode);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;
		sprintf(data, "Current   viTVMode: %X", rmode->viTVMode);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;
		
		sprintf(data, "Preferred xfbMode: %X", mvmode->xfbMode);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;		
		sprintf(data, "Current   xfbMode: %X", rmode->xfbMode);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;
		
		sprintf(data, "Preferred efbHeight: %u", mvmode->efbHeight);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;		
		sprintf(data, "Current   efbHeight: %u", rmode->efbHeight);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;
		
		sprintf(data, "Preferred viHeight: %u", mvmode->viHeight);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;		
		sprintf(data, "Current   viHeight: %u", rmode->viHeight);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;
		
		sprintf(data, "Preferred fbWidth: %u", mvmode->fbWidth);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;		
		sprintf(data, "Current   fbWidth: %u", rmode->fbWidth);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;
				
		sprintf(data, "Preferred viWidth: %u", mvmode->viWidth);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;		
		sprintf(data, "Current   viWidth: %u", rmode->viWidth);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;
		
		sprintf(data, "Preferred field_rendering: %u", mvmode->field_rendering);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;		
		sprintf(data, "Current   field_rendering: %u", rmode->field_rendering);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;
		
		sprintf(data, "VI_TVMODE_EURGB60_DS: %X", VI_TVMODE_EURGB60_DS);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;		
		
		sprintf(data, "mvmode->viTVMode >> 2: %X", mvmode->viTVMode >> 2);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;		
		
		sprintf(data, "VI_EURGB60: %X", VI_EURGB60);
		DrawStringS(x, y, 0xff, 0xff,	0xff, data); y += fh;						
		
		if(VIDEO_HaveComponentCable())
		{
			DrawStringS(x, y, 0xff, 0xff, 0xff, "CMPT"); y += fh;
		}
		else
		{
			DrawStringS(x, y, 0xff, 0xff, 0xff, "NCMPT"); y += fh;
		}
		
		EndScene();		
			
		ControllerScan();
		pressed = Controller_ButtonsDown(0);
		
		if ( pressed & PAD_BUTTON_B ) 		
			done = 1;	
	}
}

void DrawCredits(ImagePtr Back)
{
	int 	done = 0, r, g, b;	
    u32     pressed = 0, counter = 1;
	char	data[50];
#ifdef WII_VERSION
	u8 		shopcode = 0;
	char	shop[100];	
	
	if(Options.ShowWiiRegion)
	{
		if (CONF_GetShopCode(&shopcode) >= 0) {
			sprintf(shop, "Shop: %s\n", (strlen(CONF_CountryCodes[shopcode]) ? CONF_CountryCodes[shopcode] : "Unknown Country"));
		} else {		
			sprintf(shop, "Error getting shop code!\n");			
		}
	}
#endif

    if(Back)
	{
		r = Back->r;
		g = Back->b;
		b = Back->g;

		Back->r = 0x64;
		Back->g = 0x64;
		Back->b = 0x64;
	}
	
	while(!done && !EndProgram)  
	{
		int x = 20, y = 20;			

		StartScene();
		        
		DrawImage(Back);
		
        DrawStringS(x, y, 0x00, 0xff, 0x00, "Code and Patterns:"); y += fh; 
        DrawStringS(x, y, 0xff, 0xff, 0xff, "Artemio Urbina"); y += fh; 

		if(counter == 1)
			sprintf(data, "aurbina@junkerhq.net");			
		if(counter == 60*4)
			sprintf(data, "@Artemio (twitter)");					
		if(counter == 60*8)
			counter = 0;

		DrawStringS(x, y, 0x00, 0xff, 0x00, "Support and suggestions:"); y += fh; 
		DrawStringS(x+5, y, 0xff, 0xff, 0xff, data); y += fh; 

	    y += fh;
		DrawStringS(x, y, 0x00, 0xff, 0x00, "SDK:"); y += fh; 
		DrawStringS(x+5, y, 0xff, 0xff, 0xff, "devkitPPC"); y += fh; 	
        y += fh;	
		DrawStringS(x, y, 0x00, 0xff, 0x00, "Menu Pixel Art:"); y += fh; 
		DrawStringS(x+5, y, 0xff, 0xff, 0xff, "Asher"); y += fh; 		
		DrawStringS(x, y, 0x00, 0xff, 0x00, "Advisor:"); y += fh; 
		DrawStringS(x+5, y, 0xff, 0xff, 0xff, "Fudoh"); y += fh; 
		DrawStringS(x, y, 0x00, 0xff, 0x00, "Collaboration:"); y += fh; 
		DrawStringS(x+5, y, 0xff, 0xff, 0xff, "Konsolkongen & shmups regulars"); y += fh; 
        y += fh;
		DrawStringS(x, y, 0x00, 0xff, 0x00, "Info on using this suite:"); y += fh; 
		DrawStringS(x+5, y, 0xff, 0xff, 0xff, "http://junkerhq.net/240p/"); y += 2*fh; 

		DrawStringS(x, y, 0x00, 0xba, 0xba, "This program is free Software.");  y += fh;
		DrawStringS(x, y, 0x00, 0xba, 0xba, "Source code is available under GPL.");  y += fh;
		DrawStringS(x, y, 0x00, 0xba, 0xba, "Includes libcheckregion.");

		y = 58;
		
		DrawStringS(200, y, 0x0f, 0xff, 0xff, VERSION_NUMBER); y += fh;
		DrawStringS(200, y, 0x0f, 0xff, 0xff, VERSION_DATE); y += 2*fh;

#ifdef WII_VERSION		
		if(Options.ShowWiiRegion)
		{
			DrawStringS(200, y, 0xee, 0xee, 0xee, shop); y += fh;
			DrawStringS(200, y, 0xee, 0xee, 0xee, wiiregion); y += fh;
		}
#endif

		EndScene();		
		
		ControllerScan();
		pressed = Controller_ButtonsDown(0);
				
        if (pressed & PAD_TRIGGER_R)
			DrawIntro();
		if (pressed & PAD_BUTTON_B)
			done =	1;		
			
		counter ++;			
	}
	
	if(Back)
	{
		Back->r = r;
		Back->g = g;
		Back->b = b;
	}
}

void DrawIntro()
{    
	u32				counter, frames = 60;
	int				delta;
	ImagePtr		black;

	black = LoadImage(BLACKIMG, 1);
	if(!black)
		return;

	black->alpha = 0xff;
	delta = 1;
	black->w = dW;
	black->h = dH;
	black->layer = 5.0f;
		
	for(counter = 0; counter < frames*2; counter ++)
	{
		black->alpha -= delta;
		if(black->alpha < 0x00)
			black->alpha = 0x00;

		StartScene();
	
		DrawStringS(120, 115, 0xff, 0xff, 0xff, "KORDAMP PRESENTS");
		DrawImage(black);
		
		EndScene();		

		if(counter == frames)
			delta *= -1;
	}
	FreeImage(&black);
}

#ifdef WII_VERSION

void GetWiiRegion()
{
	s32 ret;
	char serialcode[SERIALCODE_SIZE] = "???\0";
	char model[MODEL_SIZE] = {0};	
	
	ret = CONF_GetSerialCode(serialcode);	
	switch (ret) {
		case CONF_CODE_JPN:
			// LJF - Confirmed
			sprintf(wiiregion, "Japanese Wii");
			break;
		case CONF_CODE_USA:
			// LU - Confirmed
			sprintf(wiiregion, "USA Wii");
			break;
		case CONF_CODE_EURH:
			// LEH - Confirmed
		case CONF_CODE_EURM:
			// LEM - Confirmed
		case CONF_CODE_EURF:
			// LEF - Confirmed
			sprintf(wiiregion, "European Wii");
			break;
		case CONF_CODE_KOR:
			// LKM - Confirmed
			sprintf(wiiregion, "Korean Wii");
			break;
		case CONF_CODE_AUS:
			// LAH - Confirmed
			sprintf(wiiregion, "n Australian Wii");
			break;
		case CONF_CODE_USAK:
			// KU - Confirmed (thanks LiNkZoR)
			sprintf(wiiregion, "new USA Wii");
			break;
		case CONF_CODE_EURHK:
			// KEH - Confirmed (thanks JohnyNyga)
		case CONF_CODE_EURMK:
			// KEM - Confirmed (thanks VampireLordAlucard)
		case CONF_CODE_EURFK:
			// KEF - Confirmed (thanks sureiya)
			sprintf(wiiregion, "new European Wii");
			break;
		case CONF_CODE_AUSK:
			// KAM - Confirmed (thanks Drexyl)
			sprintf(wiiregion, "new Australian Wii");
			break;
		case CONF_CODE_DLPH:
			// LE - Confirmed by myself
			sprintf(wiiregion, "Running Dolphin");
			break;
		case CONF_CODE_VJPNI:
			sprintf(wiiregion, "n 8GB Japanese Wii U");
			break;
		case CONF_CODE_VJPNO:
			sprintf(wiiregion, "32GB Wii U");
			break;
		case CONF_CODE_VUSAI:
			sprintf(wiiregion, "8GB USA Wii U");
			break;
		case CONF_CODE_VUSAO:
			sprintf(wiiregion, "32GB USA Wii U");
			break;
		case CONF_CODE_VEURHI:
		case CONF_CODE_VEURMI:
		case CONF_CODE_VEURFI:
			sprintf(wiiregion, "n 8GB European Wii U");
			break;
		case CONF_CODE_VEURHO:
		case CONF_CODE_VEURMO:
		case CONF_CODE_VEURFO:
			sprintf(wiiregion, "32GB European Wii U");
			break;
		case CONF_CODE_VKORI:
			sprintf(wiiregion, "n 8GB Korean Wii U");
			break;
		case CONF_CODE_VKORO:
			sprintf(wiiregion, "32GB Korean Wii U");
			break;
		case CONF_CODE_VAUSI:
			sprintf(wiiregion, "n 8GB Australian Wii U");
			break;
		case CONF_CODE_VAUSO:
			sprintf(wiiregion, "32GB Australian Wii U");
			break;
		case CONF_EBADVALUE:			
			if (!CONF_GetModel(model)) 
				sprintf(wiiregion, "Unknown SC %s model %s", serialcode, model);
			else
				sprintf(wiiregion, "Unknown SC %s", serialcode);			
			break;
		default:
			sprintf(wiiregion, "Error getting region");			
			break;
	}
}

#endif