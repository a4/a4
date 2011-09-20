from math import ceil
from ROOT import gROOT, TCanvas, gPad


def batch_mode(f):
    def wrapped(*args,**kwargs):
        r = gROOT.IsBatch()
        gROOT.SetBatch(True)
        res = f(*args, **kwargs)
        gROOT.SetBatch(r)
        return res
    return wrapped

@batch_mode
def make_page(drawables, name="plots", title="plots", landscape=False, dpi=96, margin=2, split=(2,3), format="pdf", log=False):
    inch_per_cm = 0.3937
    wx, wy = (21 - 2*margin)*inch_per_cm*dpi, (29.7 - 2*margin)*inch_per_cm*dpi
    sx, sy = split
    if landscape:
        wx, wy = wy, wx
        sx, sy = sy, sx
    wx, wy = int(wx), int(wy)

    saver = []
    n_per_page = sx*sy
    pages = int(ceil(len(drawables)*1.0/n_per_page))
    for page in range(1, pages+1):
        on_page = drawables[(page-1)*n_per_page:page*n_per_page]
        c = TCanvas("%s_p%i" % (name, page), "%s (Page %i/%i)" % (title, page, pages), wx, wy)

        c.SetRightMargin(wx/2.0)
        c.Divide(sx, sy)
        for i in range(len(on_page)):
            c.cd(i+1)
            if log:
                c.SetLogy(i+1)
            saver.append(on_page[i]())
            if log:
                gPad.SetLogy(True)
                c.SetLogy(i+1)
        c.cd()
        c.SaveAs("%s_%i.%s" % (name, page, format)) 


