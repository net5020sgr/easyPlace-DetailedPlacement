#include <fstream>
#include <cstdlib>
#include <iostream>
#include "interface.h"


bool OpenroadInterface::runSTA(std::string staDEFPath)
{
    // 檢查 OpenROAD 執行檔是否存在
    if (system((DEFAULTopenroadPath + " -version > /dev/null 2>&1").c_str()) != 0) {
        std::cerr << "[ERROR] OpenROAD binary not found or not executable.\n";
        return false;
    }

    // 生成 eval_sta.tcl
    std::ofstream tclFile(DEFAULTtclPath);
    if (!tclFile.is_open()) {
        std::cerr << "[ERROR] Cannot open TCL path for writing: " << DEFAULTtclPath << "\n";
        return false;
    }

    tclFile << R"(
foreach libFile [glob "./ASAP7/LIB/*nldm*.lib"] {
    puts "lib: $libFile"
    read_liberty $libFile
}
puts "reading lef.."
read_lef ./ASAP7/techlef/asap7_tech_1x_201209.lef
foreach lef [glob "./ASAP7/LEF/*.lef"] {
    read_lef $lef
}
puts "reading def.."
read_def )" << staDEFPath << R"(

read_sdc )" << DEFAULTinitialSDCPath << R"(
source ./ASAP7/setRC.tcl

estimate_parasitics -placement

report_tns
report_wns
report_checks -slack_max 0 -endpoint_count 10000 
exit )";

    tclFile.close();

    // 組合系統命令
    std::stringstream cmd;
    cmd << DEFAULTopenroadPath << " " << DEFAULTtclPath << " > " << DEFAULTstaReportPath;

    std::cout << "[INFO] Running OpenROAD STA...\n";
    int ret = system(cmd.str().c_str());

    if (ret != 0) {
        std::cerr << "[ERROR] OpenROAD execution failed with code: " << ret << "\n";
        return false;
    }

    std::cout << "[INFO] STA completed. Report generated: " << DEFAULTstaReportPath << "\n";
    return true;
}

void OpenroadInterface::analyzeSTAReport()
{
    // If staReportPath is empty, use the default path
    std::string staReportPath = DEFAULTstaReportPath;

    std::ifstream fin(staReportPath);
    if (!fin.is_open()) {
        std::cerr << "Failed to open " << staReportPath << "\n";
        return;
    }

    //reset p2p weights
    db->resetP2Pweights();

    std::string line;
    std::regex startpoint_regex("^Startpoint: (\\S+)");
    std::regex endpoint_regex("^Endpoint: (\\S+)");
    std::regex pin_regex("^\\s*[-+]?\\d+\\.\\d+\\s+\\d+\\.\\d+\\s+[v\\^]\\s+(\\S+)/(\\S+)");
    std::regex slack_regex("^\\s*(-?\\d+\\.\\d+)\\s+slack");

    std::string start_node, end_node;
    float slack = 0.0f;
    float WNS = DEFAULTWNS;//
    float TNS =0.0f;
    std::vector<std::pair<std::string, std::string>> pathPins;

    int NegativeSlackCount = 0;


    while (std::getline(fin, line)) {
        if (line.find("wns")!= std::string::npos) {
            std::istringstream iss(line);
            std::string wns_str;
            iss >> wns_str >> WNS; // Extract WNS value
            continue;
        }
        if (line.find("tns")!= std::string::npos) {
            std::istringstream iss(line);
            std::string tns_str;
            iss >> tns_str >> TNS; // Extract TNS value
            db->setTNS(TNS);
            continue;
        }
        std::smatch match;
        if (std::regex_search(line, match, startpoint_regex)) {
            pathPins.clear();  // reset
            start_node = match[1].str();
        }
        else if (std::regex_search(line, match, endpoint_regex)) {
            end_node = match[1].str();
        }
        else if (std::regex_search(line, match, pin_regex)) {
            std::string node = match[1].str();
            std::string pin = match[2].str();
            pathPins.emplace_back(node, pin);
        }
        else if (std::regex_search(line, match, slack_regex)) {
            slack = std::stof(match[1].str());
            if (slack < 0) {
                for (size_t i = 1; i < pathPins.size(); ++i) {
            
                        auto np1= pathPins[i - 1];
                        auto np2 = pathPins[i];
                        // cout << "node1: " << np1.first << " pin1: " << np1.second << " node2: " << np2.first << " pin2: " << np2.second << " slack: " << slack << " WNS: " << WNS << endl;
        
                        auto [module1, pin1] = db->NodePinMap[std::make_pair(np1.first, np1.second)];
                        auto [module2, pin2] = db->NodePinMap[std::make_pair(np2.first, np2.second)];
                        if (module1 == module2){
                            // cout << "module1 == module2" << endl;
                            continue;
                        }
                        assert(module1 != nullptr);
                        assert(module2 != nullptr);
                        assert(pin1 != nullptr);
                        assert(pin2 != nullptr);
                        //now two pin are both outputpin, need to find the input pin of module2
                        Pin *inputPin = nullptr;
                        for (Pin *p : module2->modulePins) {
                            
                            cout <<p->name << endl;
                            for (Pin *p2:pin1->net->netPins){
                                if (p2 == p ){
                                    inputPin = p2;
                                    break;
                                }
                            }
                            
                        }
                        if (inputPin == nullptr) {
                            cout << "Input pin not found for module: " << np2.first << endl;
                            exit(1);
                        }
                        // cout << "node1: " << np1.first << " pin1: " << np1.second << " node2: " << np2.first << " pin2: " << inputPin->name << " slack: " << slack << " WNS: " << WNS << endl;
                        
                        db->updateP2Pweight(np1.first, np1.second, np2.first, inputPin->name, slack, WNS);
                        NegativeSlackCount++;

                }
            }
        }
    }
    std::cout << "Total negative slack count: " << NegativeSlackCount << std::endl;
    std::cout << "WNS: " << WNS << std::endl;
    std::cout << "TNS: " << TNS << std::endl;
    std::cout << "P2P weights updated based on STA report." << std::endl;
    // Close the file
    fin.close();
}

void OpenroadInterface::outputSTADEF(std::string outputDEFPath)
{
    std::ifstream in(DEFAULTinitialDEFPath);
    if (!in.is_open()) {
        std::cerr << "Failed to open initial DEF file: " << DEFAULTinitialDEFPath << "\n";
        return;
    }

    std::ofstream out(outputDEFPath);
    if (!out.is_open()) {
        std::cerr << "Failed to open output DEF file: " << outputDEFPath << "\n";
        return;
    }
    string line;
    bool inComponent = false;
    int componentCount = 0;
    stringstream ss;


    while (std::getline(in, line)) {
        // cout << line << endl;
        // Skip the first 4 lines
        if (line.find("END COMPONENTS") != string::npos && inComponent == true) {
            inComponent = false;
            out << line << endl;
            continue;
        }
        else if (inComponent) {
            //- g223780 OAI21xp33_ASAP7_75t_SL + PLACED ( 36324 15858 ) N
            //;
            if (line.find("PLACED") != string::npos) {
                componentCount++;
                ss.clear();
                ss.str(line);
                std::string tmp, nodeName , nodetype;
                if (line.find("-") != string::npos)ss >>tmp >> nodeName >> nodetype; // Get the node name
                else continue;

               // Remove all '\' from nodeName
            for (int i = nodeName.size() - 1; i >= 0; --i) {
                if (nodeName[i] == '\\') {
                    nodeName.erase(i, 1);
                }
            }

            // Lookup
            auto module = db->getModuleFromName(nodeName);

            // Re-insert '\' before '[' or ']'
            for (int i = nodeName.size() - 1; i >= 0; --i) {
                if (nodeName[i] == '[' || nodeName[i] == ']') {
                    nodeName.insert(i, 1, '\\');
                }
            }

                // cout << "nodeName: " << nodeName << endl;
                // cout << "nodetype: " << nodetype << endl;
                // cout << "module: " << module << endl;

                if (module == nullptr) {
                    cout << "Module not found in database: " << nodeName << endl;
                    exit(1);
                }

                out <<"- "<< nodeName << " " << nodetype << " + PLACED ( "
                        << module->getLocation().x << " " << module->getLocation().y << " ) N ;\n";
               
            }
            continue;
        }
        else if(line.find("COMPONENTS")!=string::npos && inComponent == false){
            inComponent = true;
            out << line << endl;
            continue;
        }
        out << line << endl; // Write other lines as they are
        
    }
    in.close();
    out.close();
}