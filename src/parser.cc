#include "parser.h"
#include <cctype>
#include <memory>
#include <deque>
#include <string>
using std::unique_ptr;
int Parser::getType(char c){
        if(std::isspace(c)) return 0;
        else if(std::isalnum(c) || c == '_') return 1;
        else return 2;
}

unique_ptr<std::deque<std::string>> Parser::parse(std::string s){
        std::string news="";
        unique_ptr<std::deque<std::string>> q = std::make_unique<std::deque<std::string>>();
        int type; //0 is whitespace, 1 is alphanumeric, 2 is non-alphanumeric
        while(s.length() > 0){
                char c = s[0];
                if(news.empty()){
                        type = getType(c);
                }
                s = s.substr(1);
                if(getType(c) != type){
                        q->push_back(news);
                        news = "";
                        type = getType(c);
                }
                news.push_back(c);
        }
        q->push_back(news);
        return q;
}
 
