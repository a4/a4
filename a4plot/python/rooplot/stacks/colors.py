from ROOT import kAzure, kBlue, kWhite, kRed, kBlack, kGray, kGreen, kYellow, kTeal, kCyan, kMagenta, kSpring

clrs = (kWhite, kRed, kBlack, kGray, kGreen, kYellow, kTeal, kCyan, kSpring, kBlue)
clrs = (kWhite, kGray, kBlack, kBlue, kGreen, kTeal, kTeal, kRed, kSpring, kMagenta)
clrs = (kGray, kBlack, kBlue, kGreen, kTeal, kRed, kMagenta, kYellow, kCyan)
clrs = (kBlue, kGreen, kYellow, kCyan, kRed, kMagenta,  kTeal, kBlack, kGray)

# WW paper:
#Wjets/dijet  kBlue
#Diboson    kMagenta-7
#Drell-Yan  kCyan-7
#Top          8
#WW         kYellow-10

def set_color_1D(h, name, cnum):
    h.SetFillStyle(1001)
    if "W+jets" in name: 
        h.SetFillColor(kBlue)
    elif "Diboson" in name:
        h.SetFillColor(kMagenta-7)
    elif "Drell-Yan" in name:
        h.SetFillColor(kCyan-7)
    elif "Top" in name:
        h.SetFillColor(8)
    elif "WW" in name:
        h.SetFillColor(kYellow-10)
    elif "gamma" in name:
        h.SetFillColor(kGray)
    elif "QCD" in name:
        h.SetFillColor(kBlack)
    else:
        h.SetFillColor(clrs[cnum % len(clrs)])

#Suggested plot colours:
def set_color_1D_old(h, name, cnum):
    h.SetFillStyle(1001)
    if name == "W+jets": 
        h.SetFillColor(kAzure+1)
    elif name == "Z+jets":
        h.SetFillColor(kBlue+1)
    elif "Dijet" in name:
        h.SetFillColor(kWhite)
    elif "ttbar" in name or "t#bar{t}" in name:
        h.SetFillColor(kGreen-9)
    elif "WW" in name:
        h.SetFillColor(kRed+1)
    elif "Diboson" in name or "VV" in name:
        h.SetFillColor(kMagenta-2)
    else:
        h.SetFillColor(clrs[cnum % len(clrs)])
        
def set_color_2D(h,name):
    if name == "W+jets": 
        h.SetLineColor(kBlue+3)
    elif name == "QCD":
        h.SetFillColor(kGray+2)

def set_data_style(h):
    h.SetMarkerStyle(20)
    h.SetMarkerSize(2.0)

def set_MCTotal_style(h):
    h.SetMarkerSize(0)
    h.SetLineColor(kRed)
   
def set_signal_style_1D(h):
    h.SetLineWidth(4)
