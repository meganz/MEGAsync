/**
 * @file command.cpp
 * @brief Request command component
 *
 * (c) 2013 by Mega Limited, Wellsford, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#include "mega/command.h"
#include "mega/base64.h"

namespace mega {

Command::Command()
{
	persistent = false;
	level = -1;
	canceled = false;
}

void Command::cancel()
{
	canceled = true;
}

// returns completed command JSON string
const char* Command::getstring()
{
	return json.c_str();
}

// add opcode
void Command::cmd(const char* cmd)
{
	json.append("\"a\":\"");
	json.append(cmd);
	json.append("\"");
}

void Command::notself(MegaClient *client)
{
	json.append(",\"i\":\"");
	json.append(client->sessionid,sizeof client->sessionid);
	json.append("\"");
}

// add comma separator unless first element
void Command::addcomma()
{
	if (json.size() && !strchr("[{",json[json.size()-1])) json.append(",");
}

// add command argument name:value pair (FIXME: add proper JSON escaping)
void Command::arg(const char* name, const char* value, int quotes)
{
	addcomma();
	json.append("\"");
	json.append(name);
	json.append(quotes ? "\":\"" : "\":");
	json.append(value);
	if (quotes) json.append("\"");
}

// binary data
void Command::arg(const char* name, const byte* value, int len)
{
	char* buf = new char[len*4/3+4];

	Base64::btoa(value,len,buf);

	arg(name,buf);

	delete[] buf;
}

// 64-bit signed integer
void Command::arg(const char* name, m_off_t n)
{
	char buf[32];

	sprintf(buf,"%" PRId64,n);

	arg(name,buf,0);
}

// raw JSON data
void Command::appendraw(const char* s)
{
	json.append(s);
}

// raw JSON data with length specifier
void Command::appendraw(const char* s, int len)
{
	json.append(s,len);
}

// begin array
void Command::beginarray()
{
	addcomma();
	json.append("[");
	openobject();
}

// begin array member
void Command::beginarray(const char* name)
{
	addcomma();
	json.append("\"");
	json.append(name);
	json.append("\":[");
	openobject();
}

// close array
void Command::endarray()
{
	json.append("]");
	closeobject();
}

// begin JSON object
void Command::beginobject()
{
	addcomma();
	json.append("{");
}

// end JSON object
void Command::endobject()
{
	json.append("}");
}

// add integer
void Command::element(int n)
{
	char buf[24];

	sprintf(buf,"%d",n);

	if (elements()) json.append(",");
	json.append(buf);
}

// add handle (with size specifier)
void Command::element(handle h, int len)
{
	char buf[12];

	Base64::btoa((const byte*)&h,len,buf);

	json.append(elements() ? ",\"" : "\"");
	json.append(buf);
	json.append("\"");
}

// add binary data
void Command::element(const byte* data, int len)
{
	char* buf = new char[len*4/3+4];

	len = Base64::btoa(data,len,buf);

	json.append(elements() ? ",\"" : "\"");
	json.append(buf,len);

	delete[] buf;

	json.append("\"");
}

// open object
void Command::openobject()
{
	levels[(int)++level] = 0;
}

// close object
void Command::closeobject()
{
	level--;
}

// number of elements present in this level
int Command::elements()
{
	if (!levels[(int)level])
	{
		levels[(int)level] = 1;
		return 0;
	}

	return 1;
}

// default command result handler: ignore & skip
void Command::procresult()
{
	if (client->json.isnumeric())
	{
		client->json.getint();
		return;
	}

	for (;;)
	{
		switch (client->json.getnameid())
		{
			case EOO:
				return;

			default:
				if (!client->json.storeobject()) return;
		}
	}
}

} // namespace
