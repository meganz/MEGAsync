/**
 * @file doc/example/lsmega.cpp
 * @brief Sample application, which uses libmega
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

#include <mega.h>

using namespace mega;

struct LsApp : public MegaApp
{
    void nodes_updated(Node**, int);
    void debug_log(const char*);
    void login_result(error e);

    void request_error(error e);
};

// globals
MegaClient* client;
static handle cwd = UNDEF;
bool mega::debug;

static const char* accesslevels[] = { "read-only", "read/write", "full access" };

// this callback function is called when nodes have been updated
// save root node handle
void LsApp::nodes_updated(Node** n, int count)
{
    if (ISUNDEF(cwd)) cwd = client->rootnodes[0];
}

// callback for displaying debug logs
void LsApp::debug_log(const char* message)
{
    cout << "DEBUG: " << message << endl;
}

// this callback function is called when we have login result (success or error)
// TODO: check for errors
void LsApp::login_result(error e)
{
    cout << "Logged in !" << endl;
    // get the list of nodes
    client->fetchnodes();
}

// this callback function is called when request-level error occurred
void LsApp::request_error(error e)
{
    cout << "FATAL: Request failed  exiting" << endl;

    exit(0);
}

// traverse tree and list nodes
static void dumptree(Node* n, int recurse, int depth = 0, const char* title = NULL)
{
    if (depth)
    {
        if (!title && !(title = n->displayname())) title = "CRYPTO_ERROR";

        for (int i = depth; i--; ) cout << "\t";

        cout << title << " (";

        switch (n->type)
        {
            case FILENODE:
                cout << n->size;

                const char* p;
                if ((p = strchr(n->fileattrstring.c_str(),':'))) cout << ", has attributes " << p+1;
                break;

            case FOLDERNODE:
                cout << "folder";

                for (share_map::iterator it = n->outshares.begin(); it != n->outshares.end(); it++)
                {
                    if (it->first) cout << ", shared with " << it->second->user->email << ", access " << accesslevels[it->second->access];
                    else cout << ", shared as exported folder link";
                }

                if (n->inshare) cout << ", inbound " << accesslevels[n->inshare->access] << " share";
                break;

            default:
                cout << "unsupported type, please upgrade";
        }

        cout << ")" << (n->removed ? " (DELETED)" : "") << endl;

        if (!recurse) return;
    }

    if (n->type != FILENODE) for (node_list::iterator it = n->children.begin(); it != n->children.end(); it++) dumptree(*it,recurse,depth+1);
}

int main (int argc, char *argv[])
{
    static byte pwkey[SymmCipher::KEYLENGTH];
    Node* n;

// enable debugging if source is compiled with -DDEBUG=1 (check Makefile)
#ifdef DEBUG
    debug = true;
#else
    debug = false;
#endif

    if (!getenv ("MEGA_EMAIL") || !getenv ("MEGA_PWD")) {
        cout << "Please set both MEGA_EMAIL and MEGA_PWD env variables!" << endl;
        return 1;
    }

    // create MegaClient, providing our custom MegaApp and Waiter classes
    client = new MegaClient(new LsApp, new WAIT_CLASS, new HTTPIO_CLASS, new FSACCESS_CLASS, new DBACCESS_CLASS, "lsmega");

    // get values from env
    client->pw_key (getenv ("MEGA_PWD"), pwkey);
    client->login (getenv ("MEGA_EMAIL"), pwkey);
    cout << "Initiated login attempt..." << endl;

    // loop while we are not logged in
    while (! client->loggedin ()) {
        client->wait();
        client->exec();
    }

    // get the root node
    while (! (n = client->nodebyhandle(cwd))) {
        client->wait();
        client->exec();
    }

    // display objects
    dumptree(n, 1);

    return 0;
}
