//
// Created by vladimir on 08.08.2015.
//

#include <http/engine.h>
#include <http/request.h>
#include <misc/string_util.h>
#include <regex>
using namespace Http;

std::vector<std::string> Request::SplitURL() const { return StringUtil::Split(url, '/'); }
