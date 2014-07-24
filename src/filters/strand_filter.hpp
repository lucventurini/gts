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

#include <sstream>

#include "transcript_filter.hpp"

using std::stringstream;        

namespace gts {

typedef boost::unordered_map<string, uint32_t> IdCounter;
    
class StrandFilter : public TranscriptFilter {
      
public:
    
    StrandFilter() : TranscriptFilter() {}
    
    ~StrandFilter() {}
    
    string getName() {
        return string("Strand Filter");
    }
    
    string getDescription() {
        return string("Filters out genomic transcripts which have an inconsistent strand when compared to the GTF file");
    }
    
    
protected:    
    
    void filterInternal(GFFList& in, Maps& maps, GFFList& out) {
        
        BOOST_FOREACH(shared_ptr<GFF> gff, in) {
            
            if (gff->GetStrand() == maps.gtfMap[gff->GetRootId()]->GetStrand()) {
                out.push_back(gff);
            }
        }
        
        stringstream ss;
        
        ss << " - # Transcripts with consistent strand: " << out.size() << endl;
        
        report = ss.str();
    }
    
};
}
       
