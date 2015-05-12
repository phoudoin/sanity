#!/bin/sh

#[revol@patrick ~/Apps/BeSANE.bin]$ ./scanimage -L
#device `pnm:0' is a Noname PNM file reader virtual device
#device `pnm:1' is a Noname PNM file reader virtual device

SD="$(finddir B_DESKTOP_DIRECTORY)/Scanners"

mkdir -p "$SD"
rm -f "$SD/*"


SCANIMAGES="$HOME/Apps/BeSANE.bin/scanimage /bin/scanimage $HOME/config/bin/scanimage"
for f in $SCANIMAGES; do
	if [ -x $f ]; then
		SCANIMAGE="$f"
		break;
	fi
done

$SCANIMAGE -L | while read D1 DEV D2 D3 BRAND NAME; do
	DEV="${DEV#\`}"
	DEV="${DEV%%\'}"
	DEVNM="$DEV"
	DEV="${DEV//\//_}"
	SCANNER="$SD/$BRAND $NAME ($DEV)"
	echo "[$DEV/$BRAND/$NAME]"
	echo "#!/bin/sh\n#MIME:image/x-vnd.yT-sane-scanner-wrapper\necho nothing here" > "$SCANNER"
	addattr -t mime BEOS:TYPE image/x-vnd.yT-sane-scanner-wrapper "$SCANNER"
	addattr sane-device "$DEVNM" "$SCANNER"
	chmod 444 "$SCANNER"
done

