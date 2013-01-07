from ROOT import gROOT, gStyle, Double
from ROOT import TLegend, TLatex, TCanvas, THStack, TLine, TBox
from ROOT import kYellow, kBlack, kWhite, kRed, kWhite, kOrange

import os
import random

from colors import set_color_1D, set_color_2D, set_data_style, set_MCTotal_style, set_signal_style_1D

tsize = 0.06
tyoffset = 1.1 * 0.06 / tsize
txoffset = 2.5 * 0.06 / tsize
lmargin = 0.14

def get_legend(data, sum_mc, list_mc, signals):
    #legend = TLegend(0.2,0.65,0.4,0.94)
    llen = 1 + len(data) + len(list_mc) + len(signals)
    #mtop, mright, width, hinc = 0.01, 0.01, 0.38, 0.05
    #mtop, mright, width, hinc = 0.07, 0.25, 0.15, 0.01

    if tsize == 0.06:
        mtop, mright, width, hinc = 0.13, 0.07, 0.25, 0.6666*tsize
    else:
        mtop, mright, width, hinc = 0.13, 0.1, 0.3, 0.6666*tsize
    x1, y1, x2, y2 = 1.0 - mright - width, 1.0 - mtop, 1.0 - mright, 1.0 - mtop - hinc*llen
    print x1, y1, x2, y2
    legend = TLegend(x1, y1, x2, y2)
    legend.SetNColumns(2)
    legend.SetColumnSeparation(0.05)
    legend.SetBorderSize(0)
    legend.SetTextFont(42)
    legend.SetTextSize(tsize)
    legend.SetFillColor(0)
    legend.SetFillStyle(0)
    legend.SetLineColor(0)
    for d in data:
        legend.AddEntry(d, os.path.split(d.GetTitle())[1][:-5] if d.GetTitle()[-5:]=='.root' else os.path.split(d.GetTitle())[1], "p")
    if sum_mc:
        legend.AddEntry(sum_mc,"MC (stat)","flp")  # <== NB: omit this entry for 2D histogram
    for h in list_mc: # sorted by initial XS
        legend.AddEntry(h, os.path.split(h.GetTitle())[1][:-5] if h.GetTitle()[-5:]=='.root' else os.path.split(h.GetTitle())[1],"f")
    for s in signals:
       legend.AddEntry(s, os.path.split(s.GetTitle())[1][:-5] if s.GetTitle()[-5:]=='.root' else os.path.split(s.GetTitle())[1],"l")
    return legend

#NB: [ATLAS Preliminary label for when plots are approved only: 
def get_lumi_label(lumi="168 pb^{-1}",centermass="8", atlas=True, draft=True):
    x, y = lmargin + 0.03, (0.75 if atlas else 0.77)
    n = TLatex()
    n.SetNDC()
    n.SetTextFont(32)
    n.SetTextColor(kBlack)
    n.SetTextSize(tsize*1.25)
    n.DrawLatex(x, y,"#intL dt = %s, #sqrt{s} = %s TeV" % (lumi,centermass))
    #x, y = 0.21, 0.65
    x, y = 0.18, 0.85
    if not atlas:
        return n, None
    l = TLatex()
    l.SetNDC()
    l.SetTextFont(42)
    l.SetTextColor(kBlack)
    if draft:
        l.DrawLatex(x,y,"#bf{#it{ATLAS work in progress}}")
    else:
        l.DrawLatex(x,y,"#bf{#it{ATLAS preliminary}}")
    return n, l

def create_mc_sum(mc_list, existing_mc_sum=None):
    if not mc_list:
        return None, None
    if existing_mc_sum:
        mc_sum = existing_mc_sum
    else:
        mc_sum = mc_list[0].Clone("mc_sum")
        mc_sum.SetDirectory(0)
        for h in mc_list[1:]:
            for b in xrange(1, h.GetXaxis().GetNbins()+1):
                # If there is negative weight in one channel, it should not
                # be subtracted from other channels
                if not (0 < h.GetBinContent(b)):
                    h.SetBinContent(b, 0.0)
                # Sometimes negative Errors occur - they play havoc with the
                # Display of error bands...
                if not (0 < h.GetBinError(b)):
                    h.SetBinError(b, 0.0)
            mc_sum.Add(h)
    mc_sum.SetMarkerSize(0)
    mc_sum.SetLineColor(kRed)
    mc_sum.SetFillColor(kOrange)
    mc_sum.SetFillStyle(3144)
    
    mc_sum_line = mc_sum.Clone("mc_sum_line")
    mc_sum_line.SetDirectory(0)
    mc_sum_line.SetFillStyle(0)
    mc_sum_line.SetFillColor(kWhite)

    #mc_sum.SetLineStyle(0)
    mc_sum.SetTitle("SM (stat)")
    return mc_sum_line, mc_sum

def create_cuts(cuts_left, cuts_right, ymin, ymax, w):
    save = []
    hashwidth = 0.01*w
    for vl in cuts_left + cuts_right:
        l = TLine(vl, ymin, vl, ymax)
        l.SetLineColor(kRed)
        l.Draw()
        save.append(l)
    #gStyle.SetHatchesSpacing(0.01)
    #gStyle.SetHatchesLineWidth(2)
    for vl in cuts_left:
        b = TBox(vl, ymin, vl - hashwidth, ymax)
        b.SetFillStyle(3345)
        b.SetFillColor(kRed)
        b.SetLineStyle(0)
        b.Draw()
        save.append(b)
    for vl in cuts_right:
        b = TBox(vl, ymin, vl + hashwidth, ymax)
        b.SetFillStyle(3354)
        b.SetFillColor(kRed)
        b.SetLineStyle(0)
        b.Draw()
        save.append(b)
    return save
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

from ROOT import gPad, kOrange, kRed
saver = []

def stack_1D(name, data, list_mc, signals, lumi="X",centermass="8", rebin=1, sum_mc=None, rebin_to=None, range=None, compare=False, sigma=False, log=False, prelim=False, cuts_left=(), cuts_right=()):
    data = [h.Clone() for h in data]
    list_mc = [h.Clone() for h in list_mc]
    signals = [h.Clone() for h in signals]
    sum_mc = sum_mc.Clone() if sum_mc else sum_mc

    all_histos = list_mc + signals + data

    saver.extend(all_histos)
    saver.append(sum_mc)

    h = all_histos[0]
    xaxis = h.GetXaxis()
    b1, b2 = h.GetXaxis().GetFirst(), h.GetXaxis().GetLast()

    if range:
        x1, x2 = range
        x2 -= 0.000001
        range = (x1, x2)
        b1, b2 = xaxis.FindBin(x1), xaxis.FindBin(x2)

    if rebin_to:
        nbins = xaxis.GetNbins()
        if range:
            nbins = b2 - b1 + 1
        rebin = int(round(nbins*1.0/rebin_to))
        if rebin < 1:
            rebin = 1

    if rebin != 1:
        for histo in all_histos:
            histo.Rebin(rebin)

    if True: # squash overflow bins
        e = Double()
        for histo in all_histos + [sum_mc] if sum_mc else []:
            c = histo.IntegralAndError(0, b1, e)
            histo.SetBinContent(b1, c)
            histo.SetBinError(b1, e)
            c = histo.IntegralAndError(b2, histo.GetNbinsX() + 1, e)
            histo.SetBinContent(b2, c)
            histo.SetBinError(b2, e)

    x1, x2 = h.GetXaxis().GetBinLowEdge(b1), h.GetXaxis().GetBinLowEdge(b2)
   
    # set up pads 

    cpad = gPad.func()
    wh, ww = cpad.GetWh(), cpad.GetWw()
    pad_fraction = 0
    global tsize, tyoffset, txoffset
    if compare or sigma:
        tsize = 0.06 # was 0.06
        tyoffset = 1.1 * 0.06 / tsize
        txoffset = 2.5 * 0.06 / tsize
        pad_fraction = 0.3
        cpad.Divide(1, 2, 0.01, 0.01)
        cpad.cd(1).SetPad(0, pad_fraction, 1, 1.0)
        #cpad.cd(1).SetBottomMargin(0.15)
        cpad.cd(1).SetTopMargin(0.08)
        cpad.cd(1).SetBottomMargin(0.0)
        cpad.cd(1).SetLeftMargin(lmargin)
        cpad.cd(1).SetFillStyle(4000)
        #cpad.cd(1).SetGridx()
        #cpad.cd(1).SetGridy()
        if log:
            cpad.cd(1).SetLogy()
        cpad.cd(2).SetPad(0, 0.0, 1, pad_fraction+0.1)
        cpad.cd(2).SetGridx()
        cpad.cd(2).SetGridy()
        cpad.cd(2).SetFillStyle(4000)
        cpad.cd(2).SetTopMargin(0.25)
        cpad.cd(2).SetBottomMargin(0.4)
        cpad.cd(2).SetLeftMargin(lmargin)
        cpad.cd(1)
        down_pad_fraction = pad_fraction+0.1
    else:
        tsize = 0.04 # was 0.06
        tyoffset = 1.1 * 0.06 / tsize
        txoffset = 2.5 * 0.06 / tsize
        cpad.SetTopMargin(0.08)
        cpad.SetBottomMargin(0.16)
        cpad.SetLeftMargin(lmargin)
        if log:
            cpad.SetLogy()

    # sort backgrounds by integral
    list_mc.sort(key=lambda h : h.Integral())
    list_mc.sort(key=lambda h : h.GetTitle() != "QCD")

    hsave, mcstack = None, None
    if list_mc:
        mc_sum_line, mc_sum = create_mc_sum(list_mc, sum_mc)
        all_histos.append(mc_sum)
        all_histos.append(mc_sum_line)
        # Create MC stack
        mcstack = THStack()
        for h in list_mc:
            mcstack.Add(h)
        #all_histos.append(mcstack)

    # set range
    if range:
        h = all_histos[0]
        xa = h.GetXaxis()
        original_size = xa.GetBinLowEdge(xa.GetFirst()), xa.GetBinUpEdge(xa.GetLast())
        for histo in all_histos:
            xaxis = histo.GetXaxis()
            xaxis.SetRangeUser(*range)
    
    # get min/max
    ymax = (max(h.GetMaximum() for h in all_histos) + 1) * (1.5 if not log else 100)
    ymin = max(1.0 if log else 0.01, min(h.GetMinimum() for h in all_histos))

    # unset range for mc
    if range:
        for histo in list_mc:
            xaxis = histo.GetXaxis()
            xaxis.SetRangeUser(*original_size)

    # Draw everything
    axis = None
    if list_mc:
        axis = mcstack
        mcstack.Draw("Hist")
        if range:
            mcstack.GetXaxis().SetRangeUser(*range)
        mc_sum.Draw("e2same")
        mc_sum_line.Draw("hist same")
    for signal in signals:
        if not list_mc and signal == signals[0]:
            axis = signal
            signal.Draw("hist")
        else:
            signal.Draw("hist same")
    for d in data:
        if not signals and not list_mc and d == data[0]:
            axis = d
            d.Draw("pe")
        else:
            d.Draw("pe same")

    comparefactor = 1
    if compare:
        comparefactor = 0
    pad_factor = 1.0/(1 - pad_fraction)
    axis.GetYaxis().SetLabelSize(tsize * pad_factor)
    axis.GetYaxis().SetTitleSize(tsize * pad_factor)
    axis.GetYaxis().SetTitleOffset(tyoffset / pad_factor)
    axis.GetXaxis().SetLabelSize(tsize * pad_factor * comparefactor)
    axis.GetXaxis().SetTitleSize(tsize * pad_factor * comparefactor)
    axis.GetXaxis().SetTitleOffset(comparefactor * tyoffset / pad_factor)

    legend = get_legend(data,mc_sum,list(reversed(list_mc)),signals)
    legend.Draw()

    save = []

    save.extend(create_cuts(cuts_left, cuts_right, 0 if log else ymin, ymax/(1.3 if not log else 50), x2-x1))

    # Try to fix the limits...
    axis.SetMaximum(ymax)
    axis.SetMinimum(ymin)

    dhist = mcstack if mcstack else [signals + data][0]
    
    lumiLabel, atlasLabel = get_lumi_label(lumi, centermass, atlas=prelim, draft=True)
    lumiLabel.Draw()
    if atlasLabel:
        atlasLabel.Draw()
    save.extend((atlasLabel, lumiLabel))

    if (compare or sigma) and mcstack:
        cpad.cd(2)
        # Create MC sum
        cdata = [d.Clone() for d in data]
        save.extend(cdata)
        for cd in cdata:
            cd.SetDirectory(0)
        cmc  = mc_sum_line.Clone("mc_sum_zero")
        cmc2 = mc_sum_line.Clone("mc_sum_zero_line")
        cmc.SetFillColor(kOrange)
        cmc.SetFillStyle(2001)
        cmc2.SetLineColor(kRed)
        cmc2.SetFillStyle(0)
        cmc.SetDirectory(0)
        cmc2.SetDirectory(0)
        save.append(cmc)
        save.append(cmc2)

        Nbins = int(mcstack.GetXaxis().GetNbins())
        if sigma and cdata:
            for i in xrange(Nbins + 2):
                mc, mcerr = cmc.GetBinContent(i), cmc.GetBinError(i)
                for cd in cdata:
                    d, dstat = cd.GetBinContent(i), cd.GetBinError(i)
                    if dstat < 1:
                        dstat = 1
                    sf = (mcerr**2 + dstat**2)**0.5
                    if d > 0:
                        cd.SetBinContent(i, (d - mc)/sf)
                        cd.SetBinError(i, dstat/sf)
                    else:
                        pass # content and error are both already zero

                cmc.SetBinContent(i, 0.0)
                cmc.SetBinError(i, mcerr/sf)
                cmc2.SetBinContent(i, 0.0)
                cmc2.SetBinError(i, 0.0)
                #cmc2.GetYaxis().SetTitle("( Data - SM ) / #sigma_{stat,MC+Data} ")
                cmc2.GetYaxis().SetTitle("( Data - MC ) / #sigma_{stat}")
        else:
            for i in xrange(Nbins + 2):
                sf = cmc.GetBinContent(i)
                if sf > 0:
                    cmc.SetBinError(i, cmc.GetBinError(i)/sf)
                    for cd in cdata:
                        cd.SetBinContent(i, cd.GetBinContent(i)/sf)
                        cd.SetBinError(i, cd.GetBinError(i)/sf)
                else:
                    cmc.SetBinError(i, 1.0)
                    for cd in cdata:
                        cd.SetBinContent(i, 0)
                        cd.SetBinError(i, 0)
                cmc.SetBinContent(i, 1.0)
                cmc2.SetBinContent(i, 1.0)
                cmc2.GetYaxis().SetTitle("Data / MC")
        #cmc2.GetXaxis().SetTitle("")

        if cdata:
            mx = max(cd.GetBinContent(cd.GetMaximumBin())+0.2*cd.GetBinError(cd.GetMaximumBin()) for cd in cdata)
            #mn = min(cd.GetBinContent(cd.GetMinimumBin())-0.2*cd.GetBinError(cd.GetMinimumBin()) for cd in cdata)
            mn = mx
            for cd in cdata:
                minc, minbin = min([(cd.GetBinContent(i),i) for i in xrange(1, cd.GetNbinsX()+1) if cd.GetBinContent(i) > 0])
                mn = min(minc - 0.2*cd.GetBinError(minbin), mn)
            if compare:
                mx = min(max(1.3, mx), 2)
                mn = min(0.7, mn)
            for h in cdata + [cmc, cmc2]:
                h.SetMaximum(mx)
                h.SetMinimum(mn)

        cmc2.GetYaxis().SetNdivisions(5,0,0)

        cmc2.Draw("hist")
        cmc.Draw("e2 same")
        for cd in cdata:
            cd.Draw("pe same")

    
        sf = 1.0
        ysf = 0.7
        pad_factor = 1.0/down_pad_fraction
        cmc2.GetYaxis().SetLabelSize(tsize*pad_factor*sf*ysf)
        cmc2.GetYaxis().SetTitleSize(tsize*pad_factor*sf)
        cmc2.GetYaxis().SetTitleOffset(tyoffset / pad_factor / sf)
        cmc2.GetXaxis().SetLabelSize(tsize*pad_factor*sf)
        cmc2.GetXaxis().SetTitleSize(tsize*pad_factor*sf)
        cmc2.GetXaxis().SetTitleOffset(txoffset / pad_factor / sf)
        save.extend(create_cuts(cuts_left, cuts_right, mn, mx, x2-x1))
    
        cpad.cd()

    return legend, mcstack, mc_sum, mc_sum_line, save

def plot_1D(name, data, list_mc, signals, **kwargs):
    set_styles(data, list_mc, signals)
    return stack_1D(name, data, list_mc, signals, **kwargs)

#All MC stacked in this order:
#- ttbar 1st 
#- Z+jets 2nd
#- W+jets 3rd
#- dijets last
#(i.e. inversely by cross-section)

#If a separate signal sample is drawn - it should not be added to the stack, but instead drawn as a separate line (black and SetLineWidth(4)).

#-----------------

