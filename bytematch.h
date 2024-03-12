/***
 * MIT License
 * Copyright 2024 Mountcloud mountcloud@outlook.com
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _MOUNTCLOUD_BYTEMATCH_H_
#define _MOUNTCLOUD_BYTEMATCH_H_

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

namespace mc::bytematch{
typedef unsigned char BYTE;

enum PatternMatcherType{
	MT_ERROR = -1,
	MT_FULL = 0,
	MT_HIGH4 = 1,
	MT_LOW4 = 2,
	MT_ANY = 3
};
class PatternElement {
public:
    BYTE value = 0; // 字节值，-1 表示任意字节，其他表示具体字节

	int matchType = -1; // -1=error, 0 = full byte,1 = high 4 bit,2 = low 4 bit,3 = any

	bool isRepeat = false; //是否重复

    int minRepeats = 0; // 元素在序列中出现的最小次数
    int maxRepeats = 0; // 元素在序列中出现的最大次数

	PatternElement* next = NULL;

public:
	~PatternElement();
};


class Pattern{
public:
	PatternElement* head = NULL;
	int length = 0;
	~Pattern();
};


class PatternParser{
public:
	Pattern* parsePattern(std::string patternStr);
};

class PatternMatcherElement{
public:
	PatternElement* element = NULL;

	int repeatCount = 0;

	BYTE* firstMatch = NULL;
	BYTE* lastMatch = NULL;
};

class PatternMatcherResult{
public:
    BYTE* firstMatch = NULL;
    BYTE* lastMatch = NULL;
};

class PatternMatcher{
public:
    PatternMatcher(Pattern* pat);
    ~PatternMatcher();
	/***
	 * 匹配数据
	 * @param data 数据
	 * @param size 数据长度
	 */
    void matchData(BYTE* data, size_t size);

    /**
     * 单字节逐个匹配
     * @param addr 地址
     * @param data 当前字节
     */
    void match(BYTE* addr,BYTE data);

    // 获取匹配结果
    std::vector<PatternMatcherResult> getResult();

    //是否有结果
    bool hasResult();

private:
	Pattern* pattern;
	// 使用列表来跟踪每个潜在匹配的开始位置和状态
	std::vector<PatternMatcherElement> activeMatches;

    std::vector<PatternMatcherResult> matchResults;

private:
    bool matchByte(BYTE dataByte, PatternElement* elem);
};

}


#endif