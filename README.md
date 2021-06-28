# SIC/XE Assembler

- 開發語言：C++11
- sic.cpp → SIC Assembler
- sicxe.cpp → SIC/XE Assembler

## SIC Assembler

### SIC虛指令
 - START ：指定程式名稱和起始位置
 - END : 結束處並指定程式中第一個執行指令
 - BYTE : 定義字元或十六進位常數 ex: 
    - C 'EOF' = 3 (c計算引號內有多少字元)
    - X 'F1' = 1 (x：計算每2個hex為1 byte, 此例長度為1)
 - WORD : 定義一個字組的整數常數 （長度固定3）
 - RESB : 保留所示數量位元組 ex:
    - (4096)十進位＝(1000)十六進位
 - RESW : 保留所示數量字組 （乘以三）

### 轉換流程
1. 將指令轉換OP-Code
    - ex: JSUB => 48
2. 把運算元對應到符號的位置
    - pass 1 : 計算指令位置`Location`及`SymbolTable`
    - pass 2 : 組譯指令產生`object code` 及`final object program`
3. 轉換規則
    - BYTE C 'EOF' => ASCII 十六進位 454F46
    - WORD 30 => 轉十六進位 00001E
    - RES  => 不輸出 object code
    - BYTE X 'F1'  => 直接輸出文字F1
    - RSUB => 輸出OP Code且後面補滿0
    - BUFFER'X => 直接加 8000

### 運行指南

#### 配置`input`、`output`路徑並運行`sic.cpp`

   ```c++
   //指定 input source statement 路徑
   #define FILENAME "./source.txt" 
   
   //指定output pass1 SourceProgram路徑
   #define OUTPUTPASS1_1NAME "./sic_pass1_SourceProgram_onlyLoc.txt" 
   
   //指定output pass1 SymbolTable路徑
   #define OUTPUTPASS1_2NAME "./sic_pass1_SymbolTable.txt"
   
   //指定output pass2 SourceProgramt路徑
   #define OUTPUTPASS2_1NAME "./sic_pass2_sourceProgram.txt"
   
   //指定output pass2 finalObjcetProgram路徑
   #define OUTPUTPASS2_2NAME "./sic_pass2_finalObjcetProgram.txt"
   ```

#### 主要架構
```cpp
/**
 * opTable
 */
const vector<string> opTable = {"ADD", .....,"WD"};
/**
 * opCode
 */
const vector<string> opCode = {"18", ....., "DC"};

/**
 * Source Statement 資料結構(ex:COPY START 1000)
 */
typedef struct StatementInfo{
    string first;
    string second;
    string third;
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
 * 儲存 Object code 資訊
 */
vector<string> objectCodes;

/**
 * 儲存 END 的位置資訊，用於產生 final object program使用
 * 因為不呈現在source program，所以存放在location
 */
string lastAddress;

int main() {
	  //開檔並構建 StatementInfo
    openFileAndBuildStatementInfo();
  
  	//構建 SymbolTable和Location
    buildSymbolTableAndLocation();
  
  	//輸出 Pass1檔案：symbolTable、location
    showAndOutputResult_Pass1();

	  //構建 object code
  	buildObjectCodes();
  	
	  //輸出 Pass2檔案：final sourceProgram、final objectProgram
    showAndOutputResult_Pass2();
    
    return 0;
}

```

#### 輸出結果

1. **sic_pass1_SourceProgram_onlyLoc.txt**
```json
Loc	    Source Statement	
1000	COPY  	START 	1000      	
1000	FIRST 	STL   	RETADR    	
1003	CLOOP 	JSUB  	RDREC     	
1006	      	LDA   	LENGTH    	
1009	      	COMP  	ZERO      	
100C	      	JEQ   	ENDFIL    	
100F	      	JSUB  	WRREC     	
1012	      	J     	CLOOP     	
1015	ENDFIL	LDA   	EOF       	
1018	      	STA   	BUFFER    	
101B	      	LDA   	THREE     	
101E	      	STA   	LENGTH    	
1021	      	JSUB  	WRREC     	
1024	      	LDL   	RETADR    	
1027	      	RSUB  	          	
102A	EOF   	BYTE  	C'EOF'    	
102D	THREE 	WORD  	3         	
1030	ZERO  	WORD  	0         	
1033	RETADR	RESW  	1         	
1036	LENGTH	RESW  	1         	
1039	BUFFER	RESB  	4096      	
2039	RDREC 	LDX   	ZERO      	
203C	      	LDA   	ZERO      	
203F	RLOOP 	TD    	INPUT     	
2042	      	JEQ   	RLOOP     	
2045	      	RD    	INPUT     	
2048	      	COMP  	ZERO      	
204B	      	JEQ   	EXIT      	
204E	      	STCH  	BUFFER,X  	
2051	      	TIX   	MAXLEN    	
2054	      	JLT   	RLOOP     	
2057	EXIT  	STX   	LENGTH    	
205A	      	RSUB  	          	
205D	INPUT 	BYTE  	X'F1'     	
205E	MAXLEN	WORD  	4096      	
2061	WRREC 	LDX   	ZERO      	
2064	WLOOP 	TD    	OUTPUT    	
2067	      	JEQ   	WLOOP     	
206A	      	LDCH  	BUFFER,X  	
206D	      	WD    	OUTPUT    	
2070	      	TIX   	LENGTH    	
2073	      	JLT   	WLOOP     	
2076	      	RSUB  	          	
2079	OUTPUT	BYTE  	X'05'     	
    	      	END   	FIRST     	
```

2. **sic_pass1_SymbolTable.txt**
```json
LabelName	      	Address	
FIRST           	1000  	
CLOOP           	1003  	
ENDFIL          	1015  	
EOF             	102A  	
THREE           	102D  	
ZERO            	1030  	
RETADR          	1033  	
LENGTH          	1036  	
BUFFER          	1039  	
RDREC           	2039  	
RLOOP           	203F  	
EXIT            	2057  	
INPUT           	205D  	
MAXLEN          	205E  	
WRREC           	2061  	
WLOOP           	2064  	
OUTPUT          	2079  
```

3. **sic_pass2_sourceProgram.txt**
```json
Loc	  Source Statement	        Object code	
1000	COPY  	START 	1000      		
1000	FIRST 	STL   	RETADR    	141033	
1003	CLOOP 	JSUB  	RDREC     	482039	
1006	      	LDA   	LENGTH    	001036	
1009	      	COMP  	ZERO      	281030	
100C	      	JEQ   	ENDFIL    	301015	
100F	      	JSUB  	WRREC     	482061	
1012	      	J     	CLOOP     	3C1003	
1015	ENDFIL	LDA   	EOF       	00102A	
1018	      	STA   	BUFFER    	0C1039	
101B	      	LDA   	THREE     	00102D	
101E	      	STA   	LENGTH    	0C1036	
1021	      	JSUB  	WRREC     	482061	
1024	      	LDL   	RETADR    	081033	
1027	      	RSUB  	          	4C0000	
102A	EOF   	BYTE  	C'EOF'    	454F46	
102D	THREE 	WORD  	3         	000003	
1030	ZERO  	WORD  	0         	000000	
1033	RETADR	RESW  	1         		
1036	LENGTH	RESW  	1         		
1039	BUFFER	RESB  	4096      		
2039	RDREC 	LDX   	ZERO      	041030	
203C	      	LDA   	ZERO      	001030	
203F	RLOOP 	TD    	INPUT     	E0205D	
2042	      	JEQ   	RLOOP     	30203F	
2045	      	RD    	INPUT     	D8205D	
2048	      	COMP  	ZERO      	281030	
204B	      	JEQ   	EXIT      	302057	
204E	      	STCH  	BUFFER,X  	549039	
2051	      	TIX   	MAXLEN    	2C205E	
2054	      	JLT   	RLOOP     	38203F	
2057	EXIT  	STX   	LENGTH    	101036	
205A	      	RSUB  	          	4C0000	
205D	INPUT 	BYTE  	X'F1'     	F1	
205E	MAXLEN	WORD  	4096      	001000	
2061	WRREC 	LDX   	ZERO      	041030	
2064	WLOOP 	TD    	OUTPUT    	E02079	
2067	      	JEQ   	WLOOP     	302064	
206A	      	LDCH  	BUFFER,X  	509039	
206D	      	WD    	OUTPUT    	DC2079	
2070	      	TIX   	LENGTH    	2C1036	
2073	      	JLT   	WLOOP     	382064	
2076	      	RSUB  	          	4C0000	
2079	OUTPUT	BYTE  	X'05'     	05	
    	      	END   	FIRST     		
```

4. **sic_pass2_finalObjcetProgram.txt**
```json
HCOPY  00100000107A
T0010001E1410334820390010362810303010154820613C100300102A0C103900102D
T00101E150C10364820610810334C0000454F46000003000000
T0020391E041030001030E0205D30203FD8205D2810303020575490392C205E38203F
T0020571C1010364C0000F1001000041030E02079302064509039DC20792C1036
T002073073820644C000005
E001000
```

## SIC/XE Assembler

### 轉換流程
1. 將指令轉換OP-Code
    - ex: JSUB => 48
2. 把運算元對應到符號的位置
    - pass 1 : 計算指令位置`Location`及`SymbolTable`
        - **這邊跟SIC 不一樣的地方 需要儲存`BASE` 位置**
    - pass 2 : 組譯指令產生`object code` 及`final object program`
        - **需要處理 format 2、format 3、format 4**
        - **需要處理 Modification record**
3. 轉換規則
    - BYTE C 'EOF' => ASCII 十六進位 454F46
    - WORD 30 => 轉十六進位 00001E
    - RES  => 不輸出 object code
    - BYTE X 'F1'  => 直接輸出文字F1
    - RSUB => 輸出OP Code且後面補滿0
    - **format 2 ： op r1 r2**`(ex: CLEAR X → B410)`
        - 轉換對應 opcode CLEAR → B4
        - 轉換暫存器代碼
            - B → 3
            - S → 4
            - T → 5
            - F → 6
            - A → 0
            - X → 1
        - 若無 r2則補0
    - **format 3 : op nixbpe disp** `(ex:STL RETADR → 17202D)`
        - 轉換對應 opcode
        - 判斷 **nixbpe**
            - `RSUB` : `110000`
            - 包含`,X` 
                - **PC Relative** : `111010`
                - **BASE Relative** : `111100`
            - 包含`#` or `@`
                - `#0`、`#3` ...: `010000`
                - **PC Relative**
                    - `#` : `010010`
                    - `@` : `100010`
                - **BASE Relative**
                    - `#` : `010100`
                    - `@` : `100100`
            - 一般狀況
                - **PC Relative** : `110010`
                - **BASE Relative** : `110100`
        - 轉換 disp
     - **format 4 : op nixbpe address** `(ex: +JSUB WRREC → 4B10105D)`
        - 轉換對應 opcode
        - 判斷**operand** 的 **nixbpe**
            - 包含`#` : `010001`
            - 包含`@` : `100001`
            - 包含`,X` : `111001`
            - 其它 : `110001`
        - 轉換 address

### 運行指南

#### 配置`input`、`output`路徑並運行`sicxe.cpp`

   ```c++
   //指定 input source statement 路徑
   #define FILENAME "./SICXEsource.txt"
   
   //指定output pass1 SourceProgram路徑
   #define OUTPUTPASS1_1NAME "./xe_pass1_SourceProgram_onlyLoc.txt"
   
   //指定output pass1 SymbolTable路徑
   #define OUTPUTPASS1_2NAME "./xe_pass1_SymbolTable.txt"
   
   //指定output pass2 SourceProgramt路徑
   #define OUTPUTPASS2_1NAME "./xe_pass2_sourceProgram.txt"
   
   //指定output pass2 finalObjcetProgram路徑
   #define OUTPUTPASS2_2NAME "./xe_pass2_finalObjcetProgram.txt"
   ```

#### 主要架構
> 同 SIC Assembler

#### 輸出結果

1. **xe_pass1_SourceProgram_onlyLoc.txt**
```json
Loc	      	Source Statement	
0000	COPY  	START 	0         	
0000	FIRST 	STL   	RETADR    	
0003	      	LDB   	#LENGTH   	
    	      	BASE  	LENGTH    	
0006	CLOOP 	+JSUB 	RDREC     	
000A	      	LDA   	LENGTH    	
000D	      	COMP  	#0        	
0010	      	JEQ   	ENDFIL    	
0013	      	+JSUB 	WRREC     	
0017	      	J     	CLOOP     	
001A	ENDFIL	LDA   	EOF       	
001D	      	STA   	BUFFER    	
0020	      	LDA   	#3        	
0023	      	STA   	LENGTH    	
0026	      	+JSUB 	WRREC     	
002A	      	J     	@RETADR   	
002D	EOF   	BYTE  	C'EOF'    	
0030	RETADR	RESW  	1         	
0033	LENGTH	RESW  	1         	
0036	BUFFER	RESB  	4096      	
1036	RDREC 	CLEAR 	X         	
1038	      	CLEAR 	A         	
103A	      	CLEAR 	S         	
103C	      	+LDT  	#4096     	
1040	RLOOP 	TD    	INPUT     	
1043	      	JEQ   	RLOOP     	
1046	      	RD    	INPUT     	
1049	      	COMPR 	A,S       	
104B	      	JEQ   	EXIT      	
104E	      	STCH  	BUFFER,X  	
1051	      	TIXR  	T         	
1053	      	JLT   	RLOOP     	
1056	EXIT  	STX   	LENGTH    	
1059	      	RSUB  	          	
105C	INPUT 	BYTE  	X'F1'     	
105D	WRREC 	CLEAR 	X         	
105F	      	LDT   	LENGTH    	
1062	WLOOP 	TD    	OUTPUT    	
1065	      	JEQ   	WLOOP     	
1068	      	LDCH  	BUFFER,X  	
106B	      	WD    	OUTPUT    	
106E	      	TIXR  	T         	
1070	      	JLT   	WLOOP     	
1073	      	RSUB  	          	
1076	OUTPUT	BYTE  	X'05'     	
    	      	END   	FIRST     	   	
```

2. **xe_pass1_SymbolTable.txt**
```json
LabelName	      	Address	
FIRST           	0000  	
CLOOP           	0006  	
ENDFIL          	001A  	
EOF             	002D  	
RETADR          	0030  	
LENGTH          	0033  	
BUFFER          	0036  	
RDREC           	1036  	
RLOOP           	1040  	
EXIT            	1056  	
INPUT           	105C  	
WRREC           	105D  	
WLOOP           	1062  	
OUTPUT          	1076  	
```

3. **xe_pass2_sourceProgram.txt**
```json
Loc	   Source Statement	   				Object code	
0000	COPY  	START 	0         		
0000	FIRST 	STL   	RETADR    	17202D	
0003	      	LDB   	#LENGTH   	69202D	
    	      	BASE  	LENGTH    		
0006	CLOOP 	+JSUB 	RDREC     	4B101036	
000A	      	LDA   	LENGTH    	032026	
000D	      	COMP  	#0        	290000	
0010	      	JEQ   	ENDFIL    	332007	
0013	      	+JSUB 	WRREC     	4B10105D	
0017	      	J     	CLOOP     	3F2FEC	
001A	ENDFIL	LDA   	EOF       	032010	
001D	      	STA   	BUFFER    	0F2016	
0020	      	LDA   	#3        	010003	
0023	      	STA   	LENGTH    	0F200D	
0026	      	+JSUB 	WRREC     	4B10105D	
002A	      	J     	@RETADR   	3E2003	
002D	EOF   	BYTE  	C'EOF'    	454F46	
0030	RETADR	RESW  	1         		
0033	LENGTH	RESW  	1         		
0036	BUFFER	RESB  	4096      		
1036	RDREC 	CLEAR 	X         	B410	
1038	      	CLEAR 	A         	B400	
103A	      	CLEAR 	S         	B440	
103C	      	+LDT  	#4096     	75101000	
1040	RLOOP 	TD    	INPUT     	E32019	
1043	      	JEQ   	RLOOP     	332FFA	
1046	      	RD    	INPUT     	DB2013	
1049	      	COMPR 	A,S       	A004	
104B	      	JEQ   	EXIT      	332008	
104E	      	STCH  	BUFFER,X  	57C003	
1051	      	TIXR  	T         	B850	
1053	      	JLT   	RLOOP     	3B2FEA	
1056	EXIT  	STX   	LENGTH    	134000	
1059	      	RSUB  	          	4F0000	
105C	INPUT 	BYTE  	X'F1'     	F1	
105D	WRREC 	CLEAR 	X         	B410	
105F	      	LDT   	LENGTH    	774000	
1062	WLOOP 	TD    	OUTPUT    	E32011	
1065	      	JEQ   	WLOOP     	332FFA	
1068	      	LDCH  	BUFFER,X  	53C003	
106B	      	WD    	OUTPUT    	DF2008	
106E	      	TIXR  	T         	B850	
1070	      	JLT   	WLOOP     	3B2FEF	
1073	      	RSUB  	          	4F0000	
1076	OUTPUT	BYTE  	X'05'     	05	
    	      	END   	FIRST     		
```

4. **xe_pass2_finalObjcetProgram.txt**
```json
HCOPY  000000001077
T0000001D17202D69202D4B1010360320262900003320074B10105D3F2FEC032010
T00001D130F20160100030F200D4B10105D3E2003454F46
T0010361DB410B400B44075101000E32019332FFADB2013A00433200857C003B850
T0010531D3B2FEA1340004F0000F1B410774000E32011332FFA53C003DF2008B850
T001070073B2FEF4F000005
M00000705+COPY
M00001405+COPY
M00002705+COPY
E000000
```