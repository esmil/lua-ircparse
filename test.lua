#!/usr/bin/env lua
--
-- This file is part of lua-ircparse.
-- Copyright 2011 Emil Renner Berthing
--
-- lua-ircparse is free software: you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation, either version 3 of
-- the License, or (at your option) any later version.
--
-- lua-ircparse is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with lua-ircparse.  If not, see <http://www.gnu.org/licenses/>.
--

local ircparse = require 'ircparse'

local function dump(cmd)
	print(string.format('%q:', cmd))
	local t, err = ircparse(cmd)
	if not t then
		print(err)
	else
		for k, v in pairs(t) do
			print(k, v)
		end
	end

	print()
end

dump('PING :labitat.dk\r\n')
dump(':Niiiick!nick@somedomain.com PRIVMSG #labitat :æv bøv \r\n')
dump(':Niiiick@somedomain.com PRIVMSG #labitat :æv bøv\r\n')
dump(':Niiiick!nick PRIVMSG #labitat :æv bøv\r\n')
dump(':poison42!poison@2001:470:dc98:1:62eb:69ff:fe8d:b293 PRIVMSG #labitat :nice\r\n')
dump(':labitat.dk MODE #bottest +ntcCT \r\n')
