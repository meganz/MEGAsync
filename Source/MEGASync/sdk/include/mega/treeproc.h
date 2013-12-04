/**
 * @file mega/treeproc.h
 * @brief Node tree processor
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

#ifndef MEGA_TREEPROC_H
#define MEGA_TREEPROC_H 1

#include "sharenodekeys.h"
#include "node.h"

namespace mega {

// node tree processor
class TreeProc
{
public:
	virtual void proc(MegaClient*, Node*) = 0;

	virtual ~TreeProc() { }
};

class TreeProcDel : public TreeProc
{
public:
	void proc(MegaClient*, Node*);
};

class TreeProcListOutShares : public TreeProc
{
public:
	void proc(MegaClient*, Node*);
};

class TreeProcCopy : public TreeProc
{
public:
	NewNode* nn;
	unsigned nc;

	void allocnodes(void);

	void proc(MegaClient*, Node*);
	TreeProcCopy();
	~TreeProcCopy();
};

class TreeProcDU : public TreeProc
{
public:
	m_off_t numbytes;
	int numfiles;
	int numfolders;

	void proc(MegaClient*, Node*);
	TreeProcDU();
};

class TreeProcShareKeys : public TreeProc
{
	ShareNodeKeys snk;
	Node* sn;

public:
	void proc(MegaClient*, Node*);
	void get(Command*);

	TreeProcShareKeys(Node* = NULL);
};

} // namespace

#endif
