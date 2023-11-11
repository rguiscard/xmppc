# xmppc

xmppc - command line interface (CLI) XMPP Client.

xmppc is a XMPP command line interface client. It's written in C and is using
the xmpp library [libstrophe](http://strophe.im/libstrophe/).

## Dependencies

* [libc6-dev](https://packages.debian.org/buster/libc6-dev) (2.28-10)
* [libglib2.0-dev](https://packages.debian.org/buster/libglib2.0-dev) (2.58.3)
* [libstrophe-dev](https://packages.debian.org/buster/libstrophe-dev) (0.9.2-2)
* [libgpgme-dev](https://packages.debian.org/buster/libgpgme-dev) (1.12.0)

## Build

The project is using [GNU Automake](https://www.gnu.org/software/automake/).

```
apt install libstrophe-dev libc6-dev libglib2.0-dev libgpgme-dev autoconf libtool
./bootstrap.sh
./configure
make
```

## Config file

Config file: ` ~/.config/xmppc.conf`.

The `[default]` will be used, when the user doesn't provide an account and
doesn't provide a jid. 

```
[default]
jid=user@domain.tld
pwd=YourSecret

[account1]
jid=account1@domain.tld
pwd=YourSecret
```

## Usage

```
xmppc --jid user@domain.tld --pwd "password" --mode roster list
xmppc -j user@domain.tld -p "password" -m roster list
xmppc -a alice -m mam list bob@domain.tld
xmppc -m bookmark list
xmppc -h
```

More command and information see: [Wiki](https://codeberg.org/Anoxinon_e.V./xmppc/wiki)

## Documentation

* [Wiki](https://codeberg.org/Anoxinon_e.V./xmppc/wiki)

## Chat

* [xmpp:xmppc@conference.anoxinon.me?join](xmpp:xmppc@conference.anoxinon.me?join)
