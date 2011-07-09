from ROOT import gROOT, TLegend, TLatex, TCanvas, THStack
from ROOT import kYellow, kBlack, kWhite

import os
import random

from colors import set_color_1D, set_color_2D, set_data_style, set_MCTotal_style, set_signal_style_1D

def get_legend(data, sum_mc, list_mc, signals):
    #legend = TLegend(0.2,0.65,0.4,0.94)
    llen = 1 + len(data) + len(list_mc) + len(signals)
    mtop, mright, width, hinc = 0.01, 0.01, 0.38, 0.05
    x1, y1, x2, y2 = 1.0-mright-width, 1.0-mtop, 1.0 - mtop, 1.0-mright - hinc*llen
    print x1, y1, x2, y2
    legend = TLegend(x1, y1, x2, y2)
    legend.SetBorderSize(0)
    legend.SetTextFont(42)
    legend.SetTextSize(0.04)
    legend.SetFillColor(0)
    legend.SetLineColor(0)
    for d in data:
        legend.AddEntry(d, d.GetTitle(), "p")
    if sum_mc:
        legend.AddEntry(sum_mc,"Monte Carlo","flp")  # <== NB: omit this entry for 2D histogram
    for h in list_mc: # sorted by initial XS
        legend.AddEntry(h,h.GetTitle(),"f")
    for s in signals:
       legend.AddEntry(s,s.GetTitle(),"l")
    return legend

def get_lumi_label(lumi="168 pb^{-1}"):
    n = TLatex()
    n.SetNDC()
    n.SetTextFont(32)
    n.SetTextColor(kBlack)
    n.DrawLatex(0.42,0.85,"#intL dt ~ %s" % (lumi))
    return n

#-----------------
#Axis labels:
#y-axis labels: Entries / x Units (x = bin width, Units = e.g. GeV)
#x-axis labels: Quantity [Unit] (Quantity = e.g. M_{eff}, Units = e.g. GeV)
#----------------

#Other:
#no plot titles - histogram->SetTitle("");
#to change the maximum number of digits displayed - e.g. TGaxis::SetMaxDigits(3);
    
#Drawing 2D plots
#- Draw("box") for first MC (dijets)
#- then Draw("boxsame") for subsequent MC (W+jets)
#- Draw("psame") for data

def set_styles(data, mcs, signals):
    for d in data:
        set_data_style(d)
    for signal in signals:
        set_signal_style_1D(signal)
    for i, mc in enumerate(mcs):
        set_color_1D(mc,mc.GetTitle(), i)


def stack_1D(name, data, list_mc, signals, lumi="X", rebin=1, rebin_to=None, range=None):
    print len(data), len(list_mc), len(signals)
    all_histos = list_mc + signals + data

    if rebin_to:
        xaxis = all_histos[0].GetXaxis()
        nbins = xaxis.GetNbins()
        if range:
            x1, x2 = range
            nbins = xaxis.FindBin(x2) - xaxis.FindBin(x1) + 1
        rebin = int(round(nbins*1.0/rebin_to))
        if rebin < 1:
            rebin = 1

    if rebin != 1:
        for h in all_histos:
            h.Rebin(rebin)

    sum_mc, hsave, mcstack = None, None, None
    if list_mc:
        # Create MC sum
        sum_mc = list_mc[0].Clone("sum_mc")
        sum_mc.SetDirectory(0)
        sum_mc = sum_mc.Clone("hsave")
        for h in list_mc[1:]:
            sum_mc.Add(h)
        set_MCTotal_style(sum_mc)
        hsave = sum_mc.Clone("hsave")
        hsave.SetDirectory(0)
        sum_mc.SetFillColor(kYellow)
        sum_mc.SetFillStyle(3006)
        all_histos.append(sum_mc)
        all_histos.append(hsave) 

        # Create MC stack
        mcstack = THStack()
        for h in list_mc:
            mcstack.Add(h)
        all_histos.append(mcstack)

    ymax = (max(h.GetMaximum() for h in all_histos) + 1)*1.2
    ymin = max(1e-3,min(h.GetMinimum() for h in all_histos))
    print ymin, ymax
    for h in all_histos:
        h.SetMaximum(ymax)
        h.SetMinimum(ymin)

    # Draw everything
    if list_mc:
        mcstack.Draw("Hist")
        sum_mc.Draw("e2same")
        hsave.Draw("hist same")
    for signal in signals:
        if not list_mc and signal == signals[0]:
            signal.Draw("hist")
        else:
            signal.Draw("hist same")
    for d in data:
        if not signals and not list_mc and d == data[0]:
            d.Draw("pe")
        else:
            d.Draw("pe same")
    legend = get_legend(data,sum_mc,list(reversed(list_mc)),signals)
    legend.Draw()

    # Try to fix the limits...
    for h in all_histos:
        h.SetMaximum(ymax)
        h.SetMinimum(ymin)

    dhist = mcstack if mcstack else [signals + data][0]
    if range:
        xmin, xmax = range
        xaxis = dhist.GetXaxis()
        x1, x2 = xaxis.FindBin(xmin), xaxis.FindBin(xmax)
        xaxis.SetRangeUser(xmin, xmax)
    
    lumiLabel = get_lumi_label(lumi)
    lumiLabel.Draw()
    return legend, mcstack, sum_mc, hsave
    
def plot_1D(name, data, list_mc, signals, lumi="X", rebin=1, rebin_to=None, range=None):
    set_styles(data, list_mc, signals)
    return stack_1D(name, data, list_mc, signals, lumi, rebin, rebin_to, range)

#All MC stacked in this order:
#- ttbar 1st 
#- Z+jets 2nd
#- W+jets 3rd
#- dijets last
#(i.e. inversely by cross-section)

#If a separate signal sample is drawn - it should not be added to the stack, but instead drawn as a separate line (black and SetLineWidth(4)).

#-----------------

#NB: [ATLAS Preliminary label for when plots are approved only: 
def draw_preliminary():
    l = TLatex()
    l.SetNDC()
    l.SetTextFont(72)
    l.SetTextColor(kBlack)
    l.DrawLatex(0.21,0.65,"ATLAS")                                                                            
    m = TLatex()
    m.SetNDC()
    m.SetTextFont(42)
    m.DrawLatex(0.335,0.65,"Preliminary")
    return l, m
