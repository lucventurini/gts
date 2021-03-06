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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::exception;
using std::vector;

#include <boost/unordered_set.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/timer/timer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered/unordered_map.hpp>
using boost::timer::auto_cpu_timer;
using boost::shared_ptr;
using boost::unordered_set;
namespace po = boost::program_options;
namespace bfs = boost::filesystem;

#include "gff.hpp"
using gts::gff::GFF;
using gts::gff::GFFPtr;
using gts::gff::GFFListPtr;
using gts::gff::GFF3;
using gts::gff::GTF;
using gts::gff::GFFErrorInfo;
using gts::gff::GFFException;
using gts::gff::GffType;
using gts::gff::GFFModel;
using gts::gff::GFFModelPtr;

typedef std::vector<shared_ptr<GFF> > GFFList;

string helpHeader() {
    return string("\nGFF Filter Help.\n\n") +
                  "The gff_filter tool is used to filter out listed entries from the provided GFF file.  The tool will automatically try to determine parent and child relationships between the entries and filter those as well.\n\n" +
                  "Usage: gff_filter [options] -i <gff file> -l <list file> -o <gff file>\n\n" +
                 "\nAvailable options";
}


void loadEntries(string& path, unordered_set<string>& transcriptSet) {
    
    auto_cpu_timer timer(1, " = Wall time taken: %ws\n\n");
    
    cout << " - Loading entry set: " << path << endl;

    std::ifstream file(path.c_str());
    std::string line; 
    while (std::getline(file, line)) {            
        boost::trim(line);
        if (!line.empty()) {
            transcriptSet.insert(line);
        }
    }
    file.close();
        
    cout << " - Loaded " << transcriptSet.size() << " entries from file." << endl;
}

void filter(GFFModel& input, unordered_set<string>& transcriptSet, GFFModel& output) {
    
    auto_cpu_timer timer(1, " = Wall time taken: %ws\n\n");
    
    // Index genomic GFFs by Id
    BOOST_FOREACH(GFFPtr gene, *input.getGeneList()) {

        GFFList goodTranscripts;
        
        BOOST_FOREACH(GFFPtr transcript, *gene->GetChildList()) {
            
            if (transcriptSet.count(transcript->GetId()) > 0) {
                // Do nothing
            }
            else {
                goodTranscripts.push_back(transcript);
            }
        }
        
        // We only want genes with 1 or more good transcript
        if (goodTranscripts.size() >= 1) {                

            // Copy gene without child info
            GFFPtr newGene = make_shared<GFF>(*gene);

            BOOST_FOREACH(GFFPtr goodTranscript, goodTranscripts) {
                newGene->addChild(goodTranscript);
            }

            output.addGene(newGene); 
        }
    }
    cout << " - Keeping " << output.getNbGenes() << " out of " << input.getNbGenes() << " genes" << endl;
    cout << " - Keeping " << output.getTotalNbTranscripts() << " out of " << input.getTotalNbTranscripts() << " transcripts" << endl;
}

void typeFilter(GFFList& input, string typeInc, string typeExc, GFFList& output) {
   
    bool doInc = !typeInc.empty();
    bool doExc = !typeExc.empty();
    
    GffType inc = doInc ? gts::gff::gffTypeFromString(typeInc) : gts::gff::OTHER;
    GffType exc = doExc ? gts::gff::gffTypeFromString(typeExc) : gts::gff::OTHER;
    
    BOOST_FOREACH(GFFPtr gff, input) {
        
        GffType type = gff->GetType();
        
        if (doInc && doExc) {
            // Exclude wins!
            if (type == inc && type != exc) {
                output.push_back(gff);
            }
        }
        else if (doInc && !doExc) {
            if (type == inc) {
                output.push_back(gff);
            }
        }
        else if (!doInc && doExc) {
            if (type != exc) {
                output.push_back(gff);
            }
        }                
    }
    
    cout << " - Keeping " << output.size() << " out of " << input.size() << " GFF records" << endl;
}


/**
 * Start point for portculis.
 */
int main(int argc, char *argv[]) {
    
    try {
        // Portculis args
        string inputFile;
        string listFile;
        string typeInc;
        string typeExc;
        string outputFile;
        
        bool version;
        bool help;

        // Declare the supported options.
        po::options_description generic_options(helpHeader());
        generic_options.add_options()
                ("input,i", po::value<string>(&inputFile), 
                    "The input GFF file to filter")
                ("list,l", po::value<string>(&listFile),
                    "The list of GFF ids to filter")
                ("type_inc,ti", po::value<string>(&typeInc),
                    "Output will include only records with this type")
                ("type_exc,te", po::value<string>(&typeExc),
                    "Output will exclude records with this type")
                ("output,o", po::value<string>(&outputFile),
                    "The tab separated output file which will contain IDs")
                ("version", po::bool_switch(&version)->default_value(false), "Print version string")
                ("help", po::bool_switch(&help)->default_value(false), "Produce help message")
                ;

        // Combine non-positional options
        po::options_description cmdline_options;
        cmdline_options.add(generic_options);

        // Parse command line
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(cmdline_options).run(), vm);
        po::notify(vm);

        // Output help information the exit if requested
        if (argc == 1 || argc == 2 && help) {
            cout << generic_options << endl;
            return 1;
        }

        // Output version information then exit if requested
        if (version) {
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "GTS"
#endif

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "0.1.0"
#endif
            cout << PACKAGE_NAME << " V" << PACKAGE_VERSION << endl;
            return 0;
        }
        
        auto_cpu_timer timer(1, "\nTotal wall time taken: %ws\n\n");
        
        if (!boost::filesystem::exists(inputFile)) {
            BOOST_THROW_EXCEPTION(GFFException() << GFFErrorInfo(string(
                    "Could not find input file at: ") + inputFile));
        }
        
        cout << "Loading gene model" << endl;
        GFFModelPtr geneModel = GFFModel::load(inputFile);            

        GFFModelPtr in1 = geneModel;
        GFFModelPtr in2;
        GFFModelPtr in3;
        GFFModelPtr in4;
            
        if (!listFile.empty()) {
            
            cout << "Loading transcript IDs (mRNAs) to filter" << endl;
            unordered_set<string> transcriptsToExclude;
            loadEntries(listFile, transcriptsToExclude);

            cout << "Filtering listed entries from GFF" << endl;
            in2 = make_shared<GFFModel>();
            GFFModelPtr gmOut = make_shared<GFFModel>();
            filter(*in1, transcriptsToExclude, *in2);
        }
        else {
            in2 = geneModel;
        }
        
        /*if (!typeInc.empty() || !typeExc.empty()) {
            cout << "Filtering by type" << endl;
            in4 = make_shared<GFFList>();
            typeFilter(*in3, typeInc, typeExc, *in4);            
        }
        else {
            in4 = in3;
        }*/
        
        cout << "Writing filtered GFF to disk" << endl;        
        in2->save(outputFile);
        
        cout << "Completed" << endl;
                
    } catch (boost::exception &e) { 
        std::cerr << boost::diagnostic_information(e); 
        return 4;
    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 5;
    } catch (const char* msg) {
        cerr << "Error: " << msg << endl;
        return 6;
    } catch (...) {
        cerr << "Error: Exception of unknown type!" << endl;
        return 7;
    }

    return 0;
}


