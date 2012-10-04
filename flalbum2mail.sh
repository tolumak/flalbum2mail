#!/bin/sh

ALBUM_NAME="$1"
ALBUM_URL="$2"
AUTH="$3"
FROM="$4"
FROM_NAME="$5"

shift 5

photos=`./flalbum2mail --album "$ALBUM_NAME" --auth "$AUTH" | grep -e "^A" | cut -d ' ' -f 3`

photo_count=0
for i in $photos ; do
    photo_count=$(( $photo_count + 1 ))
    photo_list_as_cells="${photo_list_as_cells}<td><img src=$i></td>"
done

sed_script="sed -e 's/album_name/$ALBUM_NAME/g' -e 's/photo_count/$photo_count/g' -e 's#photo_list_as_cells#$photo_list_as_cells#g' -e 's#album_url#$ALBUM_URL#g'"

message=`cat message | eval $sed_script`
subject=`cat subject | eval $sed_script`

if [ $photo_count -gt 0 ] ; then
    echo $message | mailx $* -s "`echo $subject`" -a 'Content-Type: text/html' -- -F "$FROM_NAME" -r "$FROM"
fi