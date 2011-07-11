from ROOT import kAzure, kBlue, kWhite, kRed, kBlack, kGray, kGreen, kYellow, kTeal, kCyan, kMagenta, kSpring

clrs = (kWhite, kRed, kBlack, kGray, kGreen, kYellow, kTeal, kCyan, kSpring, kBlue)
clrs = (kWhite, kGray, kBlack, kBlue, kGreen, kTeal, kTeal, kRed, kSpring, kMagenta)
clrs = (kGray, kBlack, kBlue, kGreen, kTeal, kRed, kSpring, kMagenta, kYellow, kCyan)

#Suggested plot colours:
def set_color_1D(h, name, cnum):
    h.SetFillStyle(1001)
    if name == "W+jets": 
        h.SetFillColor(kAzure+1)
    elif name == "Z+jets":
        h.SetFillColor(kBlue+1)
    elif "Dijets" in name:
        h.SetFillColor(kWhite)
    elif "ttbar" in name or "t#bar{t}" in name:
        h.SetFillColor(kGreen-9)
    elif "WW" in name:
        h.SetFillColor(kBlue-2)
    else:
        h.SetFillColor(clrs[cnum % len(clrs)])
        
def set_color_2D(h,name):
    if name == "W+jets": 
        h.SetLineColor(kBlue+3)
    elif name == "QCD":
        h.SetFillColor(kGray+2)

def set_data_style(h):
    h.SetMarkerStyle(20)
    h.SetMarkerSize(1.2)

def set_MCTotal_style(h):
    h.SetMarkerSize(0)
    h.SetLineColor(kRed)
   
def set_signal_style_1D(h):
    h.SetLineWidth(4)
