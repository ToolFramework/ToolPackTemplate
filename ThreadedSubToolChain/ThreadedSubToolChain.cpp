#include "ThreadedSubToolChain.h"
#include "ToolChain.h"

ThreadedSubToolChain_args::ThreadedSubToolChain_args():Thread_args(){}

ThreadedSubToolChain_args::~ThreadedSubToolChain_args(){}


ThreadedSubToolChain::ThreadedSubToolChain():Tool(){}


bool ThreadedSubToolChain::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  std::string tools_conf="";
  int errorlevel=0;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
  if(!m_variables.Get("Tools_file",tools_conf)) return false;
  if(!m_variables.Get("error_level",errorlevel)) errorlevel=2;

  int threadcount=0;
  bool auto_start=true;
  if(!m_variables.Get("Threads",threadcount)) threadcount=1;
  if(!m_variables.Get("Thread_auto_start",auto_start)) auto_start=true;
  if(!m_variables.Get("force_terminate",force_terminate)) force_terminate=false;

  m_util=new Utilities();

  for(int i=0;i<threadcount;i++){
    ThreadedSubToolChain_args* tmparg=new ThreadedSubToolChain_args();   
    tmparg->busy=auto_start;
    DataModel* tmpDM=new DataModel();
    tmpDM->thread_busy_flag =&(tmparg->busy);
    m_data->thread_data_models.push_back(tmpDM);
    tmparg->tool_chain=new ToolChain(m_verbose, errorlevel, true, false, "",false, tmpDM);
    if(!tmparg->tool_chain->LoadTools(tools_conf)) return false;
    if(tmparg->tool_chain->Initialise()) return false;
    args.push_back(tmparg);
    std::stringstream tmp;
    tmp<<"T"<<i; 
    m_util->CreateThread(tmp.str(), &Thread, args.at(i));
  }
    
  
  return true;
}


bool ThreadedSubToolChain::Execute(){
  
  
  return true;
}


bool ThreadedSubToolChain::Finalise(){
  
  bool ret=true;
  
  for(int i=0;i<args.size();i++){
    if(force_terminate) args.at(i)->tool_chain->m_data->vars.Set("StopLoop",true);
    while(args.at(i)->busy) usleep(100);
    if(args.at(i)->tool_chain->Finalise()) ret=false;
    delete args.at(i)->tool_chain;
    args.at(i)->tool_chain=0;
    m_util->KillThread(args.at(i));
  }    

  args.clear();
  
  m_data->thread_data_models.clear();

  delete m_util;
  m_util=0;

  return ret;
}

void ThreadedSubToolChain::Thread(Thread_args* arg){

  ThreadedSubToolChain_args* args=reinterpret_cast<ThreadedSubToolChain_args*>(arg);

  while(args->busy){
    bool StopLoop=false;
    args->tool_chain->m_data->vars.Set("StopLoop",StopLoop);
    while(!args->tool_chain->m_data->vars.Get<bool>("StopLoop"))  args->tool_chain->Execute();
    args->busy=false;
  }
  usleep(100);


}
