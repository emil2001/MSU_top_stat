
class Analyser():
  def __init__(self):
    default_variables = []
    tmva_variables    = []
    custom_variables  = []


###
plot_variables = []
default_plots = []
corr_plots    = []
comp_plots    = []
cut_plots     = []

###
class PlotVariable():
  def __init__(self, key, name, xmin, xmax, nbins = 30, draw_mode="ab", hist_name=None):
    self.key  = key
    self.name = name
    self.xmin = xmin
    self.xmax = xmax
    self.nbins = nbins
    self.draw_mode = draw_mode
    self.hist_name = hist_name
    self.frozen = False
    global plot_variables
    plot_variables.append( self )

class DefaultPlot():
  def __init__(self, var, draw_mode):
    self.var = var
    self.draw_mode = draw_mode
    self.drawer = None
    global default_plots
    default_plots.append( self )

class CorrPlot():
  def __init__(self, plot_var_x, plot_var_y):
    self.var_x    = plot_var_x
    self.var_y    = plot_var_y
    self.hist_name = None
    self.drawer = None
    self.draw_mode = None
    global corr_plots
    corr_plots.append( self )

class CompPlot():
  def __init__(self, channel, short_name, vars, vars_names, xtitle, draw_mode=None):
    self.channel    = channel
    self.vars       = vars
    self.vars_names = vars_names
    self.title      = xtitle
    self.short_name = short_name
    self.drawer = None
    self.draw_mode = draw_mode
    self.xmin, self.xmax = 0, 0
    global comp_plots
    comp_plots.append( self )

class CutPlot():
  def __init__(self, plot_var, cut, cut_title):
    self.var    = plot_var
    self.cut    = cut
    self.cut_title     = cut_title
    self.hist_cut_name = None
    self.drawer_def = None
    self.drawer_cut = None
    self.draw_mode = None
    global cut_plots
    cut_plots.append( self )















