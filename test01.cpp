#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>

using namespace std;

#define FILENAME "./source.txt"
#define OUTPUTPASS2_1NAME "./output.txt"


string opTable[59] = {"ADD", "ADDF", "ADDR", "AND", "CLEAR", "COMP", "COMPF", "COMPR", "DIV", "DIVF", "DIVR",
                        "FIX", "FLOAT", "HIO", "J", "JEQ", "JGT", "JLT", "JSUB", "LDA", "LDB", "LDCH", "LDF", "LDL", "LDS",
                        "LDT", "LDX", "LPS", "MUL", "MULF", "MULR", "NORM", "OR", "RD", "RMO", "RSUB", "SHIFTL", "SHIFTR",
                        "SIO", "SSK", "STA", "STB", "STCH", "STF", "STI", "STL", "STS", "STSW", "STT", "STX", "SUB", "SUBF",
                        "SUBR", "SVC", "TD", "TIO", "TIX", "TIXR", "WD"};
string opCode[59] = {"18", "58", "90", "40", "B4", "28", "88", "A0", "24", "64", "9C", "C4", "C0", "F4", "3C",
                     "30", "34", "38", "48", "00", "68", "50", "70", "08", "6C", "74", "04", "E0", "20", "60", "98", "C8",
                     "44", "D8", "AC", "4C", "A4", "A8", "F0", "EC", "0C", "78", "54", "80", "D4", "14", "7C", "E8", "84",
                     "10", "1C", "5C", "94", "B0", "E0", "F8", "2C", "B8", "DC"};

typedef struct StatementInfo{
    string first;
    string second;
    string third;
};

vector<StatementInfo> statementInfos;
vector<string > location;

void test01();
void test02();
void test03();
void test04();
void test05();
void test06();
void test07();
void test08();
void test09();
void test10();
void test11();
void test12();

int main() {
//    test01();
//    test02();
//    test03();
//    test04();
//    test05();
//    test06();
//    test07();
//    test08();
//    test09();
//    test10();
//    test11();
    test12();
    return 0;
}

template< typename T >
string int_to_hex( T i )
{
    stringstream stream;
//    stream << "0x"
//           << setfill ('0') << setw(sizeof(T)*2)
//           << hex << i;
    stream << setfill('0') << setw(sizeof(T))
           << uppercase
           << hex << i;
    return stream.str();
}

string intToHexString(int i) {
    stringstream ss;
    ss << hex << showbase << i;
    return ss.str();
}

void test01() {
    int len = 0;
    int n = 0;
    int decLoc = 0;

    n = stoi("1000");
    string t = "1000";
//    decLoc = atoi(t.data());
    decLoc = (int)strtol(t.data(), NULL, 16);
}

void test02() {
    int a = 8888;
    cout << intToHexString(a).data() << endl;
    cout << int_to_hex(a).data() << endl;
}

void test03(){
    string  s = "C'EOF'";
    if (s.find('C') != string::npos) {
        std::cout << "found c!" << '\n';
    } else{
        std::cout << "not found c!" << '\n';
    }
}

void test04(){
    string  s = "C'EOF'";
    unsigned long i = s.find('\'');
    cout<< i <<endl;
    cout<< s.length() - i - 2 <<endl;

    string s2 = s.substr(i +1, s.length() - i - 2 );
    cout << s2 << endl;
    cout<<s2.length()<<endl;
}

void test05(){
    string  a = "4015";
    string  b = "8000";

    int res_10 = (int)strtol(a.data(), NULL, 16)+(int)strtol(b.data(), NULL, 16);;

    cout << res_10 << endl;
    cout<< int_to_hex(res_10) << endl;
}

void test06(){
    string  a = "EOF";
//    const char *ac = a.c_str();
    string r = "";

    for (int i = 0; i < a.length(); ++i) {
        r +=  int_to_hex((int)a[i]).substr(2,2);
    }

    cout << r << endl;
//    cout<< int_to_hex(res_10) << endl;
}

void test07(){
    string  s = "TABLE,X'";

    string s2 = s.substr(0, s.length() - 3 );
    cout << s2 << endl;
    cout<<s2.length()<<endl;
}

void test08(){
    string  a = "4096";

//    cout << int_to_hex((int)strtol(a.data(), NULL, 16));

    long i = stoi(a.data());
    cout << i << endl;
    cout << int_to_hex(a.data());
}

void test09(){
    string  a = "207A";
    string  b = "1000";

    cout << int_to_hex((int)strtol(a.data(), NULL, 16)-(int)strtol(b.data(), NULL, 16));
}

void test10(){
    string  t = "COPY";
    cout << setw(6) << left << setfill(' ') << t;
}

void test11(){
    int  t = 60;
    cout << int_to_hex(t/2).substr(2,2);
}

void test12(){
    string t = "T00101E1E0C10364820610810334C0000454F46000003000000$$$$$$$$$$$$$$$$$$";
    cout << regex_replace(t, regex("\\$"), "");
}