#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>

using namespace std;

#define FILENAME "./source.txt"
#define OUTPUTPASS1_1NAME "./sic_pass1_SourceProgram_onlyLoc.txt"
#define OUTPUTPASS1_2NAME "./sic_pass1_SymbolTable.txt"
#define OUTPUTPASS2_1NAME "./sic_pass2_sourceProgram.txt"
#define OUTPUTPASS2_2NAME "./sic_pass2_finalObjcetProgram.txt"

/**
 * opTable
 */
const vector<string> opTable = {"ADD", "ADDF", "ADDR", "AND", "CLEAR", "COMP", "COMPF", "COMPR", "DIV", "DIVF", "DIVR",
                        "FIX", "FLOAT", "HIO", "J", "JEQ", "JGT", "JLT", "JSUB", "LDA", "LDB", "LDCH", "LDF", "LDL", "LDS",
                        "LDT", "LDX", "LPS", "MUL", "MULF", "MULR", "NORM", "OR", "RD", "RMO", "RSUB", "SHIFTL", "SHIFTR",
                        "SIO", "SSK", "STA", "STB", "STCH", "STF", "STI", "STL", "STS", "STSW", "STT", "STX", "SUB", "SUBF",
                        "SUBR", "SVC", "TD", "TIO", "TIX", "TIXR", "WD"};
/**
 * opCode
 */
const vector<string> opCode = {"18", "58", "90", "40", "B4", "28", "88", "A0", "24", "64", "9C", "C4", "C0", "F4", "3C",
                     "30", "34", "38", "48", "00", "68", "50", "70", "08", "6C", "74", "04", "E0", "20", "60", "98", "C8",
                     "44", "D8", "AC", "4C", "A4", "A8", "F0", "EC", "0C", "78", "54", "80", "D4", "14", "7C", "E8", "84",
                     "10", "1C", "5C", "94", "B0", "E0", "F8", "2C", "B8", "DC"};

/**
 * Source Statement 資料結構(ex:COPY START 1000)
 */
typedef struct StatementInfo{
    string first;
    string second;
    string third;
    string opcode;
} StatementInfo;

/**
 * SymbolTable 資料結構：包含 label名稱及 address位置
 */
typedef struct SymbolTablePair{
    string label;
    string address;
} SymbolTablePair;

/**
 * 儲存 Source Statement 資訊
 */
vector<StatementInfo> statementInfos;

/**
 * 儲存 location 資訊
 */
vector<string> location;

/**
 * 儲存 Symbol Table 資訊
 */
vector<SymbolTablePair> symbolTable;

/**
 * 儲存 Object code資訊
 */
vector<string> objectCodes;

/**
 * 儲存 END 的位置資訊，用於產生 final object program使用
 * 因為不呈現在source program，所以存放在location
 */
string lastAddress;

void buildSymbolTableAndLocation();
void buildObjectCodes();
void showAndOutputResult_Pass2();
void showAndOutputResult_Pass1();

template<typename T>
string int_to_hex(T v);
string string_format(const string& fmt, ...);

int getNextLen(const StatementInfo &si);

void mergeObjectCodeByScanSymbolTable(const StatementInfo &si, stringstream &objectCode);

void mergeObjectCodeBySecond(const StatementInfo &si, stringstream &objectCode);

/**
 * 開檔並構建 StatementInfo
 */
void openFileAndBuildStatementInfo() {
    ifstream file;

    // 開檔
    file.open(FILENAME, ios::in);

    if (!file) {
        cout << "Can't open the file" << endl;
        system("pause");
    }

    cout << "File is opened" << endl;

    // 將檔案資料寫入Vector
    string fs;
    while (getline(file, fs)) {

        istringstream in(fs);
        vector<char> v;
        vector<string> ss;
        // split by blank
        string s;
        while (in >> s) {
            ss.push_back(s);
        }
        StatementInfo info = StatementInfo();
        if (ss.size() == 3){
            info.first = ss[0];
            info.second = ss[1];
            info.third = ss[2];
        }
        if (ss.size() == 2){
            info.first = "";
            info.second = ss[0];
            info.third = ss[1];
        }

        if (ss.size() == 1){
            info.first = "";
            info.second = ss[0];
            info.third = "";
        }

        // scan opCode
        for (int j = 0; j < opTable.size(); ++j) {
            if (info.second == opTable[j]){
                info.opcode = opCode[j];
                break;
            }
        }

        statementInfos.push_back(info);
    }

    // 關檔
    file.close();
    cout << "File is closed" << endl;
}

/**
 * 開Output檔並回傳 ofstream
 * @return
 */
ofstream openOutputStream(const string& f){
    // 匯出檔案
    ofstream ofs;
    ofs.open(f);
    if (!ofs.is_open()) {
        cout << "Can't open the file" << endl;
        system("pause");
    }

    return ofs;
}

int main() {
    openFileAndBuildStatementInfo();
    buildSymbolTableAndLocation();
    showAndOutputResult_Pass1();
    buildObjectCodes();
    showAndOutputResult_Pass2();
    return 0;
}

/**
 * 構建 SymbolTable和Location
 */
void buildSymbolTableAndLocation() {
    int nextLen = 0;
    int startLen = 0;
    int decLoc = 0;
    string hexLoc;

    for (int i = 0; i < statementInfos.size(); ++i) {

        StatementInfo &si = statementInfos[i];

        //計算位置
        if (i == 0){
            startLen = stoi(si.third);
            decLoc = (int)strtol(to_string(startLen).data(), nullptr, 16);

            //location初始值設定
            location.push_back(to_string(startLen));
            continue;
        }

        if (i == 1){
            hexLoc = to_string(startLen);
        }
        else {
            decLoc += (int)strtol(to_string(nextLen).data(), nullptr, 16);
            hexLoc = int_to_hex(decLoc);

            //end無位置
            if (i == statementInfos.size() - 1) {
                lastAddress = hexLoc;
                hexLoc = "    ";
            }
        }
        location.push_back(hexLoc);

        nextLen = getNextLen(si);

        //建立SYM_TAB
        if (!si.first.empty()){
            SymbolTablePair p = SymbolTablePair();
            p.label = si.first;
            p.address = hexLoc;
            symbolTable.push_back(p);
        }
    }
}

/**
 * 計算下回合長度
 * @param si
 * @return
 */
int getNextLen(const StatementInfo &si) {
    if (si.second == "BYTE") {//byte分為C和X
        if (si.third.find('C') != string::npos) {
            unsigned long idx = si.third.find('\'', 0);
            return si.third.length() - idx - 2;
        }

        //當X'F1'時長度1
        return 1;
    }

    //數字*3
    if (si.second == "RESW") {
        return stoi(si.third) * 3;
    }

    if (si.second == "RESB") {
        return stoi(int_to_hex(stoi(si.third)));
    }

    return  3;
}

/**
 * 構建 object code
 */
void buildObjectCodes() {
    for (auto & si : statementInfos) {

        if (si.second == "END"){
            objectCodes.emplace_back("");
            break;
        }

        stringstream objectCode;
        objectCode << si.opcode;
        mergeObjectCodeByScanSymbolTable(si, objectCode);
        mergeObjectCodeBySecond(si, objectCode);

        objectCodes.push_back(objectCode.str());
    }
}

/**
 * 走訪 symbolTable 並 合併 objectCode
 * @param si
 * @param objectCode
 */
void mergeObjectCodeByScanSymbolTable(const StatementInfo &si, stringstream &objectCode) {
    for (auto & symbol : symbolTable) {
        //有X ,+8000h
        if (si.third.find(",X") != string::npos){
            string labelOfX = si.third.substr(0, si.third.length() - 2 );
            if (symbol.label == labelOfX){
                //先16->10(8000也16->10)再相加，最後再轉16進位
                int res_dec = (int)strtol(symbol.address.data(), nullptr, 16) + (int)strtol("8000", nullptr, 16);
                objectCode << int_to_hex(res_dec);
                break;
            }
        } else if(symbol.label == si.third){
            //小於6位元補0
            int length = objectCode.str().length() + symbol.address.length();
            if (length != 6) {
                for (int k = 0; k < 6 - length; ++k) {
                    objectCode << "0";
                }
                objectCode << symbol.address;
            } else{
                objectCode << symbol.address;
            }
            break;
        }
    }
}

/**
 * 根據 second指令 做 objectCode 最後合併
 * @param si
 * @param objectCode
 */
void mergeObjectCodeBySecond(const StatementInfo &si, stringstream &objectCode) {
    if (si.second == "BYTE"){
        unsigned long idx = si.third.find('\'');
        string thirdSplit = si.third.substr(idx +1, si.third.length() - idx - 2 );
        //處理BYTE：分為X和C
        for (char c : thirdSplit) {
           if (si.third.find('C') != string::npos){
               //C
               objectCode << int_to_hex((int)c).substr(2, 2);
           } else{
               //X
               objectCode << c;
           }
        }
    } else if(si.second == "WORD"){
        //直接輸出並補足6位元,先不考慮超過6位元的處理
        objectCode << setw(6) << setfill('0') << int_to_hex(stoi(si.third));
    } else if(si.second == "RSUB"){
        //RSUB=>opCode後補滿0六位
        objectCode << "0000";
    }
}

/**
 * 輸出Pass1檔案：symbolTable、location
 */
void showAndOutputResult_Pass1() {

    ofstream ofs1 = openOutputStream(OUTPUTPASS1_1NAME);

    const string &title = string_format("%s\t%-6s\t%-6s\t\n",
                                         "Loc"," ","Source Statement");


    cout << "================PASS1_Source Program===============" << endl;
    cout << title;
    ofs1 << title;

    for (int i = 0; i < statementInfos.size(); ++i) {
        const string &rs = string_format("%s\t%-6s\t%-6s\t%-10s\t\r\n",
                                         location[i].data(),
                                         statementInfos[i].first.data(),
                                         statementInfos[i].second.data(),
                                         statementInfos[i].third.data());
        cout << rs;
        ofs1 << rs;
    }

    ofstream ofs2 = openOutputStream(OUTPUTPASS1_2NAME);
    const string &title2 = string_format("%s\t%-6s\t%-6s\t\n",
                                         "LabelName"," ","Address");
    cout << "================PASS1_Symbol Table===============" << endl;
    cout << title2;
    ofs2 << title2;
    for (auto & s : symbolTable) {
        const string &st = string_format("%-16s\t%-6s\t\r\n",
                                         s.label.data(),
                                         s.address.data());
        cout << st;
        ofs2 << st;
    }
}

/**
 * 輸出Pass2檔案：final sourceProgram、final objectProgram
 */
void showAndOutputResult_Pass2() {
    ofstream ofs = openOutputStream(OUTPUTPASS2_1NAME);

    const string &title = string_format("%s\t%-6s\t%-6s\t%-3s\t%s\t\r\n",
                                        "Loc", " ", "Source Statement", " ", "Object code");

    cout << "================PASS2_Source Program===============" << endl;
    cout << title;
    ofs << title;

    for (int i = 0; i < statementInfos.size(); ++i) {
        const string &rs = string_format("%s\t%-6s\t%-6s\t%-10s\t%s\t\r\n",
                                         location[i].data(),
                                         statementInfos[i].first.data(),
                                         statementInfos[i].second.data(), statementInfos[i].third.data(),
                                         objectCodes[i].data());
        cout << rs;
        ofs << rs;
    }

    cout << "================PASS2_Final Object Program===============" << endl;

    ofstream ofs2 = openOutputStream(OUTPUTPASS2_2NAME);
    const string &totalLen = int_to_hex(
            (int) strtol(lastAddress.data(), nullptr, 16) - (int) strtol(location.begin()->data(), nullptr, 16));

    //Header record
    stringstream ss;
    ss << "H"
       << setw(6) << left << setfill(' ') << statementInfos[0].first
       << setw(6) << right << setfill('0') << location[0]
       << setw(6) << right << setfill('0') << totalLen
       << endl;

    cout << ss.str();
    ofs2 << ss.str();

    ss.str("");

    //Text record
    string lineLen;
    string lineStartAddr;
    bool tagLineStartAddr = true;
    for (int i = 0; i < statementInfos.size(); ++i) {
        if (tagLineStartAddr) {
            lineStartAddr = location[i];
            tagLineStartAddr = false;
        }

        //一般狀況，先進行長度計算，如果超過60，則進行打印輸出
        if (ss.str().length() + objectCodes[i].length() > 60) {
            lineLen = int_to_hex((int) ss.str().length() / 2).substr(2, 2);
            cout << "T" << setw(6) << right << setfill('0') << lineStartAddr << lineLen << ss.str() << endl;
            ofs2 << "T" << setw(6) << right << setfill('0') << lineStartAddr << lineLen << ss.str() << endl;
            ss.str("");
            tagLineStartAddr = true;
        }

        StatementInfo &si = statementInfos[i];

        //不是start 且 沒有objectCode，計算長度後判斷有無超過60
        if (si.second != "START" && objectCodes[i].empty()) {
            int ll = (int) strtol(location[i].data(), nullptr, 16) - (int) strtol(location[i - 1].data(), nullptr, 16);
            if (ss.str().length() + ll * 2 > 60) {
                string outs = regex_replace(ss.str(), regex("\\$"), "");
                lineLen = int_to_hex((int) outs.length() / 2).substr(2, 2);
                cout << "T" << setw(6) << right << setfill('0') << lineStartAddr << lineLen << outs << endl;
                ofs2 << "T" << setw(6) << right << setfill('0') << lineStartAddr << lineLen << outs << endl;
                ss.str("");
                tagLineStartAddr = true;
            }
        }

        //最後一次迴圈，不輸出
        if (i == statementInfos.size() - 1) {
            break;
        }

        ////==== 以上的檢查都通過才進行objectCode的append =====/////


        //不是start 且 沒有objectCode，還是需要計算長度
        //因為 i = 0 是START，沒有objectCode，所以從i + 1開始
        if (si.second != "START"
            && objectCodes[i + 1].empty()) {
            int ll = (int) strtol(location[i].data(), nullptr, 16) - (int) strtol(location[i - 1].data(), nullptr, 16);

            //這是最後一次就不用再append "$" 了
            if (i == statementInfos.size() - 2) {
                continue;
            }

            //append "$" ，稍候計算長度
            for (int j = 0; j < ll * 2; ++j) {
                ss << "$";
            }
        } else {
            ss << objectCodes[i + 1];
        }
    }

    //End record
    ss.str("");
    ss << "E"<< setw(6) << right << setfill('0') << location[0];

    cout << ss.str();
    ofs2 << ss.str();
}

template<typename T>
string int_to_hex(T v) {
    stringstream stream;
    stream << setfill('0') << setw(sizeof(T))
           << uppercase
           << hex << v;
    return stream.str();
}

string string_format(const string& fmt, ...) {
    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    string str;
    va_list ap;
    while (true) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            return str;
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }
}