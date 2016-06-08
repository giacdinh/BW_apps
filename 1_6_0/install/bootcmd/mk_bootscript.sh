if ! [[ -e $1 ]]
then
 	echo "Please provide source script file"
	exit
fi

echo "Make boot script from source: $1"
mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n 'Boot script' -d $1 boot.scr

echo "Copy boot script to p0-boot partition ... "
