/* Copyright (c) houjincheng, Inc. All Rights Reserved
 * *********************************************************
 * @file luahelper.h
 * @author houjincheng
 * @date 2018.04.14
 * @version
 * @brief
 **/
#pragma once

#include <lua.hpp>
#include <map>
#include <string>

namespace nice {
namespace utils {

class LuaHelper {
public:
    using LuaState = ::lua_State;
    using LuaParams = std::map<std::string, std::string>;

    LuaHelper();
    virtual ~LuaHelper();

    virtual int32_t init(const std::string& luaf, const LuaParams* params = nullptr);
    virtual void destroy();

    virtual bool good() const { return _lua_state != nullptr; }

    virtual std::string get_configure_value(const std::string&, const std::string&) const;

protected:
    LuaState* _lua_state = nullptr;
};

}
}
/* Powered by Microsoft Visual Studio 2013, @2019*/
