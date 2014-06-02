## End user installation instructions

##### Table of Contents
[Ubuntu 14.10](#ubuntu_14.10)
[Ubuntu 13.10](#ubuntu_13.10)
[Ubuntu 12.10](#ubuntu_12.10)
[Ubuntu 12.04](#ubuntu_12.04)
[Debian 7](#debian_7)
[Fedora 20](#fedora_20)
[Fedora 19](#fedora_19)


<a name="ubuntu_14.04"/>
### Ubuntu 14.04

```
wget -qO - http://build.developers.mega.co.nz:82/MEGASync/xUbuntu_14.04/Release.key | sudo apt-key add -
sudo echo "deb http://build.developers.mega.co.nz:82/MEGASync/xUbuntu_14.04/ ./" > /etc/apt/sources.list.d/megasync.list
sudo apt-get update
sudo apt-get install megasync nautilus-megasync
nautilus --quit
```
Use search function on the Dash or Applications menu to find and launch *megasync* application.

<a name="ubuntu_13.10"/>
### Ubuntu 13.10

```
wget -qO - http://build.developers.mega.co.nz:82/MEGASync/xUbuntu_13.10/Release.key | sudo apt-key add -
sudo echo "deb http://build.developers.mega.co.nz:82/MEGASync/xUbuntu_13.10/ ./" > /etc/apt/sources.list.d/megasync.list
sudo apt-get update
sudo apt-get install megasync nautilus-megasync
nautilus --quit
```
Use search function on the Dash or Applications menu to find and launch *megasync* application.

<a name="ubuntu_12.10"/>
### Ubuntu 12.10

```
wget -qO - http://build.developers.mega.co.nz:82/MEGASync/xUbuntu_12.10/Release.key | sudo apt-key add -
sudo echo "deb http://build.developers.mega.co.nz:82/MEGASync/xUbuntu_12.10/ ./" > /etc/apt/sources.list.d/megasync.list
sudo apt-get update
sudo apt-get install megasync nautilus-megasync
nautilus --quit
```
Use search function on the Dash or Applications menu to find and launch *megasync* application.

<a name="ubuntu_12.04"/>
### Ubuntu 12.04

```
wget -qO - http://build.developers.mega.co.nz:82/MEGASync/xUbuntu_12.04/Release.key | sudo apt-key add -
sudo echo "deb http://build.developers.mega.co.nz:82/MEGASync/xUbuntu_12.04/ ./" > /etc/apt/sources.list.d/megasync.list
sudo apt-get update
sudo apt-get install megasync nautilus-megasync
nautilus --quit
```
Use search function on the Dash or Applications menu to find and launch *megasync* application.

<a name="debian_7"/>
### Debian 7.0 “wheezy”

```
wget -qO - http://build.developers.mega.co.nz:82/MEGASync/Debian_7.0/Release.key  | sudo apt-key add -
sudo echo "deb http://build.developers.mega.co.nz:82/MEGASync/Debian_7.0/ ./" > /etc/apt/sources.list.d/megasync.list
sudo apt-get update
sudo apt-get install megasync nautilus-megasync
nautilus --quit
```

<a name="fedora_20"/>
### Fedora 20

```
sudo wget http://build.developers.mega.co.nz:82/MEGASync/Fedora_20/MEGASync.repo -O /etc/yum.repos.d/MEGASync.repo
sudo yum update
sudo yum install megasync nautilus-megasync
nautilus --quit
```

<a name="fedora_19"/>
### Fedora 19

```
sudo wget http://build.developers.mega.co.nz:82/MEGASync/Fedora_19/MEGASync.repo -O /etc/yum.repos.d/MEGASync.repo
sudo yum update
sudo yum install megasync nautilus-megasync
nautilus --quit
```
