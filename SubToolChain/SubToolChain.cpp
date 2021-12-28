#include "SubToolChain.h"
#include "ToolChain.h"

SubToolChain::SubToolChain():Tool(){}


bool SubToolChain::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  std::string tools_conf;
  if(!m_variables.Get("ToolsConfig",tools_conf)) return false;

  m_subtoolchain=new ToolChain();
  if(!m_subtoolchain->LoadTools(tools_conf)) return false;
  if(m_subtoolchain->Initialise()) return false;

  return true;
}


bool SubToolChain::Execute(){

  if(m_subtoolchain->Execute()) return false;

  return true;
}


bool SubToolChain::Finalise(){

  bool ret=true;
  
  if(m_subtoolchain->Finalise()) ret=false;
  delete m_subtoolchain;
  m_subtoolchain=0;

  return ret;
}
