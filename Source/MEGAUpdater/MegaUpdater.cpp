#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "mega/types.h"
#include "mega/crypto/cryptopp.h"
#include "mega.h"

#define KEY_LENGTH 4096
#define SIGNATURE_LENGTH 512

using namespace mega;
using namespace std;

const char APP_VERSION[] = "1001";
const char SERVER_BASE_URL[] = "http://g.static.mega.co.nz/upd/wsync/";

const char *TARGET_PATHS[] = {
    "MEGAsync.exe",
    "ShellExtX32.dll",
    "ShellExtX64.dll",
    "QtCore4.dll",
    "QtGui4.dll",
    "QtNetwork4.dll",
    "imageformats/qgif4.dll",
    "imageformats/qico4.dll",
    "imageformats/qjpeg4.dll",
    "imageformats/qmng4.dll",
    "imageformats/qsvg4.dll",
    "imageformats/qtga4.dll",
    "imageformats/qtiff4.dll"
};

const char *UPDATE_FILES[] = {
    "MEGAsync.exe",
    "ShellExtX32.dll",
    "ShellExtX64.dll",
    "QtCore4.dll",
    "QtGui4.dll",
    "QtNetwork4.dll",
    "qgif4.dll",
    "qico4.dll",
    "qjpeg4.dll",
    "qmng4.dll",
    "qsvg4.dll",
    "qtga4.dll",
    "qtiff4.dll"
};

template <typename T, std::size_t N>
char (&static_sizeof_array( T(&)[N] ))[N];
#define SIZEOF_ARRAY( x ) sizeof(static_sizeof_array(x))

void printUsage(const char* appname)
{
    cout << "Usage: " << endl;
    cout << "Sign an update:" << endl;
    cout << "    " << appname << " -s <update folder> <keyfile>" << endl;
    cout << "Generate a keypair" << endl;
    cout << "    " << appname << " -g" << endl;
}

const byte *signFile(const char * filePath, AsymmCipher* key)
{
    byte *signature = new byte[SIGNATURE_LENGTH];
    HashSignature signatureGenerator(new Hash());
    char buffer[1024];

    ifstream input(filePath);
    while(input.good())
    {
        input.read(buffer, sizeof(buffer));
        signatureGenerator.add((byte *)buffer, input.gcount());
    }

    if(input.bad() || !signatureGenerator.get(key, signature, SIGNATURE_LENGTH))
    {
        delete signature;
        return NULL;
    }

    return signature;
}

int main(int argc, char *argv[])
{
    HashSignature signatureGenerator(new Hash());
    AsymmCipher aprivk;
    vector<string> downloadURLs;
    vector<string> signatures;
    string pubk;
    string privk;

    if((argc==2) && !strcmp(argv[1], "-g"))
    {
        //Generate a keypair
        CryptoPP::Integer pubk[AsymmCipher::PUBKEY];
        string pubks;
        string privks;

        AsymmCipher asymkey;
        asymkey.genkeypair(asymkey.key,pubk,KEY_LENGTH);
        AsymmCipher::serializeintarray(pubk,AsymmCipher::PUBKEY,&pubks);
        AsymmCipher::serializeintarray(asymkey.key,AsymmCipher::PRIVKEY,&privks);

        int len = pubks.size();
        char* pubkstr = new char[len*4/3+4];
        Base64::btoa((const byte *)pubks.data(),len,pubkstr);

        len = privks.size();
        char *privkstr = new char[len*4/3+4];
        Base64::btoa((const byte *)privks.data(),len,privkstr);

        cout << pubkstr << endl;
        cout << privkstr << endl;

        delete pubkstr;
        delete privkstr;
        return 0;
    }
    else if((argc == 4) && !strcmp(argv[1], "-s"))
    {
        //Sign an update

        //Prepare the update folder path
        string updateFolder(argv[2]);
        if(updateFolder[updateFolder.size()-1] != '/')
            updateFolder.append("/");

        //Read keys
        ifstream keyFile(argv[3]);
        if(keyFile.bad())
        {
            printUsage(argv[0]);
            return 2;
        }
        getline(keyFile, pubk);
        getline(keyFile, privk);
        if(!pubk.size() || !privk.size())
        {
            cout << "Invalid key file" << endl;
            keyFile.close();
            return 3;
        }
        keyFile.close();

        //Initialize AsymmCypher
        string privks;
        privks.resize(privk.size()/4*3+3);
        privks.resize(Base64::atob(privk.data(), (byte *)privks.data(), privks.size()));
        aprivk.setkey(AsymmCipher::PRIVKEY,(byte*)privks.data(), privks.size());

        //Generate update file signature
        signatureGenerator.add((const byte *)APP_VERSION, strlen(APP_VERSION));
        for(unsigned int i=0; i<SIZEOF_ARRAY(UPDATE_FILES); i++)
        {
            string filePath = updateFolder + UPDATE_FILES[i];
            const byte *signature = signFile(filePath.data(), &aprivk);
            if(!signature)
            {
                cout << "Error signing file: " << filePath << endl;
                return 4;
            }

            string s;
            s.resize((SIGNATURE_LENGTH*4)/3+4);
            Base64::btoa((byte *)signature,SIGNATURE_LENGTH, (char *)s.data());
            signatures.push_back(s);
            delete signature;

            string fileUrl(SERVER_BASE_URL);
            fileUrl.append(UPDATE_FILES[i]);
            downloadURLs.push_back(fileUrl);

            signatureGenerator.add((const byte*)fileUrl.data(), fileUrl.size());
            signatureGenerator.add((const byte*)TARGET_PATHS[i], strlen(TARGET_PATHS[i]));
            signatureGenerator.add((const byte*)s.data(), s.length());
        }

        byte buffer[SIGNATURE_LENGTH];
        signatureGenerator.get(&aprivk, buffer, sizeof(buffer));
        string updateFileSignature;
        updateFileSignature.resize((SIGNATURE_LENGTH*4)/3+4);
        Base64::btoa((byte *)buffer,sizeof(buffer), (char *)updateFileSignature.data());

        //Print update file
        cout << APP_VERSION << endl;
        cout << updateFileSignature << endl;
        for(unsigned int i=0; i<SIZEOF_ARRAY(UPDATE_FILES); i++)
        {
            cout << downloadURLs[i] << endl;
            cout << TARGET_PATHS[i] << endl;
            cout << signatures[i] << endl;
        }

        return 0;
    }

    printUsage(argv[0]);
    return 1;
}
