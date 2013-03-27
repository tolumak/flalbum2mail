#!/bin/sh

ALBUM_NAME="$1"
ALBUM_URL="$2"
AUTH="$3"

shift 3 

photos=`./flalbum2mail --album "$ALBUM_NAME" --auth "$AUTH" | grep -e "^A" | cut -d ' ' -f 3-4 --output-delimiter=','`

photo_count=0
for i in $photos ; do
    photo_count=$(( $photo_count + 1 ))
    page=`echo $i | cut -d ',' -f 1`
    img=`echo $i | cut -d ',' -f 2`
    photo_list_as_cells="${photo_list_as_cells}<td><a href=$page><img src=$img></a></td>\n"
done

sed_script="sed -e 's/album_name/$ALBUM_NAME/g' -e 's/photo_count/$photo_count/g' -e 's#photo_list_as_cells#$photo_list_as_cells#g' -e 's#album_url#$ALBUM_URL#g'"

rm -f "$ALBUM_NAME.msg"
if [ $photo_count -gt 0 ] ; then
    cat album.tmpl | eval $sed_script > "$ALBUM_NAME.msg"
fi

exit 0
if [ $photo_count -gt 0 ] ; then
    echo $message | mailx $* -s "`echo $subject`" -a 'Content-Type: text/html' -- -F "$FROM_NAME" -r "$FROM"
fi
