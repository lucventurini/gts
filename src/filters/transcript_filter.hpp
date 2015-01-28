//  ********************************************************************
//  This file is part of GTS (Good Transcript Selector).
//
//  GTS is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Portculis is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with GTS.  If not, see <http://www.gnu.org/licenses/>.
//  *******************************************************************

#pragma once

#include "../gff.hpp"

namespace gts {
 
using namespace gts::gff;
    
typedef boost::error_info<struct TranscriptFilterError,string> TranscriptFilterErrorInfo;
struct TranscriptFilterException: virtual boost::exception, virtual std::exception { };

typedef boost::unordered_map<string, shared_ptr<DBAnnot> > DBAnnotIdMap;

struct Maps {
   GFFIdMap gtfMap;
   DBAnnotIdMap uniqFlnCds;
   DBAnnotIdMap uniqFlnNcCds;   
};

class TranscriptFilter {
    
public:
    
    TranscriptFilter() {}
    
    virtual ~TranscriptFilter() = 0;
    
    void filter(GFFModel& in, Maps& maps, GFFModel& out) {
        
        auto_cpu_timer timer(1, "Wall time taken: %ws\n\n");
        
        filterInternal(in, maps, out);
    }
    
    virtual string getName() = 0;
    
    virtual string getDescription() = 0;
    
    string getReport() {
        return report;
    }
    

protected:    
    
    string report;
    
    virtual void filterInternal(GFFModel& in, Maps& maps, GFFModel& out) = 0; 
};

TranscriptFilter::~TranscriptFilter() {}
        
}