#include <iostream>
#include <fstream>
#include <vector>
#include <string>

namespace mega {
// within ::mega namespace, byte is unsigned char (avoids ambiguity when std::byte from c++17 and perhaps other defined ::byte are available)
#if defined(USE_CRYPTOPP) && (CRYPTOPP_VERSION >= 600) && ((__cplusplus >= 201103L) || (__RPCNDR_H_VERSION__ == 500))
using byte = CryptoPP::byte;
#elif __RPCNDR_H_VERSION__ != 500
typedef unsigned char byte;
#endif
}

// signed 64-bit generic offset
typedef int64_t m_off_t;

#ifndef MEGA_API
 #define MEGA_API
#endif

#include "mega/crypto/cryptopp.h"
#include "mega/base64.h"

#define KEY_LENGTH 4096
#define SIGNATURE_LENGTH 512

using namespace mega;
using std::string;
using std::ostringstream;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;

class HashSignature
{
    Hash* hash;

public:
    // add data
    void add(const byte*, unsigned);

    // generate signature
    unsigned get(AsymmCipher*, byte*, unsigned);

    // verify signature
    bool checksignature(AsymmCipher*, const byte*, unsigned);

    HashSignature(Hash*);
    ~HashSignature();
};

// cryptographic signature generation/verification
HashSignature::HashSignature(Hash* h)
{
    hash = h;
}

HashSignature::~HashSignature()
{
    delete hash;
}

void HashSignature::add(const byte* data, unsigned len)
{
    hash->add(data, len);
}

unsigned HashSignature::get(AsymmCipher* privk, byte* sigbuf, unsigned sigbuflen)
{
    string h;

    hash->get(&h);

    return privk->rawdecrypt((const byte*)h.data(), h.size(), sigbuf, sigbuflen);
}

bool HashSignature::checksignature(AsymmCipher* pubk, const byte* sig, unsigned len)
{
    string h, s;
    unsigned size;

    hash->get(&h);

    s.resize(h.size());

    if (!(size = pubk->rawencrypt(sig, len, (byte*)s.data(), s.size())))
    {
        return 0;
    }

    if (size < h.size())
    {
        // left-pad with 0
        s.insert(0, h.size() - size, 0);
        s.resize(h.size());
    }

    return s == h;
}


void printUsage(const char* appname)
{
    cerr << "Usage: " << endl;
    cerr << "Generate a keypair" << endl;
    cerr << "    " << appname << " -g" << endl;
    cerr << "Sign an update:" << endl;
    cerr << "    " << appname << " <update folder> <keyfile> --file <contentsfile>" << endl;
    cerr << "    e.g:" << endl;
    cerr << "        " << appname << " /tmp/updatefiles /tmp/key.pem --file /megasync/contrib/updater/fileswin.txt" << endl;
}

unsigned signFile(const char * filePath, AsymmCipher* key, ::mega::byte* signature, unsigned signbuflen)
{
    HashSignature signatureGenerator(new Hash());
    char buffer[1024];

    ifstream input(filePath, std::ios::in | std::ios::binary);
    if (input.fail())
    {
        return 0;
    }

    while (input.good())
    {
        input.read(buffer, sizeof(buffer));
        signatureGenerator.add((::mega::byte *)buffer, (unsigned)input.gcount());
    }

    if (input.bad())
    {
        return 0;
    }

    unsigned signatureSize = signatureGenerator.get(key, signature, signbuflen);
    if (signatureSize < signbuflen)
    {
        int padding = signbuflen - signatureSize;
        for (int i = signbuflen - 1; i >= 0; i--)
        {
            if (i >= padding)
            {
                signature[i] = signature[i - padding];
            }
            else
            {
                signature[i] = 0;
            }
        }
        signatureSize = signbuflen;
    }
    return signatureSize;
}


bool generateHash(const char * filePath, string *hash)
{
    HashSHA256 hashGenerator;
    char buffer[1024];

    ifstream input(filePath, std::ios::in | std::ios::binary);
    if (input.fail())
    {
        return false;
    }

    while (input.good())
    {
        input.read(buffer, sizeof(buffer));
        hashGenerator.add((::mega::byte *)buffer, (unsigned)input.gcount());
    }

    if (input.bad())
    {
        return false;
    }

    string binaryhash;
    hashGenerator.get(&binaryhash);

    static const char hexchars[] = "0123456789abcdef";
    ostringstream oss;
    for (size_t i=0;i<binaryhash.size();++i)
    {
        oss.put(hexchars[(binaryhash[i] >> 4) & 0x0F]);
        oss.put(hexchars[binaryhash[i] & 0x0F]);
    }
    *hash = oss.str();

    return true;
}


bool generateHashFromContents(string contents, string *hash)
{
    contents.append("\n");

    HashSHA256 hashGenerator;
    hashGenerator.add((const ::mega::byte *)contents.data(),contents.size());

    string binaryhash;
    hashGenerator.get(&binaryhash);

    static const char hexchars[] = "0123456789abcdef";
    ostringstream oss;
    for (size_t i=0;i<binaryhash.size();++i)
    {
        oss.put(hexchars[(binaryhash[i] >> 4) & 0x0F]);
        oss.put(hexchars[binaryhash[i] & 0x0F]);
    }
    *hash = oss.str();

    return true;
}

bool extractarg(vector<const char*>& args, const char *what)
{
    for (int i = int(args.size()); i--; )
    {
        if (!strcmp(args[i], what))
        {
            args.erase(args.begin() + i);
            return true;
        }
    }
    return false;
}

bool extractargparam(vector<const char*>& args, const char *what, std::string& param)
{
    for (int i = int(args.size()) - 1; --i >= 0; )
    {
        if (!strcmp(args[i], what) && int(args.size()) > i)
        {
            param = args[i + 1];
            args.erase(args.begin() + i, args.begin() + i + 2);
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[])
{
    vector<const char*> args;
    if (argc > 1)
    {
        args = vector<const char*>(argv + 1, argv + argc);
    }

    string fileInput;
    bool externalfile = extractargparam(args, "--file", fileInput);
    bool generate = extractarg(args, "-g");

    HashSignature signatureGenerator(new Hash());
    AsymmCipher aprivk;
    vector<string> downloadURLs;
    vector<string> signatures;
    ::mega::byte signature[SIGNATURE_LENGTH];
    unsigned signatureSize;
    string pubk;
    string privk;

    if (generate)
    {
        //Generate a keypair
        CryptoPP::Integer pubk[AsymmCipher::PUBKEY];
        string pubks;
        string privks;
        // pseudo-random number generator
        PrnGen rng;

        AsymmCipher asymkey;
        asymkey.genkeypair(rng,pubk,KEY_LENGTH);
        AsymmCipher::serializeintarray(pubk,AsymmCipher::PUBKEY,&pubks);
        AsymmCipher::serializeintarray(asymkey.getKey(),AsymmCipher::PRIVKEY,&privks);

        int len = pubks.size();
        char* pubkstr = new char[len*4/3+4];
        Base64::btoa((const ::mega::byte *)pubks.data(),len,pubkstr);

        len = privks.size();
        char *privkstr = new char[len*4/3+4];
        Base64::btoa((const ::mega::byte *)privks.data(),len,privkstr);

        cout << pubkstr << endl;
        cout << privkstr << endl;

        delete [] pubkstr;
        delete [] privkstr;
        return 0;
    }
    else if (args.size() == 2 && externalfile)
    {
        //Sign an update

        //Prepare the update folder path
        string updateFolder(args.at(0));
        if (updateFolder[updateFolder.size()-1] != '/')
        {
            updateFolder.append("/");
        }

        //Read keys
        ifstream keyFile(args.at(1), std::ios::in);
        if (keyFile.bad())
        {
            printUsage(argv[0]);
            return 2;
        }
        getline(keyFile, pubk);
        getline(keyFile, privk);
        if (!pubk.size() || !privk.size())
        {
            cerr << "Invalid key file" << endl;
            keyFile.close();
            return 3;
        }
        keyFile.close();

        //Initialize AsymmCypher
        string privks = Base64::atob(privk);
        if (!aprivk.setkey(AsymmCipher::PRIVKEY,(::mega::byte*)privks.data(), privks.size()))
        {
            cerr << "Priv RSA key problem during initialization.";
            return 0;
        }

        //Generate update file signature
        vector<string> filesVector;
        vector<string> targetPathsVector;
        vector<string> hashesVector;
        string sversioncode;
        string baseUrl="UNSET";
        string pubkeyhash;

        //read input file
        filesVector.clear();
        targetPathsVector.clear();
        ifstream infile(fileInput.c_str());
        string line;
        while (getline(infile, line))
        {
            if (line.length() && line[line.length()-1] == '\r')
            {
                line = line.substr(0, line.length()-1);
            }
            if (line.length() > 0 && line[0] != '#')
            {
                string fileToDl, targetpah;
                size_t pos = line.find(";");
                fileToDl = line.substr(0, pos);
                if (pos != string::npos && ((pos + 1) < line.size()))
                {
                    string rest = line.substr(pos + 1);
                    pos = rest.find(";");
                    targetpah = rest.substr(0, pos);
                    if (pos != string::npos && ((pos + 1) < rest.size()))
                    {
                        hashesVector.push_back(rest.substr(pos + 1));
                    }
                    else
                    {
                        hashesVector.push_back("UNKNOWN");
                    }
                }
                else
                {
                    targetpah = fileToDl;
                }
                filesVector.push_back(fileToDl.c_str());
                targetPathsVector.push_back(targetpah.c_str());
            }
            else
            {
                if (line.find("#version=") == 0)
                {
                    sversioncode=line.substr(9);
                }
                else if (line.find("#baseurl=") == 0)
                {
                    baseUrl=line.substr(9);
                }
                else if (line.find("#pubkeysha256sum=") == 0)
                {
                   pubkeyhash =line.substr(strlen("#pubkeysha256sum="));
                }
            }
        }

        string pubkeyhashFromKey;
        if (generateHashFromContents(pubk,&pubkeyhashFromKey))
        {
            if (pubkeyhashFromKey != pubkeyhash)
            {
                cerr << "Error checking hash for pubkey: " << endl
                     << " calculated=" << pubkeyhashFromKey << endl
                     << "   expected=" << pubkeyhash << endl
                        << "   pubkey=" << pubk << endl;

                return 7;
            }
        }
        else
        {
            cerr << "Error generating hash for pubkey: " << endl;
            return 8;
        }


        long versionCode;
        versionCode = strtol (sversioncode.c_str(), NULL, 10);
        if (!versionCode)
        {
            cerr << "Invalid version code" << endl;
            return 5;
        }

        signatureGenerator.add((const ::mega::byte *)sversioncode.c_str(), strlen(sversioncode.c_str()));

        for (unsigned int i = 0; i < filesVector.size(); i++)
        {
            string filePath = updateFolder + filesVector.at(i);

            if (hashesVector.at(i) != "UNKNOWN")
            {
                string hashFile;
                generateHash(filePath.c_str(),&hashFile);

                if (hashFile != hashesVector.at(i))
                {
                    cerr << "Error checking hash for file: " << filePath << endl
                          << " calculated=" << hashFile << endl
                          << "   expected=" << hashesVector.at(i) << endl;
                    return 6;
                }
            }


            signatureSize = signFile(filePath.data(), &aprivk, signature, sizeof(signature));
            if (!signatureSize)
            {
                cerr << "Error signing file: " << filePath << endl;
                return 4;
            }

            string s;
            s.resize((signatureSize*4)/3+4);
            s.resize(Base64::btoa((::mega::byte *)signature, signatureSize, (char *)s.data()));
            signatures.push_back(s);

            string fileurl = baseUrl + filesVector.at(i);
            downloadURLs.push_back(fileurl);

            signatureGenerator.add((const ::mega::byte*)fileurl.data(), fileurl.size());
            signatureGenerator.add((const ::mega::byte*)targetPathsVector.at(i).data(),
                                   targetPathsVector.at(i).size());
            signatureGenerator.add((const ::mega::byte*)s.data(), s.length());
        }

        signatureSize = signatureGenerator.get(&aprivk, signature, sizeof(signature));
        if (!signatureSize)
        {
            cerr << "Error signing the update file" << endl;
            return 6;
        }

        if (signatureSize < sizeof(signature))
        {
            int padding = sizeof(signature) - signatureSize;
            for (int i = sizeof(signature) - 1; i >= 0; i--)
            {
                if (i >= padding)
                {
                    signature[i] = signature[i - padding];
                }
                else
                {
                    signature[i] = 0;
                }
            }
            signatureSize = sizeof(signature);
        }

        string updateFileSignature;
        updateFileSignature.resize((signatureSize*4)/3+4);
        updateFileSignature.resize(Base64::btoa((::mega::byte *)signature, signatureSize, (char *)updateFileSignature.data()));

        //Print update file
        cout << versionCode << endl;
        cout << updateFileSignature << endl;
        for (unsigned int i = 0; i < targetPathsVector.size(); i++)
        {
            cout << downloadURLs[i] << endl;
            cout << targetPathsVector.at(i) << endl;
            cout << signatures[i] << endl;
        }

        return 0;
    }

    printUsage(argv[0]);
    return 1;
}
