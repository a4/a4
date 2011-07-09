//
//   @file    AtlasLabels.h         
//   
//   @author M.Sutton
// 
//   Copyright (C) 2010 Atlas Collaboration
//
//   $Id: AtlasLabels.h, v0.0   Thu 25 Mar 2010 10:34:20 CET $


#ifndef __ATLASLABELS_H
#define __ATLASLABELS_H

#include "RTypes.h"

void ATLASLabelAutoScale(Double_t x,Double_t y,bool Preliminary=false,Color_t color=1); 

void ATLASLabelOld(Double_t x,Double_t y,bool Preliminary=false,Color_t color=1); 

void ATLASVersion(char* version=NULL,Double_t x=0.88,Double_t y=0.975,Color_t color=1); 

void myText(Double_t x,Double_t y,Color_t color,char *text); 

void myBoxText(Double_t x, Double_t y,Double_t boxsize,Int_t mcolor,char *text); 

void myMarkerText(Double_t x,Double_t y,Int_t color,Int_t mstyle,char *text); 

#endif // __ATLASLABELS_H
