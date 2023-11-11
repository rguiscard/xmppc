#!/bin/bash
export TERM=xterm

# Creating necessary directories
mkdir -p ./deb/usr/local/xmppc
mkdir -p ./deb/usr/bin
mkdir -p ./deb/DEBIAN

# Copying the program
cp ./xmppc ./deb/usr/local/xmppc/xmppc
chmod 755 ./deb/usr/local/xmppc/xmppc

# Creating a symbolic link
ln -s /usr/local/xmppc/xmppc /usr/bin/xmppc

# Generating MD5
md5deep -l -o f -r ./deb/usr > ./deb/DEBIAN/md5sums

# Creating DEB package
fakeroot dpkg-deb --build deb

# Renaming and moving the package
mv ./deb.deb ./xmppc.deb
