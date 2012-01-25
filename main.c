#include <stdio.h>
#include <flickcurl.h>

int main(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  flickcurl *fc;
  flickcurl_photo *photo;
  flickcurl_photo_field_type field_type;
  flickcurl_photoset * set;
  flickcurl_photoset ** sets;
  int i;

  flickcurl_init(); /* optional static initialising of resources */
  fc=flickcurl_new();

  /* Set configuration, or more likely read from a config file */
  flickcurl_set_api_key(fc, "...");
  flickcurl_set_shared_secret(fc, "...");
  flickcurl_set_auth_token(fc, "...");

  sets = flickcurl_photosets_getList(fc, NULL);

  for (set = sets[0], i = 0 ; set ; set = sets[++i]) {
	  printf("%s %s %d photos\n", set->id, set->title, set->photos_count);
  }


#if 0
  photo=flickcurl_photos_getInfo(fc, "6756767665"); /* photo ID */

  for(field_type=0; field_type <= PHOTO_FIELD_LAST; field_type++) {
    flickcurl_field_value_type datatype=photo->fields[field_type].type;

    if(datatype != VALUE_TYPE_NONE)
      fprintf(stderr, "field %s (%d) with %s value: '%s' / %d\n", 
              flickcurl_get_photo_field_label(field_type), (int)field_type,
              flickcurl_get_field_value_type_label(datatype),
              photo->fields[field_type].string,
              photo->fields[field_type].integer);
  }

  for(i=0; i < photo->tags_count; i++) {
    flickcurl_tag* tag=photo->tags[i];
    fprintf(stderr,
            "%d) %s tag: id %s author ID %s name %s raw '%s' cooked '%s' count %d\n",
            i, (tag->machine_tag ? "machine" : "regular"),
            tag->id, tag->author, 
            (tag->authorname ? tag->authorname : "(Unknown)"), 
            tag->raw, tag->cooked,
            tag->count);
  }

  flickcurl_free_photo(photo);
#endif
  flickcurl_free_photosets(sets);
  flickcurl_free(fc);
  flickcurl_finish(); /* optional static free of resources */

  return 0;
}
