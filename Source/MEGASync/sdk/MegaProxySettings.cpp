#include "MegaProxySettings.h"

using namespace std;

MegaProxySettings::MegaProxySettings() { proxyType = AUTO; }
void MegaProxySettings::setProxyType(int proxyType) { this->proxyType = proxyType; }
void MegaProxySettings::setProxyURL(string *proxyURL) { this->proxyURL = *proxyURL; }
void MegaProxySettings::setCredentials(string *username, string *password) { this->username = *username; this->password = *password; }
int MegaProxySettings::getProxyType() { return proxyType; }
string MegaProxySettings::getProxyURL() { return this->proxyURL; }
bool MegaProxySettings::credentialsNeeded() { return (username.size() != 0); }
string MegaProxySettings::getUsername() { return username; }
string MegaProxySettings::getPassword() { return password; }
