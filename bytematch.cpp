#include "bytematch.h"

namespace mc::bytematch{
    PatternElement::~PatternElement() {
		if(next != NULL){
			delete next;
			next = NULL;
		}
	}

    Pattern::~Pattern() {
		if(head != NULL){
			delete head;
            head = NULL;
		}
	}

    Pattern* PatternParser::parsePattern(std::string patternStr) {
		Pattern* pattern = NULL;
		PatternElement* lastElement = NULL;

		PatternElement* tempElement = NULL;

		bool error = false;
		bool inRepeat = false;
		bool repeatMax = false;
		for(int i=0;i<patternStr.size();i++){
			BYTE c = patternStr[i];
			if(c==' '){
				if(tempElement != NULL){
					if(pattern == NULL){
						pattern = new Pattern();
						pattern->head = tempElement;
						lastElement = tempElement;
					}else{
						lastElement->next = tempElement;
						lastElement = tempElement;
					}
					tempElement = NULL;
				}
				continue;
			}else{
				//if c is a hex char
				if(tempElement == NULL){
					tempElement = new PatternElement();
				}

				if(c=='?'){
					if(tempElement->matchType == bytematch::PatternMatcherType::MT_ERROR){
						tempElement->matchType = bytematch::PatternMatcherType::MT_LOW4;
					}else if(tempElement->matchType == bytematch::PatternMatcherType::MT_LOW4){
						tempElement->matchType = bytematch::PatternMatcherType::MT_ANY;
					}else if(tempElement->matchType == bytematch::PatternMatcherType::MT_HIGH4){
						continue;
					}else{
						error = true;
						break;
					}
				}else if((c>='0' && c<='9') || (c>='A' && c<='F') || (c>='a' && c<='f')){
					if(!inRepeat){
						if(tempElement->matchType == bytematch::PatternMatcherType::MT_ERROR){
							tempElement->matchType = bytematch::PatternMatcherType::MT_HIGH4;
						}else if(tempElement->matchType == bytematch::PatternMatcherType::MT_HIGH4){
							tempElement->matchType = bytematch::PatternMatcherType::MT_FULL;
						}

						BYTE tempvalue = 0;
						//c to int
						if(c>='0' && c<='9'){
							tempvalue = c - '0';
						}else if(c>='A' && c<='F'){
							tempvalue = c - 'A' + 10;
						}else if(c>='a' && c<='f'){
							tempvalue = c - 'a' + 10;
						}

						if(tempElement->value == 0){
							tempElement->value = tempvalue;
						}else{
							tempElement->value = (tempElement->value << 4) + tempvalue;
						}
					}else{
						if(repeatMax){
							tempElement->maxRepeats = tempElement->maxRepeats * 10 + (c - '0');
						}else{
							tempElement->minRepeats = tempElement->minRepeats * 10 + (c - '0');
						}
					}
				}else if(c == '{') {
					tempElement->isRepeat = true;
					inRepeat = true;
				}else if(c == ',' && inRepeat){
					repeatMax = true;
				}else if(c== '}'){
					inRepeat = false;
					repeatMax = false;
					continue;
				}else{
					error = true;
					break;
				}
			}
		}

		if(error){
			if(pattern != NULL){
				delete pattern;
			}
			return NULL;
		}
		
		return pattern;
	}

    PatternMatcher::PatternMatcher(Pattern* pat) : pattern(pat){

    }
    PatternMatcher::~PatternMatcher() {
        //pattern不删除，因为设计上认为pattern是可重复利用的规则，而Matcher是临时或一次性的
        activeMatches.clear();
        matchResults.clear();
    }

    void PatternMatcher::matchData(BYTE* data, size_t size) {
        if (!data || size == 0) return;

        for (size_t dataIndex = 0; dataIndex < size; ++dataIndex) {
            BYTE currentByte = data[dataIndex];
            match(data + dataIndex , currentByte);
        }
    }

    void PatternMatcher::match(BYTE* addr,BYTE data) {
        if (!addr) return;

        // 为避免在迭代过程中修改容器，收集需要添加或移除的元素
        std::vector<decltype(activeMatches.begin())> toRemove;
        std::vector<PatternMatcherElement> toAdd;

        for (auto currentMatcher = activeMatches.begin(); currentMatcher != activeMatches.end(); ++currentMatcher) {
            // PatternMatcherElement currentMatcher = it;
            PatternElement* currentElement = currentMatcher->element;

            if (currentElement == NULL) {
                PatternMatcherResult pmc;
                pmc.firstMatch = currentMatcher->firstMatch;
                pmc.lastMatch = currentMatcher->lastMatch;
            	matchResults.push_back(pmc);
                toRemove.push_back(currentMatcher);
                continue;
            }

            bool matched = matchByte(data, currentElement);

            if (matched) {
                if(currentElement->next==NULL){
                    bool isMatched = false;
                    if(currentElement->isRepeat){
                            if(currentElement->maxRepeats > 0 && currentMatcher->repeatCount >= currentElement->minRepeats && currentMatcher->repeatCount <= currentElement->maxRepeats){
                            isMatched = true;
                        }else if(currentElement->maxRepeats == 0 && currentMatcher->repeatCount == currentElement->minRepeats){
                            isMatched = true;
                        }
                    }else{
                        isMatched = true;
                    }
                    if(isMatched){
                        PatternMatcherResult pmc;
                        pmc.firstMatch = currentMatcher->firstMatch;
                        pmc.lastMatch = currentMatcher->lastMatch;
                        matchResults.push_back(pmc);
                        toRemove.push_back(currentMatcher);
                    }
                }else{
                    // 更新当前匹配路径的状态
                    bool addNew = false;
                    if(currentElement->isRepeat){
                        currentMatcher->repeatCount = currentMatcher->repeatCount + 1;
                        if(currentMatcher->repeatCount >= currentElement->minRepeats && currentMatcher->repeatCount < currentElement->maxRepeats){
                            addNew = true;
                        }
                    }
                    if(addNew){
                        //add new
                        PatternMatcherElement pme;
                        pme.element = currentElement->next;
                        pme.firstMatch = currentMatcher->firstMatch;
                        pme.lastMatch = addr;
                        toAdd.push_back(pme);
                    }else{
                        //next
                        currentMatcher->element = currentElement->next;
                        currentMatcher->repeatCount = 0;
                        currentMatcher->lastMatch = addr;
                    }
                    
                }
            } else {
                // 当前路径匹配失败，准备移除
                toRemove.push_back(currentMatcher);
            }
        }

        // 处理重叠的可能性
        if(matchByte(data,pattern->head)){
            PatternMatcherElement pme;
            pme.element = pattern->head->next;
            pme.firstMatch = addr;
            pme.lastMatch = addr;
            activeMatches.push_back(pme);
        }


        // 更新潜在匹配列表
        for (auto& it : toRemove) activeMatches.erase(it);
        activeMatches.insert(activeMatches.end(), toAdd.begin(), toAdd.end());
        toRemove.clear();
        toAdd.clear();
    }


    bool PatternMatcher::matchByte(BYTE dataByte, PatternElement* elem) {
        switch (elem->matchType) {
            case MT_FULL:
                return dataByte == elem->value;
            case MT_HIGH4:
                return (dataByte >> 4) == elem->value;
            case MT_LOW4:
                return (dataByte & 0xF) == elem->value;
            case MT_ANY:
                return true;
            default:
                return false;
        }
    }

        // 获取匹配结果
    std::vector<PatternMatcherResult> PatternMatcher::getResult(){
        return matchResults;
    }

    //是否有结果
    bool PatternMatcher::hasResult(){
        return matchResults.size() > 0;
    }
}