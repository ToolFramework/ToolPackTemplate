# ThreadedSubToolChain

this Tool will create threaded SubToolchains that will execute asyncronously. It requires compilation with make twise and the addition of two variables to the DataModel

std::vector<DataModel*> thread_data_models;
bool* thread_busy_flag;

This allows the main ToolChain to transfer data into the DataModel classes of the threaded chains and also a set of busy and interupt flags to ensure no race conditions.

To use one must wait till the busy flag becomes false before writing to the Thread's data model. 

    	 while(m_data->thread_data_models.at(0)->thread_busy_flag) usleep(100);
	 m_data->thread_data_models.at(0)-><your data objects> 	= .......


Once data is either written or retreived, busy needs to be set back to true for the thread to continue to work on it. 

	 m_data->thread_data_models.at(0)->thread_busy_flag=true;   

Note: The normal stoploop variable (m_data->vars.Set("StopLoop",True)) needs to be used by the threaded toolchain to automatically change the busy flag back to false when operations are complete. The busy and stoploop flags can also be set to false and true manaually to force the thread to stop execution.