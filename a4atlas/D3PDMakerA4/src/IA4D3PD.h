// Dear emacs, this is -*- c++ -*-
#ifndef D3PDMAKERREADER_IA4D3PD_H
#define D3PDMAKERREADER_IA4D3PD_H

#include <fstream>
#include <string>

#include "GaudiKernel/ToolHandle.h"

#include "D3PDMakerInterfaces/ID3PD.h"
#include "D3PDMakerInterfaces/IObjFillerTool.h"


namespace D3PD {


   class IA4D3PD : public ID3PD {

   public:
      virtual ~IA4D3PD() {}
      
      virtual StatusCode recordInfo(std::ofstream& output, const std::string& prefix, const std::string& classname, const ToolHandle<IObjFillerTool>& tool) const = 0;
   };


}

#endif
