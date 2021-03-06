#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>

using namespace std;

#define FILENAME "./SICXEsource.txt"
#define OUTPUTPASS1_1NAME "./xe_pass1_SourceProgram_onlyLoc.txt"
#define OUTPUTPASS1_2NAME "./xe_pass1_SymbolTable.txt"
#define OUTPUTPASS2_1NAME "./xe_pass2_sourceProgram.txt"
#define OUTPUTPASS2_2NAME "./xe_pass2_finalObjcetProgram.txt"

const vector<vector<string>> opTable = {{"ADD", "3", "18"}, {"ADDF", "3", "58"}, {"ADDR", "2", "90"}, {"AND", "3", "40"}, {"CLEAR", "2", "B4"}, {"COMPF", "3", "88"}, {"COMPR", "2", "A0"}, {"COMP", "3", "28"}, {"DIVF", "3", "64"}
        , {"DIVR", "2", "9C"}, {"DIV", "3", "24"}, {"FIX", "1", "C4"}, {"FLOAT", "1", "C0"}, {"HIO", "1", "F4"}, {"J", "3", "3C"}, {"JEQ", "3", "30"}, {"JGT", "3", "34"}, {"JLT", "3", "38"}, {"JSUB", "3", "48"}
        , {"LDA", "3", "00"}, {"LDB", "3", "68"}, {"LDCH", "3", "50"}, {"LDF", "3", "70"}, {"LDL", "3", "08"}, {"LDS", "3", "6C"}, {"LDT", "3", "74"}, {"LDX", "3", "04"}, {"LPS", "3", "E0"}, {"UML", "3", "20"}
        , {"MULF", "3", "60"}, {"MULR", "2", "98"}, {"NORM", "1", "C8"}, {"OR", "3", "44"}, {"RD", "3", "D8"}, {"RMO", "2", "AC"}, {"RSUB", "3", "4C"}, {"SHIFTL", "2", "A4"}, {"SHIFTR", "2", "A8"}, {"SIO", "1", "F0"}
        , {"SSK", "3", "EC"}, {"STA", "3", "0C"}, {"STB", "3", "78"}, {"STCH", "3", "54"}, {"STF", "3", "80"}, {"STI", "3", "D4"}, {"STL", "3", "14"}, {"STSW", "3", "E8"}, {"STS", "3", "7C"}, {"STT", "3", "84"}
        , {"STX", "3", "10"}, {"SUBF", "3", "5C"}, {"SUBR", "2", "94"}, {"SUB", "3", "1C"}, {"SVC", "2", "B0"}, {"TD", "3", "E0"}, {"TIO", "1", "F8"}, {"TIXR", "2", "B8"}, {"TIX", "3", "2C"}, {"WD", "3", "DC"}};

typedef struct StatementInfo {
    string first;
    string second;
    string third;
    string opcode;
    string format;
} StatementInfo;

typedef struct SymbolTablePair {
    string label;
    string address;
} SymbolTablePair;

vector<StatementInfo> statementInfos;
vector<string> location;
vector<string> length;
vector<SymbolTablePair> symbolTable;
vector<string> objectCodes;
string base;
string lastAddress;


void buildSymbolTableAndLocation();
void buildObjectCodes();
void showAndOutputResult_Pass2();
void showAndOutputResult_Pass1();
string &getN1FromSymbolTable(const StatementInfo &si);
vector<string> splitByRegex(string text, const string& regex);
template<typename T>
string int_to_hex(T v);
string string_format(const string& fmt, ...);

/**
 * ??????????????? StatementInfo
 */
void openFileAndBuildStatementInfo() {
    ifstream file;

    // ??????
    file.open(FILENAME, ios::in);

    if (!file) {
        cout << "Can't open the file" << endl;
        system("pause");
    }

    cout << "File is opened" << endl;

    // ?????????????????????Vector
    string fs;
    int len = 0;
    string second;
    string format;
    string opcode;
    while (getline(file, fs)) {

        istringstream in(fs);
        vector<char> v;
        vector<string> ss;
        // splitByRegex by blank
        string s;
        while (in >> s) {
            ss.push_back(s);
        }
        StatementInfo info = StatementInfo();
        if (ss.size() == 3) {
            info.first = ss[0];
            info.second = ss[1];
            info.third = ss[2];
        }
        if (ss.size() == 2) {
            info.first = "";
            info.second = ss[0];
            info.third = ss[1];
        }

        if (ss.size() == 1) {
            info.first = "";
            info.second = ss[0];
            info.third = "";
        }

        second = info.second;
        format = "";
        if (info.second != "START"
            && info.second != "END"
            && info.second != "WORD"
            && info.second != "BYTE"
            && info.second != "RESB"
            && info.second != "RESW"
            && info.second != "BASE") {

            if (info.second.find('+') != string::npos) {
                len = 1;
                second = regex_replace(second, regex("\\+"), "");
            }
            for (int i = 0; i < opTable.size(); ++i) {
                if (second == opTable[i][0]) {
                    len += stoi(opTable[i][1]);
                    format = to_string(len);
                    opcode = opTable[i][2];
                    break;;
                }
            }
        }

        info.format = format;
        info.opcode = opcode;
        length.push_back(to_string(len));
        statementInfos.push_back(info);

        format = "";
        len = 0;
        opcode = "";
    }

    // ??????
    file.close();
    cout << "File is closed" << endl;
}

/**
 * ???Output???????????? ofstream
 * @return
 */
ofstream openOutputStream(const string &f) {
    // ????????????
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
 * ?????? SymbolTable???Location
 */
void buildSymbolTableAndLocation() {
    int len = 0;
    int decLoc = 0;
    string hexLoc;

    for (int i = 0; i < statementInfos.size(); ++i) {

        StatementInfo &si = statementInfos[i];

        //????????????
        if (si.second.find("BASE") != string::npos) {
            //?????????base????????????????????????SYM_TAB????????????(Base Relative)
            base = si.third;
            location.emplace_back("    ");//blank character *4

            decLoc += (int) strtol(length[i - 1].data(), nullptr, 16);
            hexLoc = int_to_hex(decLoc);
        } else {
            if (i > 1) {
                decLoc += (int) strtol(length[i - 1].data(), nullptr, 16);
                hexLoc = int_to_hex(decLoc);

                //end?????????
                if (i == statementInfos.size() - 1) {
                    lastAddress = hexLoc;
                    hexLoc = "    ";
                }
            }  else {
                hexLoc = "0000";
            }

            location.push_back(hexLoc);
        }


        //location???????????????
//        if (i == 0){
//            n = stoi(si.third);
//            decLoc = (int)strtol(to_string(n).data(), nullptr, 16);
//
//            location[0] = to_string(n);
//        }

        len = stoi(length[i]);
        //????????????
        if (i == 0) {
            len = 0;
        } else if (si.second == "BYTE") {//byte??????C???X
            if (si.third.find('C') != string::npos) {
                unsigned long idx = si.third.find('\'', 0);
                len = si.third.length() - idx - 2;
            } else {
                //???X'F1'?????????1
                len = 1;
            }
        } else if (si.second == "RESW") {//??????*3
            len = stoi(si.third) * 3;
        } else if (si.second == "RESB") {
            len = stoi(int_to_hex(stoi(si.third)));
        } else if (si.second == "WORD") {
            len = 3;
        } else if (si.second == "CLEAR") {
            len = 2;
        }
        length[i] = to_string(len);

        //??????SYM_TAB
        if (i != 0 && !si.first.empty()) {
            if (base == si.first)
                base = hexLoc;

            SymbolTablePair p = SymbolTablePair();
            p.label = si.first;
            p.address = hexLoc;
            symbolTable.push_back(p);
        }
    }
}

/**
 * ?????? object code
 */
void buildObjectCodes() {
    for (int i = 0; i < statementInfos.size(); ++i) {
        stringstream ss;
        StatementInfo &si = statementInfos[i];
        if (si.second == "END") {
            objectCodes.emplace_back("");
            break;
        }

        // format 2 : op r1 r2
        if (si.format == "2") {
            ss << si.opcode;
            const vector<string> &splitThird = splitByRegex(si.third, ",");
            int j = 0;
            for (j = 0; j < splitThird.size(); ++j) {
                if (splitThird[j] == "B") {
                    ss << "3";
                } else if (splitThird[j] == "S") {
                    ss << "4";
                } else if (splitThird[j] == "T") {
                    ss << "5";
                } else if (splitThird[j] == "F") {
                    ss << "6";
                } else if (splitThird[j] == "A") {
                    ss << "0";
                } else if (splitThird[j] == "X") {
                    ss << "1";
                }
            }
            //??????r2??????0
            if (j == 1) {
                ss << "0";
            }
        }

        //format 4 : op nixbpe address
        if (si.format == "4") {
            string nixbpe = "";
            string b = bitset<8>(strtol(si.opcode.data(), nullptr, 16)).to_string();
            string bb = b.substr(0, 6);
            bb.erase(0, bb.find_first_not_of('0'));

            if (bb == "0") {
                bb = "000000";   //LDA??????0??????opcode=0
            }

            if (si.third.find('#') != string::npos) {
                nixbpe = "010001";
            } else if (si.third.find('@') != string::npos) {
                nixbpe = "100001";
            } else if (si.third.find(",X") != string::npos) {
                nixbpe = "111001";
            } else {
                nixbpe = "110001";
            }

            string r = int_to_hex(strtol((bb + nixbpe).data(), nullptr, 2));
            r.erase(0, r.find_first_not_of('0'));

            //?????? LDA ???0
            ss << setw(3) << right << setfill('0') << r;

            for (int j = 0; j < symbolTable.size(); ++j) {
                if (si.third.find(symbolTable[j].label) != string::npos) {
                    if (symbolTable[j].address.length() != 5) { //SYM 5?????????0 (20/4=5)
                        for (int k = symbolTable[j].address.length(); k < 5; ++k) {
                            ss << "0";
                        }
                    }
                    ss << symbolTable[j].address;
                    break;
                }
            }

            const char *ch = si.third.data();
            if (ch[1] - '0' >= 0
                && ch[1] - '0' <= 9) {
                //#4096????????????->1000 ??????hex
                string hex = int_to_hex(stoi(si.third.substr(1, si.third.length() - 1)));
                if (hex.length() != 5) {//hex??? 5????????????0 (20/4=5)
                    for (int j = hex.length(); j < 5; ++j) {
                        ss << "0";
                    }
                }
                ss << hex;
            }
        }

        //format 3 : op nixbpe disp
        if (si.format == "3") {
            string nixbpe = "";
            string b = bitset<8>(strtol(si.opcode.data(), nullptr, 16)).to_string();
            string bb = b.substr(0, 6);
            bb.erase(0, bb.find_first_not_of('0'));

            if (bb == "0") {
                bb = "000000";   //LDA??????0??????opcode=0
            }

            if (si.third.empty()
                || si.second == "RSUB") {
                nixbpe = "110000";
                string r = int_to_hex(strtol((bb + nixbpe).data(), nullptr, 2));
                r.erase(0, r.find_first_not_of('0'));
                r += "000";
                ss << r;
            } else if (si.third.find(",X") != string::npos) {
                string n1 = getN1FromSymbolTable(si);
                string n2 = location[i + 1];

                int disp = strtol(n1.data(), nullptr, 16) - strtol(n2.data(), nullptr, 16);
                //pc Relative
                if (disp < 2047 && disp > -2048) {
                    nixbpe = "111010";
                    string r = int_to_hex(strtol((bb + nixbpe).data(), nullptr, 2));
                    r.erase(0, r.find_first_not_of('0'));

                    //?????? LDA ???0
                    ss << setw(3) << right << setfill('0') << r;

                    string dispHex = int_to_hex(disp);
                    dispHex = (dispHex == "0000") ? "0" : dispHex.erase(0, dispHex.find_first_not_of('0'));
                    if (dispHex.length() != 3) {
                        for (int j = dispHex.length(); j < 3; ++j) {
                            ss << "0";
                        }
                    }

                    ss << dispHex;
                } else {
                    //base Relative
                    nixbpe = "111100";
                    disp = strtol(n1.data(), nullptr, 16) - strtol(base.data(), nullptr, 16);
                    string r = int_to_hex(strtol((bb + nixbpe).data(), nullptr, 2));
                    r.erase(0, r.find_first_not_of('0'));

                    //?????? LDA ???0
                    ss << setw(3) << right << setfill('0') << r;

                    string dispHex = int_to_hex(disp);
                    dispHex = (dispHex == "0000") ? "0" : dispHex.erase(0, dispHex.find_first_not_of('0'));
                    if (dispHex.length() != 3) {
                        for (auto j = dispHex.length(); j < 3; ++j) {
                            ss << "0";
                        }
                    }
                    ss << dispHex;
                }

            } else if (si.third.find('#') != string::npos
                       || si.third.find('@') != string::npos) {
                //#3??? op #c
                const char *ch = si.third.data();
                if (ch[1] - '0' >= 0
                    && ch[1] - '0' <= 9) {

                    string n1 = int_to_hex(stoi(si.third.substr(1, si.third.length() - 1)));
                    n1 = (n1 == "0000") ? "0" : n1.erase(0, n1.find_first_not_of('0'));
                    nixbpe = si.third.find('#') != string::npos ? "010000" : "100000";

                    string r = int_to_hex(strtol((bb + nixbpe).data(), nullptr, 2));
                    r.erase(0, r.find_first_not_of('0'));

                    //?????? LDA ???0
                    ss << setw(3) << right << setfill('0') << r;
                    if (n1.length() != 3) {
                        for (auto j = n1.length(); j < 3; ++j) {
                            ss << "0";
                        }
                    }
                    ss << n1;
                } else {
                    string n1 = getN1FromSymbolTable(si);
                    string n2 = location[i + 1];

                    if (n2 == "    ") {
                        //??????BASE??????????????????
                        n2 = location[i + 2];
                    }
                    int disp = strtol(n1.data(), nullptr, 16) - strtol(n2.data(), nullptr, 16);

                    if (disp < 2047 && disp > -2048) {
                        nixbpe = si.third.find('#') != string::npos ? "010010" : "100010";
                        string r = int_to_hex(strtol((bb + nixbpe).data(), nullptr, 2));
                        r.erase(0, r.find_first_not_of('0'));

                        //?????? LDA ???0
                        ss << setw(3) << right << setfill('0') << r;

                        string dispHex = int_to_hex(disp);
                        dispHex = (dispHex == "0000") ? "0" : dispHex.erase(0, dispHex.find_first_not_of('0'));
                        if (dispHex.length() != 3) {
                            for (int j = dispHex.length(); j < 3; ++j) {
                                ss << "0";
                            }
                        }
                        ss << dispHex;
                    } else {
                        //base Relative
                        nixbpe = si.third.find('#') != string::npos ? "010100" : "100100";
                        disp = strtol(n1.data(), nullptr, 16) - strtol(base.data(), nullptr, 16);
                        string r = int_to_hex(strtol((bb + nixbpe).data(), nullptr, 2));
                        r.erase(0, r.find_first_not_of('0'));

                        //?????? LDA ???0
                        ss << setw(3) << right << setfill('0') << r;

                        string dispHex = int_to_hex(disp);
                        dispHex = (dispHex == "0000") ? "0" : dispHex.erase(0, dispHex.find_first_not_of('0'));
                        if (dispHex.length() != 3) {
                            for (auto j = dispHex.length(); j < 3; ++j) {
                                ss << "0";
                            }
                        }
                        ss << dispHex;
                    }
                }
            } else {
                //????????????
                string n1 = getN1FromSymbolTable(si);
                string n2 = location[i + 1];

                int disp = strtol(n1.data(), nullptr, 16) - strtol(n2.data(), nullptr, 16);
                //pc Relative
                if (disp < 2047 && disp > -2048) {
                    nixbpe = "110010";
                    string r = int_to_hex(strtol((bb + nixbpe).data(), nullptr, 2));
                    r.erase(0, r.find_first_not_of('0'));

                    //?????? LDA ???0
                    ss << setw(3) << right << setfill('0') << r;

                    string dispHex = int_to_hex(disp);
                    dispHex = dispHex.erase(0, dispHex.find_first_not_of('0'));
                    if (dispHex.length() < 3) {
                        for (int j = dispHex.length(); j < 3; ++j) {
                            ss << "0";
                        }
                    }
                    //??????????????????????????????
                    if (dispHex.length() > 3) {
                        ss << dispHex.substr(dispHex.length() - 3, 3);
                    } else {
                        ss << dispHex;
                    }
                } else {
                    //base Relative
                    nixbpe = "110100";
                    disp = strtol(n1.data(), nullptr, 16) - strtol(base.data(), nullptr, 16);
                    string r = int_to_hex(strtol((bb + nixbpe).data(), nullptr, 2));
                    r.erase(0, r.find_first_not_of('0'));

                    //?????? LDA ???0
                    ss << setw(3) << right << setfill('0') << r;
                    string dispHex = int_to_hex(disp);
                    dispHex = (dispHex == "0000") ? "0" : dispHex.erase(0, dispHex.find_first_not_of('0'));
                    if (dispHex.length() != 3) {
                        for (auto j = dispHex.length(); j < 3; ++j) {
                            ss << "0";
                        }
                    }

                    //??????????????????????????????
                    if (dispHex.length() > 3) {
                        ss << dispHex.substr(dispHex.length() - 3, 3);
                    } else {
                        ss << dispHex;
                    }
                }
            }
        }

        if (si.second == "BYTE") {
            unsigned long idx = si.third.find('\'');
            string thirdSplit = si.third.substr(idx + 1, si.third.length() - idx - 2);
            //??????BYTE?????????X???C
            for (int j = 0; j < thirdSplit.length(); ++j) {
                //C
                if (si.third.find('C') != string::npos) {
                    ss << int_to_hex((int) thirdSplit[j]).substr(2, 2);
                } else {
                    //X
                    ss << thirdSplit[j];
                }
            }
        }

        objectCodes.push_back(ss.str());
    }
}

/**
 * ??????Pass1?????????symbolTable???location
 */
void showAndOutputResult_Pass1() {

    ofstream ofs1 = openOutputStream(OUTPUTPASS1_1NAME);

    const string &title = string_format("%s\t%-6s\t%-6s\t\n",
                                        "Loc", " ", "Source Statement");


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
                                         "LabelName", " ", "Address");
    cout << "================PASS1_Symbol Table===============" << endl;
    cout << title2;
    ofs2 << title2;
    for (auto &s : symbolTable) {
        const string &st = string_format("%-16s\t%-6s\t\r\n",
                                         s.label.data(),
                                         s.address.data());
        cout << st;
        ofs2 << st;
    }
}

/**
 * ??????Pass2?????????final sourceProgram???final objectProgram
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
    stringstream mr;
    bool tagLineStartAddr = true;
    for (int i = 0; i < statementInfos.size(); ++i) {
        if (tagLineStartAddr) {
            lineStartAddr = location[i];
            tagLineStartAddr = false;
        }

        //???????????????????????????????????????????????????60????????????????????????
        if (ss.str().length() + objectCodes[i].length() > 60) {
            lineLen = int_to_hex((int) ss.str().length() / 2).substr(2, 2);
            cout << "T" << setw(6) << right << setfill('0') << lineStartAddr << lineLen << ss.str() << endl;
            ofs2 << "T" << setw(6) << right << setfill('0') << lineStartAddr << lineLen << ss.str() << endl;
            ss.str("");
            tagLineStartAddr = true;
        }

        StatementInfo &si = statementInfos[i];

        //??????start ??? ??????objectCode????????????????????????????????????60
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

        //?????????????????????????????????
        if (i == statementInfos.size() - 1) {
            break;
        }

        ////==== ?????????????????????????????????objectCode???append =====/////


        //??????start ??? ??????objectCode???????????????????????????
        //?????? i = 0 ???START?????????objectCode????????????i + 1??????
        //?????? BASE ??????????????????objectCode
        if (si.second != "START"
            && objectCodes[i + 1].empty()
            && statementInfos[i + 1].second != "BASE") {
            int ll = (int) strtol(location[i].data(), nullptr, 16) - (int) strtol(location[i - 1].data(), nullptr, 16);

            //??????????????????????????????append "$" ???
            if (i == statementInfos.size() - 2) {
                continue;
            }

            //append "$" ?????????????????????
            for (int j = 0; j < ll * 2; ++j) {
                ss << "$";
            }
        } else {
            //add modification record
            // contain "+"
            // not contain "#"???"@"???",X"
            if (si.second.find('+') != string::npos
            && si.third.find('#')  == string::npos
            && si.third.find('@')  == string::npos
            && si.third.find(",X")  == string::npos
            ) {

                string mLoc = int_to_hex(strtol(location[i].data(), nullptr, 16) + (int) strtol("1", nullptr, 16));
                mLoc.erase(0, mLoc.find_first_not_of('0'));

                mr << "M"
                << setw(6) << right << setfill('0')
                << mLoc
                << "05" //??????????????????????????????"06"?????????
                << "+" + statementInfos[0].first
                << endl;
            }
            // add objectCodes
            ss << objectCodes[i + 1];
        }
    }

    ss.str("");

    //modification record
    ss << mr.str();

    //End record
    ss << "E" << setw(6) << right << setfill('0') << location[0];

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

string string_format(const string &fmt, ...) {
    int size = ((int) fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    string str;
    va_list ap;
    while (true) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *) str.data(), size, fmt.c_str(), ap);
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

vector<string> splitByRegex(string text, const string &regex) {
    std::regex rgx(regex);
    std::sregex_token_iterator iter(text.begin(),
                                    text.end(),
                                    rgx,
                                    -1);
    std::sregex_token_iterator end;
    vector<string> vs;
    for (; iter != end; ++iter)
        vs.push_back(*iter);
    return vs;
}

string &getN1FromSymbolTable(const StatementInfo &si) {
    for (auto &pair : symbolTable) {
        if (si.third.find(pair.label) != string::npos) {
            return pair.address;
        }
    }
}
