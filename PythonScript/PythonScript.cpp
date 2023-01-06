#include "PythonScript.h"

PythonScript::PythonScript():Tool(){}

bool PythonScript::Initialise(std::string configfile, DataModel &data){

  /////////////////// Usefull header ///////////////////////
  if(configfile!="")  m_variables.Initialise(configfile); //loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////

  m_variables.Get("PythonScript",pythonscript);
  initialisefunction="Initialise";
  executefunction="Execute";
  finalisefunction="Finalise";
  m_variables.Get("InitialiseFunction",initialisefunction);
  m_variables.Get("ExecuteFunction",executefunction);
  m_variables.Get("FinaliseFunction",finalisefunction);

  // Initialising Python
  pyinit=0;
  if(!(m_data->CStore.Get("PythonInit",pyinit))){
    /* Add a built-in module, before Py_Initialize */
    //PyImport_AppendInittab("Store", PyInit_Store);
    Py_Initialize();
  }

  //// if multiple python scipts this is to keep track of which is last for finalisation
  pyinit++;
  m_data->CStore.Set("PythonInit",pyinit);
  
  /// Starting python thread for this tool
  //pythread=Py_NewInterpreter();
  //PyThreadState_Swap(pythread);

  // Loading store API into python env
////  Py_InitModule("Store", StoreMethods);
////  PyModule_Create(&StoreModule);
//  PyImport_ImportModule("Store");

  // Loading python script/module
  //  std::cout<<pythonscript.c_str()<<std::endl;
  //pName = PyString_FromString(pythonscript.c_str());
  pName = PyUnicode_FromString(pythonscript.c_str());

  // Error checking of pName to be added later
  pModule = PyImport_Import(pName);
  //pModule = PyImport_ReloadModule(pName); 
  
  // deleting pname
  Py_DECREF(pName);

  //loading python fuctions
  if (pModule != NULL) {

    pFuncT = PyObject_GetAttrString(pModule, "SetToolChainVars");
    pFuncI = PyObject_GetAttrString(pModule, initialisefunction.c_str());
    pFuncE = PyObject_GetAttrString(pModule, executefunction.c_str());
    pFuncF = PyObject_GetAttrString(pModule, finalisefunction.c_str());
    
    if (pFuncT && pFuncI && pFuncE && pFuncF && PyCallable_Check(pFuncT) && PyCallable_Check(pFuncI) && PyCallable_Check(pFuncE) && PyCallable_Check(pFuncF)) {
      
      // call the template-provided SetToolChainVars method,
      // to pass base variables (m_data, m_variables, m_log) to the Tool
      // get the pointers
      intptr_t ptrs[] = {reinterpret_cast<intptr_t>(m_data),
                                 reinterpret_cast<intptr_t>(&m_variables),
                                 reinterpret_cast<intptr_t>(m_data->Log)};
      // form into a Python tuple (we need to use a tuple to pass multiple arguments)
      pArgs = PyTuple_New(3);
      for(unsigned int i=0; i<3; ++i){
          intptr_t aptr = ptrs[i];
          // try to form intptr_t into a suitable PyObject
          pValue = PyLong_FromLong(aptr);
          // if the conversion failed, cleanup and abort
          if (!pValue) {
             Py_DECREF(pArgs);
             Py_DECREF(pModule);
             fprintf(stderr, "Cannot convert argument\n");
             return 0;
          }
          // otherwise it succeeded. Add to the tuple; pValue reference stolen here:
          int err = PyTuple_SetItem(pArgs, i, pValue);
          if(err){
             Py_DECREF(pArgs);
             Py_DECREF(pModule);
             std::clog<<"Cannot insert argument "<<i<<" into Tuple! Returned: "<<err<<std::endl;
             return err;
          }
          
      }
      // call SetToolChainVars to import these items into the python environment
      pValue = PyObject_CallObject(pFuncT, pArgs);
      // delete args after running
      Py_DECREF(pArgs);
      
      // check call return for errors
      if(pValue != NULL){
        // function call succeeded, check return value
        if(!(PyLong_AsLong(pValue))){
          // bad return value
          return false;
        } // else OK!
        Py_DECREF(pValue);
      } else {
        // something went wrong with function call
        Py_DECREF(pFuncI);
        Py_DECREF(pModule);
        PyErr_Print();
        fprintf(stderr,"Python SetToolChainVars call failed\n");
        return false;
      }
      
      // ========================
      
      // Same process over again for now calling the user's Initialise function
      pArgs = PyTuple_New(0);
      
      //pArgs = Py_BuildValue("(i)",pyinit);
      
      // run python Initialise function
      pValue = PyObject_CallObject(pFuncI, pArgs);
      Py_DECREF(pArgs); //delete args after running
      
      
      if(pValue != NULL){
        //if (!(PyInt_AsLong(pValue))){ 
        if(!(PyLong_AsLong(pValue))){
          std::cout<<"Python script returned internal error in initialise "<<std::endl;
          return false;
        }
        Py_DECREF(pValue);
      }
      else { //something went wrong with function call
        Py_DECREF(pFuncI);
        Py_DECREF(pModule);
        PyErr_Print();
        fprintf(stderr,"Python Initialise call failed\n");
        return false;
      }
    }
    else {
      if (PyErr_Occurred())
        PyErr_Print();
      fprintf(stderr, "Cannot find python functions");
      Py_XDECREF(pFuncE);
      Py_XDECREF(pFuncF);
      Py_DECREF(pModule);
    }
    Py_XDECREF(pFuncI);
    
  }
  else {
    
    PyErr_Print();
    fprintf(stderr, "Failed to load python script/module");
    return false;
  }
  
  return true;
}


bool PythonScript::Execute(){
    
  //PyThreadState_Swap(pythread);

  if (pModule != NULL) {

    if (pFuncE && PyCallable_Check(pFuncE)) {
      pArgs = PyTuple_New(0);//no arguments
      
      pValue = PyObject_CallObject(pFuncE, pArgs); // call the function
      Py_DECREF(pArgs); // delete arguments
      
      if(pValue != NULL){
        //if (!(PyInt_AsLong(pValue))){
        if (!(PyLong_AsLong(pValue))){
          std::cout<<"Python script returned internal error in execute "<<std::endl;
          return false;
        }
        Py_DECREF(pValue);
      }
      else {
        Py_DECREF(pFuncE);
        Py_DECREF(pModule);
        PyErr_Print();
        fprintf(stderr,"Call to Python Execute failed\n");
        return false;
      }
    }
    else {
      if (PyErr_Occurred())
        PyErr_Print();
      fprintf(stderr, "Cannot python execute function \n");
      Py_XDECREF(pFuncE);
      Py_DECREF(pModule);
      return false;
    }
    
  }
 
  
  return true;
}


bool PythonScript::Finalise(){

  //PyThreadState_Swap(pythread);
  
  if (pModule != NULL) {
    
    if (pFuncF && PyCallable_Check(pFuncF)) {
      pArgs = PyTuple_New(0);
      
      pValue = PyObject_CallObject(pFuncF, pArgs);
      Py_DECREF(pArgs);
      
      if (pValue != NULL) {
        //if (!(PyInt_AsLong(pValue))){
        if (!(PyLong_AsLong(pValue))){
          std::cout<<"Python script returned internal error in finalise "<<std::endl;
          return false;
        }
        Py_DECREF(pValue);
      }
      else {
        Py_DECREF(pFuncF);
        Py_DECREF(pModule);
        PyErr_Print();
        fprintf(stderr,"Call to Python Finalise failed\n");
        return false;
      }
    }
    else {
      if (PyErr_Occurred())
        PyErr_Print();
      fprintf(stderr, "Cannot python finalise function \n");
      Py_XDECREF(pFuncF);
      Py_DECREF(pModule);
      return false;
    }

  }
  
  int tmpinit=0;
  m_data->CStore.Get("PythonInit",tmpinit);
  if(tmpinit==pyinit)  Py_Finalize();
  
  return true;
}

