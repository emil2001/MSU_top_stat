
import cog

cog_key_counter = 0
def get_cog_key():
  global cog_key_counter
  cog_key_counter += 1
  return str(cog_key_counter-1)

def if_then_else(if_str, then_str, else_str=""):
  string = "if(" + if_str + "){" + then_str + "\n}"
  if else_str : string += "else { \n" + else_str + "\n }"
  cog.outl( string )

def cpp_class(class_type, name, arguments):
  string = class_type + " " + name + " = "
  if "*" in class_type : 
    string += "new "
    string += class_type[:class_type.find("*")] + "("
  else : string += class_type + "("
  for i, argument in enumerate(arguments) :
    #if type(argument) == type("str") : string += "\"" + argument + "\""
    #else:                              string += str(argument)
    string += str(argument)
    if i != len(arguments)-1:      string += ", "
  string += ");"
  cog.outl( string )

def cpp_TH1(name, title, nbins, xmin, xmax, type="D*"):
  hist_type = "TH1" + type
  hist_name = "hist_" + get_cog_key()
  if not name : name = "\"" + hist_name + "\""
  cpp_class(hist_type, hist_name, [name, title, nbins, xmin, xmax])

  # string = "hists_1d.push_back( make_pair(\"" + var.key + "\", hist_" + var.key + ") );"
  # cog.outl( string )
  return hist_name

def cpp_TH2(name, title, xbins, xmin, xmax, ybins, ymin, ymax, type="D*"):
  hist_type = "TH2" + type
  hist_name = "hist_" + get_cog_key()
  if not name : name = "\"" + hist_name + "\""
  cpp_class(hist_type, hist_name, [name, title, xbins, xmin, xmax, ybins, ymin, ymax])

  # string = "hists_1d.push_back( make_pair(\"" + var.key + "\", hist_" + var.key + ") );"
  # cog.outl( string )
  return hist_name




#================== THREADS





















