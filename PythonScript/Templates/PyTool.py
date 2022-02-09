# python Tool script
# ------------------
# import cppyy and ctypes for interaction with c++ entities
import cppyy, cppyy.ll
import ctypes
# streamline accessing std namespace
std = cppyy.gbl.std
# pull in classes from the DataModel
cppyy.add_include_path('include')
cppyy.include('DataModel.h')
cppyy.load_library('libDataModel.so')
from cppyy.gbl import DataModel
from cppyy.gbl import Store
from cppyy.gbl import Logging
# create global variables
m_data = DataModel()
m_variables = Store()
m_log = Logging()

# helper function to bootstrap access to the parent ToolChain
# do not modify or remove this function!
def SetToolChainVars(m_data_in, m_variables_in, m_log_in):
    m_data = cppyy.ll.reinterpret_cast['DataModel*'](m_data_in)
    m_variables = cppyy.ll.reinterpret_cast['Store*'](m_variables_in)
    m_log = cppyy.ll.reinterpret_cast['Logging*'](m_log_in)
    return 1

###########################
## Your Python Tool Here ##
###########################

# Tool 'member' variables that will be carried over
# between Initialise/Execute/Finalise calls
verbosity = 1
v_error=0;
v_warning=1;
v_message=2;
v_debug=3;

def Initialise():
    m_log.Log(__file__+" Initialising", v_debug, verbosity)
    
    # retrieve tool verbosity from m_variables store
    verbose_ref = ctypes.c_int(1)
    ok = m_variables.Get("verbose",verbose_ref)
    if ok != 0: verbosity = verbose_ref.value
    
    return 1

def Execute():
    m_log.Log(__file__+" Executing", v_debug, verbosity)
    
    return 1

def Finalise():
    
    m_log.Log(__file__+" Finalising",v_debug, verbosity)
    
    return 1


