#!/bin/sh

SD="$(finddir B_DESKTOP_DIRECTORY)/Scanners"

mkdir -p "$SD"
rm -f "$SD/*"

SCANIMAGES="/bin/scanimage"
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
	echo "#!/bin/sh" > "$SCANNER"
	echo "#MIME:image/x-vnd.sane-scanner-wrapper" >> "$SCANNER"
	echo "nothing here" >> "$SCANNER"
	addattr -t mime BEOS:TYPE image/x-vnd.sane-scanner-wrapper "$SCANNER"
	addattr sane-device "$DEVNM" "$SCANNER"
	chmod 444 "$SCANNER"
done

