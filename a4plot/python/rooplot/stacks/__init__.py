from atlas_style import AtlasStyle

atlas_style = AtlasStyle()

from ROOT import gROOT
gROOT.SetStyle("AtlasStyle")
gROOT.ForceStyle()

from stacks import get_legend, get_lumi_label, set_styles, stack_1D, plot_1D
