# LICENSE
当然是大家都爱的MIT。

Of course, it's MIT that everyone loves.

# 核心目的 Core Purpose
为了某一个内存区间内搜索特征，定位特征位置。

To search for features within a certain memory interval and locate feature positions.

# 介绍 DESC
规则方面支持模糊或者范围定义匹配规则，搜索结果直接返回内存地址。

In terms of rules, it supports fuzzy or range defined matching rules, and the search results are directly returned to the memory address.

# 提示 TIPS
如果有BUG欢迎提出，我会尽力修复，也欢迎一起参与维护。

If there are any bugs, please feel free to raise them. I will do my best to fix them and also welcome you to participate in maintenance together.

# 使用方式 DEMO
## 规则 RULES：
```
1：基础语法 basic grammar：
字节之间用空格隔开，比如：01 02，不允许连在一起（错误示范）：0102。
Bytes are separated by spaces, for example: 01 02, not allowed to be connected together (error demonstration): 0102.

2：模糊匹配 fuzzy matching：
模糊符号是英文符号：?，如果需要高位模糊则：?1 ?2，低位模糊则：?1 ?2，如果需要完全模糊匹配这个字节：??
The fuzzy symbol is an English symbol:?, If high-level blur is required:? 1? 2. If the low bit is blurry:? 1? 2. If a complete fuzzy match is required for this byte:??

3：范围匹配 range match：
范围匹配语法格式：{min,max} 或 {count}，用法是字节匹配后紧跟大括号：{1,10} 或 {10}。
Range matching syntax format: {min, max} or {count}, usage is byte matching followed by curly braces: {1,10} or {10}.

3.1：匹配固定个数 {count} Matching a fixed number of {count}
01{10} 则01匹配10个。
01{10} matches 10.

3.2：匹配范围个数 {min,max} Number of matching ranges {min, max}
01{1,10} 则01至少存在1个，最多10个。
01{1,10} means that 01 has at least one and a maximum of 10.

3.3：模糊字节范围匹配 Fuzzy Byte Range Matching
支持与模糊匹配相结合，比如：0?{10} 0?{1,10} ?1{10} ?1{1,10} ??{10} ??{1,10}
Support the combination of fuzzy matching, such as: 0?{10} 0?{1,10} ?1{10} ?1{1,10} ??{10} ??{1,10}
```

## DEMO CODE：
```
#include "bytematch.h"

#include <string>
#include <iostream>

// 主函数 main
int main() {
    //parser pattern
	mc::bytematch::PatternParser parser;
    std::string searchPattern = "41 57 41 56 41 55 41 54 56 57 55 53 48 83 EC ??{1,4} 44 89 C6 48 89 D7 48 89 CB 48 8B 05 ??{1,4}";
	
	auto pattern = parser.parsePattern(searchPattern);

	mc::bytematch::PatternMatcher* pm = new mc::bytematch::PatternMatcher(pattern);

	mc::bytematch::BYTE codes[] = {
		0xAE, 0xAA
		, 0x41, 0x57
		, 0x41, 0x56
		, 0x41, 0x55
		, 0x41, 0x54
		, 0x56
		, 0x57
		, 0x55
		, 0x53
		, 0x48, 0x83, 0xEC, 0x58
		, 0x44, 0x89, 0xC6
		, 0x48, 0x89, 0xD7
		, 0x48, 0x89, 0xCB
		, 0x48, 0x8B, 0x05, 0x38, 0xD2, 0xA1, 0x0C

		, 0xAE, 0xAA
		, 0x41, 0x57
		, 0x41, 0x56
		, 0x41, 0x55
		, 0x41, 0x54
		, 0x56
		, 0x57
		, 0x55
		, 0x53
		, 0x48, 0x83, 0xEC, 0x90, 0x00, 0x00, 0x00
		, 0x44, 0x89, 0xC6
		, 0x48, 0x89, 0xD7
		, 0x48, 0x89, 0xCB 
		, 0x48, 0x8B, 0x05, 0xAA, 0xDF, 0x03, 0x11
		, 0xFA, 0x22
	};

	pm->matchData(codes,sizeof(codes));

	auto result = pm->getResult();
	if(result.size()>0 && pm->hasResult()){
		std::cout << "Matched" << std::endl;
	}else{
		std::cout << "Not Matched" << std::endl;
	}

    return 0;
}

```