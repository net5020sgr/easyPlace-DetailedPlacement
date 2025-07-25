#pragma once
#include <string>
#include <vector>
#include <utility>
#include "global.h"
#include "objects.h"
#include "placedb.h"

class OpenroadInterface {
public:
    OpenroadInterface(PlaceDB *_db)
    {
        db = _db;
    }
    void intializePaths(std::string initialDEFPath, std::string openroadPath, std::string tclPath, std::string staReportPath)
    {
                // 檢查 OpenROAD 執行檔是否存在
        if (system((DEFAULTopenroadPath + " -version > /dev/null 2>&1").c_str()) != 0) {
            std::cerr << "[ERROR] OpenROAD binary not found or not executable.\n";
            return false;
        }
        DEFAULTinitialDEFPath = initialDEFPath;
        DEFAULTopenroadPath = openroadPath;
        DEFAULTtclPath = tclPath;
        DEFAULTstaReportPath = staReportPath;
        DEFAULTTNS = 100000.0f;
        DEFAULTWNS = 300.0f;
    }

    bool runSTA (std::string staDEFPath);
    void analyzeSTAReport();
    void outputSTADEF(std::string outputDEFPath);

private:
    PlaceDB *db;
    std::string DEFAULTinitialDEFPath; // 路徑用於存儲eplace glbal placement DEF文件
    std::string DEFAULTstaReportPath; // 路徑用於存儲報告
    std::string DEFAULTtclPath; // 用於存儲TCL腳本的路徑
    std::string DEFAULTopenroadPath; 

    float DEFAULTTNS, DEFAULTWNS;
};
