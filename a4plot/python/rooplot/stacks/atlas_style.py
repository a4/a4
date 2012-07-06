## @package ROOT.AtlasStyle
#  @short Module for the class setting up ROOT in the "ATLAS style"
#
#  All plots of tov_checkResults.py are created using the style defined
#  in this module.
#
#  @author Attila Krasznahorkay Jr.
#
#  $Revision: 186902 $
#  $Date: 2009-05-26 12:33:25 -0400 (Tue, 26 May 2009) $

# ROOT import(s):
import ROOT

##
# @short The "ATLAS style" used by tov_checkResults.py
#
# Class for setting up a standard ROOT session to draw things in the "ATLAS
# style". It's a "port" of a C++ class I'm using since a long time ...
#
# @author Attila Krasznahorkay Jr.
#
# $Revision: 186902 $
# $Date: 2009-05-26 12:33:25 -0400 (Tue, 26 May 2009) $
class AtlasStyle( ROOT.TStyle ):

    ##
    # @short Constructor for the object
    #
    # This constructor has the same signature as ROOT.TStyle.__init__. It creates
    # a new style object with the specified name and description.
    #
    # @param name Name of the style object created (optional)
    # @param description Short description of the created object (optional)
    def __init__( self, name = "AtlasStyle", description = "Default ATLAS style" ):

        ROOT.TStyle.__init__( self, name, description )

        # Use plain black-on-white colours:
        icol = 0
        self.SetFrameBorderMode( icol )
        self.SetFrameFillColor( icol )
        self.SetFrameFillStyle( 0 )
        self.SetCanvasBorderMode( icol )
        self.SetCanvasColor( icol )
        self.SetPadBorderMode( icol )
        self.SetPadColor( icol )
        self.SetStatColor( icol )

        # Set the paper & margin sizes:
        self.SetPaperSize( 20, 26 )
        self.SetPadTopMargin( 0.05 )
        self.SetPadRightMargin( 0.05 )
        self.SetPadBottomMargin( 0.16 )
        self.SetPadLeftMargin( 0.16 )

        # set title offsets (for axis label)
        self.SetTitleXOffset(1.4)
        self.SetTitleYOffset(1.4)

        # Use large fonts:
        font = 42
        tsize = 0.08
        
        self.SetTextFont( font )
        self.SetLabelFont( font, "x" )
        self.SetTitleFont( font, "x" )
        self.SetLabelFont( font, "y" )
        self.SetTitleFont( font, "y" )
        self.SetLabelFont( font, "z" )
        self.SetTitleFont( font, "z" )

        self.SetTextSize( tsize )
        self.SetLabelSize( tsize, "x" )
        self.SetTitleSize( tsize, "x" )
        self.SetLabelSize( tsize, "y" )
        self.SetTitleSize( tsize, "y" )
        self.SetLabelSize( tsize, "z" )
        self.SetTitleSize( tsize, "z" )

        # Use bold lines and markers:
        self.SetMarkerStyle( 20 )
        self.SetMarkerSize( 1.0 )
        self.SetHistLineWidth( 2 )
        self.SetLineStyleString( 2, "[12 12]" )

        self.SetEndErrorSize(0.)

        # Do not display any of the standard histogram decorations:
        self.SetOptTitle( 0 )
        self.SetOptStat( 0 )
        self.SetOptFit( 0 )

        # Put tick marks on RHS of plots:
        #self.SetPadTickY( 1 )
        self.SetPadTickX( 1 ) # JOHANNES

