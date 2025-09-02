#include "qplace.h"
#include "parser.h"
#include "arghandler.h"
#include "eplace.h"
#include "opt.hpp"
#include "nesterov.hpp"
#include "legalizer.h"
#include "detailed.h"
#include "plot.h"
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
    BookshelfParser parser;
    string inital_def_path ="";
    string inital_sdc_path ="";
    PlaceDB *placedb = new PlaceDB();
    gArg.Init(argc, argv);

    if (argc < 2)
    {
        return 0;
    }

    string auxPath;
    if (gArg.CheckExist("aux"))
    {
        gArg.GetString("aux", &auxPath);
        cout << "Use Bookshelf placement format" << endl;
        string path = auxPath.substr(0, auxPath.find_last_of('/') + 1);
        string design = auxPath.substr(auxPath.find_last_of('/') + 1);
        design = design.substr(0, design.find_last_of('.'));
        gArg.Override("path", path.c_str());
        gArg.Override("design", design.c_str());
        gArg.Override("plot", "GP"); // default plot name
        parser.ReadFile(auxPath, *placedb);
        inital_def_path = auxPath.substr(0, auxPath.rfind(".")) + ".def";
        inital_sdc_path = auxPath.substr(0, auxPath.rfind(".")) + ".sdc";
    }

   
    placedb->showDBInfo();
    string plPath;
    if (gArg.GetString("loadpl", &plPath))
    {
        //! modules will be moved to center in QP, so if QP is not skipped, loading module locations from an existing pl file is meaningless
        parser.ReadPLFile(plPath, *placedb, false);
    }

    QPPlacer *qpplacer = new QPPlacer(placedb);
    if (!gArg.CheckExist("noQP"))
    {
        qpplacer->quadraticPlacement();
    }

    if (gArg.CheckExist("addNoise"))
    {
        placedb->addNoise(); // the noise range is [-avgbinStep,avgbinStep]
    }

    double mGPTime;
    double mLGTime;
    double FILLERONLYtime;
    double cGPTime;
    bool isTiming = false;
    if (gArg.CheckExist("timing")|| gArg.CheckExist("TDP"))
    {
        isTiming = true;
        cout << "Timing optimization enabled!\n";
    }
    else
    {
        cout << "Timing optimization disabled!\n";
    }

    EPlacer_2D *eplacer = new EPlacer_2D(placedb);
    OpenroadInterface* openroadInterface = new OpenroadInterface(placedb);
    string initialDEFPath = inital_def_path ;//"./testcase/aes_cipher_top/aes_cipher_top.def";
    string initialSDCPath = inital_sdc_path ;
    string openroadPath = "/usr/bin/openroad";
    string tclPath = "./eplace_sta.tcl";
    string staReportPath = "./eplace_sta_report.rpt";
    openroadInterface->intializePaths(initialDEFPath, initialSDCPath, openroadPath, tclPath, staReportPath);

    float targetDensity;
    if (!gArg.GetFloat("targetDensity", &targetDensity))
    {
        targetDensity = 1.0;
    }

    eplacer->setTargetDensity(targetDensity);
    eplacer->initialization();

    if (isTiming) {
        cout << "isTiming" << endl;
    }
    else {
        cout << "not isTiming" << endl;
    }
    FirstOrderOptimizer<VECTOR_3D> *opt = new EplaceNesterovOpt<VECTOR_3D>(eplacer, isTiming, openroadInterface);

    if (!gArg.CheckExist("nomGP"))
    {
        AbacusLegalizer *legalizer = new AbacusLegalizer(placedb);
        legalizer->legalization();

        // DetailedPlacer *detailedPlacer = new DetailedPlacer(placedb);
        // detailedPlacer->detailedPlacement();

        cout << "mGP started!\n";

        time_start(&mGPTime);
        opt->opt();
        time_end(&mGPTime);

        cout << "mGP finished!\n";
        cout << "Final HPWL: " << int(placedb->calcHPWL()) << endl;
        cout << "mGP time: " << mGPTime << endl;
        PLOTTING::plotCurrentPlacement("mGP result", placedb);
    }

    ///////////////////////////////////////////////////
    // legalization and detailed placement
    ///////////////////////////////////////////////////

    if (placedb->dbMacroCount > 0 && !gArg.CheckExist("nomLG"))
    {
        SAMacroLegalizer *macroLegalizer = new SAMacroLegalizer(placedb);
        macroLegalizer->setTargetDensity(targetDensity);
        cout << "Start mLG, total macro count: " << placedb->dbMacroCount << endl;
        time_start(&mLGTime);
        macroLegalizer->legalization();
        time_end(&mLGTime);

        PLOTTING::plotCurrentPlacement("mLG result", placedb);
        cout << "mLG finished. HPWL after mLG: " << int(placedb->calcHPWL()) << endl;
        cout << "mLG time: " << mLGTime << endl;
        // exit(0);

        if (!gArg.CheckExist("nocGP"))
        {
            eplacer->switch2FillerOnly();
            cout << "filler placement started!\n";

            time_start(&FILLERONLYtime);
            opt->opt();
            time_end(&FILLERONLYtime);

            cout << "filler placement finished!\n";
            cout << "FILLERONLY time: " << mGPTime << endl;
            PLOTTING::plotCurrentPlacement("FILLERONLY result", placedb);

            eplacer->switch2cGP();
            cout << "cGP started!\n";

            time_start(&cGPTime);
            opt->opt();
            time_end(&cGPTime);

            cout << "cGP finished!\n";
            cout << "cGP Final HPWL: " << int(placedb->calcHPWL()) << endl;
            cout << "cGP time: " << mGPTime << endl;
            PLOTTING::plotCurrentPlacement("cGP result", placedb);
        }
    }

    placedb->outputBookShelf("eGP",true); // output, files will be used for legalizers such as ntuplace3

    if (!gArg.CheckExist("noLegal"))
    {
        string legalizerPath;
        if (gArg.GetString("legalizerPath", &legalizerPath))
        {
            string outputAUXPath;
            string outputPLPath;
            string outputPath;
            string benchmarkName;

            gArg.GetString("outputAUX", &outputAUXPath);
            gArg.GetString("outputPL", &outputPLPath);
            gArg.GetString("outputPath", &outputPath);
            gArg.GetString("benchmarkName", &benchmarkName);

            string cmd = legalizerPath + "/ntuplace3" + " -aux " + outputAUXPath + " -loadpl " + outputPLPath + " -noglobal" + " -out " + outputPath + benchmarkName + "-ntu" + " > " + outputPath + "ntuplace3-log.txt";

            cout << RED << "Running legalizer and detailed placer: " << cmd << RESET << endl;
            system(cmd.c_str());
        }
        else if (gArg.GetString("internalLegal", &legalizerPath))
        {
            // for std cell design only, since we don't have macro legalizer for now
            cout << "Calling internal legalizer: " << endl;
            cout << "Global HPWL: " << int(placedb->calcHPWL()) << endl;

            AbacusLegalizer *legalizer = new AbacusLegalizer(placedb);
            legalizer->legalization();

            cout << "Legal HPWL: " << int(placedb->calcHPWL()) << endl;
            PLOTTING::plotCurrentPlacement("Cell legalized result", placedb);

            placedb->outputBookShelf("eLG",true);
            placedb->outputDEF("eLP",inital_def_path);

            if (isTiming) {
                cout << "isTiming" << endl;
                string staDEFPath = "./eLP_result.def";
        
    
                openroadInterface->outputSTADEF(staDEFPath);
                openroadInterface->runSTA(staDEFPath);
                openroadInterface->analyzeSTAReport();
            }
            else {
                cout << "not isTiming" << endl;
            }
        }
        else
        {
            cout<<"Legalization not done!!!\n";
            exit(0);
        }
    }

    if (gArg.CheckExist("internalDP"))// currently only works after internal legalization
    {
        cout << "Calling internal detailed placement: " << endl;
        cout << "HPWL before detailed placement: " << int(placedb->calcHPWL()) << endl;

        DetailedPlacer *detailedPlacer = new DetailedPlacer(placedb);
        detailedPlacer->detailedPlacement();

        cout << "HPWL after detailed placement: " << int(placedb->calcHPWL()) << endl;
        PLOTTING::plotCurrentPlacement("Detailed placement result", placedb);

        placedb->outputBookShelf("eDP",false);
        placedb->outputDEF("eDP",inital_def_path);
        if (isTiming) {
            cout << "isTiming" << endl;
            string staDEFPath = "./eDP_result.def";
            openroadInterface->outputSTADEF(staDEFPath);
            openroadInterface->runSTA(staDEFPath);
            openroadInterface->analyzeSTAReport();
        }
        else {
            cout << "not isTiming" << endl;
        }
    }
}