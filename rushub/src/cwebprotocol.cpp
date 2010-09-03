/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2010 by Setuper
 * E-Mail: setuper at gmail dot com (setuper@gmail.com)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cwebprotocol.h"
#include "cserver.h" /** for mServer */
#include "cwebconn.h" /** for cWebConn */

namespace nWebServer
{

namespace nProtocol
{

cWebProtocol::cWebProtocol()
{
  SetClassName("cWebProtocol");
}

cWebProtocol::~cWebProtocol()
{
}

void cWebProtocol::SetServer(cServer * server)
{
  mServer = server;
}

int cWebProtocol::DoCmd(cParser *, cConn *)
{
  return 0;
}

}; // nProtocol

}; // nWebServer
