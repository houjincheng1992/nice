/* Copyright (c) houjincheng, Inc. All Rights Reserved
 * *********************************************************
 * @file luahelper.cpp
 * @author houjincheng
 * @date 2019.04.14
 * @version
 * @brief
 **/
#include "luahelper.h"
#include "utils/logger.h"

namespace nicer {
namespace utils {

LuaHelper::LuaHelper() {}

LuaHelper::~LuaHelper() {
    destroy();
}

int32_t LuaHelper::init(const std::string& luaf, const LuaParams* params) {
    destroy();
    _lua_state = ::luaL_newstate();
    if (!_lua_state) {
        ERRLOG << "Open lua failed";// << luaf << " failed";
        return -1;
    }

    luaL_openlibs(_lua_state);

    lua_pushstring(_lua_state, "__c++__");
    lua_setglobal(_lua_state, "__runtime__");

    if (params) {
        for (auto& it : *params) {
            lua_pushstring(_lua_state, it.second.c_str());
            lua_setglobal(_lua_state, it.first.c_str());
        }
    }

    int32_t errcode = luaL_dofile(_lua_state, luaf.c_str());
    if (errcode) {
        destroy();
    }

    return errcode;
}

void LuaHelper::destroy() {
    if (_lua_state) {
        lua_settop(_lua_state, 0);
        lua_close(_lua_state), _lua_state = nullptr;
    }
}

std::string LuaHelper::get_configure_value(const std::string& section,
        const std::string& field) const {
    lua_getglobal(_lua_state, section.c_str());
    if (!lua_istable(_lua_state, -1)) {
        lua_pop(_lua_state, 1);
        DBGLOG << "Cannot get table:" << section;
        return "";
    }

    lua_pushlstring(_lua_state, field.data(), field.size());
    lua_gettable(_lua_state, -2);
    if (!lua_isstring(_lua_state, -1)) {
        lua_pop(_lua_state, 2);
        DBGLOG << "Cannot get value:" << section << "." << field;
        return "";
    }

    std::string value = lua_tostring(_lua_state, -1);
    lua_pop(_lua_state, 2);
    return value;
}

}
}
/* Powered by Microsoft Visual Studio 2013, @2017*/
