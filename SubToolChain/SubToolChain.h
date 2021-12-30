#ifndef SubToolChain_H
#define SubToolChain_H

#include <string>
#include <iostream>

#include "Tool.h"

class ToolChain;

/**
 * \class SubToolChain
 *
 * This Tool chreats a Sub ToolCahin that runs inline with the main TtoolChain.
*
* $Author: B.Richards $
* $Date: 2021/12/30 14:22:00 $
*/

class SubToolChain: public Tool {


 public:

  SubToolChain(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resorces. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Executre function used to perform Tool perpose. 
  bool Finalise(); ///< Finalise funciton used to clean up resorces.


 private:

  ToolChain* m_subtoolchain;



};


#endif
