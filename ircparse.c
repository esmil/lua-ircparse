/*
 * This file is part of lua-ircparse.
 *
 * lua-ircparse is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or(at your option) any later version.
 *
 * lua-ircparse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with lua-ircparse.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

enum classes {
	C_NUL,   /* \0 */
	C_LF,    /* \n */
	C_CR,    /* \r */
	C_SPACE, /*    */
	C_BANG,  /* !  */
	C_COLON, /* :  */
	C_AT,    /* @  */
	C_ETC,   /* the rest */
	C_MAX
};

/*
 * This array maps the first ASCII characters into character classes
 * The remaining characters should be mapped to C_ETC
 */
static const unsigned char ascii_class[] = {
	C_NUL,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
	C_ETC,   C_ETC,   C_LF,    C_ETC,   C_ETC,   C_CR,    C_ETC,   C_ETC,
	C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
	C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,

	C_SPACE, C_BANG,  C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
	C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
	C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
	C_ETC,   C_ETC,   C_COLON, C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,

	C_AT
};

enum states {
	S_BEG,
	S_NI1,
	S_NIC,
	S_NIS,
	S_US1,
	S_USR,
	S_USS,
	S_HO1,
	S_HOS,
	S_HSS,
	S_PRS,
	S_CM1,
	S_CMD,
	S_CMS,
	S_PA1,
	S_PAR,
	S_PAS,
	S_LP1,
	S_LPA,
	S_CR0,
	S_CR1,
	S_LFF,
	S_MAX,
	S____ = S_MAX
};

static const unsigned char state_table[S_MAX][C_MAX] = {
/*              \0     \n     \r            !      :      @     etc */
/* S_BEG */ { S____, S____, S____, S____, S____, S_NI1, S____, S_CM1 },
/* S_NI1 */ { S____, S____, S____, S____, S____, S____, S____, S_NIC },
/* S_NIC */ { S____, S____, S____, S_NIS, S_US1, S____, S_HO1, S_NIC },
/* S_NIS */ { S____, S____, S____, S_PRS, S____, S____, S____, S_CM1 },
/* S_US1 */ { S____, S____, S____, S____, S____, S____, S____, S_USR },
/* S_USR */ { S____, S____, S____, S_USS, S____, S____, S_HO1, S_USR },
/* S_USS */ { S____, S____, S____, S_PRS, S____, S____, S____, S_CM1 },
/* S_HO1 */ { S____, S____, S____, S____, S____, S_HOS, S____, S_HOS },
/* S_HOS */ { S____, S____, S____, S_HSS, S____, S_HOS, S____, S_HOS },
/* S_HSS */ { S____, S____, S____, S_PRS, S____, S____, S____, S_CM1 },
/* S_PRS */ { S____, S____, S____, S_PRS, S____, S____, S____, S_CM1 },
/* S_CM1 */ { S____, S____, S____, S_CMS, S____, S____, S____, S_CMD },
/* S_CMD */ { S____, S____, S____, S_CMS, S____, S____, S____, S_CMD },
/* S_CMS */ { S____, S____, S____, S_CMS, S_PA1, S_LP1, S_PA1, S_PA1 },
/* S_PA1 */ { S____, S____, S____, S_PAS, S_PAR, S_PAR, S_PAR, S_PAR },
/* S_PAR */ { S____, S____, S_CR1, S_PAS, S_PAR, S_PAR, S_PAR, S_PAR },
/* S_PAS */ { S____, S____, S_CR0, S_PAS, S_PA1, S_LP1, S_PA1, S_PA1 },
/* S_LP1 */ { S____, S____, S_CR1, S_LPA, S_LPA, S_LPA, S_LPA, S_LPA },
/* S_LPA */ { S____, S____, S_CR1, S_LPA, S_LPA, S_LPA, S_LPA, S_LPA },
/* S_CR0 */ { S____, S_LFF, S____, S____, S____, S____, S____, S____ },
/* S_CR1 */ { S____, S_LFF, S____, S____, S____, S____, S____, S____ },
/* S_LFF */ { S____, S____, S____, S____, S____, S____, S____, S____ }
};

static int
parse(lua_State *L)
{
	size_t len;
	const char *str = luaL_checklstring(L, 1, &len);
	const char *start;
	unsigned char state = S_BEG;
	int arg = 0;

	lua_settop(L, 1);
	lua_createtable(L, 4, 4);

	for (; len > 0; len--, str++) {
		unsigned char c = *str;

		state = state_table[state][c < sizeof(ascii_class) ?
			                   ascii_class[c] : C_ETC];
		switch (state) {
		case S_NI1:
		case S_LP1:
			start = str + 1;
			break;

		case S_CM1:
		case S_PA1:
			start = str;
			break;

		case S_US1:
		case S_NIS:
			lua_pushlstring(L, start, str - start);
			lua_setfield(L, -2, "nick");
			start = str + 1;
			break;

		case S_USS:
		case S_HO1:
			lua_pushlstring(L, start, str - start);
			lua_setfield(L, -2, "user");
			start = str + 1;
			break;

		case S_HSS:
			lua_pushlstring(L, start, str - start);
			lua_setfield(L, -2, "host");
			break;

		case S_CMS:
			lua_pushlstring(L, start, str - start);
			lua_setfield(L, -2, "command");
			break;

		case S_PAS:
		case S_CR1:
			lua_pushlstring(L, start, str - start);
			lua_rawseti(L, -2, ++arg);
			break;

		case S____:
			goto error;
		}
	}

	if (state == S_LFF)
		return 1;

error:
	lua_pushnil(L);
	lua_pushliteral(L, "parse error");
	return 2;
}

LUALIB_API int
luaopen_ircparse(lua_State *L)
{
	lua_pushcfunction(L, parse);
	return 1;
}
