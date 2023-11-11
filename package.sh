#!/bin/bash
export TERM=xterm
#Programm
cp ./xmppc ./deb/usr/local/xmppc/xmppc
chmod 755 ./deb/usr/local/xmppc/xmppc
#Link Programm
rm ./deb/usr/bin/xmppc
ln -s /usr/local/xmppc/xmppc ./deb/usr/bin/xmppc
#MD5
md5deep -l -o f -r ./deb/usr > ./deb/DEBIAN/md5sums
#Create DEB pakege
fakeroot dpkg-deb --build deb
#Rename and move
mv ./deb.deb ./xmppc.deb
