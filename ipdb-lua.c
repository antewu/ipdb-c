#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "lua.h"
#include "lauxlib.h"
#include "ipdb.h"

struct
{
    ipdb_reader *r;
} ipdb;

int ipdb_lua_init(lua_State *L)
{
    ipdb_reader *r;

    int err = ipdb_reader_new(luaL_checkstring(L, 1), &r);
    if (err)
    {
        if (err != ErrFileSize)
        {
            ipdb_reader_free(&r);
        }
        lua_pushboolean(L, 0);

        return 0;
    }

    if (ipdb.r)
    {
        ipdb_reader *old = ipdb.r;
        ipdb.r = r;

        ipdb_reader_free(&old);
    }
    else
    {
        ipdb.r = r;
    }

    lua_pushboolean(L, 1);
    return 1;
}

int ipdb_lua_info(lua_State *L)
{

    if (!ipdb.r)
    {
        lua_pushboolean(L, 0);
        return 0;
    }

    lua_newtable(L);

    lua_pushstring(L, "build_time");
    lua_pushfstring(L, "%d", ipdb.r->meta->build_time);
    lua_settable(L, -3);

    lua_pushstring(L, "ipv4");
    lua_pushboolean(L, ipdb_reader_is_ipv4_support(ipdb.r));
    lua_settable(L, -3);

    lua_pushstring(L, "ipv6");
    lua_pushboolean(L, ipdb_reader_is_ipv6_support(ipdb.r));
    lua_settable(L, -3);

    return 1;
}

int ipdb_lua_find(lua_State *L)
{

    if (!ipdb.r)
    {
        lua_pushboolean(L, 0);
        return 0;
    }

    char body[512];
    char tmp[64];
    int err = ipdb_reader_find(ipdb.r, luaL_checkstring(L, 1), "EN", body);
    if (err)
    {
        lua_pushboolean(L, 0);
        return 0;
    }

    lua_newtable(L);

    int f = 0, p1 = 0, p2 = -1;
    do
    {
        if (*(body + p1) == '\t' || !*(body + p1))
        {
            strncpy(tmp, body + p2 + 1, (size_t)p1 - p2);
            tmp[p1 - p2] = 0;

            int lastChr = strlen(tmp) - 1;
            if (tmp[lastChr] == '\t') {
                tmp[lastChr] = 0;
            }

            lua_pushstring(L, ipdb.r->meta->fields[f]);
            lua_pushfstring(L, tmp);
            lua_settable(L, -3);

            p2 = p1;
            ++f;
        }
    } while (*(body + p1++));

    return 1;
}

int ipdb_lua_close(lua_State *L)
{

    if (ipdb.r)
    {
        ipdb_reader_free(&ipdb.r);
    }

    lua_pushboolean(L, 1);
    return 1;
}

static const struct luaL_Reg functions[] = {
    {"init", ipdb_lua_init},
    {"info", ipdb_lua_info},
    {"find", ipdb_lua_find},
    {"close", ipdb_lua_close},
    {NULL, NULL}};

int luaopen_ipdb(lua_State *L)
{

#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 503
    lua_newtable(L);
    luaL_setfuncs(L, functions, 0);
#else
    luaL_register(L, "ipdb", functions);
#endif

    return 1;
}